if(USE_EXTERNAL_CATCH2)
  find_package(Catch2 REQUIRED)
else()
  message(STATUS "Fetching local copy of Catch2 library for unit-tests...")
  # Build Catch2 with the default flags, to avoid generating warnings when we
  # build it
  set(CXX_FLAGS_CMAKE_USED ${CMAKE_CXX_FLAGS})
  set(CMAKE_CXX_FLAGS ${CXX_FLAGS_CMAKE_DEFAULTS})
  Include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        037ddbc75cc5e58b93cf5a010a94b32333ad824d
    )
  FetchContent_MakeAvailable(Catch2)
  set(CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras ${CMAKE_MODULE_PATH})


  # Hack around the fact, that the include directories are not declared as
  # SYSTEM for the targets defined this way. Otherwise warnings can still occur
  # in Catch2 code when templates are evaluated (which happens quite a bit)
  get_target_property(CATCH2_IF_INC_DIRS Catch2 INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(Catch2 PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CATCH2_IF_INC_DIRS}")

  # Reset the flags
  set(CMAKE_CXX_FLAGS ${CXX_FLAGS_CMAKE_USED})
endif()

add_executable(unittests unittest.cpp)
target_link_libraries(unittests PUBLIC TestDataModel PRIVATE Catch2::Catch2WithMain)
if(NOT USE_SANITIZER MATCHES "Memory(WithOrigin)?")
  # This can fail with memory sanitizer because it actually runs an executable.
  # Hence, we enable this only for non memory-sanitizer runs
  include(Catch)
  catch_discover_tests(unittests
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    TEST_PREFIX "UT_" # make it possible to filter easily with -R ^UT
    )
else()
  add_test(NAME unittests COMMAND unittests)
  set_property(TEST unittests
    PROPERTY ENVIRONMENT
    LD_LIBRARY_PATH=$<TARGET_FILE_DIR:TestDataModel>:$<TARGET_FILE_DIR:podio::podio>:$ENV{LD_LIBRARY_PATH}
    )
endif()
