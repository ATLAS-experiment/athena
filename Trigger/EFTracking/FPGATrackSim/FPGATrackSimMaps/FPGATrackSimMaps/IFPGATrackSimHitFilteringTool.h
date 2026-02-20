// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef IFPGATrackSimHITFILTERINGTOOL_H
#define IFPGATrackSimHITFILTERINGTOOL_H

/**
 * @file IFPGATrackSimHitFilteringTool.h
 * @author Will Kalderon - willam.kalderon@cern.ch, Julian Wollrath - wollrath@cern.ch
 * @date 2021
 * @brief Declares an abstract class that implements an interface for hit/cluster filtering.
 * This class is implemented in
 *      FPGATrackSimHitFilteringTool.h
 */

#include "GaudiKernel/IAlgTool.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimPlaneMap.h"

class IFPGATrackSimHitFilteringTool : virtual public ::IAlgTool {
 public:
  DeclareInterfaceID(IFPGATrackSimHitFilteringTool, 1, 0);
  virtual ~IFPGATrackSimHitFilteringTool() = default;
  
  virtual StatusCode DoRandomRemoval(FPGATrackSimLogicalEventInputHeader &, bool) const = 0;
  virtual StatusCode GetPairedStripPhysLayers(const FPGATrackSimPlaneMap*, std::vector<int> &) = 0;
  virtual StatusCode DoHitFiltering(FPGATrackSimLogicalEventInputHeader &,
                                    std::vector<int>, std::vector<int>,
                                    std::vector<FPGATrackSimCluster> &) = 0;

};

#endif // IFPGATrackSimHITFILTERINGTOOL_H
