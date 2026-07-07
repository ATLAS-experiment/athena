/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSEXTRAPOLATIONALG_H
#define ACTSGEOMETRY_ACTSEXTRAPOLATIONALG_H

// ATHENA
#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "CxxUtils/checker_macros.h"

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometry/RecordedMaterialTrackCollection.h"

// ACTS
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "MuonTesterTree/MuonTesterTree.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/RNGWrapper.h"

// STL
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>


namespace Acts {
  class TrackingGeometry;
  namespace detail {
    struct Step;
  }
}


class EventContext;
class IAthRNGSvc;


namespace ActsTrk {
class ExtrapolationTestAlg : public AthHistogramAlgorithm {
public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx)  override;
  virtual StatusCode finalize() override;

private:
  using StepVector = std::vector<Acts::detail::Step>;
  StatusCode writePropagationSteps(const EventContext& ctx, const StepVector& steps);

  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "AthRNGSvc", "AthRNGSvc"};

  ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool", "ActsExtrapolationTool"};

  PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};

  // poor-mans Particle Gun is included here right now
  Gaudi::Property<std::vector<double>> m_etaRange{this, "EtaRange", {-3, 3}, "The eta range for particles"};
  Gaudi::Property<std::vector<double>> m_ptRange{this, "PtRange", {0.1, 1000}, "The pt range for particles"};
  Gaudi::Property<size_t> m_nParticlePerEvent{this, "NParticlesPerEvent", 10, "The number of particles per event"};

  // material track writer for the material map validation
  Gaudi::Property<bool> m_writeMaterialTracks{this, "WriteMaterialTracks", false, "Write material track"};
  Gaudi::Property<bool> m_writePropStep{this, "WritePropStep", false, "Write propagation step"};

  /// The RecordedMaterialTrackCollection to write
  SG::WriteHandleKey<ActsTrk::RecordedMaterialTrackCollection> m_materialTrackCollectionKey {this, "MaterialTrackCollectionKey", "MaterialTracks", "Name of the RecordedMaterialTrackCollection"};


  MuonVal::MuonTesterTree m_tree{"propsteps", "ActsExtrapolationRecord"};


  MuonVal::ScalarBranch<int>& m_eventNum{m_tree.newScalar<int>("event_nr")};
  /// @brief  Global x position of the step
  MuonVal::VectorBranch<float>& m_s_pX{m_tree.newVector<float>("step_x")};
  /// @brief  Global y position of the step  
  MuonVal::VectorBranch<float>& m_s_pY{m_tree.newVector<float>("step_y")};
  /// @brief  Global z position of the step  
  MuonVal::VectorBranch<float>& m_s_pZ{m_tree.newVector<float>("step_z")};
   /// @brief  Global radial position of the step sqrt(x^{2} + y^{2})
  MuonVal::VectorBranch<float>& m_s_pR{m_tree.newVector<float>("step_r")};

  MuonVal::VectorBranch<int>& m_s_volumeID{m_tree.newVector<int>("volume_id")};    ///< volume identification
  MuonVal::VectorBranch<int>& m_s_boundaryID{m_tree.newVector<int>("boundary_id")};    ///< boundary identification
  MuonVal::VectorBranch<int>& m_s_layerID{m_tree.newVector<int>("layer_id")};     ///< layer identification
  MuonVal::VectorBranch<int>& m_s_approachID{m_tree.newVector<int>("approach_id")};     ///< approach identification
  MuonVal::VectorBranch<int>& m_s_sensitiveID{m_tree.newVector<int>("sensitive_id")};   ///< sensitive identification

};
}
#endif // ActsGeometry_ActsExtrapolation_h
