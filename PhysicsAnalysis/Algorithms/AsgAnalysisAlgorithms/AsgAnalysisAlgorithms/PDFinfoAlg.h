/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef ASG_ANALYSIS_ALGORITHMS__PDFINFO__ALG_H
#define ASG_ANALYSIS_ALGORITHMS__PDFINFO__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include "xAODTruth/TruthEventContainer.h"
#include <xAODEventInfo/EventInfo.h>

namespace CP {
class PDFinfoAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventsKey{
      this, "truthEvents", "TruthEvents", "the name of the truth events container"};
};

}  // namespace CP

#endif
