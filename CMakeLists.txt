# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.22)
project(TrumpetVST VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TPT_BUILD_TESTS  "Build the frozen test suite"              ON)
option(TPT_BUILD_PLUGIN "Build the JUCE VST3/AU/Standalone target" ON)

include(FetchContent)
# All third-party sources are pinned to full 40-hex commit SHAs (checked by T-031).
FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        317ac1ed4c0bb6e6b91eafc817e05c488feffcb3 # v3.16.0
  GIT_SHALLOW    FALSE)
FetchContent_Declare(nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        55f93686c01528224f448c19128836e7df245f72 # v3.12.0
  GIT_SHALLOW    FALSE)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        be29c81492b6151c8ea8d14c840e1311963b3a83 # 9.0.3
  GIT_SHALLOW    FALSE)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)
add_subdirectory(core)

if(TPT_BUILD_TESTS)
  FetchContent_MakeAvailable(Catch2)
  enable_testing()
  add_subdirectory(tests)
endif()

if(TPT_BUILD_PLUGIN)
  FetchContent_MakeAvailable(JUCE)
  add_subdirectory(plugin)
endif()
