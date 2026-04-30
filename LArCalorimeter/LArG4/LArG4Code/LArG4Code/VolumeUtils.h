/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4CODE_VOLUMEUTILS_H
#define LARG4CODE_VOLUMEUTILS_H

/**
  @file VolumeUtils.h
  @brief Standalone functions for dealing with LAr G4 volumes
  @author Steve Farrell <Steven.Farrell@cern.ch>
 */

// System includes
#include <string>
#include <vector>
#include <set>

// Gaudi includes
#include "GaudiKernel/MsgStream.h"

namespace LArG4
{

  /** @brief Search for logical volumes in the G4 volume store.
   *
   *  Uses the global G4LogicalVolumeStore to search the geometry.
   *  The pattern may contain '*' wildcards.
   *
   *  @return a set of available volume names matching the wildcard pattern.
   */
  std::set<std::string> findLogicalVolumes(const std::string& pattern);


  /** @brief Search for multiple logical volumes in G4 volume store.
   *
   *  This function does a search for each input pattern in the
   *  G4LogicalVolumeStore using the findLogicalVolumes function above.
   *  It uses the provided MsgStream object to print a warning for every
   *  pattern that yields zero matches.
   *
   *  @return a list of available volume names matching the wildcard patterns.
   */
  std::vector<std::string>
  findLogicalVolumes(const std::vector<std::string>& patterns,
                     MsgStream& msg);

  //---------------------------------------------------------------------------
  /// @brief Helper function for matching strings with wildcards.
  /// It's a iterative function that checks if two given strings match.
  /// The first string may contain wildcard characters.
  //---------------------------------------------------------------------------
  bool matchStrings(std::string_view pattern, std::string_view text) noexcept;

}

#endif
