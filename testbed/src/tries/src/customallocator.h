#include <cstddef>


template<typename T>
class my_allocator
{
public:
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  my_allocator() {}
  my_allocator(const my_allocator&) {}



  pointer   allocate(size_type n, const void * = 0)
 	  	  	  {
              T* t = (T*) malloc(n * sizeof(T));
              std::cout
              << "-used my_allocator to allocate   at address "
              << t << " (+)" << std::endl;

             // S::getInstance().pointer_to_tree->allocate_mem(5);
             // S::getInstance().pointer_to_tree->read_mem(4);
              return t;
            }

  void      deallocate(void* p, size_type) {
              if (p) {
                free(p);
                std::cout
                << "-used my_allocator to deallocate at address "
                << p << " (-)" <<
                std::endl;
              }
            }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  my_allocator<T>&  operator=(const my_allocator&) {

	  	  	  	  	  	  std::cout<<"-my_allocator <T>=& called"<<std::endl;

	  	  	  	  	  	  return *this;
  	  	  	  	  	  }
  void              construct(pointer p, const T& val)
                    {
	  	  	  	  	  std::cout<<"-my_allocator Construct called"<<std::endl;

	  	  	  	  	  new ((T*) p) T(val);
                    }
  void              destroy(pointer p) { p->~T(); }

  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef my_allocator<U> other; };

  template <class U>
  my_allocator(const my_allocator<U>&) {}

  template <class U>
  my_allocator& operator=(const my_allocator<U>&) {
						  std::cout<<"-my_allocator &= called"<<std::endl;
						  return *this;
  }

};//end my_allocator
