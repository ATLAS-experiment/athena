/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "BeamBackgroundFiller.h"

#include <cmath>

#include "AthenaKernel/errorcheck.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODJet/JetConstituentVector.h"

namespace {
constexpr float const& myConst = 1e-3 / 3e8 / 1e-9;
}

//------------------------------------------------------------------------------
StatusCode BeamBackgroundFiller::initialize() {
  CHECK(m_edmHelperSvc.retrieve());
  CHECK(m_idHelperSvc.retrieve());

  ATH_CHECK(m_nswSegmentContainerReadHandleKey.initialize(!m_nswSegmentContainerReadHandleKey.empty())); 
  ATH_CHECK(m_cscSegmentContainerReadHandleKey.initialize(!m_cscSegmentContainerReadHandleKey.empty())); 
  ATH_CHECK(m_mdtSegmentContainerReadHandleKey.initialize());
  ATH_CHECK(m_caloClusterContainerReadHandleKey.initialize());
  ATH_CHECK(m_jetContainerReadHandleKey.initialize());

  ATH_CHECK(m_beamBackgroundDataWriteHandleKey.initialize());
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode BeamBackgroundFiller::execute(const EventContext& ctx) const {

  Cache cache{};
  // find muon segments from beam background muon candidates and match them with
  // calorimeter clusters
  ATH_CHECK(FillMatchMatrix(ctx, cache));
  // apply Beam Background Identifiaction Methods
  SegmentMethod(cache);
  NSWMDTMatching(ctx, cache);
  OneSidedMethod(cache);
  TwoSidedMethod(cache);
  ClusterShapeMethod(cache);
  // identify fake jets
  FindFakeJets(ctx, cache);

  // fill the results into BeamBackgroundData
  SG::WriteHandle<BeamBackgroundData> beamBackgroundDataWriteHandle(
      m_beamBackgroundDataWriteHandleKey, ctx);
  ATH_CHECK(beamBackgroundDataWriteHandle.record(
      std::make_unique<BeamBackgroundData>()));
  FillBeamBackgroundData(beamBackgroundDataWriteHandle, cache);

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
/**
 * This function selects the muon segments with the direction parallel to the
 * beam pipe and calorimeter clusters above certain energy threshold. Matching
 * matrix is created to store the results of beam background identification for
 * each cluster and segment
 */
StatusCode BeamBackgroundFiller::FillMatchMatrix(const EventContext& ctx,
                                           Cache& cache) const {
  //
  cache.m_numMatched = 0;
  cache.m_indexSeg.clear();
  cache.m_resultSeg.clear();
  cache.m_indexClus.clear();
  cache.m_matchMatrix.clear();
  cache.m_resultClus.clear();

 
  if (!m_nswSegmentContainerReadHandleKey.empty()) {
    // select only NSW segments with the global direction parallel to the beam pipe
    SG::ReadHandle<Trk::SegmentCollection> nswSegmentReadHandle(m_nswSegmentContainerReadHandleKey, ctx);

    if (!nswSegmentReadHandle.isValid()) {
      ATH_MSG_ERROR("Could not load the " << m_nswSegmentContainerReadHandleKey.key() << " segment container");
      return StatusCode::FAILURE;
    }

    unsigned int nswSegmentCounter = 0;
    for (const auto *thisNSWSegment : *nswSegmentReadHandle) {

      nswSegmentCounter++;
      const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(thisNSWSegment);

      Identifier id = m_edmHelperSvc->chamberId(*seg);
      if (!id.is_valid() || !m_idHelperSvc->isMuon(id)) {
        continue;
      }
      if (!m_idHelperSvc->isMM(id) && !m_idHelperSvc->issTgc(id)) {
        continue;
      }

      const Amg::Vector3D& globalDir = seg->globalDirection();
      double thetaDir = globalDir.theta();

      if ((std::abs(thetaDir)) > m_cutThetaNsw) {
        continue;
      }

      ElementLink<Trk::SegmentCollection> segLink;
      segLink.toIndexedElement(*nswSegmentReadHandle, nswSegmentCounter - 1);

      if (!segLink.isValid()) {
        ATH_MSG_INFO("Failed to create valid ElementLink for NSW segment index " << (nswSegmentCounter - 1));
        continue;
      }

      cache.m_indexSeg.push_back(segLink);
    }
  } 

  if (m_idHelperSvc->hasCSC()) {
    // select only the CSC segments with the global direction parallel to the
    // beam pipe
    SG::ReadHandle<Trk::SegmentCollection> cscSegmentReadHandle(
        m_cscSegmentContainerReadHandleKey, ctx);

    if (!cscSegmentReadHandle.isValid()) {
      ATH_MSG_WARNING("Invalid ReadHandle to Trk::SegmentCollection with name: "
                      << m_cscSegmentContainerReadHandleKey);
    } else {
      ATH_MSG_DEBUG(m_cscSegmentContainerReadHandleKey
                    << " retrieved from StoreGate");

      unsigned int cscSegmentCounter = 0;
      for (const auto *thisCSCSegment : *cscSegmentReadHandle) {
        cscSegmentCounter++;
        const Muon::MuonSegment* seg =
            dynamic_cast<const Muon::MuonSegment*>(thisCSCSegment);

        if (!seg)
          std::abort();

        Identifier id = m_edmHelperSvc->chamberId(*seg);
        if (!id.is_valid())
          continue;
        if (!m_idHelperSvc->isMuon(id))
          continue;

        if (!m_idHelperSvc->isCsc(id))
          continue;

        const Amg::Vector3D& globalPos = seg->globalPosition();
        const Amg::Vector3D& globalDir = seg->globalDirection();
        double thetaPos = globalPos.theta();
        double thetaDir = globalDir.theta();

        
        if (std::cos(2. * (thetaPos - thetaDir)) >
            std::cos(2. * m_cutThetaCsc))
          continue;

        ElementLink<Trk::SegmentCollection> segLink;
        segLink.toIndexedElement(*cscSegmentReadHandle, cscSegmentCounter - 1);
        cache.m_indexSeg.push_back(segLink);
      }
    }
  }

  // select only the MDT segments with the global direction parallel to the beam
  // pipe
  SG::ReadHandle<Trk::SegmentCollection> mdtSegmentReadHandle(
      m_mdtSegmentContainerReadHandleKey,ctx);

  if (!mdtSegmentReadHandle.isValid()) {
    ATH_MSG_WARNING("Invalid ReadHandle to Trk::SegmentCollection with name: "
                    << m_mdtSegmentContainerReadHandleKey);
  } else {

    ATH_MSG_DEBUG(m_mdtSegmentContainerReadHandleKey
                  << " retrieved from StoreGate");

    unsigned int mdtSegmentCounter = 0;
    for (const auto *thisMDTSegment : *mdtSegmentReadHandle) {
      mdtSegmentCounter++;
      const Muon::MuonSegment* seg =
          dynamic_cast<const Muon::MuonSegment*>(thisMDTSegment);
      if (!seg)
        std::abort();

      Identifier id = m_edmHelperSvc->chamberId(*seg);
      if (!id.is_valid())
        continue;
      if (!m_idHelperSvc->isMuon(id))
        continue;

      Muon::MuonStationIndex::ChIndex chIndex = m_idHelperSvc->chamberIndex(id);

      if (chIndex != Muon::MuonStationIndex::EIL &&
            chIndex != Muon::MuonStationIndex::EIS)
        continue;

      const Amg::Vector3D& globalPos = seg->globalPosition();
      const Amg::Vector3D& globalDir = seg->globalDirection();
      double thetaPos = globalPos.theta();
      double thetaDir = globalDir.theta();

      if (std::cos(2. * (thetaPos - thetaDir)) >
          std::cos(2. * m_cutThetaMdtI))
        continue;

      ElementLink<Trk::SegmentCollection> segLink;
      segLink.toIndexedElement(*mdtSegmentReadHandle, mdtSegmentCounter - 1);
      cache.m_indexSeg.push_back(segLink);
    }
  }

  cache.m_resultSeg.assign(cache.m_indexSeg.size(), int(0));

  // find matching clusters
  SG::ReadHandle<xAOD::CaloClusterContainer> caloClusterContainerReadHandle(
      m_caloClusterContainerReadHandleKey,ctx);

  if (!caloClusterContainerReadHandle.isValid()) {
    ATH_MSG_WARNING("Invalid ReadHandle to CaloClusterContainer with name: "
                    << m_caloClusterContainerReadHandleKey);
  } else {

    ATH_MSG_DEBUG(m_caloClusterContainerReadHandleKey
                  << " retrieved from StoreGate");

    unsigned int caloClusterCounter = 0;
    for (const auto *thisCaloCluster : *caloClusterContainerReadHandle) {
      caloClusterCounter++;

      double eEmClus =
          thisCaloCluster->eSample(CaloSampling::CaloSample::PreSamplerB) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EMB1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EMB2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EMB3) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::PreSamplerE) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EME1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EME2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::EME3) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::FCAL0);
      double eHadClus =
          thisCaloCluster->eSample(CaloSampling::CaloSample::HEC0) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::HEC1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::HEC2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::HEC3) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileBar0) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileBar1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileBar2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileGap1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileGap2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileGap3) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileExt0) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileExt1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::TileExt2) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::FCAL1) +
          thisCaloCluster->eSample(CaloSampling::CaloSample::FCAL2);
      double eClus = eEmClus + eHadClus;

      // ignore low energy clusters
      if (eClus < m_cutEnergy)
        continue;

      double rClus(0.);
      if (!thisCaloCluster->retrieveMoment(xAOD::CaloCluster_v1::CENTER_MAG,
                                           rClus))
        rClus = 0;
      rClus = rClus / cosh(thisCaloCluster->eta());

      double phiClus = thisCaloCluster->phi();

      // remove clusters at low radius (outside the CSC acceptance)
      if (rClus < m_cutRadiusLow)
        continue;
      if (rClus > m_cutRadiusHigh)
        continue;

      std::vector<int> matchedSegmentsPerCluster;
      matchedSegmentsPerCluster.assign(cache.m_indexSeg.size(), int(0));
      bool matched = false;

      for (unsigned int j = 0; j < cache.m_indexSeg.size(); j++) {
        const Muon::MuonSegment* seg =
            dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[j]));
        if (!seg)
          std::abort();

        const Amg::Vector3D& globalPos = seg->globalPosition();
        double phiSeg = globalPos.phi();
        double rSeg = globalPos.perp();

        if ( m_isRun3 ) { 
          // match in phi  
          if (std::abs(phiClus - phiSeg) > m_cutPhiNsw) continue; 
          // match in radius 
          if (std::abs(rClus - rSeg) > m_cutRadiusNsw) continue; 
        } else { 
          Identifier id = m_edmHelperSvc->chamberId(*seg); 
          bool isCsc = m_idHelperSvc->isCsc(id); 
          // match in phi 
          if (std::cos(phiClus - phiSeg) < std::cos(m_cutPhiCsc) && isCsc) continue; 
          if (std::cos(phiClus - phiSeg) < std::cos(m_cutPhiMdtI) && !isCsc) continue; 
          // match in radius 
          if (std::abs(rClus - rSeg) > m_cutRadiusCsc && isCsc) continue; 
          if (std::abs(rClus - rSeg) > m_cutRadiusMdtI && !isCsc) continue; 
        }

        matchedSegmentsPerCluster[j] = 1;
        matched = true;
        cache.m_resultSeg[j] =
            cache.m_resultSeg[j] | BeamBackgroundData::Matched;
      }

      if (!matched)
        continue;

      ElementLink<xAOD::CaloClusterContainer> clusLink;
      clusLink.toIndexedElement(*caloClusterContainerReadHandle,
                                caloClusterCounter - 1);
      cache.m_indexClus.push_back(clusLink);
      cache.m_matchMatrix.push_back(matchedSegmentsPerCluster);
      cache.m_numMatched++;
    }
  }
  cache.m_resultClus.assign(cache.m_indexClus.size(), int(1));

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
/**
 * This function looks at the segments found by the FillMatchMatrix function.
 * The following results are set:
 * - number of segments with the direciton parallel to the beam pipe
 *   (all the segments in the matching matrix)
 * - number of such segments with early time
 * - number of segment pairs on side A and C
 * - number of segment pairs on side A and C with the corresponding time
 * difference
 */
