# SPDX-License-Identifier: Apache-2.0
# Non-frozen plugin variant tests (S-013); only configured when the plugin is built.
add_executable(tpt_plugin_variant_tests test_plugin_variants.cpp)
target_link_libraries(tpt_plugin_variant_tests PRIVATE TrumpetVST Catch2::Catch2)
add_test(NAME tpt_plugin_variant_tests COMMAND tpt_plugin_variant_tests)
set_tests_properties(tpt_plugin_variant_tests PROPERTIES LABELS "extra;plugin")
