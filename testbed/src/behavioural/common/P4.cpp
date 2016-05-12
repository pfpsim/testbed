#include "P4.h"
#include <bm_sim/lookup_structures.h>
#include <cassert>
#include <limits>

#include "tlmsizedefs.h"
#define LPM_TRIE PrefixTree
#define EXACT_TRIE HashTrie
#define tlmsize_ValueStruct 10
#ifdef TARGET_POINTER_SIZE
#undef TARGET_POINTER_SIZE
#endif
#define TARGET_POINTER_SIZE 2

#include "tries/src/Trie.h"
#include "tries/src/HashTrie.h"
#include "tries/src/PrefixTree.h"
#include "tries/src/LCTrie.h"
#include "tries/src/MultibitTrie.h"
#include "tries/src/RangeTrie.h"


map<string, P4*> P4::instances;
std::shared_ptr<P4::MemAwareLookupStructureFactory> P4::factory{new P4::MemAwareLookupStructureFactory{}};

P4::P4(){
  import_primitives();
}

int  P4::receive(int, const char *, int){
  return 0;
}

void P4::start_and_return(){
}

P4::~P4(){
}

namespace { // anonymous

  BitString byte_container_2_bitstring(const bm::ByteContainer & bc,
                                       int nbits=std::numeric_limits<int>::max()){
    BitString bs;
    int bits = 0;
    for(const char b : bc){ // TODO byte order
      for(int i = 7; i >= 0 && bits < nbits; --i, ++bits){   // TODO bit order
        bs.push_back( b & (1<<i) );
      }
    }
    return bs;
  }

  class MemAwareExactMap : public bm::ExactLookupStructure {
    private:
      Trie<Value> *hashtrie;
      // HashTrie<Value> hashtrie;
    public:
      MemAwareExactMap()
      :hashtrie(0){}

      bool lookup(const bm::ByteContainer & key_data,
                  bm::internal_handle_t * handle) const override {
        if(hashtrie){
          Value v = hashtrie->exactPrefixMatch
            (byte_container_2_bitstring(key_data));
          *handle = v.handle;
          return v.match;
        } else {
          return false;
        }
      }

      bool entry_exists(const bm::ExactMatchKey & key) const override {
        if(hashtrie){
          Value v = hashtrie->exactPrefixMatch
            (byte_container_2_bitstring(key.data));
          return v.match;
        } else {
          return false;
        }
      }

      void add_entry(const bm::ExactMatchKey & key,
                     bm::internal_handle_t handle) override {
        RoutingTableEntry<Value> entry(byte_container_2_bitstring(key.data),std::numeric_limits<int>::max(), Value(handle), sizeof(bm::internal_handle_t));
        if (hashtrie){
          hashtrie->update(&entry, 1, Trie<Value>::Action::Add);
        } else {
          hashtrie = new EXACT_TRIE<Value>(&entry, 1, Value(false), sizeof(bm::internal_handle_t));
        }
      }

      void store_entry(const std::vector<bm::ExactMatchKey> & keys, std::vector<bm::internal_handle_t> handles) override {
        std::vector<RoutingTableEntry<Value>> entries;
        for(int i = 0; i < keys.size(); i++) {
          RoutingTableEntry<Value> entry(byte_container_2_bitstring(keys[i].data), std::numeric_limits<int>::max(), Value(handles[i]), sizeof(bm::internal_handle_t));
          entries.push_back(entry);
        }
        if(hashtrie){
          hashtrie->update(entries.data(), entries.size(), Trie<Value>::Action::Add);
        } else {
          hashtrie = new EXACT_TRIE<Value>(entries.data(),entries.size(),Trie<Value>::Action::Add,sizeof(bm::internal_handle_t));
        }
      }

      void delete_entry(const bm::ExactMatchKey & key) override {
        RoutingTableEntry<Value> entry(byte_container_2_bitstring(key.data),std::numeric_limits<int>::max(), Value(false), sizeof(bm::internal_handle_t));
        if (hashtrie){
          hashtrie->update(&entry, 1, Trie<Value>::Action::Remove);
        }
        else{
          cout<<"Exact Match Trie could not delete - Trie does not exist"<<__FILE__<<endl;
        }
      }

