/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGPUEVENTCNV_CLUSTERVALIDATIONALG_H
#define ACTSGPUEVENTCNV_CLUSTERVALIDATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "StoreGate/ReadHandleKey.h"

namespace ActsTrk {

/*! Algo that compares two sets of xAOD clusters
 *
 * Its main usage is to validate that different Traccc measurement conversion
 * produce the same set of cluster containers.
 *
 * Order of measurements matter so the algorithm expect to find the same
 * data at the same position.
 */

class ClusterValidationAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_referencePixelsKey{
    this, "ReferencePixels", "", "The reference Pixel clusters"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_referenceStripsKey{
    this, "ReferenceStrips", "", "The reference Strip clusters"};
  SG::ReadHandleKey<xAOD::PixelClusterContainer> m_monitoredPixelsKey{
    this, "MonitoredPixels", "", "The monitored Pixel clusters"};
  SG::ReadHandleKey<xAOD::StripClusterContainer> m_monitoredStripsKey{
    this, "MonitoredStrips", "", "The monitored Strip clusters"};
};

} // namespace ActsTrk

#endif // ACTSGPUEVENTCNV_CLUSTERVALIDATIONALG_H