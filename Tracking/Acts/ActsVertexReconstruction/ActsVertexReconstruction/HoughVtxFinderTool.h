/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSVERTEXRECONSTRUCTION_HOUGHVTXFINDERTOOL_H
#define ACTSVERTEXRECONSTRUCTION_HOUGHVTXFINDERTOOL_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/EventContext.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "InDetRecToolInterfaces/IVertexFinder.h"
#include "xAODInDetMeasurement/SpacePoint.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "ActsInterop/Logger.h"
#include "Acts/Definitions/Algebra.hpp"
#include "Acts/Vertexing/HoughVertexFinder.hpp"

#include <cmath>
#include <memory> // unique_ptr
#include <utility> // pair

namespace ActsTrk {

class HoughVtxFinderTool : public extends<AthAlgTool, InDet::IVertexFinder> {
public:
  virtual StatusCode initialize() override;

  using base_class::base_class;

  // no vertex finding with tracks
  virtual std::pair<xAOD::VertexContainer *, xAOD::VertexAuxContainer *>
  findVertex(const EventContext & /*ctx*/, const TrackCollection * /*trackTES*/) const override {
    ATH_MSG_ERROR("Can't call HoughVtxFinderTool::findVertex(ctx, trackTES)");
    return std::make_pair(nullptr, nullptr);
  }
  virtual std::pair<xAOD::VertexContainer *, xAOD::VertexAuxContainer *>
  findVertex(const EventContext & /*ctx*/, const xAOD::TrackParticleContainer * /*trackParticles*/) const override {
    ATH_MSG_ERROR("Can't call HoughVtxFinderTool::findVertex(ctx, trackParticles)");
    return std::make_pair(nullptr, nullptr);
  }

  // vertex finding with spacepoints
  std::pair<std::unique_ptr<xAOD::VertexContainer>, std::unique_ptr<xAOD::VertexAuxContainer>>
  findVertex(const EventContext &ctx, const xAOD::SpacePointContainer &spacePointContainer) const;

private:
  /// logging instance
  std::unique_ptr<const Acts::Logger> m_logger{nullptr};
  const Acts::Logger &logger() const { return *m_logger; }

  // spacepoint is required to have "x()", "y()", "z()", and "r()" methods
  struct SpacePoint {
    SpacePoint(const xAOD::SpacePoint *sp) : m_x(sp->x()), m_y(sp->y()), m_z(sp->z()) {}
    double x() const { return m_x; }
    double y() const { return m_y; }
    double z() const { return m_z; }
    double r() const { return std::sqrt(m_x * m_x + m_y * m_y); }
   private:
    double m_x, m_y, m_z;
  };

  using VertexFinder = Acts::HoughVertexFinder<SpacePoint>;
  VertexFinder::Config m_finderCfg;

  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

  UnsignedIntegerProperty m_minSPs{this, "minSPs", 100, "Minimum amount of spacepoints to attempt vertex finding"};

  // workaround for a bug in ACTS - defVtxPosition should be (0,0,0), until Athena uses ACTS version that includes PR #5060
  const bool m_useBeamSpot = false;
  // BooleanProperty m_useBeamSpot{this, "useBeamSpot", false, "Use beam spot XY positions as the default vertex position"};

  // Configuration variables
  // For details check ACTS documentation
  //
  UnsignedIntegerProperty m_targetSPs{this, "targetSPs", 20000, "Ideal amount of spacepoints"};
  DoubleProperty m_minAbsEta{this, "minAbsEta", 0.3, "Minimum range in |eta|"};
  DoubleProperty m_maxAbsEta{this, "maxAbsEta", 4.0, "Maximum range in |eta|"};
  UnsignedIntegerProperty m_minHits{this, "minHits", 4, "Minimum number of hits in Hough plane to consider the cell to contain a track"};
  UnsignedIntegerProperty m_fillNeighbours{this, "fillNeighbours", 0, "Number of neighbouring bins in Hough plane to fill"};
  // the two arrays below have to have the same size
  DoubleArrayProperty m_absEtaRanges{this, "absEtaRanges", {2.0, 4.0}, "Upper threshold for eta ranges"};
  DoubleArrayProperty m_absEtaFractions{this, "absEtaFractions", {0.4, 0.6}, "Amount of spacepoints in such eta ranges"};
  // the three arrays below have to have the same size
  DoubleArrayProperty m_rangeIterZ{this, "rangeIterZ", {200.0, 30.0, 16.0}, "Maximum vertex range to consider"};
  UnsignedIntegerArrayProperty m_nBinsZIterZ{this, "nBinsZIterZ", {800, 180, 80}, "Number of bins in Z direction"};
  UnsignedIntegerArrayProperty m_nBinsCotThetaIterZ{this, "nBinsCotThetaIterZ", {8000, 8000, 8000}, "Number of bins in cot(theta) direction"};
  DoubleProperty m_binsCotThetaDecrease{this, "binsCotThetaDecrease", 1.35, "For every magnitude (in natural log) below targetSPs, the number of bins in cot(theta) will decrease by this factor"};
  UnsignedIntegerProperty m_peakWidth{this, "peakWidth", 3, "Width of the peak when estimating vertex position"};
  DoubleArrayProperty m_defVtxPosition{this, "defVtxPosition", {0.0, 0.0, 0.0}, "Default position of the vertex, might be overwritten by beamspot XY position"};
};

} // namespace ActsTrk

#endif // ACTSVERTEXRECONSTRUCTION_HOUGHVTXFINDERTOOL_H
