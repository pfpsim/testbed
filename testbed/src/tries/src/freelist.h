//Free List tracker for ppol allocator
#include<iostream>
using namespace std;
class Freelist
{
public:
  Freelist(void* start, void* end, size_t elementSize, size_t alignment, size_t offset);

  void* Obtain(void);

  void Return(void* ptr);

private:
  Freelist* m_next;
};
