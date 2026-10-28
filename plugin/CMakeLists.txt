# SPDX-License-Identifier: Apache-2.0
set(TPT_PLUGIN_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND TPT_PLUGIN_FORMATS AU)
endif()

juce_add_plugin(TrumpetVST
  COMPANY_NAME "qualitycoding"
  PLUGIN_MANUFACTURER_CODE Qcod
  PLUGIN_CODE Tpts
  FORMATS ${TPT_PLUGIN_FORMATS}
  PRODUCT_NAME "Trumpet"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Synth)

target_sources(TrumpetVST PRIVATE src/PluginProcessor.cpp src/PluginEditor.cpp src/TrumpetView.cpp)
target_include_directories(TrumpetVST PUBLIC src)
# Short git sha shown in the editor (D-025); -DTPT_GIT_SHA_OVERRIDE=<sha7> wins (release-candidate job).
if(TPT_GIT_SHA_OVERRIDE)
  set(TPT_GIT_SHA "${TPT_GIT_SHA_OVERRIDE}")
else()
  execute_process(COMMAND git rev-parse --short=7 HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE TPT_GIT_SHA OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE TPT_GIT_RC)
  if(NOT TPT_GIT_RC EQUAL 0 OR TPT_GIT_SHA STREQUAL "")
    set(TPT_GIT_SHA "unknown")
  endif()
endif()
target_compile_definitions(TrumpetVST PUBLIC TPT_GIT_SHA="${TPT_GIT_SHA}")
target_compile_definitions(TrumpetVST PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_ASIO=0)
target_link_libraries(TrumpetVST
  PUBLIC tpt_core nlohmann_json::nlohmann_json juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(TPT_BUILD_TESTS)
  add_executable(tpt_plugin_tests ${CMAKE_SOURCE_DIR}/tests/plugin/test_plugin.cpp)
  target_link_libraries(tpt_plugin_tests PRIVATE TrumpetVST Catch2::Catch2)
  add_test(NAME plugin COMMAND tpt_plugin_tests)
  set_tests_properties(plugin PROPERTIES LABELS plugin)
endif()

# G-004 editor snapshots (S-014); run under xvfb-run on Linux.
add_executable(tpt_ui_snapshots ${CMAKE_SOURCE_DIR}/tools/render/snapshot_ui.cpp)
target_link_libraries(tpt_ui_snapshots PRIVATE TrumpetVST)
