/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_COLLECTIONALGS_SPACEPOINTS_H
#define ACTS_COLLECTIONALGS_SPACEPOINTS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

// EDM
#include "xAODInDetMeasurement/SpacePointContainer.h"

namespace ActsTrk {

class SpacePointReader
  : public AthReentrantAlgorithm {
 public:
  SpacePointReader(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SpacePointReader() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override;

 private:
  SG::ReadHandleKey< xAOD::SpacePointContainer > m_spacePointKey
  {this, "SpacePointKey", "",
      "Key for input space point container"};

  SG::WriteDecorHandleKey< xAOD::SpacePointContainer > m_clusterDecoration
    {this, "ClusterDecorationKey", "measurements",
    "Decoration key for the cluster bare pointer"};
};

} // namespace ActsTrk

#endif
