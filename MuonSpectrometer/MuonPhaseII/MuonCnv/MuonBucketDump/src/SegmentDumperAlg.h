/*
   Copyright (C) 2002-2025 CERN
   for the benefit of the ATLAS collaboration
*/
#ifndef MUONBUCKETDUMP_SegmentDumperAlg_H
#define MUONBUCKETDUMP_SegmentDumperAlg_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

#include <MuonSpacePoint/SpacePointContainer.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <ActsGeometryInterfaces/GeometryContext.h>

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/ThreeVectorBranch.h"

namespace MuonR4 {

class SegmentDumperAlg : public AthHistogramAlgorithm {
public:
  using AthHistogramAlgorithm::AthHistogramAlgorithm;
  ~SegmentDumperAlg() override = default;

  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) override final;
  StatusCode finalize() override final;

private:
  /// Inputs
  SG::ReadHandleKeyArray<SpacePointContainer> m_spacePointKeys{
      this, "SpacePointKeys", {"MuonSpacePoints"},
      "Keys to the SpacePoint containers"};

  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKeys{
      this, "SegmentKeys", "MuonSegmentsFromR4",
      "Keys to the reconstructed segment containers (aligned by index with SpacePointKeys)"};

  /// Truth decoration (MC)
  SG::ReadDecorHandleKeyArray<xAOD::MuonSegmentContainer> m_truthDecorKeys{
      this, "TruthDecorLinks", {}, "truthParticleLink decoration on segments"};

  /// Allow the dumper to label otherwise-unmatched reco segments from sim-hit G4/HepMC ids.
  Gaudi::Property<bool> m_includeG4TrackTruth{
      this, "IncludeG4TrackTruth", false,
      "Use sim-hit HepMC/G4 track identifiers for reco segment truth labels"};

  /// Minimum number of reco segments that must share the same G4/HepMC id 
  Gaudi::Property<unsigned int> m_minG4TrackTruthSegments{
      this, "MinG4TrackTruthSegments", 2,
      "Require a G4/HepMC track id to appear in at least this many reco segments"};

  ActsTrk::GeoContextReadKey_t m_geoCtxKey{
      this, "AlignmentKey", "ActsAlignment", "Geometry alignment context"};

  /// Output tree
  MuonVal::MuonTesterTree m_tree{"MuonSegmentDump", "MuonSegmentDump"};

  // Per-bucket scalars
  MuonVal::ScalarBranch<uint16_t>& m_bucket_spacePoints{
      m_tree.newScalar<uint16_t>("bucket_spacePoints", 0)};
  MuonVal::ScalarBranch<uint16_t>& m_bucket_layers{
      m_tree.newScalar<uint16_t>("bucket_layers", 0)};
  MuonVal::ScalarBranch<uint8_t>& m_bucket_chamberIdx{
      m_tree.newScalar<uint8_t>("bucket_chamberIndex", 0)};
  MuonVal::ScalarBranch<uint8_t>& m_bucket_sector{
      m_tree.newScalar<uint8_t>("bucket_sector", 0)};
  MuonVal::ScalarBranch<uint16_t>& m_bucket_segments{
      m_tree.newScalar<uint16_t>("bucket_segments", 0)};

  // Per-segment (reco) vectors
  MuonVal::ThreeVectorBranch m_segmentPos{m_tree, "segmentPosition"};
  MuonVal::ThreeVectorBranch m_segmentDir{m_tree, "segmentDirection"};
  MuonVal::VectorBranch<float>& m_segment_chiSquared{
      m_tree.newVector<float>("segment_chiSquared")};
  MuonVal::VectorBranch<float>& m_segment_numberDoF{
      m_tree.newVector<float>("segment_numberDoF")};

  // NEW: reco eta/phi derived from the segment direction (always filled)
  MuonVal::VectorBranch<float>& m_segmentRecoEta{
      m_tree.newVector<float>("segmentEta")};
  MuonVal::VectorBranch<float>& m_segmentRecoPhi{
      m_tree.newVector<float>("segmentPhi")};

  // Truth labels & kinematics (one entry per dumped segment; -1/NaN if unmatched)
  MuonVal::VectorBranch<int32_t>& m_segmentTruthIdx{
      m_tree.newVector<int32_t>("segmentTruthPart")};
  MuonVal::VectorBranch<int32_t>& m_segmentTruthSource{
      m_tree.newVector<int32_t>("segmentTruthSource")};
  MuonVal::VectorBranch<uint8_t>& m_segmentTruthAmbiguous{
      m_tree.newVector<uint8_t>("segmentTruthAmbiguous")};
  MuonVal::VectorBranch<int32_t>& m_segmentTruthAltPartOffsets{
      m_tree.newVector<int32_t>("segmentTruthAltPartOffsets")};
  MuonVal::VectorBranch<int32_t>& m_segmentTruthAltParts{
      m_tree.newVector<int32_t>("segmentTruthAltParts")};
  MuonVal::VectorBranch<float>& m_segmentTruthLabelWeight{
      m_tree.newVector<float>("segmentTruthLabelWeight")};
  MuonVal::VectorBranch<int32_t>& m_segmentTruthPDGId{
      m_tree.newVector<int32_t>("segmentTruthPDGId")};
  MuonVal::VectorBranch<float>& m_segmentTruthPt{
      m_tree.newVector<float>("segmentTruthPt")};
  MuonVal::VectorBranch<float>& m_segmentTruthEta{
      m_tree.newVector<float>("segmentTruthEta")};
  MuonVal::VectorBranch<float>& m_segmentTruthPhi{
      m_tree.newVector<float>("segmentTruthPhi")};
  MuonVal::VectorBranch<uint8_t>& m_segmentHasTruth{
      m_tree.newVector<uint8_t>("segmentHasTruth")};

  // Helper: for a given SP bucket, count unique layer numbers
  uint16_t countLayersInBucket(const SpacePointBucket& bucket) const;
};

} // namespace MuonR4

#endif
