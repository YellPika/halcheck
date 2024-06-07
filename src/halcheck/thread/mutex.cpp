#include "halcheck/thread/mutex.hpp"

namespace halcheck { namespace thread {

void mutex::lock() {
  impl.lock();
  thread::version::join(version);
}

void mutex::unlock() {
  version = thread::version::next();
  impl.unlock();
}

}} // namespace halcheck::thread
