#ifndef __RESOURCE_STACK_HPP__
#define __RESOURCE_STACK_HPP__

#include <stack>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

namespace com { namespace wavii { namespace pfp {

// thread-safe way of grabbing a resource from a pool
template<typename T>
class resource_stack
{
private:

  std::stack< T * > resources_;
  boost::mutex      mutex_;
  boost::condition  cond_;

public:

  ~resource_stack()
  {
    while (!resources_.empty())
    {
      delete resources_.top();
      resources_.pop();
    }
  }

  // raii resource access
  class scoped_resource
  {
  private:
    T *                  pr_;
    resource_stack &     rs_;
  public:
    scoped_resource(resource_stack & rs): rs_(rs)
    {
      boost::mutex::scoped_lock lock(rs_.mutex_);
      while (rs_.resources_.empty())
        rs_.cond_.wait(lock);
      pr_ = rs_.resources_.top();
      rs_.resources_.pop();
    }
    ~scoped_resource()
    {
      boost::mutex::scoped_lock lock(rs_.mutex_);
      rs_.resources_.push(pr_);
      rs_.cond_.notify_one();
    }
    T & operator* () { return *pr_; }
    T * operator->() { return pr_; }
  };
  // add a resource to the stack
  // we take ownership
  void add_resource(T * presource)
  {
    boost::mutex::scoped_lock lock(mutex_);
    resources_.push(presource);
    cond_.notify_one();
  }
};

}}} // com::wavii::pfp

#endif // __RESOURCE_STACK_HPP__
