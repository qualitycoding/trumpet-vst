# SPDX-License-Identifier: Apache-2.0
add_library(tpt_core STATIC
  src/Pitch.cpp src/Valves.cpp src/Fingering.cpp src/ResonatorTable.cpp src/LipModel.cpp
  src/ReferenceSim.cpp src/Analysis.cpp src/TrumpetVoice.cpp src/Layout.cpp src/State.cpp)
target_include_directories(tpt_core PUBLIC include)
target_compile_features(tpt_core PUBLIC cxx_std_20)
target_link_libraries(tpt_core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(tpt_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
  target_compile_options(tpt_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(tpt_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()
