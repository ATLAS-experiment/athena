/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
    2020 Matthias Schott - Uni Mainz
*/

#include "MuonTrackMonitoring/MuonTrackMonitorAlgorithm.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "xAODEventInfo/EventInfo.h"


namespace{
  constexpr double MeVtoGeV = 1.e-3;
}


MuonTrackMonitorAlgorithm::MuonTrackMonitorAlgorithm (const std::string& name, ISvcLocator* pSvcLocator)
    :AthMonitorAlgorithm(name,pSvcLocator){}


StatusCode MuonTrackMonitorAlgorithm::initialize()
{
    ATH_CHECK(AthMonitorAlgorithm::initialize());
    ATH_CHECK(m_MuonContainerKey.initialize());
    ATH_CHECK(m_MuonIsoDecorKey.initialize());
    ATH_CHECK(m_VertexContainerKey.initialize(!m_VertexContainerKey.empty()));
    ATH_CHECK(m_derEventInfoKey.initialize());
    /// https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODEventInfoCnv/src/EventInfoBeamSpotDecoratorAlg.h#L64-78
    
    for (const std::string beam : {"beamPosSigmaX", "beamPosSigmaY", "beamPosSigmaZ", "beamPosSigmaXY"}) {
        m_beamSpotKey.emplace_back(m_derEventInfoKey.key() + "."+beam);
    }
    ATH_CHECK(m_beamSpotKey.initialize(m_useBeamSpot));
    return StatusCode::SUCCESS;
}


//========================================================================================================
StatusCode MuonTrackMonitorAlgorithm::FillTrackInformation(const std::string& sIdentifier, const xAOD::Muon* muon, const xAOD::Vertex *pvtx, const std::string& sTrack, const xAOD::EventInfo &evt) const
{
    
    
    const double beamPosSigmaX  = m_useBeamSpot ? evt.beamPosSigmaX() : -1.;
    const double beamPosSigmaY  = m_useBeamSpot ? evt.beamPosSigmaY() : -1.;
    const double beamPosSigmaXY = m_useBeamSpot ? evt.beamPosSigmaXY() : -1.;

    ///Declaring all track variables
    using namespace Monitored;
    auto tool = getGroup("MuonTrackMonitorAlgorithm");
    auto Author = Monitored::Scalar<float>((sIdentifier+sTrack+"Author").c_str(), -1);
    auto Quality = Monitored::Scalar<float>((sIdentifier+sTrack+"Quality").c_str(), -1);
    auto Type = Monitored::Scalar<float>((sIdentifier+sTrack+"Quality").c_str(), -1);
    auto Eta = Monitored::Scalar<float>((sIdentifier+sTrack+"Eta").c_str(), -9);
    auto Phi = Monitored::Scalar<float>((sIdentifier+sTrack+"Phi").c_str(), -9);
    auto Pt = Monitored::Scalar<float>((sIdentifier+sTrack+"Pt").c_str(), -9);
    auto D0 = Monitored::Scalar<float>((sIdentifier+sTrack+"D0").c_str(), -9);
    auto Z0 = Monitored::Scalar<float>((sIdentifier+sTrack+"Z0").c_str(), -9);
    auto deltaZ0 = Monitored::Scalar<float>((sIdentifier+sTrack+"deltaZ0").c_str(), -9);
    auto D0sig = Monitored::Scalar<float>((sIdentifier+sTrack+"D0sig").c_str(), -9);
    auto chi2ndof = Monitored::Scalar<float>((sIdentifier+sTrack+"chi2ndof").c_str(), -9);

    Author = muon->author();
    Quality = muon->quality();
    Type = muon->type();

    // fill track particle hists
    const xAOD::TrackParticle *tp = nullptr;
    if (sTrack == "ME") {
        tp = muon->trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
    }
    if (sTrack == "MS") {
        tp = muon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);
    }    
    if (tp) {
        Eta = tp->eta();
        Phi = tp->phi();
        Pt = tp->pt() * MeVtoGeV;
        D0 = tp->d0();
        Z0 = tp->z0();
        chi2ndof = tp->chiSquared()/std::max(1.f,tp->numberDoF());

        if (pvtx) {
            deltaZ0 = tp->z0() + tp->vz() - pvtx->z();
        }

        D0sig = m_useBeamSpot ? xAOD::TrackingHelpers::d0significance( tp, beamPosSigmaX, beamPosSigmaY, beamPosSigmaXY ) : -1.;

        fill(tool, Author, Quality, Type, Eta, Phi, Pt, D0, Z0, chi2ndof, deltaZ0, D0sig);
    }
    return StatusCode::SUCCESS;
}


