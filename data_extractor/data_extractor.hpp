#pragma once

#include <string>

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
  int run(size_t const argc, char **argv);
  bool stop();

private:
  net::io_context *m_ioContext = nullptr;
  bool m_contextIsRunning = false;
};