void BeamBackgroundFiller::SegmentMethod(Cache& cache) const {
  //
  cache.m_numSegment = 0;
  cache.m_numSegmentEarly = 0;
  cache.m_numSegmentACNoTime = 0;
  cache.m_numSegmentAC = 0;

  for (unsigned int segIndex = 0; segIndex < cache.m_indexSeg.size(); segIndex++) {

    const Muon::MuonSegment* seg =
        dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndex]));
    if (!seg)
      std::abort();

    const Amg::Vector3D& globalPos = seg->globalPosition();
    double zSeg = globalPos.z();

    double tSeg = seg->time();

    cache.m_numSegment++;
    cache.m_resultSeg[segIndex] =
        cache.m_resultSeg[segIndex] | BeamBackgroundData::Segment;

    // muon segment: in-time (1), early (2), ambiguous (0)
    int timeStatus = 0;
    double inTime = -(-std::abs(zSeg) + globalPos.mag()) * myConst;
    double early = -(std::abs(zSeg) + globalPos.mag()) * myConst;
    if (std::abs(tSeg - inTime) < m_cutMuonTime)
      timeStatus = 1;
    if (std::abs(tSeg - early) < m_cutMuonTime)
      timeStatus = 2;

    if (timeStatus == 2) {
      cache.m_numSegmentEarly++;
      cache.m_resultSeg[segIndex] =
          cache.m_resultSeg[segIndex] | BeamBackgroundData::SegmentEarly;
    }

    // take only the segments on side A (z > 0)
    if (zSeg < 0.)
      continue;

    unsigned int segIndexA = segIndex;

    double tSegA = tSeg;
    double timeStatusA = timeStatus;

    double phiSegA = globalPos.phi();

    for (unsigned int segIndexC = 0; segIndexC < cache.m_indexSeg.size();
         segIndexC++) {

      const Muon::MuonSegment* segC =
          dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndexC]));
      if (!segC)
        std::abort();

      const Amg::Vector3D& globalPos = segC->globalPosition();
      double zSegC = globalPos.z();

      double tSegC = seg->time();

      // take only the segments on side C (z < 0)
      if (zSegC > 0.)
        continue;

      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatusC = 0;
      double inTime = -(-std::abs(zSegC) + globalPos.mag()) * myConst;
      double early = -(std::abs(zSegC) + globalPos.mag()) * myConst;
      if (std::abs(tSegC - inTime) < m_cutMuonTime)
        timeStatusC = 1;
      if (std::abs(tSegC - early) < m_cutMuonTime)
        timeStatusC = 2;

      double phiSegC = globalPos.phi();

      // match in phi
      double phiCut = m_isRun3 ? m_cutPhiSegNsw : m_cutPhiSeg;

      if (std::cos(phiSegA - phiSegC) < std::cos(phiCut)) {
        continue;
      }

      cache.m_numSegmentACNoTime++;
      cache.m_resultSeg[segIndexA] =
          cache.m_resultSeg[segIndexA] | BeamBackgroundData::SegmentACNoTime;
      cache.m_resultSeg[segIndexC] =
          cache.m_resultSeg[segIndexC] | BeamBackgroundData::SegmentACNoTime;

      if (timeStatusA == 0 || timeStatusC == 0)
        continue;

      // check the time difference
      if (std::abs(tSegA - tSegC) > m_cutTimeDiffAC) {
        cache.m_numSegmentAC++;
        cache.m_resultSeg[segIndexA] =
            cache.m_resultSeg[segIndexA] | BeamBackgroundData::SegmentAC;
        cache.m_resultSeg[segIndexC] =
            cache.m_resultSeg[segIndexC] | BeamBackgroundData::SegmentAC;
      }
    }
  }
}
// Match EI NSW segments to MDT segments in the EM 

