#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/ssl.hpp>
#include <optional>
#include <mutex>
#include <queue>

namespace backtesting {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ip = net::ip;

struct pipeline_data_t {
  std::string remotePath;
  std::string filenameOnDisk;
  std::function<void()> onFileIsReady = nullptr;
};

class remote_file_downloader_t {
  using resolver = ip::tcp::resolver;
  using results_type = resolver::results_type;

public:
  explicit remote_file_downloader_t(net::ssl::context &sslContext);
  inline void getFile(std::string const &path, std::string const &filename,
                      std::function<void()> onReadyFile) {
    getFile({path, filename, onReadyFile});
  }

  void getFile(pipeline_data_t const &data);
private:
  void initiateHttpConnection();
  void connectToResolvedNames(results_type const &resolved_names);
  void performSSLHandshake(results_type::endpoint_type const &ep);
  void downloadFile();
  void decodeMessage();
  void createHttpRequest(std::string const &path);
  void sendHttpRequest(std::function<void()>);
  void readHttpRequest(std::function<void()>);
  void onKeepAliveTimerTimedout();
  void setTimerCallback();
  inline void start();

  net::ssl::context &m_sslContext;
  std::shared_ptr<net::io_context> m_ioContext = nullptr;
  std::optional<resolver> m_resolver = std::nullopt;
  std::optional<beast::ssl_stream<beast::tcp_stream>> m_sslWebStream = std::nullopt;
  std::optional<beast::flat_buffer> m_rwBuffer = std::nullopt;
  std::optional<http::request<http::empty_body>> m_httpRequest = std::nullopt;
  std::optional<http::response<http::string_body>> m_httpResponse = std::nullopt;
  std::optional<net::deadline_timer> m_keepAliveTimer = std::nullopt;
  std::queue<pipeline_data_t> m_pipeline;
  std::mutex m_pipelineMutex{};

  bool m_isRunning = false;
  static char const *const rest_api_host;
  static uint16_t const rest_api_port;
};

std::shared_ptr<net::io_context> getContextObject();
}
