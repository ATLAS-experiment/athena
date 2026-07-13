/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPRIVXFINDER__HOUGHVTXFINDER_H
#define INDETPRIVXFINDER__HOUGHVTXFINDER_H

#include "ActsVertexReconstruction/HoughVtxFinderTool.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODTracking/VertexContainer.h"

#include <memory> // unique_ptr
#include <utility> // pair

class HoughVtxFinder : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  ~HoughVtxFinder() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<xAOD::SpacePointContainer> m_inputPixelSpacePoints{this, "inputPixelSpacePoints", "xAODInputPixelSpacePoints", "input pixel spacepoints"};
  SG::WriteHandleKey<xAOD::VertexContainer> m_outputHoughVtx{this, "outputHoughVtx", "HoughVertices", "output vertex container"};

  ToolHandle<ActsTrk::HoughVtxFinderTool> m_VertexFinderTool{this, "HoughVtxFinderTool", "", "Hough vertex finder tool"};
};

#endif  // INDETPRIVXFINDER__HOUGHVTXFINDER_H