void BeamBackgroundFiller::NSWMDTMatching(const EventContext& ctx, Cache& cache) const {

  // Run this entire method only for Run-3 data
  if (!m_isRun3) return;

  cache.m_numSegmentMDT = 0;
  
  SG::ReadHandle<Trk::SegmentCollection> mdtSegmentReadHandle(m_mdtSegmentContainerReadHandleKey, ctx);
  if (!mdtSegmentReadHandle.isValid()) {
      throw std::runtime_error("Could not load the " + m_mdtSegmentContainerReadHandleKey.key() + "segment container");
  }
  
  ATH_MSG_DEBUG("NSWMDTMatching: NSW indexSeg size = " << cache.m_indexSeg.size());
  
  for (unsigned int segIndexNSW = 0; segIndexNSW < cache.m_indexSeg.size(); ++segIndexNSW) {
    
    const Muon::MuonSegment* segNSW = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndexNSW]));

    const Identifier idNSW = m_edmHelperSvc->chamberId(*segNSW);

    const Amg::Vector3D& globalPos = segNSW->globalPosition();
    double zSegNSW   = globalPos.z();
    double phiSegNSW = globalPos.phi();
    double rSegNSW   = globalPos.perp();

    for (const Trk::Segment* thisMDTSegment : *mdtSegmentReadHandle) {
      
      const Muon::MuonSegment* segMDT = dynamic_cast<const Muon::MuonSegment*>(thisMDTSegment);

      const Identifier idMDT = m_edmHelperSvc->chamberId(*segMDT);
      if (!m_idHelperSvc->isMuon(idMDT)) continue;

      Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(idMDT);
      if (stIndex != Muon::MuonStationIndex::StIndex::EM) continue;

      const Amg::Vector3D& globalPosMDT = segMDT->globalPosition();
      double zSegMDT   = globalPosMDT.z();
      double phiSegMDT = globalPosMDT.phi();
      double rSegMDT   = globalPosMDT.perp();

      // same side
      if (zSegNSW * zSegMDT < 0.) continue;

      // same sector
      if (m_idHelperSvc->sector(idNSW) != m_idHelperSvc->sector(idMDT)) continue;

      // phi matching
      if (std::abs(phiSegNSW - phiSegMDT) > m_cutPhiSegMdt) continue;

      // radius matching
      if ((rSegNSW - rSegMDT) < m_cutRadSegMdt) continue;

      ATH_MSG_DEBUG("NSW-MDT match FOUND for NSW index " << segIndexNSW);

      cache.m_numSegmentMDT++;
      cache.m_resultSeg[segIndexNSW] |= BeamBackgroundData::SegmentMDT;
    }
  }
}

