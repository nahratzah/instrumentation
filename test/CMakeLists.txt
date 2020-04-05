include(CTest)
find_package(UnitTest++)

macro (do_test binary)
  add_executable (test_${binary} ${binary}.cc)
  target_link_libraries (test_${binary} instrumentation)
  target_link_libraries (test_${binary} UnitTest++ test_support)
  target_compile_features (test_${binary} PUBLIC cxx_std_17)
  set_target_properties (test_${binary} PROPERTIES CXX_EXTENSIONS OFF)

  add_test (NAME ${binary} COMMAND $<TARGET_FILE:test_${binary}>)
endmacro (do_test)

if (UnitTest++_FOUND)
  include_directories(${UTPP_INCLUDE_DIRS})

  add_library (test_support STATIC test_collector.cc)
  target_link_libraries (test_support instrumentation)
  target_compile_features (test_support PUBLIC cxx_std_17)
  set_target_properties (test_support PROPERTIES CXX_EXTENSIONS OFF)

  do_test (counter)
  do_test (gauge)
  do_test (string)
  do_test (timing)
endif ()
