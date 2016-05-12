#ifndef COMMON_P4_H_
#define COMMON_P4_H_

#include <string>
using std::string;
#include <map>
using std::map;

#include <bm_sim/switch.h>
#include <bm_sim/lookup_structures.h>
#include <bm_sim/logger.h>

#include <iostream>
#include <cassert>

#include "ReadWriteLock.h"

#include "pfpsim/pfpsim.h"

extern int import_primitives();

struct Value {
  Value(bool match) : match{match} {}
  Value(bm::internal_handle_t handle) : handle{handle} {}
  bm::internal_handle_t handle = ~0;
  bool match                   = true;

  // Various ctors needed by trie types
  Value() {}
  Value(int) : match{false} {}
  Value(const Value & other) = default;
  Value(Value && other) = default;
  Value & operator=(const Value & other) = default;

  bool operator != (const Value & v) const{
    return match != v.match || handle != v.handle;
  }

  bool operator==(const Value& v) const {
    return match == v.match && handle == v.handle;
  }

  bool operator<(const Value& v) const {
    return handle < v.handle;
  }
};

/**
 * Multiton class that allows managing instances
 * of P4 switches. For now we only need a signleton
 * instance, but in the future we may want to have
 * many P4-programmed switches running in one
 * simulation.
 */
class P4 : public bm::Switch {
  private:
  class MemAwareLookupStructureFactory : public bm::LookupStructureFactory {
    std::unique_ptr<bm::ExactLookupStructure>
    create_for_exact(size_t size, size_t nbytes_key) override;

    std::unique_ptr<bm::LPMLookupStructure>
    create_for_LPM(size_t size, size_t nbytes_key) override;

    std::unique_ptr<bm::TernaryLookupStructure>
    create_for_ternary(size_t size, size_t nbytes_key) override;
  };


  private:
  static map<string, P4*> instances;
  static std::shared_ptr<MemAwareLookupStructureFactory> factory;

  public:
  static P4 * get(string name);

  private:
  // This class should only be constructed through the
  // static multiton method.
  P4();
  P4(P4&&)                 = default;
  P4(const P4&)            = default;
  P4& operator=(const P4&) = default;

  public:
  // Required but unused
  int  receive(int, const char *, int);
  void start_and_return();

  virtual ~P4();

    ReadWriteLock lock;
};


inline void setP4LoggingLevels() {
  bm::Logger::set_logger_console();
  switch (pfp::core::PFPConfig::get().get_verbose_level()) {
    default:
    case pfp::core::PFPConfig::verbosity::normal:
       bm::Logger::set_log_level(bm::Logger::LogLevel::INFO);
       break;

    case pfp::core::PFPConfig::verbosity::minimal:
       bm::Logger::set_log_level(bm::Logger::LogLevel::CRITICAL);
       break;

    case pfp::core::PFPConfig::verbosity::p4profile:
    case pfp::core::PFPConfig::verbosity::profile:
       bm::Logger::set_log_level(bm::Logger::LogLevel::TRACE);
       break;

    case pfp::core::PFPConfig::verbosity::debug:
       bm::Logger::set_log_level(bm::Logger::LogLevel::DEBUG);
       break;
  }
}

#endif
