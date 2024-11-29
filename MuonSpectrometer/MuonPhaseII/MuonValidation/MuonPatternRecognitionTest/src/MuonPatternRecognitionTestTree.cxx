/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPatternRecognitionTestTree.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"

using namespace MuonR4; 

namespace MuonValR4{

    namespace {
        constexpr double c_inv = 1. /Gaudi::Units::c_light;
    }

    StatusCode MuonPatternRecognitionTestTree::initialize(AthHistogramAlgorithm* parent){
        addBranch(std::make_shared<MuonVal::EventInfoBranch>(*this,0));
        ATH_CHECK(init(parent));
        return StatusCode::SUCCESS;
    }
    void MuonPatternRecognitionTestTree::fillChamberInfo(const MuonGMR4::SpectrometerSector* msSector){
        m_out_chamberIndex = msSector->chamberIndex();
        m_out_stationSide = msSector->side();
        m_out_stationPhi = msSector->stationPhi();
    }                
    void MuonPatternRecognitionTestTree:: fillTruthInfo(const xAOD::MuonSegment* segment, const MuonGMR4::MuonDetectorManager* detMgr, const ActsGeometryContext& gctx ) {
        if (!segment) return; 
        m_out_hasTruth = true; 

        const Amg::Vector3D segDir{segment->direction()};
        static const SG::Accessor<float> acc_pt{"pt"};
        static const SG::Accessor<float> acc_charge{"charge"};
        // eta is interpreted as the eta-location 
        m_out_gen_Eta = segDir.eta();
        m_out_gen_Phi = segDir.phi();
        m_out_gen_Pt  = acc_pt(*segment);
        m_out_gen_Q = acc_charge(*segment);

        const auto [chamberPos, chamberDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(*segment));
        m_out_gen_nHits = segment->nPrecisionHits()+segment->nPhiLayers() + segment->nTrigEtaLayers(); 
       
        m_out_gen_nMDTHits = (segment->technology() == Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nNswHits = (segment->technology() != Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nTGCHits = (segment->chamberIndex() > Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);
        m_out_gen_nRPCHits = (segment->chamberIndex() <= Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);

        m_out_gen_tantheta = houghTanTheta(chamberDir); 
        m_out_gen_tanphi   = houghTanPhi(chamberDir);
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
        m_out_gen_time = segment->t0();

        double minYhit = std::numeric_limits<double>::max();
        double maxYhit = -1 * std::numeric_limits<double>::max();
        for (auto & hit : getMatchingSimHits(*segment)){
            auto RE = detMgr->getReadoutElement(hit->identify()); 
            IdentifierHash hash{};
            if (detMgr->idHelperSvc()->isMdt(hit->identify())){
                hash = RE->measurementHash(hit->identify());
            }
            else {
                hash = RE->layerHash(hit->identify());
            }
            auto localToChamber = RE->msSector()->globalToLocalTrans(gctx) * RE->localToGlobalTrans(gctx, hash);
            auto chamberPos = localToChamber * xAOD::toEigen(hit->localPosition()); 
            minYhit = std::min(chamberPos.y(),minYhit); 
            maxYhit = std::max(chamberPos.y(),maxYhit); 
        }
        m_out_gen_minYhit = minYhit;
        m_out_gen_maxYhit = maxYhit;

        ATH_MSG_DEBUG("A true max on chamber index "<<m_out_chamberIndex.getVariable()<<" side "<<m_out_stationSide.getVariable()<<" phi "<<m_out_stationPhi.getVariable()<<" with "
                    <<m_out_gen_nMDTHits.getVariable()<<" MDT and "<<m_out_gen_nRPCHits.getVariable()+m_out_gen_nTGCHits.getVariable()<< " trigger hits is at "
                    <<m_out_gen_tantheta.getVariable()<<" and "<<m_out_gen_y0.getVariable()); 
    }
    void MuonPatternRecognitionTestTree::fillSeedInfo(const ObjectMatching& obj) {
        m_out_seed_n = obj.matchedSeeds.size();
        int iseed = -1; 
        for (auto & seed : obj.matchedSeeds){
            ++iseed;
            if (m_out_bucketEnd.getVariable() < m_out_bucketStart.getVariable()){
                m_out_bucketEnd = seed->parentBucket()->coveredMax();
                m_out_bucketStart = seed->parentBucket()->coveredMin();
            }
            double minYhit = m_out_bucketEnd.getVariable();
            double maxYhit = m_out_bucketStart.getVariable();
            for (auto & hit : seed->getHitsInMax()){
                minYhit = std::min(hit->positionInChamber().y(),minYhit); 
                maxYhit = std::max(hit->positionInChamber().y(),maxYhit); 
            }
            m_out_seed_minYhit.push_back(minYhit);
            m_out_seed_maxYhit.push_back(maxYhit);

            m_out_seed_hasPhiExtension.push_back(seed->hasPhiExtension()); 
            m_out_seed_nMatchedHits.push_back(obj.matchedSeedHits.at(iseed)); 
            m_out_seed_y0.push_back(seed->interceptY());
            m_out_seed_tantheta.push_back(seed->tanTheta());
            if (seed->hasPhiExtension()){
                m_out_seed_x0.push_back(seed->interceptX());
                m_out_seed_tanphi.push_back(seed->tanPhi());
            }
        
            m_out_seed_nHits.push_back(seed->getHitsInMax().size()); 
            m_out_seed_nEtaHits.push_back(std::accumulate(seed->getHitsInMax().begin(), seed->getHitsInMax().end(),0,
                                                [](int i, const HoughHitType & h){i += h->measuresEta();return i;})); 
            m_out_seed_nPhiHits.push_back(std::accumulate(seed->getHitsInMax().begin(), seed->getHitsInMax().end(),0,
                                                [](int i, const HoughHitType & h){i += h->measuresPhi();return i;})); 
            unsigned int nMdtSeed{0}, nRpcSeed{0}, nTgcSeed{0}, nMmSeed{0}, nsTgcSeed{0}; 
            for (const HoughHitType & houghSP: seed->getHitsInMax()){
                switch (houghSP->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType: 
                        ++nMdtSeed;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcSeed+=houghSP->measuresEta();
                        nRpcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcSeed+=houghSP->measuresEta();
                        nTgcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::sTgcStripType:
                        ++nsTgcSeed;
                        break;
                    case xAOD::UncalibMeasType::MMClusterType:
                        ++nMmSeed;
                        break;
                    default:
                        ATH_MSG_WARNING("Technology "<<houghSP->identify()
                                    <<" not yet implemented");                        
                }                    
            }
            m_out_seed_nMdt.push_back(nMdtSeed);
            m_out_seed_nRpc.push_back(nRpcSeed);
            m_out_seed_nTgc.push_back(nTgcSeed);
            m_out_seed_nsTgc.push_back(nsTgcSeed);
            m_out_seed_nMm.push_back(nMmSeed);

            m_out_seed_ledToSegment.push_back(obj.matchedSeedFoundSegment.at(iseed)); 
        }
    }
    
    void MuonPatternRecognitionTestTree::fillSegmentInfo(const ActsGeometryContext& gctx,
                                                   const ObjectMatching& obj){
        using namespace SegmentFit;

        m_out_segment_n = obj.matchedSegments.size(); 
        int iSegment = -1;
        for (auto & segment : obj.matchedSegments){
            ++iSegment; 
            m_out_segment_hasPhi.push_back(std::ranges::find_if(segment->measurements(), [](const auto& meas){  return meas->measuresPhi();}) 
                                !=segment->measurements().end());
            m_out_segment_fitIter.push_back(segment->nFitIterations());
            m_out_segment_truthMatchedHits.push_back(obj.matchedSegHits.at(iSegment));
            m_out_segment_chi2.push_back(segment->chi2());
            m_out_segment_nDoF.push_back(segment->nDoF());
            m_out_segment_hasTimeFit.push_back(segment->hasTimeFit());

            m_out_segment_err_x0.push_back(segment->covariance()(toInt(ParamDefs::x0), toInt(ParamDefs::x0)));
            m_out_segment_err_y0.push_back(segment->covariance()(toInt(ParamDefs::y0), toInt(ParamDefs::y0)));
            m_out_segment_err_tantheta.push_back(segment->covariance()(toInt(ParamDefs::theta), toInt(ParamDefs::theta)));
            m_out_segment_err_tanphi  .push_back(segment->covariance()(toInt(ParamDefs::phi), toInt(ParamDefs::phi)));
            m_out_segment_err_time.push_back(segment->covariance()(toInt(ParamDefs::time), toInt(ParamDefs::time)));
            const auto [locPos, locDir] = makeLine(localSegmentPars(gctx, *segment));
            m_out_segment_tanphi.push_back(houghTanPhi(locDir));
            m_out_segment_tantheta.push_back(houghTanTheta(locDir));
            m_out_segment_y0.push_back(locPos.y());
            m_out_segment_x0.push_back(locPos.x());
            m_out_segment_time.push_back(segment->segementT0() + segment->position().mag() * c_inv);

            unsigned int nMdtHits{0}, nRpcEtaHits{0}, nRpcPhiHits{0}, nTgcEtaHits{0}, nTgcPhiHits{0};
            for (const auto & meas : segment->measurements()){
                switch (meas->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType:
                        ++nMdtHits;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcEtaHits += meas->measuresEta();
                        nRpcPhiHits += meas->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcEtaHits += meas->measuresEta();
                        nTgcPhiHits += meas->measuresPhi();
                        break;
                    default:
                        break;
                }
            }
            m_out_segment_nMdtHits.push_back(nMdtHits);
            m_out_segment_nRpcEtaHits.push_back(nRpcEtaHits);
            m_out_segment_nRpcPhiHits.push_back(nRpcPhiHits);
            m_out_segment_nTgcEtaHits.push_back(nTgcEtaHits);
            m_out_segment_nTgcPhiHits.push_back(nTgcPhiHits);

            double minYhit = std::numeric_limits<double>::max();
            double maxYhit = -1 * std::numeric_limits<double>::max();
            for (auto & hit : segment->measurements()){
                // skip dummy measurement from beam spot constraint
                if (hit->type() == xAOD::UncalibMeasType::Other) continue;
                minYhit = std::min(hit->positionInChamber().y(),minYhit); 
                maxYhit = std::max(hit->positionInChamber().y(),maxYhit); 
            }
            m_out_segment_minYhit.push_back(minYhit);
            m_out_segment_maxYhit.push_back(maxYhit);
        }
    }




} 
