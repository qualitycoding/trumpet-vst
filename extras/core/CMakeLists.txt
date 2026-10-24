# SPDX-License-Identifier: Apache-2.0
function(tpt_extra_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE tpt_core Catch2::Catch2WithMain nlohmann_json::nlohmann_json)
  target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/tests/core ${CMAKE_SOURCE_DIR}/core/src)
  target_compile_definitions(${name} PRIVATE TPT_FIXTURE_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures" _USE_MATH_DEFINES)
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES LABELS "extra")
endfunction()

tpt_extra_test(tpt_support_selftest test_support_selftest.cpp)
