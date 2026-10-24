# SPDX-License-Identifier: Apache-2.0
add_executable(tpt_f0 f0.cpp)
target_link_libraries(tpt_f0 PRIVATE tpt_core)

add_executable(tpt_calibrate calibrate.cpp)
target_link_libraries(tpt_calibrate PRIVATE tpt_core nlohmann_json::nlohmann_json)
target_include_directories(tpt_calibrate PRIVATE ${CMAKE_SOURCE_DIR}/core/src)
