PROJECT(pfp)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

SET(CMAKE_VERBOSE_MAKEFILE ON)

# binaries get installed here
SET(CMAKE_INSTALL_PREFIX "/usr/")

# add definitions, compiler switches, etc.

IF(APPLE)
   ADD_DEFINITIONS(-Wall -O3 -DNDEBUG)
ELSE(APPLE)
   ADD_DEFINITIONS(-Wall -O3 -DNDEBUG -march=native -mtune=native `getconf LFS_CFLAGS`)
ENDIF(APPLE)

INCLUDE_DIRECTORIES(include /usr/include/python2.6)

ADD_EXECUTABLE(test
               src/test/lexicon.cpp
               src/test/pcfg_parser.cpp
               src/test/tokenizer.cpp
               src/test/pfp.cpp
               src/test/main.cpp
               )

ADD_EXECUTABLE(pfpc
               src/pfpc/main.cpp
               )

ADD_EXECUTABLE(pfpc_token
               src/pfpc/pfpc_token.cpp
               )

ADD_LIBRARY(pfp SHARED
            src/pfp/config
            src/pfp/tokenizer.yy
            )

ADD_EXECUTABLE(pfpd
               src/pfpd/main
               src/pfpd/pfpd_handler
               src/moost/http/mime_types
               src/moost/http/reply
               src/moost/http/request_parser
               )

IF(APPLE)
   TARGET_LINK_LIBRARIES(pfpd pfp boost_filesystem-mt boost_thread-mt boost_regex-mt boost_system-mt icuio)
   TARGET_LINK_LIBRARIES(pfpc pfp boost_filesystem-mt boost_thread-mt boost_regex-mt boost_system-mt icuio)
   TARGET_LINK_LIBRARIES(pfpc_token pfp boost_filesystem-mt boost_thread-mt boost_regex-mt boost_system-mt icuio)
   TARGET_LINK_LIBRARIES(test pfp boost_thread-mt boost_unit_test_framework-mt boost_regex-mt icuio)
   TARGET_LINK_LIBRARIES(pfp boost_filesystem-mt boost_thread-mt boost_regex-mt boost_system-mt icuio icuuc)
ELSE(APPLE)
   TARGET_LINK_LIBRARIES(pfpd pfp boost_filesystem boost_thread boost_regex boost_system icuio icuuc)
   TARGET_LINK_LIBRARIES(pfpc pfp boost_filesystem boost_thread boost_regex boost_system icuio icuuc)
   TARGET_LINK_LIBRARIES(pfpc_token pfp boost_filesystem boost_thread boost_regex boost_system icuio icuuc)
   TARGET_LINK_LIBRARIES(test pfp boost_filesystem boost_thread boost_unit_test_framework boost_regex boost_system icuio icuuc)
ENDIF(APPLE)

INSTALL(TARGETS pfpd DESTINATION bin)
INSTALL(TARGETS pfpc DESTINATION bin)
INSTALL(TARGETS pfpc_token DESTINATION bin)

INSTALL(TARGETS pfp LIBRARY DESTINATION lib)
INSTALL(DIRECTORY share/pfp DESTINATION share)
