//
//  RoutingTableEntry.h
//

#ifndef __Trie_Data_Structure__RoutingTableEntry__
#define __Trie_Data_Structure__RoutingTableEntry__

#include <iostream>
#include "BitString.h"

template <class T>
class RoutingTableEntry {
public:
    
    RoutingTableEntry(BitString iData, int iLength, T iAction, int iActionSize);
    RoutingTableEntry();
    
    //Getters
    BitString getData() const;
    int getLength() const;
    T getAction() const;
    int getActionSize() const;
    
    //Setters
    void setData(BitString iData);
    void setLength(int iLength);
    void setAction(T iAction);
    void setActionSize(int iSize);
    
    static bool compareEntries(RoutingTableEntry iFirst, RoutingTableEntry iSecond);
    bool isPrefixOf(RoutingTableEntry iEntry);
    
private:
    BitString mData;
    int mLength;
    T mAction;
    int mActionSize;
};

//
//  RoutingTableEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

//#include "RoutingTableEntry.h"

/// ==================================
//
//  Constructors
//
/// ==================================

template <class T>
RoutingTableEntry<T>::RoutingTableEntry(BitString iData, int iLength, T iAction, int iActionSize) : mData(iData), mLength(iLength), mAction(iAction), mActionSize(iActionSize) {
    
}

template <class T>
RoutingTableEntry<T>::RoutingTableEntry() {}

/// ==================================
//
//  Getters
//
/// ==================================

template <class T>
BitString RoutingTableEntry<T>::getData() const {
    return mData;
}

template <class T>
int RoutingTableEntry<T>::getLength() const {
    return mLength;
}

template <class T>
T RoutingTableEntry<T>::getAction() const {
    return mAction;
}

template <class T>
int RoutingTableEntry<T>::getActionSize() const {
    return mActionSize;
}

/// ==================================
//
//  Setters
//
/// ==================================

template <class T>
void RoutingTableEntry<T>::setData(BitString iData) {
    mData = iData;
}

template <class T>
void RoutingTableEntry<T>::setLength(int iLength) {
    mLength = iLength;
}

template <class T>
void RoutingTableEntry<T>::setAction(T iAction) {
    mAction = iAction;
}

template <class T>
void RoutingTableEntry<T>::setActionSize(int iSize) {
    mActionSize = iSize;
}

/// =========================================================
//
//  Compare two entries
//
//  Used to sort entries during LC-Trie construction
//  Returns true if iFirst comes before iSecond
//
/// =========================================================

template <class T>
bool RoutingTableEntry<T>::compareEntries(RoutingTableEntry iFirst, RoutingTableEntry iSecond) {
    int wFirstLength = (int)iFirst.getData().size();
    int wSecondLength = (int)iSecond.getData().size();
    int wLength = 0;
    if (wFirstLength < wSecondLength) {
        wLength = wFirstLength;
    } else {
        wLength = wSecondLength;
    }
    for (int i = 0; i < wLength; i++) {
        BitString wTempFirst = iFirst.getData();
        BitString wTempSecond = iSecond.getData();
        if (wTempFirst[i] == wTempSecond[i]) {
            continue;
        } else if (wTempFirst[i] == 1 && wTempSecond[i] == 0) {
            return false;
        } else {
            return true;
        }
    }
    if (wFirstLength < wSecondLength) {
        return true;
    } else {
        return false;
    }
}

/// =========================================================================
//
//  Checks if iEntry is a prefix of the entry on which this is called
//
/// =========================================================================

template <class T>
bool RoutingTableEntry<T>::isPrefixOf(RoutingTableEntry iEntry) {
    return (mLength == 0 || (mLength <= iEntry.getLength() && iEntry.getData().find(mData) == 0));
}

#endif /* defined(__Trie_Data_Structure__RoutingTableEntry__) */