      //! Completely remove all entries from the data structure.
      void clear() override {
        assert(!"NOT IMPLEMENTED");
      }
  };

  class MemAwareLPMTrie : public bm::LPMLookupStructure {
    private:
      Trie<Value> *trie;
    public:
      MemAwareLPMTrie() : trie(0) {}

      bool lookup(const bm::ByteContainer & key_data,
                  bm::internal_handle_t * handle) const override {
        if(trie) {
          Value v = trie->longestPrefixMatch
            (byte_container_2_bitstring(key_data));
          *handle = v.handle;
          return v.match;
        } else {
          return false;
        }
      }

      bool entry_exists(const bm::LPMMatchKey & key) const override {
        if(trie) {
          Value v = trie->exactPrefixMatch
            (byte_container_2_bitstring(key.data, key.prefix_length));
          return v.match;
        } else {
          return false;
        }
      }

      void add_entry(const bm::LPMMatchKey & key,
                     bm::internal_handle_t handle) override {
        RoutingTableEntry<Value> entry(byte_container_2_bitstring(key.data, key.prefix_length), key.prefix_length, Value(handle), sizeof(bm::internal_handle_t));
        if(trie) {
          trie->update(&entry, 1, Trie<Value>::Action::Add);
        } else {
          trie = new LPM_TRIE<Value>(&entry, 1, Value(false), sizeof(bm::internal_handle_t));
        }
      }

      void store_entry(const std::vector<bm::LPMMatchKey> & keys, std::vector<bm::internal_handle_t> handles) override {
        std::vector<RoutingTableEntry<Value>> entries;
        for(int i = 0; i < keys.size(); i++) {
          RoutingTableEntry<Value> entry(byte_container_2_bitstring(keys[i].data, keys[i].prefix_length), keys[i].prefix_length, Value(handles[i]), sizeof(bm::internal_handle_t));
          entries.push_back(entry);
        }
        if(trie) {
          try {
            trie->update(entries.data(), entries.size(), Trie<Value>::Action::Add);
          } catch (std::exception &e) {
            // TODO: this is a temporary fix for the LCTrie
            trie->update(entries.data(), entries.size(), Trie<Value>::Action::Reconstruct);
          }
        } else {
          trie = new LPM_TRIE<Value>(entries.data(), entries.size(), Value(false), sizeof(bm::internal_handle_t));
        }

      }

      void delete_entry(const bm::LPMMatchKey & key) override {
        if(trie) {
          RoutingTableEntry<Value> entry(byte_container_2_bitstring(key.data, key.prefix_length), key.prefix_length, Value(false), sizeof(bm::internal_handle_t));
          trie->update(&entry, 1, Trie<Value>::Action::Remove);
        }
      }

      //! Completely remove all entries from the data structure.
      void clear() override {
        assert(!"NOT IMPLEMENTED");
      }
  };

}

std::unique_ptr<bm::ExactLookupStructure>
P4::MemAwareLookupStructureFactory::create_for_exact(size_t size,
    size_t nbytes_key) {
  std::cout << "Create Mem Aware Exact Match Structure!!" << std::endl;
  return std::unique_ptr<bm::ExactLookupStructure>{new MemAwareExactMap()};
}

std::unique_ptr<bm::LPMLookupStructure>
P4::MemAwareLookupStructureFactory::create_for_LPM(size_t size,
    size_t nbytes_key) {
  std::cout << "Create Mem Aware LPM Match Structure!!" << std::endl;
  return std::unique_ptr<bm::LPMLookupStructure>{new MemAwareLPMTrie()};
}

std::unique_ptr<bm::TernaryLookupStructure>
P4::MemAwareLookupStructureFactory::create_for_ternary(size_t size,
    size_t nbytes_key) {
  assert(!"Not implemented");
  return nullptr;
}

P4 * P4::get(string name){
  if(instances.count(name)){
    return instances[name];
  } else {
    P4 * newp4 = new P4();
    std::cout << "Creating P4 obj and setting factory" << std::endl;
    /* 65135 */newp4->set_lookup_factory(factory);
    // 64737
    return instances[name] = newp4;
  }
}
