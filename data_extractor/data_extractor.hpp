#pragma once

#include <string>
#include <vector>

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

namespace net = boost::asio;

class data_extractor_t {

public:
  data_extractor_t();
  ~data_extractor_t();
  int run(std::vector<std::string> &&args);
  bool stop();

private:
  net::io_context *m_ioContext = nullptr;
};
