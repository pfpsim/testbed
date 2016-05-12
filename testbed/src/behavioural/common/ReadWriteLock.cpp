#include "ReadWriteLock.h"

// TODO(gordon) should be changed to pfpsim.h
#include <iostream>
#include <string>
#include "pfpsim/pfpsim.h"

static std::string module_name_ = "RW Lock";  // HACK FOR npulog
using std::cout;
using std::endl;

ReadWriteLock::ReadWriteLock()
  : writer(nullptr) {
}

void ReadWriteLock::read_lock() {
  auto this_proc = sc_core::sc_get_current_process_b();

  sc_assert(readers.find(this_proc) == readers.end() &&
                 "Attempted to get read lock while already holding it");

  npulog(profile,
    cout << this_proc->name()
         << " attempting to get read lock@" << this << endl;)

  // If someone is writing, then we wait till we can read
  while (writer) {
    sc_core::wait(cond);
  }

  npulog(profile,
    cout << this_proc->name() << " got read lock@" << this << endl;)

  // And once we can read, we mark that we are reading
  readers.insert(this_proc);
}

void ReadWriteLock::read_unlock() {
  auto this_proc = sc_core::sc_get_current_process_b();

  // And once we read, we mark that we are reading
  if (!readers.erase(this_proc)) {
    sc_assert(!"Attempted to release read lock while not holding it");
  }

  npulog(profile,
     cout << this_proc->name() << " released read lock@" << this << endl;)

  if (readers.size() == 0 && writer) {
    cond.notify();
  }
}

void ReadWriteLock::write_lock() {
  auto this_proc = sc_core::sc_get_current_process_b();
  sc_assert(writer != this_proc &&
            "Attempted to get write lock while already holding it");

  npulog(profile,
     cout << this_proc->name()
          << " attempting to get write lock@" << this << endl;)

  // If there is already a writer, wait for them to finish
  while (writer) {
    sc_core::wait(cond);
  }

  // let everyone know that we're waiting to write
  writer = this_proc;

  // Wait until there are no readers
  while (readers.size()) {
    sc_core::wait(cond);
  }

  npulog(profile,
    cout << this_proc->name() << " got write lock@" << this << endl;)
}

void ReadWriteLock::write_unlock() {
  auto this_proc = sc_core::sc_get_current_process_b();
  sc_assert(writer == this_proc &&
            "Attempted to release write lock while not holding it");

  writer = nullptr;

  npulog(profile,
    cout << this_proc->name() << " released write lock@" << this << endl;)

  cond.notify();
}
