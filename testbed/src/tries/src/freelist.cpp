#include "freelist.h"

Freelist::Freelist(void* start, void* end, size_t elementSize, size_t alignment, size_t offset){

	union
	{
	  void* as_void;
	  char* as_char;
	  Freelist* as_self;
	};

	// assume as_self points to the first entry in the free list
	m_next = as_self;
	as_char += elementSize;

	// initialize the free list - make every m_next of each element point to the next element in the list
	Freelist* runner = m_next;
	for (size_t i=1; i<elementSize; ++i)
	{
	  runner->m_next = as_self;
	  runner = as_self;
	  as_char += elementSize;
	}

	runner->m_next = nullptr;


}


void* Freelist::Obtain(void)
{
  // is there an entry left?
  if (m_next == nullptr)
  {
    // we are out of entries lets go to timbuktu
    return nullptr;
  }

  // obtain one element from the head of the free list
  Freelist* head = m_next;
  m_next = head->m_next;
  return head;
}

void Freelist::Return(void* ptr)
{
  // put the returned element at the head of the free list
  Freelist* head = static_cast<Freelist*>(ptr);
  head->m_next = m_next;
  m_next = head;
}
