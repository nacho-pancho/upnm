cmake_minimum_required (VERSION 2.6)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
project (UPNM)

set (UPNM_VERSION_MAJOR 1)
set (UPNM_VERSION_MINOR 0)

option (PARALLEL "Use parallel instructions." OFF)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  message(STATUS "Target is 64 bits")
  set(TARGETBITS "64")
else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
  message(STATUS "Target is 32 bits")
  set(TARGETBITS "32")
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

if (WIN32)
  #
  # Windows 32 bits
  #
  set (BINDIR "win${TARGETBITS}" )
  message (STATUS "Target is Windows, output dir is " ${BINDIR})
  set (CMAKE_C_FLAGS   "/arch:SSE3") # see if you want more modern ones
  set (CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS} /DDEBUG /MTd /Od /Zi")
  set (CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS} /MT /Ox")
  if(PARALLEL)
    set(CFLAGS "${CFLAGS} /openmp")
    add_definitions(-DPARALLEL)
  endif()
else()
  if(UNIX) 
    #
    # LINUX
    # 
    set (BINDIR "lin${TARGETBITS}" )
  else()
    set (BINDIR "mac${TARGETBITS}")
  endif()
  message (STATUS      "Target is UNIX, output dir is " ${BINDIR})
  set (CMAKE_FLAGS   "-mtune=native -march=native -Wformat -Wparentheses -fPIC -pedantic")
  set (CMAKE_C_FLAGS   " ${CMAKE_FLAGS}")
  if(PARALLEL)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
    add_definitions(-DPARALLEL)
  endif()
  #set (CMAKE_EXE_LINKER_FLAGS  "-static-libgcc -static-libstdc++ -static")
  set(EXTLIB "-lm")
  set (CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS} -Wall -Wextra -DDEBUG -g -O0 -fno-omit-frame-pointer"  )
  set (CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS} -O3"  )
endif()

file(GLOB LIBSRC . lib/pnm.c) #only one file for now, but GLOB might be handy in the future

add_library(upnm ${LIBSRC})

include_directories( ${PROJECT_SOURCE_DIR}/lib )

set(TESTS
	test
)
foreach (aux ${TESTS})
 add_executable (${aux} test/${aux}.c)
 if (WIN32)
   target_link_libraries(${aux} upnm)
 else() # UNIX
   target_link_libraries(${aux} upnm -lm)
 endif()
endforeach (aux)