//------------------------------------------------------------------------------
/**
 * This function is the implementation of the "No-Time Method" and the
 * "One-Sided Method". The "One-Sided Method" compares the time of the
 * calorimeter cluster with the expected time. The expected time is calculated
 * based on the direction of the beam background muon which is reconstructed
 * from the position and time of the muon segment. The "No-Time Method" does not
 * use the time information of the muon segment thus the direction of the beam
 * background muon is not known. Therefore, the cluster time is compared to two
 * expected time values (corresponding to both A->C and C->A directions).
 */
void BeamBackgroundFiller::OneSidedMethod(Cache& cache) const {
  //
  cache.m_numNoTimeLoose = 0;
  cache.m_numNoTimeMedium = 0;
  cache.m_numNoTimeTight = 0;
  cache.m_numOneSidedLoose = 0;
  cache.m_numOneSidedMedium = 0;
  cache.m_numOneSidedTight = 0;

  for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size();
       clusIndex++) {

    const xAOD::CaloCluster* clus = *(cache.m_indexClus[clusIndex]);

    double rClus(0.);
    if (!clus->retrieveMoment(xAOD::CaloCluster_v1::CENTER_MAG, rClus))
      rClus = 0;
    rClus = rClus / cosh(clus->eta());
    double zClus = rClus * sinh(clus->eta());
    double tClus = clus->time();

    // calculate expected cluster time
    double expectedClusterTimeAC =
        -(zClus + std::sqrt(rClus * rClus + zClus * zClus)) * myConst;
    double expectedClusterTimeCA =
        -(-zClus + std::sqrt(rClus * rClus + zClus * zClus)) * myConst;

    for (unsigned int segIndex = 0; segIndex < cache.m_indexSeg.size(); segIndex++) {

      if (!(cache.m_matchMatrix[clusIndex][segIndex] & 1))
        continue;

      const Muon::MuonSegment* seg =
          dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndex]));
      if (!seg)
        std::abort();

      const Amg::Vector3D& globalPos = seg->globalPosition();
      double zSeg = globalPos.z();

      double tSeg = seg->time();

      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatus = 0;
      double inTime = -(-std::abs(zSeg) + globalPos.mag()) * myConst;
      double early = -(std::abs(zSeg) + globalPos.mag()) * myConst;
      if (std::abs(tSeg - inTime) < m_cutMuonTime)
        timeStatus = 1;
      if (std::abs(tSeg - early) < m_cutMuonTime)
        timeStatus = 2;

      // reconstruct beam background direction: A->C (1), C->A (-1)
      int direction = 0;
      if ((zSeg > 0 && timeStatus == 2) || (zSeg < 0 && timeStatus == 1))
        direction = 1;
      if ((zSeg > 0 && timeStatus == 1) || (zSeg < 0 && timeStatus == 2))
        direction = -1;

      // check the cluster time without the beam background direction
      // information
      if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime ||
          std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime) {
        cache.m_matchMatrix[clusIndex][segIndex] =
            cache.m_matchMatrix[clusIndex][segIndex] |
            BeamBackgroundData::NoTimeLoose;
      }
      if ((std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime &&
           -tClus > m_cutClusTime) ||
          (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime &&
           -tClus > m_cutClusTime)) {
        cache.m_matchMatrix[clusIndex][segIndex] =
            cache.m_matchMatrix[clusIndex][segIndex] |
            BeamBackgroundData::NoTimeMedium;
      }
      if ((std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime &&
           -tClus > 2. * m_cutClusTime) ||
          (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime &&
           -tClus > 2. * m_cutClusTime)) {
        cache.m_matchMatrix[clusIndex][segIndex] =
            cache.m_matchMatrix[clusIndex][segIndex] |
            BeamBackgroundData::NoTimeTight;
      }

      // check the cluster time with the beam background direction information
      if (direction == 1) {
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedLoose;
        }
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime &&
            -tClus > m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedMedium;
        }
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime &&
            -tClus > 2. * m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedTight;
        }
      } else if (direction == -1) {
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedLoose;
        }
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime &&
            -tClus > m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedMedium;
        }
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime &&
            -tClus > 2. * m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] =
              cache.m_matchMatrix[clusIndex][segIndex] |
              BeamBackgroundData::OneSidedTight;
        }
      }

      cache.m_resultClus[clusIndex] = cache.m_resultClus[clusIndex] |
                                      cache.m_matchMatrix[clusIndex][segIndex];
      cache.m_resultSeg[segIndex] = cache.m_resultSeg[segIndex] |
                                    cache.m_matchMatrix[clusIndex][segIndex];
    }

    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::NoTimeLoose)
      cache.m_numNoTimeLoose++;
    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::NoTimeMedium)
      cache.m_numNoTimeMedium++;
    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::NoTimeTight)
      cache.m_numNoTimeTight++;

    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::OneSidedLoose)
      cache.m_numOneSidedLoose++;
    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::OneSidedMedium)
      cache.m_numOneSidedMedium++;
    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::OneSidedTight)
      cache.m_numOneSidedTight++;
  }
}

