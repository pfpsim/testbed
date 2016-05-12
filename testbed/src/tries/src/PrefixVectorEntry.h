//
//  PrefixVectorEntry.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-24.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__PrefixVectorEntry__
#define __Trie_Data_Structure__PrefixVectorEntry__

#include <iostream>

class PrefixVectorEntry {
public:
    // Constructors
    PrefixVectorEntry();
    PrefixVectorEntry(int iLength, int iPrefixPtr, int iActionPtr = 0);
    
    //Getters
    int getLength() const;
    int getPrefixPtr() const;
    int getActionPtr() const;
    
    //Setters
    void setLength(int iLength);
    void setPrefixPtr(int iPrefixPtr);
    void setActionPtr(int iActionPtr);
    
private:
    int mLength;
    int mPrefixPtr;
    int mActionPtr;
};

#endif /* defined(__Trie_Data_Structure__PrefixVectorEntry__) */