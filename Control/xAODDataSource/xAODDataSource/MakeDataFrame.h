// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODDATASOURCE_MAKEDATAFRAME_H
#define XAODDATASOURCE_MAKEDATAFRAME_H

// Framework include(s).
#include "xAODRootAccess/TEvent.h"

// ROOT include(s).
#include <ROOT/RDataFrame.hxx>

// System include(s).
#include <string>
#include <string_view>
#include <vector>

namespace xAOD {

/// Helper function for creating an xAOD reading data frame
///
/// @param fileNameGlob The glob pattern for the input file name(s)
/// @param containerName The name of the TTree/RNTuple to read from the input
///                      file(s)
/// @param verboseOutput If true, print out extra information during reading
/// @param auxmode The auxiliary access mode to use when reading xAOD objects
///
ROOT::RDataFrame MakeDataFrame(
    std::string_view fileNameGlob,
    std::string_view containerName = "CollectionTree",
    bool verboseOutput = false,
    TEvent::EAuxMode auxmode = TEvent::kClassAccess);

/// Helper function for creating an xAOD reading data frame
///
/// @param fileNames The list of input file names
/// @param containerName The name of the TTree/RNTuple to read from the input
///                      file(s)
/// @param verboseOutput If true, print out extra information during reading
/// @param auxmode The auxiliary access mode to use when reading xAOD objects
///
ROOT::RDataFrame MakeDataFrame(
    const std::vector<std::string>& fileNames,
    std::string_view containerName = "CollectionTree",
    bool verboseOutput = false,
    TEvent::EAuxMode auxmode = TEvent::kClassAccess);

}  // namespace xAOD

#endif  // XAODDATASOURCE_MAKEDATAFRAME_H
