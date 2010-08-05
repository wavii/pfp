#include <iostream>

#include <boost/shared_ptr.hpp>

#include <pfpd/pfpd_handler.h>
#include <pfp/config.h>

using namespace com::wavii::pfp;
using namespace boost;
using namespace moost;

int main(int argc, char * argv[])
{
  std::clog << "pfpd: http server for pfp!" << std::endl;
  std::clog << "build: " << __DATE__ << " (" << __TIME__ << ") of pfp version " << consts::version << " (c) Wavii,Inc. 2010" << std::endl;

  if (argc < 3)
  {
    std::cerr << "usage: " << argv[0] << " <host> <port> <max sentence length=45> <threads=1> <data dir=/usr/share/pfp/>" << std::endl;
    exit(1);
  }
  std::string host = argv[1];
  int port = lexical_cast<int>(argv[2]);
  size_t sentence_length = argc < 4 ? 45 : lexical_cast<size_t>(argv[3]);
  size_t threads = argc < 5 ? 1 : lexical_cast<size_t>(argv[4]);
  std::string data_dir = argc < 6 ? "/usr/share/pfp/" : argv[5]; // make install copies files to /usr/share/pfp by default

  http::server<pfpd_handler> server(host, port, threads);
  try
  {
    server.request_handler().init(sentence_length, threads, data_dir);
  } catch (const std::runtime_error & e)
  {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
  std::clog << "starting http service." << std::endl;
  server.run(); // our blocking event loop

  return 0;
}
