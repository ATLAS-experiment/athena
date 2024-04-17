/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BeamBackgroundFiller.h"

#include <cmath>

#include "AthenaKernel/errorcheck.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonPrepRawData/CscPrepData.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODJet/JetConstituentVector.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"

#include "MuonPrepRawData/MMPrepData.h"
#include "MuonPrepRawData/sTgcPrepData.h"
namespace {
  constexpr double inv_c = 1./Gaudi::Units::c_light;
}

//------------------------------------------------------------------------------
BeamBackgroundFiller::BeamBackgroundFiller(const std::string& name,
                                           ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {
}

//------------------------------------------------------------------------------
StatusCode BeamBackgroundFiller::initialize() {
  CHECK(m_edmHelperSvc.retrieve());
  CHECK(m_idHelperSvc.retrieve());

  ATH_CHECK(m_segmentKeys.initialize());
  ATH_CHECK(m_segmentSelector.retrieve(EnableTool{!m_segmentKeys.empty()}));
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
    FillMatchMatrix(ctx, cache);
    // apply Beam Background Identifiaction Methods
    SegmentMethod(cache);
    OneSidedMethod(cache);
    TwoSidedMethod(cache);
    ClusterShapeMethod(cache);
    // identify fake jets
    FindFakeJets(ctx, cache);

    // fill the results into BeamBackgroundData
    SG::WriteHandle<BeamBackgroundData> writeHandle(m_beamBackgroundDataWriteHandleKey, ctx);
    ATH_CHECK(writeHandle.record(std::make_unique<BeamBackgroundData>()));
    FillBeamBackgroundData(writeHandle, cache);

    return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
/**
 * This function selects the muon segments with the direction parallel to the
 * beam pipe and calorimeter clusters above certain energy threshold. Matching
 * matrix is created to store the results of beam background identification for
 * each cluster and segment
 */
void BeamBackgroundFiller::FillMatchMatrix(const EventContext& ctx,
                                           Cache& cache) const {
  //

  for (const SG::ReadHandleKey<Trk::SegmentCollection>& key : m_segmentKeys) {
      // select only the CSC segments with the global direction parallel to the
      // beam pipe
      SG::ReadHandle<Trk::SegmentCollection> ncbSegmentHandle(key, ctx);
      if(!ncbSegmentHandle.isPresent()) {
          throw std::runtime_error("Could not load the " + key.key() + " segment container");
      }
      unsigned int ncbCounter = 0;
      for (const Trk::Segment *ncbSegment : *ncbSegmentHandle) {
        ++ncbCounter;
        const Muon::MuonSegment* seg =dynamic_cast<const Muon::MuonSegment*>(ncbSegment);

        const Identifier id = m_edmHelperSvc->chamberId(*seg);
        if (!id.is_valid()|| !m_idHelperSvc->isMuon(id)) {
           ATH_MSG_WARNING("Found a muon segment in the container which pretends not to be a muon segment..");
           continue;
        }
        Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);
        /// Select only the segements from the EI station
        if (stIndex != Muon::MuonStationIndex::EI) {
            ATH_MSG_VERBOSE("Segment "<<m_idHelperSvc->toStringChamber(id)<<" is not in EI");
            continue;
        }
        const Amg::Vector3D& globalDir = seg->globalDirection();        
        if (std::abs(globalDir.theta()) < m_thetaCutNCB) {
           continue;
        }
        constexpr int highestSegQual = 3;
        if (!m_segmentSelector->select(*seg,false, highestSegQual)) {
             continue;
        }
        ElementLink<Trk::SegmentCollection> segLink{*ncbSegmentHandle, ncbCounter - 1};
        cache.m_indexSeg.push_back(segLink);
      }
  }

  cache.m_resultSeg.assign(cache.m_indexSeg.size(), 0);

  // find matching clusters
  SG::ReadHandle<xAOD::CaloClusterContainer> caloClusterContainerReadHandle(m_caloClusterContainerReadHandleKey,ctx);
  if (!caloClusterContainerReadHandle.isPresent()){
     throw std::runtime_error("Failed to load the calorimeter cluster container");
  }
  ATH_MSG_DEBUG(m_caloClusterContainerReadHandleKey<< " retrieved from StoreGate");

  constexpr std::array<CaloSampling::CaloSample, 24> caloLayers{CaloSampling::CaloSample::PreSamplerB,
                                                      CaloSampling::CaloSample::EMB1, CaloSampling::CaloSample::EMB2, CaloSampling::CaloSample::EMB3,
                                                      CaloSampling::CaloSample::PreSamplerE,
                                                      CaloSampling::CaloSample::EME1, CaloSampling::CaloSample::EME2, CaloSampling::CaloSample::EME3,
                                                      CaloSampling::CaloSample::FCAL0,
                                                      
                                                      CaloSampling::CaloSample::HEC0, CaloSampling::CaloSample::HEC1, CaloSampling::CaloSample::HEC2, CaloSampling::CaloSample::HEC3,

                                                      CaloSampling::CaloSample::TileBar0, CaloSampling::CaloSample::TileBar1, CaloSampling::CaloSample::TileBar2, 
                                                      CaloSampling::CaloSample::TileGap1, CaloSampling::CaloSample::TileGap2, CaloSampling::CaloSample::TileGap3, 
                                                      CaloSampling::CaloSample::TileExt0, CaloSampling::CaloSample::TileExt1, CaloSampling::CaloSample::TileExt2, CaloSampling::CaloSample::FCAL1,
                                                      CaloSampling::CaloSample::FCAL2};
    
    unsigned int caloClusterCounter = 0;
    for (const xAOD::CaloCluster* thisCaloCluster : *caloClusterContainerReadHandle) {
        ++caloClusterCounter;
        double eClus{0.};
        for (auto lay : caloLayers){
            eClus +=thisCaloCluster->eSample(lay);
        }
        // ignore low energy clusters
        if (eClus < m_clusEnergyCut){
          ATH_MSG_VERBOSE("Cluster with energy "<<eClus<<" is below threshold "<<m_clusEnergyCut);
          continue;
        }
        double rClus{0.};
        if (!thisCaloCluster->retrieveMoment(xAOD::CaloCluster_v1::CENTER_MAG, rClus)) {
            ATH_MSG_DEBUG("Failed to retrieve the CENTER_MAG moment");
            continue;
        }
        rClus = rClus / std::cosh(thisCaloCluster->eta());

        // remove clusters at low radius (outside the CSC acceptance)
        if (rClus < m_clusRadiusLow || rClus > m_clusRadiusHigh) {
            ATH_MSG_VERBOSE("Radius cut not passed "<<rClus<<" needs to be in "
                          <<m_clusRadiusLow<<" "<<m_clusRadiusHigh);
            continue;
        }
        const double phiClus = thisCaloCluster->phi();
        

        std::vector<int> matchedSegmentsPerCluster(cache.m_indexSeg.size(), 0);     
        bool matched{false};

        for (unsigned int j = 0; j < cache.m_indexSeg.size(); j++) {
            const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[j]));
        
            const Amg::Vector3D& globalPos = seg->globalPosition();
            const double phiSeg = globalPos.phi();

            /// match in phi       
            if (P4Helpers::deltaPhi(phiClus, phiSeg) < std::abs(m_cutDphiClusSeg)) {
                ATH_MSG_VERBOSE("Delta phi "<<P4Helpers::deltaPhi(phiClus, phiSeg)
                              <<" exceeds maximum cut "<<m_cutDphiClusSeg
                              <<"Segment: "<<Amg::toString(globalPos)<<", phi: "<<globalPos.phi()
                              <<" --- Cluster: "<<phiClus);
                continue;
            }

            const double rSeg = globalPos.perp();
            // match in radius
            if (std::abs(rClus - rSeg) > m_cutDradClusSeg) {
                ATH_MSG_VERBOSE("Radial difference "<<std::abs(rClus - rSeg)<<" exceeds maximum cut "<<m_cutDradClusSeg
                            <<"Segment: "<<Amg::toString(globalPos)<<", phi: "<<globalPos.perp()
                            <<" --- Cluster: "<<rClus);            
                continue;
            }
            matchedSegmentsPerCluster[j] = 1;
            matched = true;
            cache.m_resultSeg[j] |= BeamBackgroundData::Matched;
        }

        if (!matched) {
            ATH_MSG_VERBOSE("Calo cluster does not match with segment");
            continue;
        }
        ElementLink<xAOD::CaloClusterContainer> clusLink;
        clusLink.toIndexedElement(*caloClusterContainerReadHandle, caloClusterCounter - 1);
        cache.m_indexClus.push_back(std::move(clusLink));
        cache.m_matchMatrix.push_back(std::move(matchedSegmentsPerCluster));
        ++cache.m_numMatched;
    }
  
    cache.m_resultClus.assign(cache.m_indexClus.size(), 1);
}



