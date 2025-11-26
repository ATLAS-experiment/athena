/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// Framework includes
#include "MuonRecoChainTester.h"

#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonTesterTree/MuonTesterTreeDict.h"
#include "MuonTesterTree/TrackChi2Branch.h"
#include "MuonPRDTest/SegmentVariables.h"
#include "MuonPRDTest/ParticleVariables.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "AthContainers/ConstDataVector.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "StoreGate/ReadHandle.h"

using namespace MuonVal;
using namespace MuonPRDTest;

namespace{
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    static const SG::ConstAccessor<SegLink_t> acc_truthSegLink{"truthSegmentLink"};
}


namespace MuonValR4{
    StatusCode MuonRecoChainTester::initialize() {
        int evOpts{0};
        if (m_isMC) evOpts |= EventInfoBranch::isMC;
        m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, evOpts));

        auto initSegBranch = [this](const std::string& sgKey, const std::string& outCollName) {
            if (sgKey.empty()){
                return;
            }
            auto newColl = std::make_unique<SegmentVariables>(m_tree, sgKey, outCollName, msgLevel());
            if (m_isMC) {
                newColl->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                                        std::format("{:}_truthLink", outCollName),[this](const SG::AuxElement* aux){
                                            const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                                            const xAOD::TruthParticle* truthP = MuonR4::getTruthMatchedParticle(*seg);
                                            m_truthTrks->push_back(truthP);
                                            return m_truthTrks->find(truthP);
                                        }));
                newColl->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                    std::format("{:}_truthSegLink", outCollName),[this](const SG::AuxElement* aux) -> unsigned short{
                        if (acc_truthSegLink.isAvailable(*aux) && acc_truthSegLink(*aux).isValid()) {
                            const xAOD::MuonSegment* truthSeg {*acc_truthSegLink(*aux)};
                            return m_truthSegs->push_back(*truthSeg);
                        }
                        return -1;
                    }));
            }
            m_tree.addBranch(std::move(newColl));
        };

        initSegBranch(m_legacySegmentKey, "LegacySegments");
        initSegBranch(m_r4PatternSegmentKey, "HoughSegments");
        initSegBranch(m_segmentKeyR4, "SegmentsR4");

        ATH_CHECK(m_truthSegmentKey.initialize(m_isMC));
        if (m_isMC) {
            m_truthSegs = std::make_unique<SegmentVariables>(m_tree, m_truthSegmentKey.key(), "TruthSegments", msgLevel());
            m_tree.addBranch(m_truthSegs);
        }

        ATH_CHECK(m_legacyTrackKey.initialize());
        ATH_CHECK(m_TrackKeyHoughR4.initialize(m_r4PatternSegmentKey.value().size()));
        ATH_CHECK(m_TrackKeyR4.initialize());
        ATH_CHECK(m_spacePointKey.initialize());
        ATH_CHECK(m_truthKey.initialize(m_isMC));

        m_legacyTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "LegacyMSTrks");
        m_legacyTrks->addVariable(std::make_unique<TrackChi2Branch>(*m_legacyTrks));

        if (!m_TrackKeyHoughR4.empty()){
            m_TrksHoughR4 = std::make_unique<IParticleFourMomBranch>(m_tree, "HoughMSTrks");
            m_TrksHoughR4->addVariable(std::make_unique<TrackChi2Branch>(*m_TrksHoughR4));
            m_tree.addBranch(m_TrksHoughR4);

        }
        m_TrksSegmentR4 = std::make_unique<IParticleFourMomBranch>(m_tree, "MSTrksR4");
        m_TrksSegmentR4->addVariable(std::make_unique<TrackChi2Branch>(*m_TrksSegmentR4));

        m_tree.addBranch(m_legacyTrks);
        m_tree.addBranch(m_TrksSegmentR4);
        
        if (m_isMC) {
            m_trkTruthLinks.emplace_back(m_legacyTrackKey, "truthParticleLink");
            if (!m_TrackKeyHoughR4.empty()) {
                m_trkTruthLinks.emplace_back(m_TrackKeyHoughR4, "truthParticleLink");
                m_trkTruthLinks.emplace_back(std::format("{:}.truthParticleLink", m_r4PatternSegmentKey.value()));
                BilateralLinkerBranch::connectCollections(m_TrksHoughR4, m_truthTrks, [](const xAOD::IParticle* trk){ 
                                                          return xAOD::TruthHelpers::getTruthParticle(*trk); }, "truth", "HoughMS");
            }
            m_trkTruthLinks.emplace_back(m_TrackKeyR4, "truthParticleLink");
            m_trkTruthLinks.emplace_back(m_truthSegmentKey, "truthParticleLink");
    
            m_trkTruthLinks.emplace_back(std::format("{:}.truthParticleLink", m_segmentKeyR4.value()));

            m_truthTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "TruthMuons");
            BilateralLinkerBranch::connectCollections(m_legacyTrks, m_truthTrks, [](const xAOD::IParticle* trk){ 
                                                      return xAOD::TruthHelpers::getTruthParticle(*trk); }, "truth", "LegacyMS");
            BilateralLinkerBranch::connectCollections(m_TrksSegmentR4, m_truthTrks, [](const xAOD::IParticle* trk){ 
                                                      return xAOD::TruthHelpers::getTruthParticle(*trk); }, "truth", "MSTrksR4");
            m_tree.addBranch(m_truthTrks);
        } 

        ATH_CHECK(m_trkTruthLinks.initialize());
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
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
    
      const EventContext& ctx{Gaudi::Hive::currentContext()};
      const xAOD::TrackParticleContainer* legacyTrks{nullptr};
      ATH_CHECK(SG::get(legacyTrks, m_legacyTrackKey, ctx));

      const xAOD::TrackParticleContainer* trksFromHoughR4{nullptr};
      ATH_CHECK(SG::get(trksFromHoughR4, m_TrackKeyHoughR4, ctx));

      const xAOD::TrackParticleContainer* trksR4{nullptr};
      ATH_CHECK(SG::get(trksR4, m_TrackKeyR4, ctx));
      
      ATH_MSG_DEBUG("Fill reconstructed tracks from "<<m_legacyTrackKey.fullKey());
      for (const xAOD::TrackParticle* trk : *legacyTrks) {
        m_legacyTrks->push_back(trk);
      }
      ATH_MSG_DEBUG("Fill reconstructed tracks from "<<m_TrackKeyR4.fullKey());
      for (const xAOD::TrackParticle* trk : *trksR4) {
          m_TrksSegmentR4->push_back(trk);
      }
      ATH_MSG_DEBUG("Fill reconstructed tracks from "<<m_TrackKeyHoughR4.fullKey());
      if (trksFromHoughR4) {
        for (const xAOD::TrackParticle* trk : *trksFromHoughR4) {
            m_TrksHoughR4->push_back(trk);
        } 
      }
      if (!m_truthKey.empty()) {
          const xAOD::TruthParticleContainer* truthCont{nullptr};
          ATH_CHECK(SG::get(truthCont, m_truthKey, ctx));
          for (const xAOD::TruthParticle* truth : *truthCont) {
            m_truthTrks->push_back(truth);
        }
        const xAOD::MuonSegmentContainer* truthSeg{nullptr};
        ATH_CHECK(SG::get(truthSeg, m_truthSegmentKey, ctx));
        for (const xAOD::MuonSegment* seg : *truthSeg) {
            m_truthSegs->push_back(*seg);
        }
      }
      /** Fill the bucket summary counts */
      const MuonR4::SpacePointContainer* spContainer{nullptr};
      ATH_CHECK(SG::get(spContainer, m_spacePointKey, ctx));
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
