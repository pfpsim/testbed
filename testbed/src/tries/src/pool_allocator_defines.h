/*
 * pool_allocator_defines.h
 *
 *  Created on: June 2, 2015
 *      Author: Lemniscate Snickets
 */


#define use_pool_allocator true //This flag is useless right now TODO: add the #if defines for normal Stack type allocator
#define grow_pool_flag true;
#define debug_free_list false
#define debug_pool false
#define pool_size 4096 // 4kb for multi bit tree
#define slot_size 32 //32bytes this means we will have 4096/32 = 128 slots of 32 bytes which will be aligned @ 4bytes/32bits
#define boundry_alignment 4 //4bytes
#define offset 0
