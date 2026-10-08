# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
set(TPT_FIXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

function(tpt_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE tpt_core Catch2::Catch2WithMain nlohmann_json::nlohmann_json)
  target_include_directories(${name} PRIVATE core)
  target_compile_definitions(${name} PRIVATE TPT_FIXTURE_DIR="${TPT_FIXTURE_DIR}" _USE_MATH_DEFINES)
endfunction()

tpt_test(tpt_unit_tests
  core/test_pitch.cpp core/test_valves.cpp core/test_fingering.cpp core/test_resonator_table.cpp
  core/test_lip_model.cpp core/test_analysis.cpp core/test_state.cpp core/test_layout.cpp)
tpt_test(tpt_integration_tests
  core/test_reference_sim.cpp core/test_voice_pitch.cpp core/test_voice_overblow.cpp core/test_voice_midi_ui.cpp
  core/test_voice_timbre.cpp)
tpt_test(tpt_operational_tests core/test_voice_robustness.cpp)
tpt_test(tpt_alloc_tests core/test_alloc.cpp)
tpt_test(tpt_perf_tests core/test_perf.cpp)

add_test(NAME unit        COMMAND tpt_unit_tests)
add_test(NAME integration COMMAND tpt_integration_tests)
add_test(NAME operational COMMAND tpt_operational_tests)
add_test(NAME alloc       COMMAND tpt_alloc_tests)
add_test(NAME perf        COMMAND tpt_perf_tests)
set_tests_properties(perf PROPERTIES LABELS "perf")
