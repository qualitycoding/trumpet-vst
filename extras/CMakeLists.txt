# SPDX-License-Identifier: Apache-2.0
# Non-frozen implementer tests (PLAN.md Conventions "Extra tests"). Never under tests/ (frozen, diffed).
add_subdirectory(core)
if(TPT_BUILD_PLUGIN AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/plugin/CMakeLists.txt)
  add_subdirectory(plugin)
endif()
