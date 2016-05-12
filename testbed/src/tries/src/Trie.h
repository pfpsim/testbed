//
//  Trie.h
//

#ifndef __Trie__
#define __Trie__

#include <string>
#include "RoutingTableEntry.h"
#include "BitString.h"

using namespace std;

template <class T>
class Trie {
public:
    
    // Enumeration
    enum Action {
        Reconstruct,
        Add,
        Remove
    };
    
    // Constructor
    Trie();
    
    // Destructor
    virtual ~Trie();
    
    // Update
    virtual void update(RoutingTableEntry<T> *iRoutingTable, int iRoutingTableSize, Action iAction) = 0;
    
    // Lookup
    virtual T exactPrefixMatch(BitString iPrefix) const = 0;
    virtual T longestPrefixMatch(BitString iPrefix) const = 0;
    
};

//
//  Trie.cpp
//

//#include "Trie.h"

/// ============================
//
//  Constructor
//
/// ============================

template <class T>
Trie<T>::Trie() {}

/// ============================
//
//  Destructor
//
/// ============================

template <class T>
Trie<T>::~Trie() {}

#endif /* defined(__Trie__) */
