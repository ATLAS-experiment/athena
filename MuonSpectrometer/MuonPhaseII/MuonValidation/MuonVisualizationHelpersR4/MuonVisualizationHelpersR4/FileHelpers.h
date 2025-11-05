/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONVISUALIZATIONHELPER_MUONFILEHELPERS_H
#define MUONR4_MUONVISUALIZATIONHELPER_MUONFILEHELPERS_H

#include <string>
namespace MuonValR4 {
    /** @brief Removes all non-alpha numerical characters from a string */
    std::string removeNonAlphaNum(std::string str);
    /** @brief Ensures that the subdirectory in the path is created
     *  @param path: Reference to the path from which the directory is to be craeted */
    void ensureDirectory(const std::string& path);
}
#endif