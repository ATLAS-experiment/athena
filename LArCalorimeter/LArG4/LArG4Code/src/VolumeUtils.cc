/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4Code/VolumeUtils.h"

// STL includes
#include <set>

// Geant4 includes
#include "G4LogicalVolumeStore.hh"


namespace LArG4
{

  //---------------------------------------------------------------------------
  /// @brief Helper function for matching strings with wildcards.
  /// It's a iterative function that checks if two given strings match.
  /// The first string may contain wildcard characters.
  //---------------------------------------------------------------------------
  bool matchStrings(std::string_view pattern, std::string_view text) noexcept {
      size_t n = text.size(), m = pattern.size();
      size_t i = 0, j = 0, startIndex = std::string_view::npos, match = 0;
  
      while (i < n) {
          if (j < m && pattern[j] == '*') {
              startIndex = j++;
              match = i;
          } else if (j < m && (pattern[j] == text[i])) {
              i++;
              j++;
          } else if (startIndex != std::string_view::npos) {
              j = startIndex + 1;
              i = ++match;
          } else {
              return false;
          }
      }
  
      while (j < m && pattern[j] == '*') j++;
      return j == m;
  }

  //---------------------------------------------------------------------------
  // Search for logical volumes in the G4 volume store
  //---------------------------------------------------------------------------
  std::set<std::string> findLogicalVolumes(const std::string& pattern)
  {
    // Use a set because there can be multiple occurences with same name
    std::set<std::string> foundVolumes;

    // Iterate over the G4 volumes and look for matches
    auto *logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
    for(auto *logvol : *logicalVolumeStore) {
      auto name = logvol->GetName();
      if( matchStrings( pattern, name) ) {
        foundVolumes.emplace( std::move(name) );
      }
    }

    return foundVolumes;
  }

  //---------------------------------------------------------------------------
  // Search for multiple logical volumes in the G4 volume store
  //---------------------------------------------------------------------------
  std::vector<std::string>
  findLogicalVolumes(const std::vector<std::string>& patterns,
                     MsgStream& msg)
  {
    std::set<std::string> parsedVolumes;
    for(const auto& pattern : patterns) {
      const auto patternVols = findLogicalVolumes(pattern);
      if(patternVols.empty()) {
        msg << MSG::WARNING << "No volume found matching pattern: "
            << pattern << endmsg;
      }
      parsedVolumes.insert( patternVols.begin(), patternVols.end() );
    }
    return std::vector<std::string>( parsedVolumes.begin(), parsedVolumes.end() );
  }

}