//------------------------------------------------------------------------------
/**
 * This function is the implementation of the "Two-Sided No-Time Method" and
 * the "Two-Sided Method" that looks at the clusters matched with
 * at least one muon segment on side A and one muon segment on side C.
 * In case of the "Two-Sided Method",
 * corresponding time difference of the muon segments is required
 * and the direction of the beam background muon is also stored.
 */
void BeamBackgroundFiller::TwoSidedMethod(Cache& cache) const {
  cache.m_numTwoSidedNoTime = 0;
  cache.m_numTwoSided = 0;
  cache.m_direction = 0;

  for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size();
       clusIndex++) {

    for (unsigned int segIndexA = 0; segIndexA < cache.m_indexSeg.size();
         segIndexA++) {

      if (!(cache.m_matchMatrix[clusIndex][segIndexA] & 1))
        continue;

      const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(
          *(cache.m_indexSeg[segIndexA]));
      if (!seg)
        std::abort();

      const Amg::Vector3D& globalPos = seg->globalPosition();
      double zSegA = globalPos.z();

      double tSegA = seg->time();

      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatusA = 0;
      double inTime = -(-std::abs(zSegA) + globalPos.mag()) * myConst;
      double early = -(std::abs(zSegA) + globalPos.mag()) * myConst;
      if (std::abs(tSegA - inTime) < m_cutMuonTime)
        timeStatusA = 1;
      if (std::abs(tSegA - early) < m_cutMuonTime)
        timeStatusA = 2;

      // take only the segments on side A (z > 0)
      if (zSegA < 0.)
        continue;

      for (unsigned int segIndexC = 0; segIndexC < cache.m_indexSeg.size();
           segIndexC++) {

        if (!(cache.m_matchMatrix[clusIndex][segIndexC] & 1))
          continue;

        const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(
            *(cache.m_indexSeg[segIndexC]));
        if (!seg)
          std::abort();

        const Amg::Vector3D& globalPos = seg->globalPosition();
        double zSegC = globalPos.z();

        double tSegC = seg->time();

        // muon segment: in-time (1), early (2), ambiguous (0)
        int timeStatusC = 0;
        double inTime = -(-std::abs(zSegC) + globalPos.mag()) * myConst;
        double early = -(std::abs(zSegC) + globalPos.mag()) * myConst;
        if (std::abs(tSegC - inTime) < m_cutMuonTime)
          timeStatusC = 1;
        if (std::abs(tSegC - early) < m_cutMuonTime)
          timeStatusC = 2;

        // take only the segments on side C (z < 0)
        if (zSegC > 0.)
          continue;

        cache.m_matchMatrix[clusIndex][segIndexA] =
            cache.m_matchMatrix[clusIndex][segIndexA] |
            BeamBackgroundData::TwoSidedNoTime;
        cache.m_matchMatrix[clusIndex][segIndexC] =
            cache.m_matchMatrix[clusIndex][segIndexC] |
            BeamBackgroundData::TwoSidedNoTime;
        cache.m_resultSeg[segIndexA] =
            cache.m_resultSeg[segIndexA] |
            cache.m_matchMatrix[clusIndex][segIndexA];
        cache.m_resultSeg[segIndexC] =
            cache.m_resultSeg[segIndexC] |
            cache.m_matchMatrix[clusIndex][segIndexC];

        if (timeStatusA == 0 || timeStatusC == 0)
          continue;

        // check the time difference
        if (std::abs(tSegA - tSegC) > m_cutTimeDiffAC) {
          cache.m_matchMatrix[clusIndex][segIndexA] =
              cache.m_matchMatrix[clusIndex][segIndexA] |
              BeamBackgroundData::TwoSided;
          cache.m_matchMatrix[clusIndex][segIndexC] =
              cache.m_matchMatrix[clusIndex][segIndexC] |
              BeamBackgroundData::TwoSided;
          cache.m_resultSeg[segIndexA] =
              cache.m_resultSeg[segIndexA] |
              cache.m_matchMatrix[clusIndex][segIndexA];
          cache.m_resultSeg[segIndexC] =
              cache.m_resultSeg[segIndexC] |
              cache.m_matchMatrix[clusIndex][segIndexC];

          // direction of beam background
          if (timeStatusA == 2)
            cache.m_direction++;  // A->C
          if (timeStatusC == 2)
            cache.m_direction--;  // C->A
        }
      }

      cache.m_resultClus[clusIndex] = cache.m_resultClus[clusIndex] |
                                      cache.m_matchMatrix[clusIndex][segIndexA];
    }

    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::TwoSidedNoTime)
      cache.m_numTwoSidedNoTime++;
    if (cache.m_resultClus[clusIndex] & BeamBackgroundData::TwoSided)
      cache.m_numTwoSided++;
  }
}

