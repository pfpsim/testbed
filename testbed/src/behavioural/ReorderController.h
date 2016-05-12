#ifndef BEHAVIOURAL_REORDERCONTROLLER_H_
#define BEHAVIOURAL_REORDERCONTROLLER_H_
#include <deque>
#include <string>
#include <vector>
#include <functional>
#include <queue>
#include "../structural/ReorderControllerSIM.h"
#include "CommonIncludes.h"

typedef std::priority_queue<std::shared_ptr<PacketDescriptor>,
                            std::vector<std::shared_ptr<PacketDescriptor>>,
                            pdcomparison> pdq;
typedef std::priority_queue<std::size_t, std::vector<std::size_t>,
                            std::greater<std::size_t>> idq;


class ReorderController: public ReorderControllerSIM {
 public:
  SC_HAS_PROCESS(ReorderController);
  /*Constructor*/
  explicit ReorderController(sc_module_name nm, pfp::core::PFPObject* parent = 0, std::string configfile = "");  // NOLINT(whitespace/line_length)
  /*Destructor*/
  virtual ~ReorderController() = default;

 public:
  void init();

 private:
  void ReorderController_PortServiceThread();
  void ReorderControllerThread(std::size_t thread_id);
  /**
 * Thread disables reordering it just passes through
 */
  void NOREORDER();
  std::vector<sc_process_handle> ThreadHandles;

  //! Store the Packet order for each context
  std::vector<std::deque<std::size_t>> packet_order_;
  //! Reorder the Packets in each context
  std::vector<std::deque<std::shared_ptr<PacketDescriptor>>> packet_reorder_;
  //! Store the Packet order for each context
  std::vector<idq> ppacket_order_;
  //! Reorder the Packets in each context
  std::vector<pdq> ppacket_reorder_;
  std::vector<bool> pd_flags_;
  sc_mutex packet_order_gaurd;
  sc_mutex packet_reorder_gaurd;
  sc_event bufferevent;
};

#endif  // BEHAVIOURAL_REORDERCONTROLLER_H_
