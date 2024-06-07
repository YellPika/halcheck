#ifndef HALCHECK_THREAD_MUTEX_HPP
#define HALCHECK_THREAD_MUTEX_HPP

#include <halcheck/thread/version.hpp>

#include <mutex>

namespace halcheck { namespace thread {

class mutex {
public:
  void lock();
  void unlock();

private:
  std::mutex impl;
  thread::version version;
};

}} // namespace halcheck::thread
#endif
