#ifndef BEHAVIOURAL_PARSER_H_
#define BEHAVIOURAL_PARSER_H_
#include <string>
#include <vector>
#include "../structural/ParserSIM.h"
#include "CommonIncludes.h"
class Parser: public ParserSIM {
 public:
  SC_HAS_PROCESS(Parser);
  /*Constructor*/
  explicit Parser(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~Parser() = default;

 public:
  void init();

 private:
  void ParserThread(std::size_t thread_id);
  std::vector<sc_process_handle> ThreadHandles;
  //! Note: Number of rows equal to isolation_groups;
  //! number of columns equal to rcos
  std::vector<std::vector<std::size_t>> credit_matrix_;
};
#endif  // BEHAVIOURAL_PARSER_H_