//------------------------------------------------------------------------------
/**
 * This function is the implementation of the "Cluster-Shape Method".
 * The shape of the cluster is described by the variable dr/dz
 * which is the ratio of the standard deviation of the radial position of the
 * contianed cells and the standard deviation of the z-position of the contained
 * cells. Only the clusters matched with muon segments are checked.
 */
void BeamBackgroundFiller::ClusterShapeMethod(Cache& cache) const {
  cache.m_numClusterShape = 0;
  cache.m_drdzClus.clear();

  for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size();
       clusIndex++) {

    const xAOD::CaloCluster* clus = *(cache.m_indexClus[clusIndex]);

    double rClus(0.);
    if (!clus->retrieveMoment(xAOD::CaloCluster_v1::CENTER_MAG, rClus))
      rClus = 0;
    rClus = rClus / cosh(clus->eta());
    double zClus = rClus * sinh(clus->eta());

    // calculate dr/dz
    double dr = 0.;
    double dz = 0.;
    double drdz = -1.;
    int nCell = 0;

    if (clus->getCellLinks() != nullptr) {
      xAOD::CaloCluster::const_cell_iterator firstCell = clus->cell_begin();
      xAOD::CaloCluster::const_cell_iterator lastCell = clus->cell_end();

      for (; firstCell != lastCell; ++firstCell) {
        const CaloCell* cell = *firstCell;

        if (cell->time() == 0.)
          continue;
        if (cell->energy() < 100.)
          continue;
        nCell++;

        // double rCell = sqrt(cell->x()*cell->x() + cell->y()*cell->y());
        // double zCell = cell->z();
        const CaloDetDescrElement* dde = cell->caloDDE();
        const double rCell = dde->r();
        const double zCell = dde->z();
        dr = dr + (rCell - rClus) * (rCell - rClus);
        dz = dz + (zCell - zClus) * (zCell - zClus);
      }
    }

    if (nCell) {
      dr = sqrt(dr / nCell);
      dz = sqrt(dz / nCell);
      if (dz > 0.)
        drdz = dr / dz;
    }

    cache.m_drdzClus.push_back(drdz);

    // check dr/dz
    if (drdz < 0.)
      continue;
    if (drdz < m_cutDrdz) {
      for (unsigned int segIndex = 0; segIndex < cache.m_indexSeg.size();
           segIndex++) {
        if (!(cache.m_matchMatrix[clusIndex][segIndex] & 1))
          continue;
        cache.m_matchMatrix[clusIndex][segIndex] =
            cache.m_matchMatrix[clusIndex][segIndex] |
            BeamBackgroundData::ClusterShape;
        cache.m_resultSeg[segIndex] = cache.m_resultSeg[segIndex] |
                                      cache.m_matchMatrix[clusIndex][segIndex];
      }
      cache.m_resultClus[clusIndex] =
          cache.m_resultClus[clusIndex] | BeamBackgroundData::ClusterShape;
      cache.m_numClusterShape++;
    }
  }
}

