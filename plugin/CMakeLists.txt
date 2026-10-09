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
target_compile_definitions(TrumpetVST PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_ASIO=0)
target_link_libraries(TrumpetVST
  PUBLIC tpt_core juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(TPT_BUILD_TESTS)
  add_executable(tpt_plugin_tests ${CMAKE_SOURCE_DIR}/tests/plugin/test_plugin.cpp)
  target_link_libraries(tpt_plugin_tests PRIVATE TrumpetVST Catch2::Catch2)
  add_test(NAME plugin COMMAND tpt_plugin_tests)
  set_tests_properties(plugin PROPERTIES LABELS plugin)
endif()
