#ifndef READ_WRITE_LOCK_H_
#define READ_WRITE_LOCK_H_

#include "systemc"
#include <set>

class ReadWriteLock {
 public:
  ReadWriteLock();

  ReadWriteLock(const ReadWriteLock &) = delete;
  ReadWriteLock & operator=(const ReadWriteLock &) = delete;

  void read_lock();
  void read_unlock();

  void write_lock();
  void write_unlock();

 private:
  std::set<sc_core::sc_process_b*> readers;
  sc_core::sc_process_b* writer;

  sc_core::sc_event cond;
};

#endif
