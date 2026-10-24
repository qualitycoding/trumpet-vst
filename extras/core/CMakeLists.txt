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
tpt_extra_test(tpt_noise_power test_noise_power.cpp)

# Not a test: prints the real-time factor (S-008 action 4).
add_executable(tpt_perf_probe perf_probe.cpp)
target_link_libraries(tpt_perf_probe PRIVATE tpt_core)
target_include_directories(tpt_perf_probe PRIVATE ${CMAKE_SOURCE_DIR}/tests/core)
target_compile_definitions(tpt_perf_probe PRIVATE TPT_FIXTURE_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures")
