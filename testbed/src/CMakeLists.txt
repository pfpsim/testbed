cmake_minimum_required (VERSION 2.8.7)
set(ProjectName
  "testbed"
)
set(Executable
  "testbed-sim"
)

include(ExternalProject)
project (${ProjectName})

add_subdirectory(behavioural)
add_subdirectory(structural)
set(SOURCE_FILES
  ${STRUCTURAL_SRC}
  ${BEHAVIOURAL_SRC}
)

#Executable
add_executable (${Executable} ${SOURCE_FILES})
#add_dependencies( ${Executable} )

# Add find_package modules for SystemC and TLM
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

# Find Packages
find_package(pfpsim REQUIRED)
find_package(pfp-p4 REQUIRED)

# Support for SystemC dynamic processes
add_definitions (-DSC_INCLUDE_DYNAMIC_PROCESSES)

# Include SystemC, TLM, and third-party headers
include_directories (
   ${PFPSIM_INCLUDE_DIRS}
   "./"
 )

# Link the appropriate libraries
target_link_libraries(${Executable}
   ${PFPSIM_LIBRARIES}
   -lpthread
   -lgmp
  )

#-----------IDEs (especially VS)------------------------------------------------
source_group("" FILES ${SOURCE_FILES})
# Account for Eclipse's weird C++11 syntax handling
if (${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
   add_definitions (-DCMAKE_CXX_COMPILER_ARG1=-std=c++11)
   add_definitions (-DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE)   # Also some niceties for Eclipse project file handling
endif (${CMAKE_EXTRA_GENERATOR} MATCHES "Eclipse CDT4")
#--------------PFPDEBUGGER FLAGS------------------------------------------------
option(PFPSIMDEBUGGER "PFPSIMDEBUGGER" ON)
if("${PFPSIMDEBUGGER}" MATCHES "ON")
  message (STATUS "PFPSIM DEBUGGER FLAG IS ON")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0 -DPFPSIM_DEBUGGER")
else ()
  message (STATUS "PFPSIM DEBUGGER FLAG IS OFF")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")
endif()
#--------------Compiler Flags--------------------------------------------------
option(GCOV "GCOV" ON)
include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG ("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG ("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if (COMPILER_SUPPORTS_CXX11)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fmessage-length=0 -MMD -MP -Wno-reorder -Wno-return-type -Wno-unused -Wno-overloaded-virtual -Wno-tautological-compare -frtti")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++ -fcolor-diagnostics")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      if("${GCOV}" MATCHES "ON")
        message(STATUS "GCov flag is on")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
      endif()
    else()
      message(FATAL "UNSUPPORTED COMPILER: ${CMAKE_CXX_COMPILER} --- USE GCC or CLANG")
    endif()
  else()
    message(FATAL "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif ()
else()
  message (FATAL "MSVC is not supported - Please use GCC or CLANG ")
endif ()