//------------------------------------------------------------------------------
/**
 * This function checks whether the matched clusters are contained in any jets.
 * If yes, the jet is marked as fake jet and the corresponding result
 * of the Beam Background Identification Method is stored
 * (using the OR condition if more than one cluster is found within one jet)
 */
void BeamBackgroundFiller::FindFakeJets(const EventContext& ctx,
                                        Cache& cache) const {
  cache.m_numJet = 0;
  cache.m_indexJet.clear();
  cache.m_resultJet.clear();

  // find the jet that contains this cluster
  SG::ReadHandle<xAOD::JetContainer> jetContainerReadHandle(
      m_jetContainerReadHandleKey, ctx);

  if (!jetContainerReadHandle.isValid()) {
    ATH_MSG_WARNING("Invalid ReadHandle to JetContainer with name: "
                    << m_jetContainerReadHandleKey);
  } else {
    ATH_MSG_DEBUG(m_jetContainerReadHandleKey << " retrieved from StoreGate");

    unsigned int jetCounter = 0;
    for (const auto *thisJet : *jetContainerReadHandle) {
      bool isFakeJet = false;
      int resultJet = 0;

      xAOD::JetConstituentVector vec = thisJet->getConstituents();
      xAOD::JetConstituentVector::iterator constIt = vec.begin();
      xAOD::JetConstituentVector::iterator constItE = vec.end();

      for (; constIt != constItE; ++constIt) {
        if (constIt->type() != xAOD::Type::CaloCluster)
          continue;
        const xAOD::CaloCluster* jetConst =
            dynamic_cast<const xAOD::CaloCluster*>(constIt->rawConstituent());

        for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size();
             clusIndex++) {
          const xAOD::CaloCluster* clus = *(cache.m_indexClus[clusIndex]);

          if (jetConst == clus) {
            isFakeJet = true;
            resultJet = resultJet | cache.m_resultClus[clusIndex];
          }
        }
      }

      if (isFakeJet) {
        ElementLink<xAOD::JetContainer> jetLink;
        jetLink.toIndexedElement(*jetContainerReadHandle, jetCounter);
        cache.m_indexJet.push_back(jetLink);
        cache.m_resultJet.push_back(resultJet);
        cache.m_numJet++;
      }
      jetCounter++;
    }
  }
}

