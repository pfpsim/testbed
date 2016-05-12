//
//  BaseVectorEntry.cpp
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#include "BaseVectorEntry.h"

/// ==================================
//
//  Constructors
//
/// ==================================

BaseVectorEntry::BaseVectorEntry() {}

BaseVectorEntry::BaseVectorEntry(BitString iPrefix, int iLength,
      int iPrefixTablePtr, int iActionPtr) :
      mPrefix(iPrefix),
      mLength(iLength),
      mPrefixTablePtr(iPrefixTablePtr),
      mActionPtr(iActionPtr) {
}

/// ==================================
//
//  Getters
//
/// ==================================

BitString BaseVectorEntry::getPrefix() const {
    return mPrefix;
}

int BaseVectorEntry::getLength() const {
    return mLength;
}

int BaseVectorEntry::getPrefixTablePtr() const {
    return mPrefixTablePtr;
}

int BaseVectorEntry::getActionPtr() const {
    return mActionPtr;
}

/// ==================================
//
//  Setters
//
/// ==================================

void BaseVectorEntry::setPrefix(BitString iPrefix) {
    mPrefix = iPrefix;
}

void BaseVectorEntry::setLength(int iLength) {
    mLength = iLength;
}

void BaseVectorEntry::setPrefixTablePtr(int iPrefixTablePtr) {
    mPrefixTablePtr = iPrefixTablePtr;
}

void BaseVectorEntry::setActionPtr(int iActionPtr) {
    mActionPtr = iActionPtr;
}
