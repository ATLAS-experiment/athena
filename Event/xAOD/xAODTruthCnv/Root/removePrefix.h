// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODTRUTHCNV_REMOVEPREFIX_H
#define XAODTRUTHCNV_REMOVEPREFIX_H

// System include(s).
#include <string>
#include <string_view>

namespace xAODMaker::Details {

/// Remove a prefix from a string, if it exists
///
/// @param str The string to process
/// @param prefix The prefix to remove
/// @returns The string without the prefix, or the original string if the prefix
///          was not found
///
std::string removePrefix(std::string_view str, std::string_view prefix);

}  // namespace xAODMaker::Details

#endif  // XAODTRUTHCNV_REMOVEPREFIX_H