//========================================================================================================
StatusCode MuonTrackMonitorAlgorithm::FillMuonInformation(const std::string& sIdentifier, std::vector<const xAOD::Muon*> &vecMuons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const 
{
    /// Declaring all variables that are initialized via Python will be plotted
    using namespace Monitored;
    auto    tool = getGroup("MuonTrackMonitorAlgorithm");
    auto    MuonEta = Monitored::Scalar<float>((sIdentifier+"MuonEta").c_str(), 0); 
    auto    MuonPhi = Monitored::Scalar<float>((sIdentifier+"MuonPhi").c_str(), 0); 
    auto    MuonEtaTight = Monitored::Scalar<float>((sIdentifier+"MuonEtaTight").c_str(), 0);   
    auto    MuonPhiTight = Monitored::Scalar<float>((sIdentifier+"MuonPhiTight").c_str(), 0);   
    auto    MuonEtaMedium = Monitored::Scalar<float>((sIdentifier+"MuonEtaMedium").c_str(), 0); 
    auto    MuonPhiMedium = Monitored::Scalar<float>((sIdentifier+"MuonPhiMedium").c_str(), 0); 
    auto    MuonD0 = Monitored::Scalar<float>((sIdentifier+"MuonD0").c_str(), 0);   
    auto    MuonZ0 = Monitored::Scalar<float>((sIdentifier+"MuonZ0").c_str(), 0);   
    auto    MuonPt = Monitored::Scalar<float>((sIdentifier+"MuonPt").c_str(), 0);   
    auto    MuonDPTIDME = Monitored::Scalar<float>((sIdentifier+"MuonDPTIDME").c_str(), 0);
    auto    MuonDPTIDMS = Monitored::Scalar<float>((sIdentifier+"MuonDPTIDMS").c_str(), 0);
    auto    MuonDPTIDMECB = Monitored::Scalar<float>((sIdentifier+"MuonDPTIDMECB").c_str(), 0);
    auto    MuonDPTCBME = Monitored::Scalar<float>((sIdentifier+"MuonDPTCBME").c_str(), 0);
    auto    MuonsNBHits = Monitored::Scalar<float>((sIdentifier+"MuonNBHits").c_str(), 0);  
    auto    MuonsNPixHits = Monitored::Scalar<float>((sIdentifier+"MuonNPixHits").c_str(), 0);  
    auto    MuonsNSCTHits = Monitored::Scalar<float>((sIdentifier+"MuonNSCTHits").c_str(), 0);  
    auto    MuonsNTRTHits = Monitored::Scalar<float>((sIdentifier+"MuonNTRTHits").c_str(), 0);  
    auto    MuonsNBHitsAvg = Monitored::Scalar<float>((sIdentifier+"MuonNBHitsAvg").c_str(), 0);    
    auto    MuonsNPixHitsAvg = Monitored::Scalar<float>((sIdentifier+"MuonNPixHitsAvg").c_str(), 0);    
    auto    MuonsNSCTHitsAvg = Monitored::Scalar<float>((sIdentifier+"MuonNSCTHitsAvg").c_str(), 0);    
    auto    MuonsNTRTHitsAvg = Monitored::Scalar<float>((sIdentifier+"MuonNTRTHitsAvg").c_str(), 0);    
    auto    MuonsIDchi2ndof = Monitored::Scalar<float>((sIdentifier+"MuonIDchi2ndof").c_str(), 0);    
    auto    MuonsMEchi2ndof = Monitored::Scalar<float>((sIdentifier+"MuonMEchi2ndof").c_str(), 0);    
    auto    MuonsEtaHitsLayer1 = Monitored::Scalar<float>((sIdentifier+"MuonsEtaHitsLayer1").c_str(), 0);   
    auto    MuonsEtaHitsLayer2 = Monitored::Scalar<float>((sIdentifier+"MuonsEtaHitsLayer2").c_str(), 0);   
    auto    MuonsEtaHitsLayer3 = Monitored::Scalar<float>((sIdentifier+"MuonsEtaHitsLayer3").c_str(), 0);   
    auto    MuonsEtaHitsLayer4 = Monitored::Scalar<float>((sIdentifier+"MuonsEtaHitsLayer4").c_str(), 0);   
    auto    MuonsPhiHitsLayer1 = Monitored::Scalar<float>((sIdentifier+"MuonsPhiHitsLayer1").c_str(), 0);   
    auto    MuonsPhiHitsLayer2 = Monitored::Scalar<float>((sIdentifier+"MuonsPhiHitsLayer2").c_str(), 0);   
    auto    MuonsPhiHitsLayer3 = Monitored::Scalar<float>((sIdentifier+"MuonsPhiHitsLayer3").c_str(), 0);   
    auto    MuonsPhiHitsLayer4 = Monitored::Scalar<float>((sIdentifier+"MuonsPhiHitsLayer4").c_str(), 0);

    auto    LumiBlock = Monitored::Scalar<float>("LumiBlock", 0);
    auto    LumiBlockTrackCategory = Monitored::Scalar<float>("LumiBlockTrackCategory", 0);

    uint32_t lumiBlockID = evt.lumiBlock();
    LumiBlock = lumiBlockID;
    LumiBlockTrackCategory = getTrackCategoryID(sIdentifier);

    /// Loop over all Muons
    for(unsigned int n=0; n<vecMuons.size(); n++) {
        const xAOD::Muon* muon = vecMuons[n];
        xAOD::Muon::MuonType muonType = muon->muonType();
        xAOD::Muon::Quality muonQuality = muon->quality();

        /// Fill ME Track information
        ATH_CHECK ( FillTrackInformation(sIdentifier, muon, pvtx, "ME", evt) );

        /// Basic kinematic Information
        MuonEta = muon->eta();
        MuonPhi = muon->phi();
        MuonPt  = muon->pt() * MeVtoGeV;

        const xAOD::TrackParticle *metp = muon->trackParticle(xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle);
        const xAOD::TrackParticle *idtp = muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        const xAOD::TrackParticle *mstp = muon->trackParticle(xAOD::Muon::MuonSpectrometerTrackParticle);

        fill(tool, LumiBlock, LumiBlockTrackCategory);

        if (muonType==xAOD::Muon::Combined) {
            const xAOD::TrackParticle *cbtp = muon->trackParticle(xAOD::Muon::CombinedTrackParticle);

            if (cbtp) {
                uint8_t hitval_numberOfBLayerHits, hitval_numberOfPixelHits, hitval_numberOfSCTHits, hitval_numberOfTRTHits;
                cbtp->summaryValue(hitval_numberOfBLayerHits,   xAOD::SummaryType::numberOfInnermostPixelLayerHits);
                cbtp->summaryValue(hitval_numberOfPixelHits,    xAOD::SummaryType::numberOfPixelHits);
                cbtp->summaryValue(hitval_numberOfSCTHits,      xAOD::SummaryType::numberOfSCTHits);
                cbtp->summaryValue(hitval_numberOfTRTHits,      xAOD::SummaryType::numberOfTRTHits);        

                MuonZ0  = cbtp->z0();
                MuonD0  = cbtp->d0();

                fill(tool, MuonEta, MuonPhi, MuonPt, MuonZ0, MuonD0);

                /// Hit Information of the ID
                MuonsNBHits     = static_cast<unsigned int>(hitval_numberOfBLayerHits);
                MuonsNPixHits   = static_cast<unsigned int>(hitval_numberOfPixelHits);
                MuonsNSCTHits   = static_cast<unsigned int>(hitval_numberOfSCTHits);
                MuonsNTRTHits   = static_cast<unsigned int>(hitval_numberOfTRTHits);
                fill(tool, MuonsNBHits, MuonsNPixHits, MuonsNSCTHits, MuonsNTRTHits);
                MuonsNBHitsAvg = hitval_numberOfBLayerHits / vecMuons.size();
                MuonsNPixHitsAvg = hitval_numberOfPixelHits / vecMuons.size();
                MuonsNSCTHitsAvg = hitval_numberOfSCTHits / vecMuons.size();
                MuonsNTRTHitsAvg = hitval_numberOfTRTHits / vecMuons.size();
                fill(tool, MuonsNBHitsAvg, MuonsNPixHitsAvg, MuonsNSCTHitsAvg, MuonsNTRTHitsAvg);

                /// Hit Information per layer
                uint8_t hitval_nEtaLayer1{0}, hitval_nEtaLayer2{0}, hitval_nEtaLayer3{0}, hitval_nEtaLayer4{0};
                uint8_t hitval_nPhiLayer1{0}, hitval_nPhiLayer2{0}, hitval_nPhiLayer3{0}, hitval_nPhiLayer4{0};
                muon->summaryValue(hitval_nEtaLayer1, xAOD::MuonSummaryType::etaLayer1Hits);
                muon->summaryValue(hitval_nEtaLayer2, xAOD::MuonSummaryType::etaLayer2Hits);
                muon->summaryValue(hitval_nEtaLayer3, xAOD::MuonSummaryType::etaLayer3Hits);
                muon->summaryValue(hitval_nEtaLayer4, xAOD::MuonSummaryType::etaLayer4Hits);
                muon->summaryValue(hitval_nPhiLayer1, xAOD::MuonSummaryType::phiLayer1Hits);
                muon->summaryValue(hitval_nPhiLayer2, xAOD::MuonSummaryType::phiLayer2Hits);
                muon->summaryValue(hitval_nPhiLayer3, xAOD::MuonSummaryType::phiLayer3Hits);
                muon->summaryValue(hitval_nPhiLayer4, xAOD::MuonSummaryType::phiLayer4Hits);
                MuonsEtaHitsLayer1 = static_cast<unsigned int>(hitval_nEtaLayer1);
                MuonsEtaHitsLayer2 = static_cast<unsigned int>(hitval_nEtaLayer2);
                MuonsEtaHitsLayer3 = static_cast<unsigned int>(hitval_nEtaLayer3);
                MuonsEtaHitsLayer4 = static_cast<unsigned int>(hitval_nEtaLayer4);
                MuonsPhiHitsLayer1 = static_cast<unsigned int>(hitval_nPhiLayer1);
                MuonsPhiHitsLayer2 = static_cast<unsigned int>(hitval_nPhiLayer2);
                MuonsPhiHitsLayer3 = static_cast<unsigned int>(hitval_nPhiLayer3);
                MuonsPhiHitsLayer4 = static_cast<unsigned int>(hitval_nPhiLayer4);
                fill(tool, MuonsEtaHitsLayer1, MuonsEtaHitsLayer2, MuonsEtaHitsLayer3, MuonsEtaHitsLayer4, MuonsPhiHitsLayer1, MuonsPhiHitsLayer2, MuonsPhiHitsLayer3, MuonsPhiHitsLayer4);

                /// Save Eta/Phi Information for medium and tight muons, 
                /// to be used for lates efficiency studies
                if (muonQuality==xAOD::Muon::Medium) {
                    MuonEtaMedium = cbtp->eta();
                    MuonPhiMedium = cbtp->phi();
                    fill(tool, MuonEtaMedium, MuonPhiMedium);
                }
                if (muonQuality==xAOD::Muon::Tight) {
                    MuonEtaTight = cbtp->eta();
                    MuonPhiTight = cbtp->phi();
                    fill(tool, MuonEtaTight, MuonPhiTight);
                }
                /// Momentum Resolution and chi2 studies of MS and ID only tracks
                if (idtp && metp) {
                    MuonDPTIDME     = (idtp->pt() - metp->pt()) / idtp->pt();
                    MuonDPTCBME     = (cbtp->pt() - metp->pt()) / cbtp->pt();
                    MuonDPTIDMECB   = (idtp->pt() - metp->pt()) / cbtp->pt();
                    MuonsIDchi2ndof  = idtp->chiSquared()/std::max(1.f,idtp->numberDoF());
                    MuonsMEchi2ndof  = metp->chiSquared()/std::max(1.f,metp->numberDoF());   
                    fill(tool, MuonDPTIDME, MuonsIDchi2ndof, MuonsMEchi2ndof);
                }
            }
        }
        else {
            const xAOD::TrackParticle *ptp = muon->primaryTrackParticle();
            if (ptp) {
                MuonZ0  = ptp->z0();
                MuonD0  = ptp->d0();

                fill(tool, MuonEta, MuonPhi, MuonPt, MuonZ0, MuonD0);

                // Information on hits in each layer
                uint8_t hitval_numberOfBLayerHits{0}, hitval_numberOfPixelHits{0}, hitval_numberOfSCTHits{0}, hitval_numberOfTRTHits{0};
                ptp->summaryValue(hitval_numberOfBLayerHits, xAOD::SummaryType::numberOfInnermostPixelLayerHits);
                ptp->summaryValue(hitval_numberOfPixelHits, xAOD::SummaryType::numberOfPixelHits);
                ptp->summaryValue(hitval_numberOfSCTHits, xAOD::SummaryType::numberOfSCTHits);
                ptp->summaryValue(hitval_numberOfTRTHits, xAOD::SummaryType::numberOfTRTHits);        
                MuonsNBHits     = static_cast<unsigned int>(hitval_numberOfBLayerHits);
                MuonsNPixHits   = static_cast<unsigned int>(hitval_numberOfPixelHits);
                MuonsNSCTHits   = static_cast<unsigned int>(hitval_numberOfSCTHits);
                MuonsNTRTHits   = static_cast<unsigned int>(hitval_numberOfTRTHits);
                fill(tool, MuonsNBHits, MuonsNPixHits, MuonsNSCTHits, MuonsNTRTHits);
                
                /// Momentum Resolution and chi2 studies of MS and ID only tracks
                if (idtp && metp) {
                    MuonDPTIDME     = (idtp->pt() - metp->pt()) / idtp->pt();
                    MuonsIDchi2ndof  = idtp->chiSquared()/idtp->numberDoF();
                    MuonsMEchi2ndof  = metp->chiSquared()/metp->numberDoF(); 
                    fill(tool, MuonDPTIDME, MuonsIDchi2ndof, MuonsMEchi2ndof);
                }
            }
        }

        /// Count ID, ME and MS muons in LumiBlocks
        auto muonEta = muon->eta();
        if (mstp) {
            if (muonEta > 1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("MS_EA");
            } else if (muonEta > 0) {
                LumiBlockTrackCategory = getTrackCategoryID("MS_BA");
            } else if (muonEta > -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("MS_BC");
            } else if (muonEta <= -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("MS_EC");
            }
            fill(tool, LumiBlock, LumiBlockTrackCategory);
        }
        if (idtp) {
            auto muonEta = muon->eta();
            if (muonEta > 1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ID_EA");
            } else if (muonEta > 0) {
                LumiBlockTrackCategory = getTrackCategoryID("ID_BA");
            } else if (muonEta > -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ID_BC");
            } else if (muonEta <= -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ID_EC");
            }
            fill(tool, LumiBlock, LumiBlockTrackCategory);
        }
        if (metp) {
            auto muonEta = muon->eta();
            if (muonEta > 1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ME_EA");
            } else if (muonEta > 0) {
                LumiBlockTrackCategory = getTrackCategoryID("ME_BA");
            } else if (muonEta > -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ME_BC");
            } else if (muonEta <= -1.05) {
                LumiBlockTrackCategory = getTrackCategoryID("ME_EC");
            }
            fill(tool, LumiBlock, LumiBlockTrackCategory);
        }
    }
    return StatusCode::SUCCESS;
}
    
//========================================================================================================
StatusCode  MuonTrackMonitorAlgorithm::analyseLowLevelMuonFeatures(const std::string& sIdentifier, std::vector<const xAOD::Muon*> &Muons, const xAOD::EventInfo &evt) const 
{
    uint32_t lumiBlockID = evt.lumiBlock();

    using namespace Monitored;

    /// Declaring all variables that are initialized via Python will be plotted
    auto tool = getGroup("MuonTrackMonitorAlgorithm");
    auto MuonAuthor = Monitored::Scalar<float>((sIdentifier+"MuonAuthor").c_str(), 0);
    auto MuonQuality = Monitored::Scalar<float>((sIdentifier+"MuonQuality").c_str(), 0);
    auto MuonType   = Monitored::Scalar<float>((sIdentifier+"MuonType").c_str(), 0);
    auto MuonLargeSectorR = Monitored::Scalar<float>((sIdentifier+"MuonLargeSectorR").c_str(), 0);
    auto MuonLargeSectorZ = Monitored::Scalar<float>((sIdentifier+"MuonLargeSectorZ").c_str(), 0);
    auto MuonSmallSectorR = Monitored::Scalar<float>((sIdentifier+"MuonSmallSectorR").c_str(), 0);
    auto MuonSmallSectorZ = Monitored::Scalar<float>((sIdentifier+"MuonSmallSectorZ").c_str(), 0);
    auto MuonEta = Monitored::Scalar<float>((sIdentifier+"MuonEta").c_str(), 0);
    auto MuonPhi = Monitored::Scalar<float>((sIdentifier+"MuonPhi").c_str(), 0);
    auto MuonPt = Monitored::Scalar<float>((sIdentifier+"MuonPt").c_str(), 0);
    auto MuonEtaHi = Monitored::Scalar<float>((sIdentifier+"MuonEtaHi").c_str(), 0);
    auto MuonPhiHi = Monitored::Scalar<float>((sIdentifier+"MuonPhiHi").c_str(), 0);
    auto MuonPtHi = Monitored::Scalar<float>((sIdentifier+"MuonPtHi").c_str(), 0);
    auto MuonSector = Monitored::Scalar<float>((sIdentifier+"MuonSector").c_str(), 0);  
    auto MuonCIndex = Monitored::Scalar<float>((sIdentifier+"MuonCIndex").c_str(), 0);  
    auto MuonEta1 = Monitored::Scalar<float>((sIdentifier+"MuonEta1All").c_str(), 0);   
    auto MuonPhi1 = Monitored::Scalar<float>((sIdentifier+"MuonPhi1All").c_str(), 0);   
    auto MuonLumiBlock = Monitored::Scalar<float>((sIdentifier+"MuonLumiBlock").c_str(), 0);    
    auto SegmentXPosBarrel = Monitored::Scalar<float>((sIdentifier+"SegmentXPosBarrel").c_str(), 0);
    auto SegmentYPosBarrel = Monitored::Scalar<float>((sIdentifier+"SegmentYPosBarrel").c_str(), 0);
    auto SegmentXPosEndcap = Monitored::Scalar<float>((sIdentifier+"SegmentXPosEndcap").c_str(), 0);
    auto SegmentYPosEndcap = Monitored::Scalar<float>((sIdentifier+"SegmentYPosEndcap").c_str(), 0);

    /// Loop over all muons
    for(const auto muon : Muons) {
        xAOD::Muon::Quality muonQuality = muon->quality();
        xAOD::Muon::MuonType muonType = muon->muonType();
        xAOD::Muon::Author muonAuthor = muon->author();
        MuonLumiBlock = lumiBlockID;
        fill(tool, MuonLumiBlock);

        /// General Muon Control Plots
        MuonAuthor = muonAuthor;
        MuonQuality = muonQuality;
        MuonType = muonType;
        MuonEta = muon->eta();
        MuonPhi = muon->phi();
        MuonPt = muon->pt() * MeVtoGeV;
        fill(tool, MuonAuthor, MuonQuality, MuonType, MuonEta, MuonPhi, MuonPt);

        // Fill high pT plots
        if (muon->pt() > m_CBmuons_minPt) {
            MuonEtaHi = muon->eta();
            MuonPhiHi = muon->phi();
            MuonPtHi = muon->pt() * MeVtoGeV;
            fill(tool, MuonEtaHi, MuonPhiHi, MuonPtHi);
        }
            
        /// Do Muon Segments and Sector Plots
        for (size_t nSeg=0; nSeg < muon->nMuonSegments(); nSeg++) {
            const xAOD::MuonSegment* muonSegment = muon->muonSegment(nSeg);
            if (!muonSegment) {
                continue;
            }
            MuonSmallSectorR = MuonLargeSectorR = std::hypot(muonSegment->x(), muonSegment->y());
            MuonSmallSectorZ = MuonLargeSectorZ = muonSegment->z();
            MuonSector = muonSegment->sector();
            MuonCIndex = muonSegment->chamberIndex();
            int sector = muonSegment->sector();
            if(sector % 2 == 0) {
                fill(tool, MuonLargeSectorZ, MuonLargeSectorR, MuonSector, MuonCIndex);
            } else {
                fill(tool, MuonSmallSectorZ, MuonSmallSectorR, MuonSector, MuonCIndex);
            }
            const double muonSegmentEta = Amg::Vector3D(muonSegment->px(), muonSegment->py(), muonSegment->pz()).eta();
            if (std::abs(muonSegmentEta) > 1.05) {
                SegmentXPosEndcap = muonSegment->x();
                SegmentYPosEndcap = muonSegment->y();
                fill(tool, SegmentXPosEndcap, SegmentYPosEndcap);
            } else {
                SegmentXPosBarrel = muonSegment->x();
                SegmentYPosBarrel = muonSegment->y();
                fill(tool, SegmentXPosBarrel, SegmentYPosBarrel);
            }
        }
    }

    return StatusCode::SUCCESS;
}



//========================================================================================================
StatusCode  MuonTrackMonitorAlgorithm::analyseCombinedTracks(const xAOD::MuonContainer& Muons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const {
    using namespace Monitored;

    /// Declaring all variables that are initialized via Python will be plotted
    auto tool = getGroup("MuonTrackMonitorAlgorithm");
    auto MuonPrefix = Monitored::Scalar<const char*>("MuonPrefix", "");
    auto NMuons = Monitored::Scalar<int>("NMuons", 0);
    auto NMuonsTrig = Monitored::Scalar<int>("NMuonsTrig", 0);
    auto NMuonsTrigCB = Monitored::Scalar<int>("NMuonsTrigCB", 0);
    auto NMuonsTrigNonCB = Monitored::Scalar<int>("NMuonsTrigNonCB", 0);
    auto NMuonsNoTrigCB = Monitored::Scalar<int>("NMuonsNoTrigCB", 0);
    auto NMuonsNoTrigNonCB = Monitored::Scalar<int>("NMuonsNoTrigNonCB", 0);
    auto LumiBlockNumberOfMuonTracks = Monitored::Scalar<float>("MSLumiBlockNumberOfMuonTracks", 0);
    auto LumiBlockNumberOfSegments = Monitored::Scalar<float>("MSLumiBlockNumberOfSegments", 0);

    /// Select Combined Muons
    std::vector<const xAOD::Muon*>  vecAllCombinedMuons;
    std::vector<const xAOD::Muon*>  vecCombinedMuons;
    std::vector<const xAOD::Muon*>  vecNoTrigCombinedMuons;

    /// Select not Combined Muons
    std::vector<const xAOD::Muon*>  vecAllNonCombinedMuons;
    std::vector<const xAOD::Muon*>  vecNonCombinedMuons;
    std::vector<const xAOD::Muon*>  vecNoTrigNonCombinedMuons;

    uint32_t n_muons = 0;
    uint32_t n_muons_trig = 0;
    uint32_t n_muons_trig_cb = 0;
    uint32_t n_muons_trig_noncb = 0;
    uint32_t n_muons_no_trig_cb = 0;
    uint32_t n_muons_no_trig_noncb = 0;
    for(const auto muon : Muons) {
        n_muons++;
        bool isTriggered = false;
        for(const auto& chain : m_hltchainList){
            if(!getTrigDecisionTool().empty() && getTrigDecisionTool()->isPassed( chain ) ){
                isTriggered = true;
            }
        }

        /// Fill MS Track information
        if (isTriggered) {
            ATH_CHECK ( FillTrackInformation("Container", muon, pvtx, "MS", evt) );
        }
        else {
            ATH_CHECK ( FillTrackInformation("ContainerNoTrig", muon, pvtx, "MS", evt) );
        }

        xAOD::Muon::MuonType muonType = muon->muonType();
        if (muonType==xAOD::Muon::Combined) {
            vecAllCombinedMuons.push_back(muon);
            if (isTriggered) {
                vecCombinedMuons.push_back(muon);
                n_muons_trig++;
                n_muons_trig_cb++;
                MuonPrefix = "TrigCB";
            }
            else {
                vecNoTrigCombinedMuons.push_back(muon);
                MuonPrefix = "NoTrigCB";
                n_muons_no_trig_cb++;
            }
        }
        else {
            vecAllNonCombinedMuons.push_back(muon);
            if (isTriggered) {
                vecNonCombinedMuons.push_back(muon);
                n_muons_trig++;
                n_muons_trig_noncb++;
                MuonPrefix = "TrigNonCB";
            }
            else {
                vecNoTrigNonCombinedMuons.push_back(muon);
                MuonPrefix = "NoTrigNonCB";
                n_muons_no_trig_noncb++;
            }
        }
        fill(tool, MuonPrefix);
        for (size_t nSeg=0; nSeg < muon->nMuonSegments(); nSeg++) {
            LumiBlockNumberOfSegments = evt.lumiBlock();
            fill(tool, LumiBlockNumberOfSegments);
        }
    }
    NMuons = n_muons;
    NMuonsTrig = n_muons_trig;
    NMuonsTrigCB = n_muons_trig_cb;
    NMuonsTrigNonCB = n_muons_trig_noncb;
    NMuonsNoTrigCB = n_muons_no_trig_cb;
    NMuonsNoTrigNonCB = n_muons_no_trig_noncb;
    fill(tool, NMuons, NMuonsTrig, NMuonsTrigCB, NMuonsTrigNonCB, NMuonsNoTrigCB, NMuonsNoTrigNonCB);

    LumiBlockNumberOfMuonTracks = evt.lumiBlock();
    fill(tool, LumiBlockNumberOfMuonTracks);

    /// Fill low level Muon Information for each Muon
    ATH_CHECK (analyseLowLevelMuonFeatures("AllCB", vecAllCombinedMuons, evt) );
    ATH_CHECK (analyseLowLevelMuonFeatures("AllNonCB", vecAllNonCombinedMuons, evt) );
    ATH_CHECK (analyseLowLevelMuonFeatures("CB", vecCombinedMuons, evt) );
    ATH_CHECK (analyseLowLevelMuonFeatures("NonCB", vecNonCombinedMuons, evt) );
    ATH_CHECK (analyseLowLevelMuonFeatures("NoTrigCB", vecNoTrigCombinedMuons, evt) );
    ATH_CHECK (analyseLowLevelMuonFeatures("NoTrigNonCB", vecNoTrigNonCombinedMuons, evt) );

    /// Fill the relevant Muon Information for each Muon
    ATH_CHECK (FillMuonInformation("AllCB", vecAllCombinedMuons, pvtx, evt) );
    ATH_CHECK (FillMuonInformation("AllNonCB", vecAllNonCombinedMuons, pvtx, evt) );
    ATH_CHECK (FillMuonInformation("CB", vecCombinedMuons, pvtx, evt) );
    ATH_CHECK (FillMuonInformation("NonCB", vecNonCombinedMuons, pvtx, evt) );
    ATH_CHECK (FillMuonInformation("NoTrigCB", vecNoTrigCombinedMuons, pvtx, evt) );
    ATH_CHECK (FillMuonInformation("NoTrigNonCB", vecNoTrigNonCombinedMuons, pvtx, evt) );

    return StatusCode::SUCCESS;
}


//========================================================================================================
StatusCode  MuonTrackMonitorAlgorithm::plotResonanceCandidates(const std::string& resonanceName, std::vector<const xAOD::Muon*>& muonCandidates, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const {

    uint32_t lumiBlockID = evt.lumiBlock();

    using namespace Monitored;

    /// Declaring all variables that are initialized via Python will be plotted
    auto tool = getGroup("MuonTrackMonitorAlgorithm");
    auto Eta = Monitored::Scalar<float>((resonanceName+"Eta").c_str(), 0);
    auto Mass = Monitored::Scalar<float>((resonanceName+"Mass").c_str(), 0);
    auto MuonLumiBlock = Monitored::Scalar<float>((resonanceName+"MuonLumiBlock").c_str(), 0); 
    auto muMinusEta = Monitored::Scalar<float>((resonanceName+"muMinusEta").c_str(), -9);
    auto muPlusEta = Monitored::Scalar<float>((resonanceName+"muPlusEta").c_str(), -9);
    auto Eta2D = Monitored::Scalar<const char*>((resonanceName+"Eta2D").c_str(), "outside");
    auto Eta2 = Monitored::Scalar<int>((resonanceName+"Eta2").c_str(), -8);

    /// Z Boson related plots   
    std::map<int, int>  mapTagged_Resonance;
    std::vector<const xAOD::Muon*>  vecMuons;
    for (unsigned int n=0; n<muonCandidates.size(); n++)    
        mapTagged_Resonance[n]=0;
    for (unsigned int n=0; n<muonCandidates.size(); n++){
        const TLorentzVector& tVec1 = muonCandidates[n]->p4();
        for (unsigned int m=n+1; m<muonCandidates.size(); m++) {
            const TLorentzVector& tVec2 = muonCandidates[m]->p4();
            const TLorentzVector candidate = tVec1 + tVec2;
            const float resonance_Mass = candidate.M() * MeVtoGeV;
            const float resonance_Eta = candidate.Eta();
            if (muonCandidates[n]->charge()==muonCandidates[m]->charge()) continue;
            if ((candidate.M() < m_ZBosonSelection_minMass)&&(resonanceName=="Z")) continue;
            if ((candidate.M() > m_ZBosonSelection_maxMass)&&(resonanceName=="Z")) continue;
            if ((candidate.M() < m_JpsiSelection_minMass)&&(resonanceName=="Jpsi")) continue;
            if ((candidate.M() > m_JpsiSelection_maxMass)&&(resonanceName=="Jpsi")) continue;

            if (mapTagged_Resonance[n]!=1) vecMuons.push_back(muonCandidates[n]);
            mapTagged_Resonance[n]=1;
            if (mapTagged_Resonance[m]!=1) vecMuons.push_back(muonCandidates[m]);
            mapTagged_Resonance[m]=1;

            if (muonCandidates[n]->charge()<0){
                muMinusEta = tVec1.Eta();
                muPlusEta = tVec2.Eta();
            }
            else{
                muMinusEta = tVec2.Eta();
                muPlusEta = tVec1.Eta();
            }
            const char* EtaReg = "";
            int EtaRegio = -9;
            if ((muMinusEta>1.05)&&(muPlusEta>1.05)){
                EtaReg = "EA_EA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>1.05)&&(muPlusEta>0.)&&(muPlusEta<1.05)){
                EtaReg = "EA_BA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>1.05)&&(muPlusEta>-1.05)&&(muPlusEta<0.)){
                EtaReg = "EA_BC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>1.05)&&(muPlusEta<-1.05)){
                EtaReg = "EA_EC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>0.)&&(muMinusEta<1.05)&&(muPlusEta>1.05)){
                EtaReg = "BA_EA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>0.)&&(muMinusEta<1.05)&&(muPlusEta>0.)&&(muPlusEta<1.05)){
                EtaReg = "BA_BA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>0.)&&(muMinusEta<1.05)&&(muPlusEta>-1.05)&&(muPlusEta<0.)){
                EtaReg = "BA_BC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>0.)&&(muMinusEta<1.05)&&(muPlusEta<-1.05)){
                EtaReg = "BA_EC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>-1.05)&&(muMinusEta<0.)&&(muPlusEta>1.05)){
                EtaReg = "BC_EA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>-1.05)&&(muMinusEta<0.)&&(muPlusEta>0.)&&(muPlusEta<1.05)){
                EtaReg = "BC_BA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>-1.05)&&(muMinusEta<0.)&&(muPlusEta>-1.05)&&(muPlusEta<0.)){
                EtaReg = "BC_BC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta>-1.05)&&(muMinusEta<0.)&&(muPlusEta<-1.05)){
                EtaReg = "BC_EC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta<-1.05)&&(muPlusEta>1.05)){
                EtaReg = "EC_EA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta<-1.05)&&(muPlusEta>0.)&&(muPlusEta<1.05)){
                EtaReg = "EC_BA";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta<-1.05)&&(muPlusEta>-1.05)&&(muPlusEta<0.)){
                EtaReg = "EC_BC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else if ((muMinusEta<-1.05)&&(muPlusEta<-1.05)){
                EtaReg = "EC_EC";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
                Mass2D = resonance_Mass;
                fill(tool, Mass2D);
            } else {
                EtaReg = "out";
                auto Mass2D = Monitored::Scalar<float>((resonanceName+"Mass_"+EtaReg).c_str(), 0);
            }
            Mass = resonance_Mass;
            Eta = resonance_Eta;
            Eta2D = EtaReg;
            Eta2 = EtaRegio;
            fill(tool, Mass, Eta, Eta2, Eta2D, muMinusEta, muPlusEta);
        
            MuonLumiBlock =  lumiBlockID;
            fill(tool, MuonLumiBlock);          
        }
    }

    /// Fill the relevant Muon Information for each Z Boson Candidate Muon
    ATH_CHECK( FillMuonInformation(resonanceName, vecMuons, pvtx, evt) );

    return StatusCode::SUCCESS;
}


//========================================================================================================
StatusCode  MuonTrackMonitorAlgorithm::analyseResonanceCandidates(const xAOD::MuonContainer& Muons, const xAOD::Vertex *pvtx, const xAOD::EventInfo &evt) const {

    std::vector<const xAOD::Muon*>  vecMuons_ZBoson_Candidates;
    std::vector<const xAOD::Muon*>  vecMuons_Jpsi_Candidates;

    /// Select Muons Relevant for Z
    for(const auto muon : Muons) {
        xAOD::Muon::MuonType muonType = muon->muonType();
        if (muonType==xAOD::Muon::Combined) {
            const xAOD::TrackParticle *cbtp = nullptr;
            ElementLink<xAOD::TrackParticleContainer> cbtpLink = muon->combinedTrackParticleLink();
            if (cbtpLink.isValid()) cbtp = *cbtpLink;

            /// Select Z Boson and Jpsi
            if (cbtp) {
                float trkiso  = muon->isolation(xAOD::Iso::ptcone30)/muon->pt();
                if (muonType==xAOD::Muon::Combined &&
                    cbtp &&
                    muon->pt()>m_ZBosonSelection_minPt &&
                    std::abs(muon->eta())<m_ZBosonSelection_maxEta &&
                    trkiso<m_ZBosonSelection_trkIsolation &&
                    std::abs(cbtp->z0())<m_ZBosonSelection_Z0Cut &&
                    std::abs(cbtp->d0())<m_ZBosonSelection_D0Cut )
                        vecMuons_ZBoson_Candidates.push_back(muon);
                if (muonType==xAOD::Muon::Combined &&
                    cbtp &&
                    muon->pt()>m_JpsiSelection_minPt &&
                    std::abs(muon->eta())<m_JpsiSelection_maxEta &&
                    trkiso<m_JpsiSelection_trkIsolation &&
                    std::abs(cbtp->z0())<m_JpsiSelection_Z0Cut &&
                    std::abs(cbtp->d0())<m_JpsiSelection_D0Cut )
                        vecMuons_Jpsi_Candidates.push_back(muon);
            }
        }
    }

    ATH_CHECK( plotResonanceCandidates("Z", vecMuons_ZBoson_Candidates, pvtx, evt) );
    ATH_CHECK( plotResonanceCandidates("Jpsi", vecMuons_Jpsi_Candidates, pvtx, evt) );

    return StatusCode::SUCCESS;
}


//========================================================================================================
StatusCode MuonTrackMonitorAlgorithm::fillHistograms(const EventContext& ctx) const
{
    using namespace Monitored;

    /// Get the EventInfo
    if ((!m_derEventInfoKey.empty()) &&  (!m_MuonContainerKey.empty()) && (!m_VertexContainerKey.empty())) {
        SG::ReadHandle<xAOD::EventInfo> EventInfo{m_derEventInfoKey, ctx};
        if (ATH_UNLIKELY(! EventInfo.isValid())) {
            ATH_MSG_ERROR("Unable to retrieve Event Info " << m_MuonContainerKey);
            return StatusCode::FAILURE;
        }

        const xAOD::Vertex *pvtx = nullptr;
        SG::ReadHandle<xAOD::VertexContainer> Vertices{m_VertexContainerKey, ctx};
        if (!Vertices.isValid()) {
            ATH_MSG_ERROR("Unable to retrieve Vertex container" << m_VertexContainerKey);
            return StatusCode::FAILURE;
        }
        else {
            pvtx = getPrimaryVertex(*Vertices);
        }

        SG::ReadHandle<xAOD::MuonContainer> Muons{m_MuonContainerKey, ctx};
        if (ATH_UNLIKELY(! Muons.isValid())) {
            ATH_MSG_ERROR("Unable to retrieve muon container " << m_MuonContainerKey);
            return StatusCode::FAILURE;
        }

        ATH_CHECK( analyseCombinedTracks(*Muons, pvtx, *EventInfo) );
        ATH_CHECK( analyseResonanceCandidates(*Muons, pvtx, *EventInfo) );

    }

    return StatusCode::SUCCESS;
}


//========================================================================================================
const xAOD::Vertex* MuonTrackMonitorAlgorithm::getPrimaryVertex(const xAOD::VertexContainer& Vertices) const
{
    const xAOD::Vertex *pvtx = nullptr;
    for(const auto vertex : Vertices){
        if (vertex->vertexType() == xAOD::VxType::PriVtx) {
            pvtx = vertex;
        }
    }
    return pvtx;
}

//========================================================================================================
int MuonTrackMonitorAlgorithm::getTrackCategoryID(const std::string& sIdentifier) const
{
    int trackCategoryID = -1;
    
    if (sIdentifier == "NoTrigNonCB") trackCategoryID = 1;
    else if (sIdentifier == "NoTrigCB") trackCategoryID = 2;
    else if (sIdentifier == "NonCB") trackCategoryID = 3;
    else if (sIdentifier == "CB") trackCategoryID = 4;
    else if (sIdentifier == "AllNonCB") trackCategoryID = 5;
    else if (sIdentifier == "AllCB") trackCategoryID = 6;
    // --------------------
    else if (sIdentifier == "Z") trackCategoryID = 8;
    else if (sIdentifier == "Jpsi") trackCategoryID = 9;
    // --------------------
    else if (sIdentifier == "ME_EC") trackCategoryID = 11;
    else if (sIdentifier == "ME_BC") trackCategoryID = 12;
    else if (sIdentifier == "ME_BA") trackCategoryID = 13;
    else if (sIdentifier == "ME_EA") trackCategoryID = 14;
    // --------------------
    else if (sIdentifier == "MS_EC") trackCategoryID = 16;
    else if (sIdentifier == "MS_BC") trackCategoryID = 17;
    else if (sIdentifier == "MS_BA") trackCategoryID = 18;
    else if (sIdentifier == "MS_EA") trackCategoryID = 19;
    // --------------------
    else if (sIdentifier == "ID_EC") trackCategoryID = 21;
    else if (sIdentifier == "ID_BC") trackCategoryID = 22;
    else if (sIdentifier == "ID_BA") trackCategoryID = 23;
    else if (sIdentifier == "ID_EA") trackCategoryID = 24;
    
    return trackCategoryID;
}