double BeamBackgroundFiller::GetSegmentTime(const Muon::MuonSegment& pMuonSegment) const {
    double time{0.};
    unsigned int nMeas{0};
    for (const Trk::MeasurementBase* meas : pMuonSegment.containedMeasurements()) {
        const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(meas);
        if (!rot) {
          continue;
        }
        ++nMeas;
        const Trk::PrepRawData* prd = rot->prepRawData();
        if (prd->type(Trk::PrepRawDataType::MMPrepData)) {
            const Muon::MMPrepData* mmPrd = static_cast<const Muon::MMPrepData*>(prd);
            time += mmPrd->time();
        } else if (prd->type(Trk::PrepRawDataType::sTgcPrepData)) {
            const Muon::sTgcPrepData* sTgcPrd = static_cast<const Muon::sTgcPrepData*>(prd);
            time += sTgcPrd->time();
        } else if (prd->type(Trk::PrepRawDataType::MdtPrepData)) {
            const Muon::MdtPrepData* mdtPrd = static_cast<const Muon::MdtPrepData*>(prd);
            constexpr double tdcBinSize = 0.78125;  //25/32; exact number: (1000.0/40.079)/32.0
            time += tdcBinSize * mdtPrd->tdc();
        } else if (prd->type(Trk::PrepRawDataType::TgcPrepData)) {
           /// Need to check how to translate the bcid bitmaps into timings
           --nMeas;
        } else if (prd->type(Trk::PrepRawDataType::CscPrepData)) {
          const Muon::CscPrepData* cscPrd = static_cast<const Muon::CscPrepData*>(prd);
          time += cscPrd->time();
        } else {
            ATH_MSG_WARNING("You can't have "<<m_idHelperSvc->toString(prd->identify())<<" in a EI segment.");
            --nMeas;
        }
      
    }
    return time / std::max(nMeas, 1u);
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
  ///
  for (unsigned int segIndex = 0; segIndex < cache.m_indexSeg.size(); ++segIndex) {

    const Muon::MuonSegment* seg =dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndex]));

    const Amg::Vector3D& globalPos = seg->globalPosition();
    double zSeg = globalPos.z();

    /// take only the segments on side A (z > 0)
    if (zSeg < 0.) {
        continue;
    }

    
    double tSeg = GetSegmentTime(*seg);
    ATH_MSG_ALWAYS("Lieber Stonjeeeek.... Frecher Stonjeeeeeeek "<<tSeg<<" "
            <<m_idHelperSvc->toString(m_edmHelperSvc->chamberId(*seg)));

    cache.m_numSegment++;
    cache.m_resultSeg[segIndex] |= BeamBackgroundData::Segment;

    // muon segment: in-time (1), early (2), ambiguous (0)
    int timeStatus = 0;
    double inTime = -(-std::abs(zSeg) + globalPos.mag()) * inv_c;
    double early = -(std::abs(zSeg) + globalPos.mag()) * inv_c;
    if (std::abs(tSeg - inTime) < m_cutMuonTime)
      timeStatus = 1;
    if (std::abs(tSeg - early) < m_cutMuonTime)
      timeStatus = 2;

    if (timeStatus == 2) {
      cache.m_numSegmentEarly++;
      cache.m_resultSeg[segIndex] |= BeamBackgroundData::SegmentEarly;
    }



    unsigned int segIndexA = segIndex;

    double tSegA = tSeg;
    int timeStatusA = timeStatus;

    double phiSegA = globalPos.phi();

    for (unsigned int segIndexC = 0; segIndexC < cache.m_indexSeg.size(); segIndexC++) {

      const Muon::MuonSegment* segC = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndexC]));

      const Amg::Vector3D& globalPos = segC->globalPosition();
      double zSegC = globalPos.z();

      // take only the segments on side C (z < 0)
      if (zSegC > 0.) {
        continue;
      }
      double tSegC = GetSegmentTime(*segC);



      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatusC = 0;
      double inTime = -(-std::abs(zSegC) + globalPos.mag()) * inv_c;
      double early = -(std::abs(zSegC) + globalPos.mag()) * inv_c;
      if (std::abs(tSegC - inTime) < m_cutMuonTime)
        timeStatusC = 1;
      if (std::abs(tSegC - early) < m_cutMuonTime)
        timeStatusC = 2;

      double phiSegC = globalPos.phi();

      // match in phi
      if (std::abs(P4Helpers::deltaPhi(phiSegA, phiSegC)) > m_cutDphiSegAC) {
        continue;
      }
      cache.m_numSegmentACNoTime++;
      cache.m_resultSeg[segIndexA] |= BeamBackgroundData::SegmentACNoTime;
      cache.m_resultSeg[segIndexC] |= BeamBackgroundData::SegmentACNoTime;

      if (timeStatusA == 0 || timeStatusC == 0)
        continue;

      // check the time difference
      if (std::abs(tSegA - tSegC) > m_cutTimeDiffAC) {
        cache.m_numSegmentAC++;
        cache.m_resultSeg[segIndexA] |= BeamBackgroundData::SegmentAC;
        cache.m_resultSeg[segIndexC] |= BeamBackgroundData::SegmentAC;
      }
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
  for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size();
       clusIndex++) {

    const xAOD::CaloCluster* clus = *(cache.m_indexClus[clusIndex]);

    double rClus(0.);
    if (!clus->retrieveMoment(xAOD::CaloCluster_v1::CENTER_MAG, rClus)) {
         continue;
    }
    rClus = rClus / std::cosh(clus->eta());
    double zClus = rClus * std::sinh(clus->eta());
    double tClus = clus->time();

    // calculate expected cluster time
    double expectedClusterTimeAC =  -(zClus + std::hypot(rClus, zClus)) * inv_c;
    double expectedClusterTimeCA = -(-zClus + std::hypot(rClus, zClus)) * inv_c;

    for (unsigned int segIndex = 0; segIndex < cache.m_indexSeg.size(); segIndex++) {

      if (!(cache.m_matchMatrix[clusIndex][segIndex] & BeamBackgroundData::Matched)){
        continue;
      }
      const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndex]));
 
      const Amg::Vector3D& globalPos = seg->globalPosition();
      double zSeg = globalPos.z();

      double tSeg = GetSegmentTime(*seg);

      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatus = 0;
      double inTime = -(-std::abs(zSeg) + globalPos.mag()) * inv_c;
      double early = -(std::abs(zSeg) + globalPos.mag()) * inv_c;
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
        cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::NoTimeLoose;
      }
      if ((std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime && -tClus > m_cutClusTime) ||
          (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime && -tClus > m_cutClusTime)) {
        cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::NoTimeMedium;
      }
      if ((std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime && -tClus > 2. * m_cutClusTime) ||
          (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime && -tClus > 2. * m_cutClusTime)) {
        cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::NoTimeTight;
      }

      // check the cluster time with the beam background direction information
      if (direction == 1) {
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedLoose;
        }
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime && -tClus > m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedMedium;
        }
        if (std::abs(tClus - expectedClusterTimeAC) < m_cutClusTime && -tClus > 2. * m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedTight;
        }
      } else if (direction == -1) {
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedLoose;
        }
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime && -tClus > m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedMedium;
        }
        if (std::abs(tClus - expectedClusterTimeCA) < m_cutClusTime && -tClus > 2. * m_cutClusTime) {
          cache.m_matchMatrix[clusIndex][segIndex] |= BeamBackgroundData::OneSidedTight;
        }
      }

      cache.m_resultClus[clusIndex] |= cache.m_matchMatrix[clusIndex][segIndex];
      cache.m_resultSeg[segIndex] |=  cache.m_matchMatrix[clusIndex][segIndex];
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


  for (unsigned int clusIndex = 0; clusIndex < cache.m_indexClus.size(); clusIndex++) {

    for (unsigned int segIndexA = 0; segIndexA < cache.m_indexSeg.size(); segIndexA++) {

      if (!(cache.m_matchMatrix[clusIndex][segIndexA] & BeamBackgroundData::Matched))
        continue;

      const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndexA]));

      const Amg::Vector3D& globalPos = seg->globalPosition();
      double zSegA = globalPos.z();
      // take only the segments on side A (z > 0)
      if (zSegA < 0.) {
        continue;
      }
      double tSegA = GetSegmentTime(*seg);

      // muon segment: in-time (1), early (2), ambiguous (0)
      int timeStatusA = 0;
      double inTime = -(-std::abs(zSegA) + globalPos.mag()) * inv_c;
      double early = -(std::abs(zSegA) + globalPos.mag()) * inv_c;
      if (std::abs(tSegA - inTime) < m_cutMuonTime)
        timeStatusA = 1;
      if (std::abs(tSegA - early) < m_cutMuonTime)
        timeStatusA = 2;

  
      for (unsigned int segIndexC = 0; segIndexC < cache.m_indexSeg.size(); segIndexC++) {

        if (!(cache.m_matchMatrix[clusIndex][segIndexC] & BeamBackgroundData::Matched)){
          continue;
        }
        const Muon::MuonSegment* seg = dynamic_cast<const Muon::MuonSegment*>(*(cache.m_indexSeg[segIndexC]));

        const Amg::Vector3D& globalPos = seg->globalPosition();
        double zSegC = globalPos.z();

        // take only the segments on side C (z < 0)
        if (zSegC > 0.) {
          continue;
        }

        double tSegC = GetSegmentTime(*seg);

        // muon segment: in-time (1), early (2), ambiguous (0)
        int timeStatusC = 0;
        double inTime = -(-std::abs(zSegC) + globalPos.mag()) * inv_c;
        double early = -(std::abs(zSegC) + globalPos.mag()) * inv_c;
        if (std::abs(tSegC - inTime) < m_cutMuonTime)
          timeStatusC = 1;
        if (std::abs(tSegC - early) < m_cutMuonTime)
          timeStatusC = 2;



        cache.m_matchMatrix[clusIndex][segIndexA] |=BeamBackgroundData::TwoSidedNoTime;
        cache.m_matchMatrix[clusIndex][segIndexC] |=BeamBackgroundData::TwoSidedNoTime;
        cache.m_resultSeg[segIndexA] |= cache.m_matchMatrix[clusIndex][segIndexA];
        cache.m_resultSeg[segIndexC] |= cache.m_matchMatrix[clusIndex][segIndexC];

        if (timeStatusA == 0 || timeStatusC == 0)
          continue;

        // check the time difference
        if (std::abs(tSegA - tSegC) > m_cutTimeDiffAC) {
          cache.m_matchMatrix[clusIndex][segIndexA] |= BeamBackgroundData::TwoSided;
          cache.m_matchMatrix[clusIndex][segIndexC] |= BeamBackgroundData::TwoSided;
          cache.m_resultSeg[segIndexA] |= cache.m_matchMatrix[clusIndex][segIndexA];
          cache.m_resultSeg[segIndexC] |= cache.m_matchMatrix[clusIndex][segIndexC];

          // direction of beam background
          if (timeStatusA == 2)
            cache.m_direction++;  // A->C
          if (timeStatusC == 2)
            cache.m_direction--;  // C->A
        }
      }

      cache.m_resultClus[clusIndex] |= cache.m_matchMatrix[clusIndex][segIndexA];
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
    SG::WriteHandle<BeamBackgroundData>& writeHandle,
    Cache& cache) const{

  writeHandle->SetNumSegment(cache.m_numSegment);
  writeHandle->SetNumSegmentEarly(cache.m_numSegmentEarly);
  writeHandle->SetNumSegmentACNoTime(cache.m_numSegmentACNoTime);
  writeHandle->SetNumSegmentAC(cache.m_numSegmentAC);
  writeHandle->SetNumMatched(cache.m_numMatched);
  writeHandle->SetNumNoTimeLoose(cache.m_numNoTimeLoose);
  writeHandle->SetNumNoTimeMedium(cache.m_numNoTimeMedium);
  writeHandle->SetNumNoTimeTight(cache.m_numNoTimeTight);
  writeHandle->SetNumOneSidedLoose(cache.m_numOneSidedLoose);
  writeHandle->SetNumOneSidedMedium(cache.m_numOneSidedMedium);
  writeHandle->SetNumOneSidedTight(cache.m_numOneSidedTight);
  writeHandle->SetNumTwoSidedNoTime(cache.m_numTwoSidedNoTime);
  writeHandle->SetNumTwoSided(cache.m_numTwoSided);
  writeHandle->SetNumClusterShape(cache.m_numClusterShape);
  writeHandle->SetNumJet(cache.m_numJet);

  int decision = 0;
  for (unsigned int i = 0; i < cache.m_indexSeg.size(); i++) {
    decision |= cache.m_resultSeg[i];
  }
  for (unsigned int i = 0; i < cache.m_indexClus.size(); i++) {
    decision |= cache.m_resultClus[i];
  }
  writeHandle->SetDecision(decision);

  writeHandle->SetDirection(cache.m_direction);

  writeHandle->FillIndexSeg(cache.m_indexSeg);
  writeHandle->FillResultSeg(&cache.m_resultSeg);
  writeHandle->FillIndexClus(cache.m_indexClus);
  writeHandle->FillMatchMatrix(&cache.m_matchMatrix);

  writeHandle->FillResultClus(&cache.m_resultClus);
  writeHandle->FillIndexJet(cache.m_indexJet);
  writeHandle->FillDrdzClus(&cache.m_drdzClus);

  writeHandle->FillIndexJet(cache.m_indexJet);
  writeHandle->FillResultJet(&cache.m_resultJet);

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

