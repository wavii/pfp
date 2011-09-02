PROJECT(pfp)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_VERBOSE_MAKEFILE ON)

# binaries get installed here
SET(CMAKE_INSTALL_PREFIX "/usr/")

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -O3 -DNDEBUG -march=native -mtune=native `getconf LFS_CFLAGS`)

INCLUDE_DIRECTORIES(include /usr/include/python2.6)

ADD_EXECUTABLE(test
               src/test/lexicon.cpp
               src/test/pcfg_parser.cpp
               src/test/tokenizer.cpp
               src/test/main.cpp
               )

ADD_EXECUTABLE(pfpc
               src/pfpc/main.cpp
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

TARGET_LINK_LIBRARIES(pfpd pfp boost_filesystem boost_thread boost_regex boost_system icuio)
TARGET_LINK_LIBRARIES(pfpc pfp boost_filesystem boost_thread boost_regex boost_system icuio)
TARGET_LINK_LIBRARIES(test pfp boost_thread boost_unit_test_framework boost_regex icuio)

INSTALL(TARGETS pfpd DESTINATION bin)
INSTALL(TARGETS pfpc DESTINATION bin)

INSTALL(TARGETS pfp LIBRARY DESTINATION lib)
INSTALL(DIRECTORY share/pfp DESTINATION share)
