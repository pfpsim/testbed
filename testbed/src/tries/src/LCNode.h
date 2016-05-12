//
//  LCNode.h
//  Trie Data Structure
//
//  Created by Eric Tremblay on 2015-02-08.
//  Copyright (c) 2015 Eric Tremblay. All rights reserved.
//

#ifndef __Trie_Data_Structure__LCNode__
#define __Trie_Data_Structure__LCNode__

#include <stdio.h>
#include <iostream>

using namespace std;

class LCNode {
public:
    // Constructor
    LCNode(int iBranch = 0, int iSkip = 0);

    // Destructor
    virtual ~LCNode();

    // Getters
    int getBranchFactor() const;
    int getSkip() const;
    int getLeftNodePos() const;

    // Setters
    void setBranchFactor(int iBranch);
    void setSkip(int iSkip);
    void setLeftNodePos(int iPos);

    void* operator new(long unsigned) throw(const char*);
    void* operator new[](long unsigned) throw(const char*);
    void operator delete(void*);
    void operator delete[](void*);

private:
    int mBranchFactor;
    int mSkip;
    int mLeftNodePos;
    int tlm_addr;
};

#endif /* defined(__Trie_Data_Structure__LCNode__) */
