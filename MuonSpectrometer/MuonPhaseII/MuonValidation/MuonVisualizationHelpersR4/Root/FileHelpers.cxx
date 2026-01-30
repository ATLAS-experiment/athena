/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonVisualizationHelpersR4/FileHelpers.h"

#include "GeoModelKernel/throwExcept.h"
#include <filesystem>
#include <algorithm>
namespace MuonValR4{
     std::string removeNonAlphaNum(std::string str) {
        str.erase(std::remove_if(str.begin(),str.end(),
                  [](const unsigned char c){
                    return !std::isalnum(c);
                   }), str.end());
        return str; 
     }
    void ensureDirectory(const std::string& path) {
        const std::string dir = path.substr(0, path.rfind("/"));
        if (dir.rfind("/") != std::string::npos){
            ensureDirectory(dir);
        } else if (dir.empty()) {
            return;
        }
        if (!std::filesystem::is_directory(dir) && !std::filesystem::create_directory(dir)) {
            THROW_EXCEPTION("Failed to create "<<dir);
        }
    }
}