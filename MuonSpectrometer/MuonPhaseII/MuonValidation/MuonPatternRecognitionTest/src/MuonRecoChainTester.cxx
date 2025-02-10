/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Framework includes
#include "MuonRecoChainTester.h"

#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonTesterTree/TrackChi2Branch.h"
#include "MuonPRDTest/SegmentVariables.h"
#include "MuonPRDTest/ParticleVariables.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandle.h"

using namespace MuonVal;
using namespace MuonPRDTest;


namespace {
  template <class RefPartType, class SearchPartType>
      const SearchPartType* findClosestParticle(const RefPartType* reference,
                                                const DataVector<SearchPartType>& candidateContainer) {
          const SearchPartType* best{nullptr};
          for (const SearchPartType* candidate : candidateContainer) {
              if (!best || xAOD::P4Helpers::deltaR2(reference, candidate) <
                           xAOD::P4Helpers::deltaR2(reference, best)) {
                  best = candidate;
              }
          }
          return best;            
      }
  static const SG::Decorator<int> acc_truthMatched{"truthMatched"};

}
namespace MuonValR4{
    StatusCode MuonRecoChainTester::initialize() {
        int evOpts{0};
        if (m_isMC) evOpts |= EventInfoBranch::isMC;
        m_tree.addBranch(std::make_shared<EventInfoBranch>(m_tree, evOpts));

        m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_legacySegmentKey, "LegacySegments", msgLevel()));
        m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_r4PatternSegmentKey, "HoughSegments", msgLevel()));
        m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_segmentKeyR4, "SegmentsR4", msgLevel()));
        if (m_isMC) {
            m_tree.addBranch(std::make_shared<SegmentVariables>(m_tree, m_truthSegmentKey, "TruthSegments", msgLevel()));
        }

        ATH_CHECK(m_legacyTrackKey.initialize());
        ATH_CHECK(m_TrackKeyHoughR4.initialize());
        ATH_CHECK(m_TrackKeyR4.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
        ATH_CHECK(m_truthKey.initialize(m_isMC));

        m_legacyTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "LegacyMSTrks");
        m_legacyTrks->addVariable(std::make_shared<TrackChi2Branch>(*m_legacyTrks));
        m_legacyTrks->addVariable<int>("truthMatched");

        m_TrksHoughR4 = std::make_shared<IParticleFourMomBranch>(m_tree, "HoughMSTrks");
        m_TrksHoughR4->addVariable(std::make_shared<TrackChi2Branch>(*m_TrksHoughR4));
        m_TrksHoughR4->addVariable<int>("truthMatched");

        m_TrksSegmentR4 = std::make_shared<IParticleFourMomBranch>(m_tree, "MSTrksR4");
        m_TrksSegmentR4->addVariable(std::make_shared<TrackChi2Branch>(*m_TrksSegmentR4));
        m_TrksSegmentR4->addVariable<int>("truthMatched");

        m_tree.addBranch(m_legacyTrks);
        m_tree.addBranch(m_TrksSegmentR4);
        m_tree.addBranch(m_TrksHoughR4);
        
        if (m_isMC) {
            m_truthTrks = std::make_shared<IParticleFourMomBranch>(m_tree, "TruthMuons");
            m_truthTrks->addVariable<int>("legacyMatched");
            m_truthTrks->addVariable<int>("houghMatched");
            m_truthTrks->addVariable<int>("r4Matched");
            
            m_tree.addBranch(m_truthTrks);
        }
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    void MuonRecoChainTester::matchTrackToTruth(const xAOD::TruthParticle* truth,
                                                const xAOD::TrackParticleContainer& tracks,
                                                const SG::Decorator<int>& decRecoMatch,
                                                const std::shared_ptr<MuonVal::IParticleFourMomBranch>& trkBranch) const {
      constexpr double matchDR = 0.2;

      decRecoMatch(*truth) = -1;
      const xAOD::TrackParticle* closest = findClosestParticle(truth, tracks);

      if (!closest || xAOD::P4Helpers::deltaR(truth, closest) > matchDR) {
          return;
      }
      // Decorate the truth particle index to the reco particle
      acc_truthMatched(*closest) = static_cast<int>(m_truthTrks->size());
      trkBranch->push_back(closest);
      /// Decorate the reco particle index to the truth particle
      decRecoMatch(*truth) = trkBranch->find(closest); 
    }
    void MuonRecoChainTester::fillBucketsPerStation(const MuonR4::SpacePointContainer& spContainer,
                                                    const StIdx station,
                                                    MuonVal::ScalarBranch<uint16_t>& outBranch) const{
      outBranch = std::count_if(spContainer.begin(), spContainer.end(),
                                [station](const MuonR4::SpacePointBucket* bucket){
                                    return Muon::MuonStationIndex::toStationIndex(bucket->msSector()->chamberIndex()) == station;
                                });
    }

    StatusCode MuonRecoChainTester::execute() {
    
      const SG::Decorator<int> acc_legacyMatched{"legacyMatched"};
      const SG::Decorator<int> acc_houghMatched{"houghMatched"};
      const SG::Decorator<int> acc_r4Matched{"r4Matched"};

      
      const EventContext& ctx{Gaudi::Hive::currentContext()};
      SG::ReadHandle legacyTrks{m_legacyTrackKey, ctx};
      ATH_CHECK(legacyTrks.isPresent());
      SG::ReadHandle trksFromHoughR4{m_TrackKeyHoughR4, ctx};
      ATH_CHECK(trksFromHoughR4.isPresent());      
      SG::ReadHandle trksR4{m_TrackKeyR4, ctx};
      ATH_CHECK(trksR4.isPresent());
      
      for (const xAOD::TrackParticle* trk : *legacyTrks) {
          acc_truthMatched(*trk) = -1;
      }
      for (const xAOD::TrackParticle* trk : *trksR4) {
          acc_truthMatched(*trk) = -1;
      }
      for (const xAOD::TrackParticle* trk : *trksFromHoughR4) {
          acc_truthMatched(*trk) = -1;
      }
      
      ConstDataVector<xAOD::TruthParticleContainer> truthParts{SG::VIEW_ELEMENTS};
      if (!m_truthKey.empty()) {
          SG::ReadHandle readHandle{m_truthKey, ctx};
          ATH_CHECK(readHandle.isPresent());
          for (const xAOD::TruthParticle* truth : *readHandle) {
              if (!truth->isMuon()) continue;
              if (truth->status() != 1) continue;
              truthParts.push_back(truth);
          }
      }
      for (const xAOD::TruthParticle* truth : truthParts) {
          matchTrackToTruth(truth, *legacyTrks, acc_legacyMatched, m_legacyTrks);
          matchTrackToTruth(truth, *trksFromHoughR4, acc_houghMatched, m_TrksHoughR4);
          matchTrackToTruth(truth, *trksR4, acc_r4Matched, m_TrksSegmentR4);
          if (truth->eta() > 2. && acc_legacyMatched(*truth) != -1
              && acc_r4Matched(*truth) == -1 ) {
            ATH_MSG_VERBOSE("In event "<<ctx.eventID()<<" the new chain is inefficient. eta: "
                          <<truth->eta()<<", pT: "<<truth->pt()
                          <<" legacy: "<<acc_legacyMatched(*truth)<<", r4-hough: "<<
                          acc_houghMatched(*truth)<<", r4 seg: "<<acc_r4Matched(*truth));
          }
          m_truthTrks->push_back(truth);
      }
      for (const xAOD::TrackParticle* trk : *legacyTrks) {
          m_legacyTrks->push_back(trk);
      }
      for (const xAOD::TrackParticle* trk : *trksR4) {
          m_TrksSegmentR4->push_back(trk);
      }
      for (const xAOD::TrackParticle* trk : *trksFromHoughR4) {
          m_TrksHoughR4->push_back(trk);
      } 
      /** Fill the bucket summary counts */
      SG::ReadHandle spContainer{m_spacePointKey, ctx};
      ATH_CHECK(spContainer.isPresent());
      m_nBucket = spContainer->size();
      
      fillBucketsPerStation(*spContainer, StIdx::BI, m_nBucketBI);
      fillBucketsPerStation(*spContainer, StIdx::BM, m_nBucketBM);
      fillBucketsPerStation(*spContainer, StIdx::BO, m_nBucketBO);
      fillBucketsPerStation(*spContainer, StIdx::BE, m_nBucketBE);

      fillBucketsPerStation(*spContainer, StIdx::EI, m_nBucketEI);
      fillBucketsPerStation(*spContainer, StIdx::EM, m_nBucketEM);
      fillBucketsPerStation(*spContainer, StIdx::EO, m_nBucketEO);
      fillBucketsPerStation(*spContainer, StIdx::EE, m_nBucketEE);

      
      if(!m_tree.fill(ctx)) {
          return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
    }
    StatusCode MuonRecoChainTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
}
