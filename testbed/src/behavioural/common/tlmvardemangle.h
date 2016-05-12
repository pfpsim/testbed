#include <iostream>
#include <memory>
#include <string>
#include <cxxabi.h>
using namespace std;

inline std::string demangle(const char* name, void* obj)
{
  int status = -1;
  const std::type_info  &ti = typeid(obj);
  std::unique_ptr<char, void(*)(void*)> res
  { abi::__cxa_demangle(type_info.name(), NULL, NULL, &status), std::free };
  return (status == 0) ? res.get() : std::string(name);
}
