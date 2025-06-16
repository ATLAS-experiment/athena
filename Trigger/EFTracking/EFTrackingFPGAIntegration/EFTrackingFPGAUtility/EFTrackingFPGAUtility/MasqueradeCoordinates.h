/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MASQUERADE_COORDINATES_H
#define MASQUERADE_COORDINATES_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

/* 
 *  The pathfinder NN has been trained on cartesian data but EFTracking 
 *  coordinates are cyclindrical. To avoid delaying integration into athena we 
 *  simply hide the x, y, z coordinates in the r, phi, z fields of the hit 
 *  words. This is a temporary hack. Once the NN has been retrained and updated 
 *  for cyclindical coordinates this Algorithm can be removed.
 */
class MasqueradeCoordinates : public AthReentrantAlgorithm {
  SG::ReadHandleKey<std::vector<unsigned long>> m_cylindricalDataStreamKey{
    this,
    "cylindricalDataStream",
    "cylindricalDataStream",
    ""
  };

  SG::WriteHandleKey<std::vector<unsigned long>> m_cartesianDataStreamKey{
    this,
    "cartesianDataStream",
    "cartesianDataStream",
    ""
  };

 public:
  MasqueradeCoordinates(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif 

