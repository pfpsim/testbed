//#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <iostream>

#include"syscmodules.h"

SC_MODULE(Top)
{
   // Initiator *initiator;
	trie_module* trie;
	Memory* mem;
    SC_CTOR(Top)
    {
    	trie = new  trie_module("system");
    	mem    = new Memory   ("memory");
    	trie->socket_t.bind( mem->socket );
    	//S::getInstance().pointer_to_tree=trie;

    }
};


int sc_main(int argc, char *argv[])  {

    //trie_module prefix_tree("system");
    Top top("top");
    sc_start();

    return 0;
}
