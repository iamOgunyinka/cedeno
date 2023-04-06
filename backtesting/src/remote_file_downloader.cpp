#include "remote_file_downloader.hpp"
#include "log_info.hpp"

#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <fstream>

extern bool verbose;

namespace backtesting {

char const *const remote_file_downloader_t::rest_api_host = "";
uint16_t const remote_file_downloader_t::rest_api_port = 443;

remote_file_downloader_t::remote_file_downloader_t(net::ssl::context &sslContext)
    : m_sslContext(sslContext), m_ioContext(getContextObject()) {
}

void remote_file_downloader_t::start() {
  if (m_isRunning)
    return;

  m_isRunning = true;
  initiateHttpConnection();
}

void remote_file_downloader_t::getFile(backtesting::pipeline_data_t const &data) {
  m_pipelineMutex.lock();
  m_pipeline.push(data);

  if (m_pipeline.size() > 1)
    return m_pipelineMutex.unlock();

  m_pipelineMutex.unlock();

  if (m_keepAliveTimer) {
    boost::system::error_code ec;
    m_keepAliveTimer->cancel(ec);
    m_keepAliveTimer.reset();
    return downloadFile();
  }

  start();
}

void remote_file_downloader_t::initiateHttpConnection() {
  m_resolver.emplace(*m_ioContext);
  m_resolver->async_resolve(
      rest_api_host, "https",
      [this](auto const ec,
             net::ip::tcp::resolver::results_type const &results) {
        if (ec) {
          PRINT_ERROR(ec.message())
          return;
        }
        connectToResolvedNames(results);
      });
}

void remote_file_downloader_t::connectToResolvedNames(
    results_type const &resolvedNames) {
  m_resolver.reset();
  m_sslWebStream.emplace(*m_ioContext, m_sslContext);
  beast::get_lowest_layer(*m_sslWebStream)
      .expires_after(std::chrono::seconds(30));

  beast::get_lowest_layer(*m_sslWebStream)
      .async_connect(
          resolvedNames,
          [this](auto const ec,
                 net::ip::tcp::resolver::results_type::endpoint_type const
                 &connectedName) {
            if (ec) {
              PRINT_ERROR(ec.message())
              return;
            }
            performSSLHandshake(connectedName);
          });
}

void remote_file_downloader_t::performSSLHandshake(
    [[maybe_unused]]results_type::endpoint_type const &ep) {
  beast::get_lowest_layer(*m_sslWebStream)
      .expires_after(std::chrono::seconds(15));
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if (!SSL_set_tlsext_host_name(m_sslWebStream->native_handle(),
                                rest_api_host)) {
    auto const ec = beast::error_code(static_cast<int>(::ERR_get_error()),
                                      net::error::get_ssl_category());
    PRINT_ERROR(ec.message())
    return;
  }

  m_sslWebStream->async_handshake(
      net::ssl::stream_base::client, [this](beast::error_code const ec) {
        if (ec) {
          PRINT_ERROR(ec.message())
          return;
        }
        return downloadFile();
      });
}

void remote_file_downloader_t::createHttpRequest(
    std::string const &path) {
  using http::field;
  using http::verb;

  std::string const host = fmt::format("{}:{}", rest_api_host, rest_api_port);
  auto &request = m_httpRequest.emplace();
  request.method(verb::get);
  request.version(11);
  request.target(path);
  request.set(field::host, host);
  request.set(field::user_agent, "Backtesting/0.0.1");
  request.set(field::accept, "*/*");
  request.set(field::accept_language, "en-US,en;q=0.5 --compressed");
}

void remote_file_downloader_t::sendHttpRequest(std::function<void()> onDataRead) {
  beast::get_lowest_layer(*m_sslWebStream).expires_after(std::chrono::seconds(30));
  http::async_write(
      *m_sslWebStream, *m_httpRequest,
      [this, callback = std::move(onDataRead)](beast::error_code const ec, size_t const)mutable {
        if (ec) {
          PRINT_ERROR(ec.message())
          return;
        }
        readHttpRequest(std::move(callback));
      });
}

void remote_file_downloader_t::readHttpRequest(std::function<void()> callback) {
  m_httpRequest.reset();
  m_httpResponse.emplace();
  m_rwBuffer.emplace();

  http::async_read(
      *m_sslWebStream, *m_rwBuffer, *m_httpResponse,
      [onDataRead = std::move(callback)](beast::error_code const ec, size_t const)mutable {
        if (ec) {
          PRINT_ERROR(ec.message())
          return;
        }
        onDataRead();
      });
}

void remote_file_downloader_t::setTimerCallback() {
  if (!m_keepAliveTimer)
    m_keepAliveTimer.emplace(*m_ioContext);

  // ping the server periodically to keep the connection alive
  m_keepAliveTimer->expires_from_now(boost::posix_time::seconds(1));
  m_keepAliveTimer->async_wait(
      [this](auto const ec) {
        if (ec == net::error::operation_aborted)
          return;
        this->onKeepAliveTimerTimedout();
      });
}

void remote_file_downloader_t::onKeepAliveTimerTimedout() {
  // create a GET request, this is to keep the connection alive
  createHttpRequest("/");
  sendHttpRequest([this] {
    // do nothing with the response
    m_httpResponse.reset();
    m_rwBuffer.reset();
    setTimerCallback();
  });
}

void remote_file_downloader_t::decodeMessage() {
  auto &data = m_httpResponse->body();
  if (data.empty())
    return;

  auto &pipelineData = m_pipeline.front();
  std::ofstream file(pipelineData.filenameOnDisk, std::ios::out | std::ios::binary);
  if (!file)
    return;
  file << data << std::endl;
  file.close();

  {
    std::lock_guard<std::mutex> lockG{m_pipelineMutex};
    pipelineData.onFileIsReady();
    m_pipeline.pop();
    if (m_pipeline.empty())
      return setTimerCallback();
  }

  downloadFile();
}

void remote_file_downloader_t::downloadFile() {
  createHttpRequest(m_pipeline.front().remotePath);
  sendHttpRequest([this] { decodeMessage(); });
}

}
