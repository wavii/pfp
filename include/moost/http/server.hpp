#ifndef __MOOST_HTTP_SERVER_HPP__
#define __MOOST_HTTP_SERVER_HPP__

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "moost/http/connection.hpp"
#include "moost/http/request_handler_base.hpp"

namespace moost { namespace http {

/// The top-level class of the HTTP server.
template<class RequestHandler>
class server
  : private boost::noncopyable
{
public:
  /// Construct the server to listen on the specified TCP address and port, and
  /// serve up files from the given directory.
  explicit server(const std::string& address, int port,
      std::size_t thread_pool_size);

  RequestHandler & request_handler()
  {
    return request_handler_;
  }

  /// Run the server's io_service loop.
  void run();

  /// Stop the server.
  void stop();

private:
  /// Handle completion of an asynchronous accept operation.
  void handle_accept(const boost::system::error_code& e);

  /// The number of threads that will call io_service::run().
  std::size_t thread_pool_size_;

  /// The io_service used to perform asynchronous operations.
  boost::asio::io_service io_service_;

  /// Acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The handler for all incoming requests.
  RequestHandler request_handler_;

  /// The next connection to be accepted.
  boost::shared_ptr< connection<RequestHandler> > new_connection_;

  /// The endpoint of the address to bind
  boost::asio::ip::tcp::endpoint endpoint_;
};

template<class RequestHandler>
server<RequestHandler>::server(const std::string& address, int port,
    std::size_t thread_pool_size)
  : thread_pool_size_(thread_pool_size),
    acceptor_(io_service_),
    request_handler_(),
    new_connection_(new connection<RequestHandler>(io_service_, request_handler_))
{
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(address, boost::lexical_cast<std::string>(port));
  endpoint_ = *resolver.resolve(query);
}

template<class RequestHandler>
void server<RequestHandler>::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    new_connection_->start();
    new_connection_.reset(new connection<RequestHandler>(io_service_, request_handler_));
  }
  acceptor_.async_accept(new_connection_->socket(),
    boost::bind(&server<RequestHandler>::handle_accept, this, boost::asio::placeholders::error));
}

template<class RequestHandler>
void server<RequestHandler>::run()
{
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  acceptor_.open(endpoint_.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint_);
  acceptor_.listen();

  // pump the first async accept into the loop
  acceptor_.async_accept(new_connection_->socket(),
    boost::bind(&server<RequestHandler>::handle_accept, this,
    boost::asio::placeholders::error));

  // Create a pool of threads to run all of the io_services.
  std::vector<boost::shared_ptr<boost::thread> > threads;
  for (std::size_t i = 0; i < thread_pool_size_; ++i)
  {
    boost::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &io_service_)));
    threads.push_back(thread);
  }

  // Wait for all threads in the pool to exit.
  for (std::size_t i = 0; i < threads.size(); ++i)
    threads[i]->join();
}

template<class RequestHandler>
void server<RequestHandler>::stop()
{
  io_service_.stop();
}

}} // moost::http

#endif // __MOOST_HTTP_SERVER_HPP__
