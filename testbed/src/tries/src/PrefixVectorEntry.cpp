//
//  PrefixVectorEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "PrefixVectorEntry.h"

/// ==================================
//
//  Constructors
//
/// ==================================

PrefixVectorEntry::PrefixVectorEntry() {}

PrefixVectorEntry::PrefixVectorEntry(int iLength, int iPrefixPtr, int iActionPtr) : mLength(iLength), mPrefixPtr(iPrefixPtr), mActionPtr(iActionPtr) {
    
}

/// ==================================
//
//  Getters
//
/// ==================================

int PrefixVectorEntry::getLength() const {
    return mLength;
}

int PrefixVectorEntry::getPrefixPtr() const {
    return mPrefixPtr;
}

int PrefixVectorEntry::getActionPtr() const {
    return mActionPtr;
}

/// ==================================
//
//  Setters
//
/// ==================================

void PrefixVectorEntry::setLength(int iLength) {
    mLength = iLength;
}

void PrefixVectorEntry::setPrefixPtr(int iPrefixPtr) {
    mPrefixPtr = iPrefixPtr;
}

void PrefixVectorEntry::setActionPtr(int iActionPtr) {
    mActionPtr = iActionPtr;
}