//------------------------------------------------------------------------------
/**
 * This function stores all the results in BeamBackgroundData
 */
void BeamBackgroundFiller::FillBeamBackgroundData(
    SG::WriteHandle<BeamBackgroundData>& beamBackgroundDataWriteHandle,
    Cache& cache) const{

  beamBackgroundDataWriteHandle->SetNumSegment(cache.m_numSegment);
  beamBackgroundDataWriteHandle->SetNumSegmentEarly(cache.m_numSegmentEarly);
  beamBackgroundDataWriteHandle->SetNumSegmentACNoTime(cache.m_numSegmentACNoTime);
  beamBackgroundDataWriteHandle->SetNumSegmentAC(cache.m_numSegmentAC);
  beamBackgroundDataWriteHandle->SetNumSegmentMDT(cache.m_numSegmentMDT);
  beamBackgroundDataWriteHandle->SetNumMatched(cache.m_numMatched);
  beamBackgroundDataWriteHandle->SetNumNoTimeLoose(cache.m_numNoTimeLoose);
  beamBackgroundDataWriteHandle->SetNumNoTimeMedium(cache.m_numNoTimeMedium);
  beamBackgroundDataWriteHandle->SetNumNoTimeTight(cache.m_numNoTimeTight);
  beamBackgroundDataWriteHandle->SetNumOneSidedLoose(cache.m_numOneSidedLoose);
  beamBackgroundDataWriteHandle->SetNumOneSidedMedium(cache.m_numOneSidedMedium);
  beamBackgroundDataWriteHandle->SetNumOneSidedTight(cache.m_numOneSidedTight);
  beamBackgroundDataWriteHandle->SetNumTwoSidedNoTime(cache.m_numTwoSidedNoTime);
  beamBackgroundDataWriteHandle->SetNumTwoSided(cache.m_numTwoSided);
  beamBackgroundDataWriteHandle->SetNumClusterShape(cache.m_numClusterShape);
  beamBackgroundDataWriteHandle->SetNumJet(cache.m_numJet);

  int decision = 0;
  for (unsigned int i = 0; i < cache.m_indexSeg.size(); i++) {
    decision = decision | cache.m_resultSeg[i];
  }
  for (unsigned int i = 0; i < cache.m_indexClus.size(); i++) {
    decision = decision | cache.m_resultClus[i];
  }
  beamBackgroundDataWriteHandle->SetDecision(decision);

  beamBackgroundDataWriteHandle->SetDirection(cache.m_direction);

  beamBackgroundDataWriteHandle->FillIndexSeg(std::move(cache.m_indexSeg));
  beamBackgroundDataWriteHandle->FillResultSeg(&cache.m_resultSeg);
  beamBackgroundDataWriteHandle->FillIndexClus(std::move(cache.m_indexClus));
  beamBackgroundDataWriteHandle->FillMatchMatrix(&cache.m_matchMatrix);

  beamBackgroundDataWriteHandle->FillResultClus(&cache.m_resultClus);
  beamBackgroundDataWriteHandle->FillDrdzClus(&cache.m_drdzClus);

  beamBackgroundDataWriteHandle->FillIndexJet(std::move(cache.m_indexJet));
  beamBackgroundDataWriteHandle->FillResultJet(&cache.m_resultJet);

  ATH_MSG_DEBUG("parallel segments "
                << cache.m_numSegment << " " << cache.m_numSegmentEarly << " "
                << cache.m_numSegmentACNoTime << " " << cache.m_numSegmentAC);

  ATH_MSG_DEBUG("matched clusters "
                << cache.m_numMatched << " " << cache.m_numNoTimeLoose << " "
                << cache.m_numNoTimeMedium << " " << cache.m_numNoTimeTight
                << " " << cache.m_numOneSidedLoose << " "
                << cache.m_numOneSidedMedium << " " << cache.m_numOneSidedTight
                << " " << cache.m_numTwoSidedNoTime << " "
                << cache.m_numTwoSided << " " << cache.m_numClusterShape);
}

