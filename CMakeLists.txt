cmake_minimum_required(VERSION 2.8)

project(ftp)
# Set your target platform 'x86' or 'arm'
set(TARGET_PLATFORM x86)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(DDS_TARGET linux_gcc_x86)
set(CMAKE_C_COMPILER gcc)
set(DDS_HOME /opt/PrismTech/Vortex_v2/Device/VortexLite/2.0.7)

# DDS common config
set(DDS_LIB ${DDS_HOME}/lib/${DDS_TARGET}/libdds.so)
set(DDS_INCLUDE ${DDS_HOME}/include)

# compile config
set(EXT_C_FLAGS "-pipe -Wall -fno-strict-aliasing -O3 -std=c99 -Wstrict-prototypes -Wmissing-prototypes -Dos_${DDS_TARGET} -DNDEBUG -D_GNU_SOURCE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXT_C_FLAGS}")

# Set your IDL file and path
set(DDS_IDL_INCLUDE idl)
set(DDS_IDL_SRC idl/FTP.c)

## Build executable file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY release)
include_directories(${DDS_IDL_INCLUDE})
include_directories(${DDS_INCLUDE})
add_executable(publisher publisher/publisher.c ${DDS_IDL_SRC})
target_link_libraries(publisher ${DDS_LIB})

## Build executable file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY release)
include_directories(${DDS_IDL_INCLUDE})
include_directories(${DDS_INCLUDE})
add_executable(subscriber subscriber/subscriber.c ${DDS_IDL_SRC})
target_link_libraries(subscriber ${DDS_LIB})

