//
//  BaseVectorEntry.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __TRIE__BASEVECTORENTRY__
#define __TRIE__BASEVECTORENTRY__

#include <iostream>
#include "BitString.h"

class BaseVectorEntry {
 public:
    // Constructors
    BaseVectorEntry();
    BaseVectorEntry(BitString iPrefix, int iLength,
        int iPrefixTablePtr, int iActionPtr = 0);

    // Getters
    BitString getPrefix() const;
    int getLength() const;
    int getPrefixTablePtr() const;
    int getActionPtr() const;

    // Setters
    void setPrefix(BitString iPrefix);
    void setLength(int iLength);
    void setPrefixTablePtr(int iPrefixTablePtr);
    void setActionPtr(int iActionPtr);

 private:
    BitString mPrefix;
    int mLength;
    int mPrefixTablePtr;
    int mActionPtr;
};

#endif  // __TRIE__BASEVECTORENTRY__
