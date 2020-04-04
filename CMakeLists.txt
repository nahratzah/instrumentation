cmake_minimum_required(VERSION 3.8)
project (instrumentation)
enable_testing()

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(INSTRUMENTATION_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(INSTRUMENTATION_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(INSTRUMENTATION_VERSION ${INSTRUMENTATION_VERSION_MAJOR}.${INSTRUMENTATION_VERSION_MINOR} CACHE STRING "version" FORCE)

# On windows, DLLs must be either in the search path, or in the same directory as the executable.
# Since our test binaries are in a subdirectory, our tests fail under windows because they can't load the DLL.
# In order to sidestep that, we use the CMAKE_RUNTIME_OUTPUT_DIRECTORY to get all executables into the same directory.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set(headers
    include/instrumentation/fwd.h
    include/instrumentation/counter.h
    include/instrumentation/counter-inl.h
    include/instrumentation/engine.h
    include/instrumentation/gauge.h
    include/instrumentation/metric_name.h
    include/instrumentation/prometheus_engine.h
    include/instrumentation/string.h
    include/instrumentation/tags.h
    include/instrumentation/time_track.h
    include/instrumentation/timing.h
    include/instrumentation/collector.h
    )
set(headers_detail
    include/instrumentation/detail/metric_group.h
    )

include_directories (include)
add_library (instrumentation
    src/engine.cc
    src/gauge.cc
    src/metric_name.cc
    src/prometheus_engine.cc
    src/string.cc
    src/timing.cc
    src/collector.cc
    )
set_property (TARGET instrumentation PROPERTY VERSION ${INSTRUMENTATION_VERSION})
target_compile_features (instrumentation PUBLIC cxx_std_17)
set_target_properties (instrumentation PROPERTIES CXX_EXTENSIONS OFF)

find_package (Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
  target_link_libraries(instrumentation INTERFACE ${CMAKE_THREAD_LIBS_INIT})
endif()

target_include_directories(instrumentation PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(FILES ${headers} DESTINATION "include/instrumentation")
install(FILES ${headers_detail} DESTINATION "include/instrumentation/detail")
install(TARGETS instrumentation EXPORT instrumentation DESTINATION "lib")
install(EXPORT instrumentation DESTINATION "lib/cmake/instrumentation")

configure_file(instrumentation-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/instrumentation-config-version.cmake @ONLY)
install(FILES instrumentation-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/instrumentation-config-version.cmake DESTINATION "lib/cmake/instrumentation")

add_subdirectory (test)

find_package(Doxygen COMPONENTS mscgen OPTIONAL_COMPONENTS dot)

if(DOXYGEN_FOUND)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
  doxygen_add_docs(instrumentation-doc ${DOXYGEN_USE_MDFILE_AS_MAINPAGE} include doxygen)
endif()
