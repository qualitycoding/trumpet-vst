# SPDX-License-Identifier: Apache-2.0
# Embeds data/trumpet_resonators.json into tpt_core at configure time (D-004). The JSON is registered as a configure
# dependency, so `cmake --build` re-configures and re-embeds after every change.
function(tpt_embed_resonator_table target json)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${json}")
  file(READ "${json}" hex HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
  set(out "${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp")
  file(WRITE "${out}.tmp"
"// Generated from data/trumpet_resonators.json by core/embed_table.cmake. Do not edit.
namespace tpt {
namespace {
const unsigned char kTable[] = {${bytes}0x00};
}
const char* embeddedResonatorJson() { return reinterpret_cast<const char*>(kTable); }
} // namespace tpt
")
  # keep the timestamp when unchanged
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${out}.tmp" "${out}")
  target_sources(${target} PRIVATE "${out}")
endfunction()
