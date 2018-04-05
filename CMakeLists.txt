cmake_minimum_required(VERSION 3.3)
project (instrumentation)

set(INSTRUMENTATION_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(INSTRUMENTATION_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(INSTRUMENTATION_VERSION ${INSTRUMENTATION_VERSION_MAJOR}.${INSTRUMENTATION_VERSION_MINOR} CACHE STRING "version" FORCE)

enable_testing()

include (CheckCXXCompilerFlag)
check_cxx_compiler_flag ("-std=c++17" STD_CXX17)
check_cxx_compiler_flag ("-std=c++1z" STD_CXX1Z)
if(STD_CXX17)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(STD_CXX1Z)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
else()
  message(WARNING "Cannot figure out how to enable C++17 on compiler.")
endif()

set(headers
    include/instrumentation/group.h
    )
set(headers_detail
    )

add_library (instrumentation INTERFACE)

find_package (Boost COMPONENTS context REQUIRED)
target_link_libraries (instrumentation INTERFACE ${Boost_CONTEXT_LIBRARY})
find_package (Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
  target_link_libraries(instrumentation INTERFACE ${CMAKE_THREAD_LIBS_INIT})
endif()

target_include_directories(instrumentation INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${Boost_INCLUDE_DIR})

install(FILES ${headers} DESTINATION "include/instrumentation")
install(FILES ${headers_detail} DESTINATION "include/instrumentation/detail")
install(TARGETS instrumentation EXPORT instrumentation DESTINATION "lib")
install(EXPORT instrumentation DESTINATION "lib/cmake/instrumentation")

configure_file(instrumentation-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/instrumentation-config-version.cmake @ONLY)
install(FILES instrumentation-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/instrumentation-config-version.cmake DESTINATION "lib/cmake/instrumentation")

add_subdirectory (test)

find_package(Doxygen REQUIRED mscgen OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
  set(DOXYGEN_USE_MATHJAX "YES")
  set(DOXYGEN_EXTRA_PACKAGES "amsmath")
  set(DOXYGEN_MATHJAX_EXTENSIONS "TeX/AMSmath;TeX/AMSsymbols")
  set(DOXYGEN_MATHJAX_RELPATH "https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.1/")
  doxygen_add_docs(instrumentation-doc ${DOXYGEN_USE_MDFILE_AS_MAINPAGE} include doxygen)
endif()
