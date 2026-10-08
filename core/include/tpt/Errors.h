// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdexcept>
#include <string>

namespace tpt {

/// Thrown by every planning stub until the implementing step replaces it (D-018).
struct NotImplemented : std::logic_error {
    explicit NotImplemented(const std::string& what) : std::logic_error("not implemented: " + what) {}
};

/// Thrown by parsers (resonator table) on malformed or out-of-limit input. Never thrown on the audio thread.
struct ParseError : std::runtime_error {
    explicit ParseError(const std::string& what) : std::runtime_error(what) {}
};

} // namespace tpt
