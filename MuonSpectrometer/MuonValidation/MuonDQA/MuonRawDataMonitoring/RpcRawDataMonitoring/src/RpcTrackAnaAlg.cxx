/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// C/C++
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>

// root
#include "TObjArray.h"

// Athena
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadDecorHandle.h"
#include "xAODTracking/TrackParticlexAODHelpers.h"

// Boost package to read XML
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Local
#include "RpcTrackAnaAlg.h"

//================================================================================================
RpcTrackAnaAlg::RpcTrackAnaAlg(const std::string& name,
                               ISvcLocator* pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

//================================================================================================
RpcTrackAnaAlg::~RpcTrackAnaAlg() {}

//================================================================================================
StatusCode RpcTrackAnaAlg::initialize() {
    ATH_MSG_INFO(" RpcTrackAnaAlg initialize begin ");
    ATH_CHECK(AthMonitorAlgorithm::initialize());

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_beamSigmaX.initialize());
    ATH_CHECK(m_beamSigmaY.initialize());
    ATH_CHECK(m_beamSigmaXY.initialize());

    ATH_CHECK(m_MuonRoIContainerKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_MuonContainerKey.initialize());
    ATH_CHECK(m_rpcPrdKey.initialize());
    ATH_CHECK(m_PrimaryVertexContainerKey.initialize(SG::AllowEmpty));

    ATH_CHECK(readElIndexFromXML());
    ATH_CHECK(initRpcPanel());

    ATH_CHECK(initTrigTag());

    std::vector<std::string> sectorStr = {
        "sector1",  "sector2",  "sector3",  "sector4",  "sector5",  "sector6",
        "sector7",  "sector8",  "sector9",  "sector10", "sector11", "sector12",
        "sector13", "sector14", "sector15", "sector16"};
    m_SectorGroup =
        Monitored::buildToolMap<int>(m_tools, "RpcTrackAnaAlg", sectorStr);

    std::vector<std::string> triggerThrs = {"thr1", "thr2", "thr3",
                                            "thr4", "thr5", "thr6"};
    m_TriggerThrGroup =
        Monitored::buildToolMap<int>(m_tools, "RpcTrackAnaAlg", triggerThrs);

    ATH_MSG_INFO(" initialize extrapolator ");
    ATH_CHECK(m_extrapolator.retrieve());

    ATH_MSG_INFO(" RpcTrackAnaAlg initialize END ");
    return StatusCode::SUCCESS;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::initRpcPanel() {
    /*
      Iterate over all RpcDetectorElements and RpcReadoutElements
      and cache locally all panel
    */
    ATH_MSG_INFO(name() << " - RpcTrackAnaAlg::initRpcPanel - start");

    unsigned int nValidPanel = 0, nTriedPanel{0};
    const RpcIdHelper& rpcIdHelper = m_idHelperSvc->rpcIdHelper();
    const MuonGM::MuonDetectorManager* muonMgr{nullptr};
    ATH_CHECK(detStore()->retrieve(muonMgr));

    ATH_MSG_INFO("MuonGM::MuonDetectorManager::RpcDetElMaxHash= "
                 << rpcIdHelper.module_hash_max());

    m_StationNames[BI] = {};
    m_StationNames[BM1] = {2, 3, 8, 53};  // doubletR = 1
    m_StationNames[BM2] = {2, 3, 8, 53};  // doubletR = 2
    m_StationNames[BO1] = {4, 5, 9, 10};  // doubletR = 1
    m_StationNames[BO2] = {4, 5, 9, 10};  // doubletR = 2

    std::vector<int> BMBO_StationNames = {2, 3, 4, 5, 8, 9, 10, 53};
    for (unsigned idetEl = 0;
         idetEl < MuonGM::MuonDetectorManager::RpcRElMaxHash; ++idetEl) {
        IdentifierHash hash{idetEl};
        const MuonGM::RpcReadoutElement* readoutEl =
            muonMgr->getRpcReadoutElement(hash);
        if (!readoutEl)
            continue;

        const unsigned int ngasgap = 2;
        const unsigned int doubletZ = readoutEl->getDoubletZ();
        if (readoutEl->getDoubletPhi() != 1) {
            continue;
        }

        const Identifier readEl_id = readoutEl->identify();

        int stName = rpcIdHelper.stationName(readEl_id);

        if (!std::count(BMBO_StationNames.begin(), BMBO_StationNames.end(),
                        stName)) {
            continue;  // Will be changed to include BIS
        }

        if (!std::count(BMBO_StationNames.begin(), BMBO_StationNames.end(),
                        stName)) {
            continue;  // Will be changed to include BIS
        }

        for (unsigned doubletPhi = 1; doubletPhi <= 2; ++doubletPhi) {
            for (unsigned gasgap = 1; gasgap <= ngasgap; ++gasgap) {
                std::shared_ptr<GasGapData> gap = std::make_shared<GasGapData>(
                    *m_idHelperSvc, readoutEl, doubletZ, doubletPhi, gasgap);

                std::pair<int, int> st_dbR = std::make_pair(stName, gap->doubletR);
                m_gasGapData[st_dbR].push_back(gap);

                std::shared_ptr<RpcPanel> rpcPanel_eta = std::make_shared<RpcPanel>(
                    *m_idHelperSvc, readoutEl, doubletZ, doubletPhi, gasgap, 0);
                ATH_CHECK(setPanelIndex(rpcPanel_eta));

                std::shared_ptr<RpcPanel> rpcPanel_phi = std::make_shared<RpcPanel>(
                    *m_idHelperSvc, readoutEl, doubletZ, doubletPhi, gasgap, 1);
                ATH_CHECK(setPanelIndex(rpcPanel_phi));

                if (rpcPanel_eta->panel_valid) {
                    ATH_MSG_DEBUG(" Panel  stationName:"
                                  << rpcPanel_eta->stationName
                                  << " stationEta:" << rpcPanel_eta->stationEta
                                  << " stationPhi:" << rpcPanel_eta->stationPhi
                                  << " doubletR:" << rpcPanel_eta->doubletR
                                  << "doubletZ:" << rpcPanel_eta->doubletZ
                                  << " doubletPhi:" << rpcPanel_eta->doubletPhi
                                  << " gasGap:" << rpcPanel_eta->gasGap
                                  << " measPhi:" << rpcPanel_eta->measPhi);

                    m_rpcPanelMap.insert(
                        std::map<Identifier, std::shared_ptr<RpcPanel>>::value_type(
                            rpcPanel_eta->panelId, rpcPanel_eta));
                    nValidPanel++;
                }

                if (rpcPanel_phi->panel_valid) {
                    ATH_MSG_DEBUG(" Panel  stationName:"
                                  << rpcPanel_phi->stationName
                                  << " stationEta:" << rpcPanel_phi->stationEta
                                  << " stationPhi:" << rpcPanel_phi->stationPhi
                                  << " doubletR:" << rpcPanel_phi->doubletR
                                  << "doubletZ:" << rpcPanel_phi->doubletZ
                                  << " doubletPhi:" << rpcPanel_phi->doubletPhi
                                  << " gasGap:" << rpcPanel_phi->gasGap
                                  << " measPhi:" << rpcPanel_phi->measPhi);

                    m_rpcPanelMap.insert(
                        std::map<Identifier, std::shared_ptr<RpcPanel>>::value_type(
                            rpcPanel_phi->panelId, rpcPanel_phi));
                    nValidPanel++;
                }

                gap->RpcPanel_eta_phi = std::make_pair(rpcPanel_eta, rpcPanel_phi);
                ++nTriedPanel;
            }
        }
    }

    ATH_MSG_INFO("Number of valid panels = " << nValidPanel << " tried panels "
                                             << nTriedPanel);
    return StatusCode::SUCCESS;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::setPanelIndex(std::shared_ptr<RpcPanel> panel) {
    /*
      Specify every panel with a unique index according the information in xml
    */
    std::string ele_str = panel->getElementStr();

    std::map<std::string, int>::iterator it_eleInd =
        m_elementIndex.find(ele_str);

    if (it_eleInd == m_elementIndex.end()) {
        ATH_MSG_ERROR("Can not find element: stationName = "
                      << panel->stationName
                      << ", stationEta = " << panel->stationEta
                      << ", stationPhi = " << panel->stationPhi
                      << ", doubletR = " << panel->doubletR
                      << ", doubletZ = " << panel->doubletZ);
        return StatusCode::FAILURE;
    }

    int ele_index = it_eleInd->second;
    int panel_index = (ele_index - 1) * 8 + (panel->doubletPhi - 1) * 4 +
                      (panel->gasGap - 1) * 2 + panel->measPhi;

    panel->SetPanelIndex(panel_index);

    return StatusCode::SUCCESS;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::readElIndexFromXML() {
    /*
      Read xml file in share/Element.xml to give each element(in the afterwards,
      for panel) a index
    */
    ATH_MSG_INFO(name() << " - read element index ");

    //
    // Read xml file
    //
    const std::string xml_file =
        PathResolver::find_file(m_elementsFileName, "DATAPATH");
    if (xml_file.empty()) {
        ATH_MSG_ERROR(m_elementsFileName << " not found!");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO(name() << " - read xml file: " << xml_file);

    boost::property_tree::ptree pt;
    read_xml(xml_file, pt);

    for (boost::property_tree::ptree::value_type& child:
                   pt.get_child("Elements")) {
        if (child.first == "Element") {
            int index = child.second.get<int>("<xmlattr>.index");
            int stName = child.second.get<int>("<xmlattr>.stationName");
            int stEta = child.second.get<int>("<xmlattr>.stationEta");
            int stPhi = child.second.get<int>("<xmlattr>.stationPhi");
            int dbR = child.second.get<int>("<xmlattr>.doubletR");
            int dbZ = child.second.get<int>("<xmlattr>.doubletZ");

            ATH_MSG_DEBUG(" index = " << index << ", stationName = " << stName
                                      << ", stationEta = " << stEta
                                      << ", stationPhi = " << stPhi
                                      << ", doubletR = " << dbR
                                      << ", doubletZ = " << dbZ);
            std::ostringstream ele_key;

            ele_key << stName << "_" << stEta << "_" << stPhi << "_" << dbR
                    << "_" << dbZ;

            std::map<std::string, int>::iterator it_eleInd =
                m_elementIndex.find(ele_key.str());
            if (it_eleInd == m_elementIndex.end()) {
                m_elementIndex.insert(std::map<std::string, int>::value_type(
                    ele_key.str(), index));
            } else {
                ATH_MSG_ERROR("These is duplicated elements");
            }
        } else {
            ATH_MSG_ERROR(" node key != Element");
        }
    }

    ATH_MSG_INFO(
        "Total number of elements: "
        << m_elementIndex.size()
        << ", should be consistent with the number of elements in xml file!");

    return StatusCode::SUCCESS;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::initTrigTag() {
    TString trigStr = m_trigTagList.value();
    std::unique_ptr<TObjArray> tagList(trigStr.Tokenize(";"));

    // TObjArray* tagList = TString(m_trigTagList.value()).Tokenize(",");
    std::set<TString> alllist;
    for (int i = 0; i < tagList->GetEntries(); i++) {
        TString tagTrig = tagList->At(i)->GetName();
        if (alllist.find(tagTrig) != alllist.end())
            continue;
        alllist.insert(tagTrig);
        std::unique_ptr<TObjArray> arr(tagTrig.Tokenize(";"));
        if (arr->GetEntries() == 0)
            continue;
        TagDef def;
        def.eventTrig = TString(arr->At(0)->GetName());
        def.tagTrig = def.eventTrig;
        if (arr->GetEntries() == 2)
            def.tagTrig = TString(arr->At(1)->GetName());
        m_trigTagDefs.push_back(def);
    }

    return StatusCode::SUCCESS;
}

//================================================================================================
StatusCode RpcTrackAnaAlg::fillHistograms(const EventContext& ctx) const {
    using namespace Monitored;

    if (m_plotMuonEff) {
        ATH_CHECK(fillMuonExtrapolateEff(ctx));
    }

    if (m_plotPRD) {
        ATH_CHECK(fillHistPRD(ctx));
    }

    auto tool = getGroup(m_packageName);
    auto evtLB = Scalar<int>("evtLB", ctx.eventID().lumi_block());
    auto run = Scalar<int>("run", ctx.eventID().run_number());
    fill(tool, evtLB, run);

    return StatusCode::SUCCESS;
}

//================================================================================================
StatusCode RpcTrackAnaAlg::fillMuonExtrapolateEff(
    const EventContext& ctx) const {
    using namespace Monitored;
    auto tool = getGroup(m_packageName);

    //
    // read PrimaryVertex Z
    //
    const xAOD::Vertex* primVertex = nullptr;
    if (!m_PrimaryVertexContainerKey.empty()) {
        SG::ReadHandle<xAOD::VertexContainer> primVtxContainer(
            m_PrimaryVertexContainerKey, ctx);
        if (primVtxContainer.isValid()) {
            for (const auto vtx : *primVtxContainer) {
                if (vtx->vertexType() == xAOD::VxType::VertexType::PriVtx) {
                    primVertex = vtx;
                    break;
                }
            }
        }
    }
    const double primaryVertexZ = primVertex ? (primVertex->z()) : (-999);

    //
    // read beam position sigma from eventinfo
    //
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamSigmaX(m_beamSigmaX, ctx);
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamSigmaY(m_beamSigmaY, ctx);
    SG::ReadDecorHandle<xAOD::EventInfo, float> beamSigmaXY(m_beamSigmaXY, ctx);

    const float beamPosSigmaX = beamSigmaX(0);
    const float beamPosSigmaY = beamSigmaY(0);
    const float beamPosSigmaXY = beamSigmaXY(0);

    //
    // read muon
    //
    SG::ReadHandle<xAOD::MuonContainer> muons(m_MuonContainerKey, ctx);
    ATH_MSG_DEBUG(" muons size = " << muons->size());

    if (!muons.isValid()) {
        ATH_MSG_ERROR("evtStore() does not contain muon Collection with name "
                      << m_MuonContainerKey);
        return StatusCode::FAILURE;
    }

    //
    // select out tag and probe muons
    //
    std::vector<std::shared_ptr<MyMuon>> tagmuons;
    std::vector<std::shared_ptr<MyMuon>> probemuons;
    for (const xAOD::Muon* muon : *muons) {
        if (std::abs(muon->eta()) > m_maxEta)
            continue;

        auto mymuon = std::make_shared<MyMuon>();
        mymuon->muon = muon;
        mymuon->fourvec.SetPtEtaPhiM(muon->pt(), muon->eta(), muon->phi(),
                                     m_muonMass.value());
        mymuon->tagged =
            triggerMatching(muon, m_trigTagDefs) == StatusCode::SUCCESS;

        // muon quality
        if (muon->quality() > xAOD::Muon::Medium)
            continue;

        //
        // calculate muon z0sin(\theta) and d0significance
        auto track = muon->primaryTrackParticle();
        const double z0 = track->z0() + track->vz() - primaryVertexZ;

        double z0sin = z0 * std::sin(track->theta());
        double d0sig = xAOD::TrackingHelpers::d0significance(
            track, beamPosSigmaX, beamPosSigmaY, beamPosSigmaXY);

        // select probe muons
        if (std::abs(z0sin) < 0.5 && std::abs(d0sig) < 3.0) {
            mymuon->tagProbeOK = true;
        }
        probemuons.push_back(mymuon);

        // select tag muons
        if (muon->pt() > 27.0e3 && std::abs(z0sin) < 1.0 &&
            std::abs(d0sig) < 5.0) {
            tagmuons.push_back(mymuon);
        }
    }

    //
    // Z tag & probe
    //
    for (auto tag_muon : tagmuons) {
        if (!(tag_muon->tagged))
            continue;

        for (auto probe_muon : probemuons) {
            if (tag_muon->muon == probe_muon->muon)
                continue;

            // probe muon
            if (!probe_muon->tagProbeOK)
                continue;

            // Opposite charge
            if (tag_muon->muon->charge() == probe_muon->muon->charge())
                continue;

            // Z mass window
            float dimuon_mass = (tag_muon->fourvec + probe_muon->fourvec).M();
            if (dimuon_mass < m_zMass_lowLimit || dimuon_mass > m_zMass_upLimit)
                continue;

            // angular separation
            float dr = (tag_muon->fourvec).DeltaR(probe_muon->fourvec);
            if (dr < m_isolationWindow)
                continue;

            probe_muon->tagProbeAndZmumuOK = true;
        }
    }

    //
    // read rois: raw LVL1MuonRoIs
    //
    std::vector<const xAOD::MuonRoI*> roisBarrel;
    if (!m_MuonRoIContainerKey.empty()) {
        SG::ReadHandle<xAOD::MuonRoIContainer> muonRoIs(m_MuonRoIContainerKey,
                                                        ctx);

        if (!muonRoIs.isValid()) {
            ATH_MSG_ERROR(
                "evtStore() does not contain muon L1 ROI Collection with name "
                << m_MuonRoIContainerKey);
            return StatusCode::FAILURE;
        }
        const xAOD::MuonRoIContainer* rois = muonRoIs.cptr();

        std::vector<float> roiEtaVec{}, roiBarrelEtaVec{};
        std::vector<int> roiBarrelThrVec{};

        roiEtaVec.reserve(muonRoIs->size());
        roiBarrelEtaVec.reserve(muonRoIs->size());
        roiBarrelThrVec.reserve(muonRoIs->size());
        roisBarrel.reserve(muonRoIs->size());
        for (const xAOD::MuonRoI* roi : *rois) {
            roiEtaVec.push_back(roi->eta());
            if (roi->getSource() != xAOD::MuonRoI::RoISource::Barrel)
                continue;

            roiBarrelEtaVec.push_back(roi->eta());
            roiBarrelThrVec.push_back(roi->getThrNumber());
            roisBarrel.push_back(roi);
        }

        auto roiEtaCollection = Collection("roiEta", roiEtaVec);
        auto roiBarrelEtaCollection =
            Collection("roiBarrelEta", roiBarrelEtaVec);
        auto roiBarrelThrCollection =
            Collection("roiBarrelThr", roiBarrelThrVec);
        fill(tool, roiEtaCollection);
        fill(tool, roiBarrelEtaCollection);
        fill(tool, roiBarrelThrCollection);
    }

    //
    // Fill Lv1 trigger efficiency
    // Fill muon detection efficiency
    auto i_pt_allMu = Scalar<double>("muPt_allMu", 0.);
    auto i_eta_allMu = Scalar<double>("muEta_allMu", 0.);
    auto i_phi_allMu = Scalar<double>("muPhi_allMu", 0.);
    auto i_pt_zMu = Scalar<double>("muPt_MuonFromZ", 0.);
    auto i_eta_zMu = Scalar<double>("muEta_MuonFromZ", 0.);
    auto i_phi_zMu = Scalar<double>("muPhi_MuonFromZ", 0.);
    auto i_eta_zMu_p =
        Scalar<double>("muEta_p_MuonFromZ",
                       0.);  // kine variable at the plateau of pt turn-on curve
    auto i_phi_zMu_p =
        Scalar<double>("muPhi_p_MuonFromZ",
                       0.);  // kine variable at the plateau of pt turn-on curve

    std::vector<GasGapResult> results;
    int nmuon{0}, nmuon_barrel{0};
    for (const auto& probe_muon : probemuons) {
        nmuon++;
        double pt = probe_muon->muon->pt();
        double eta = probe_muon->muon->eta();
        double phi = probe_muon->muon->phi();

        // barrel muon
        if (std::abs(eta) < m_barrelMinEta || std::abs(eta) > m_barrelMaxEta)
            continue;
        nmuon_barrel++;

        // has MuonSpectrometerTrackParticle
        const xAOD::TrackParticle* track = probe_muon->muon->trackParticle(
            xAOD::Muon::MuonSpectrometerTrackParticle);
        if (track) {
            results.clear();

            ATH_CHECK(extrapolate2RPC(track, Trk::anyDirection, results, BI));
            ATH_CHECK(readHitsPerGasgap(ctx, results, AllMuon));
        }

        // fill kine variables for probe muons
        i_pt_allMu = pt;
        i_eta_allMu = eta;
        i_phi_allMu = phi;
        fill(tool, i_pt_allMu, i_eta_allMu, i_phi_allMu);

        //
        // Probe muons with Z tag&probe method
        // 1) Plot L1 trigger efficiency use probe muons
        // 2) Plot muon detection efficiency use probe muons
        if (probe_muon->tagProbeAndZmumuOK) {
            //
            // do match muon with ROI
            std::vector<bool> isMatcheds(6, 0);
            for (const xAOD::MuonRoI* roi : roisBarrel) {
                const double dphi = TVector2::Phi_mpi_pi(roi->phi() - phi);
                const double deta = roi->eta() - eta;
                const double dr = std::hypot(dphi, deta);

                if (dr > m_l1trigMatchWindow)
                    continue;

                int thr = std::min(7, roi->getThrNumber());
                for (int i_thr = 1; i_thr <= thr; i_thr++) {
                    isMatcheds[i_thr - 1] = true;
                }
            }

            //
            // fill L1 trigger Efficiency
            auto i_pt = Scalar<double>("muPt_l1", pt);
            auto i_eta = Scalar<double>("muEta_l1", eta);
            auto i_phi = Scalar<double>("muPhi_l1", phi);
            auto i_passTrigger = Scalar<bool>("passTrigger", false);
            auto i_passTrigger_1 = Scalar<bool>("passTrigger_plateau", false);

            for (int i_thr = 1; i_thr < 7; i_thr++) {
                i_passTrigger = isMatcheds[i_thr - 1];
                i_passTrigger_1 = isMatcheds[i_thr - 1];

                // plot L1 trigger pt turn-on curve
                fill(m_tools[m_TriggerThrGroup.at("thr" +
                                                  std::to_string(i_thr))],
                     i_pt, i_passTrigger);

                // plot L1 trigger efficiency on the plateau of pt turn-on curve
                if (std::abs(pt) > m_minPt) {
                    fill(m_tools[m_TriggerThrGroup.at("thr" +
                                                      std::to_string(i_thr))],
                         i_eta, i_phi, i_passTrigger_1);
                }
            }

            //
            // muon track
            // fill muon detection Efficiency
            //
            if (track) {
                ATH_CHECK(readHitsPerGasgap(ctx, results, ZCand));
            }

            // fill kine variables for probe muons in ZTP
            i_pt_zMu = pt;
            i_eta_zMu = eta;
            i_phi_zMu = phi;
            fill(tool, i_pt_zMu, i_eta_zMu, i_phi_zMu);

            if (std::abs(pt) > m_minPt) {
                i_eta_zMu_p = eta;
                i_phi_zMu_p = phi;
                fill(tool, i_eta_zMu_p, i_phi_zMu_p);
            }
        }  // tagProbeAndZmumuOK
    }      // probemuons

    auto Nmuon = Scalar<int>("nMu", nmuon);
    auto Nmuon_barrel = Scalar<int>("nMuBarrel", nmuon_barrel);

    //
    // Fill histograms
    //
    fill(tool, Nmuon, Nmuon_barrel);

    return StatusCode::SUCCESS;
}

//================================================================================================
StatusCode RpcTrackAnaAlg::fillHistPRD(const EventContext& ctx) const {
    using namespace Monitored;
    //
    // Read RPC Prepare data
    //

    SG::ReadHandle<Muon::RpcPrepDataContainer> rpcContainer(m_rpcPrdKey, ctx);
    const RpcIdHelper& rpcIdHelper = m_idHelperSvc->rpcIdHelper();

    const int i_lb = ctx.eventID().lumi_block();
    std::vector<double> v_prdTime = {};

    auto prd_sec_all = Scalar<int>("prd_sec", 0);
    auto prd_layer_all = Scalar<int>("prd_layer", 0);
    auto prd_sec_1214 = Scalar<int>("prd_sec_1214", 0);
    auto prd_layer_1214 = Scalar<int>("prd_layer_1214", 0);

    auto prd_sec_all_eta = Scalar<int>("prd_sec_eta", 0);
    auto prd_layer_all_eta = Scalar<int>("prd_layer_eta", 0);
    auto prd_sec_all_phi = Scalar<int>("prd_sec_phi", 0);
    auto prd_layer_all_phi = Scalar<int>("prd_layer_phi", 0);

    auto i_prd_LB = Scalar<int>("LB", i_lb);
    auto i_panelIndex = Scalar<int>("panelInd", 0);

    auto tool = getGroup(m_packageName);

    int panel_index;
    std::pair<int, int> sec_layer;

    //
    // loop on RpcPrepData container
    //
    for (const Muon::RpcPrepDataCollection* rpcCollection : *rpcContainer) {
        //
        // loop on RpcPrepData
        //
        for (const Muon::RpcPrepData* rpcData : *rpcCollection) {

            Identifier id = rpcData->identify();
            const int measphi = rpcIdHelper.measuresPhi(id);

            auto temp_panel = std::make_unique<RpcPanel>(
                    *m_idHelperSvc, rpcData->detectorElement(), 
                    rpcIdHelper.doubletZ(id), rpcIdHelper.doubletPhi(id), 
                    rpcIdHelper.gasGap(id), rpcIdHelper.measuresPhi(id));

            std::map<Identifier, std::shared_ptr<RpcPanel>>::const_iterator
                i_panel = m_rpcPanelMap.find(temp_panel->panelId);
            if (i_panel == m_rpcPanelMap.end()) {
                ATH_MSG_WARNING(
                    "The panelID corresponding prd hit does NOT link to a "
                    "known Panel "
                    << m_idHelperSvc->toString(id) << " <"
                    << m_idHelperSvc->toString(
                           rpcData->detectorElement()->identify())
                    << "> !!!");
                continue;
            } else {
                panel_index = i_panel->second->panel_index;
            }

            sec_layer = temp_panel->getSectorLayer();
            prd_sec_all = sec_layer.first;
            prd_layer_all = sec_layer.second;

            if (std::abs(sec_layer.first) == 12 ||
                std::abs(sec_layer.first) == 14) {
                prd_sec_1214 = sec_layer.first;
                prd_layer_1214 = sec_layer.second;
            }

            fill(tool, prd_sec_all, prd_layer_all, prd_sec_1214,
                 prd_layer_1214);

            if (measphi == 0) {
                prd_sec_all_eta = sec_layer.first;
                prd_layer_all_eta = sec_layer.second;
                fill(tool, prd_sec_all_eta, prd_layer_all_eta);
            } else {
                prd_sec_all_phi = sec_layer.first;
                prd_layer_all_phi = sec_layer.second;
                fill(tool, prd_sec_all_phi, prd_layer_all_phi);
            }

            i_panelIndex = panel_index;
            fill(tool, i_prd_LB, i_panelIndex);

            v_prdTime.push_back(rpcData->time());
        }  // loop on RpcPrepData
    }      // loop on RpcPrepData container

    auto prdTimeCollection = Collection("prdTime", v_prdTime);
    fill(tool, prdTimeCollection);

    ATH_MSG_DEBUG(" fillHistPRD finished ");
    return StatusCode::SUCCESS;
}

//================================================================================================
StatusCode RpcTrackAnaAlg::triggerMatching(
    const xAOD::Muon* offline_muon,
    const std::vector<TagDef>& list_of_triggers) const {
    if (!m_TagAndProbe.value())
        return StatusCode::SUCCESS;
    if (getTrigDecisionTool().empty())
        return StatusCode::SUCCESS;
    TVector3 muonvec;
    muonvec.SetPtEtaPhi(offline_muon->pt(), offline_muon->eta(),
                        offline_muon->phi());

    for (const auto& tagTrig : list_of_triggers) {
        if (!getTrigDecisionTool()->isPassed(tagTrig.eventTrig.Data()))
            continue;

        ATH_MSG_DEBUG("tagTrig.eventTrig = " << tagTrig.eventTrig
                                             << ";  tagTrig.tagTrig = "
                                             << tagTrig.tagTrig);
        // ATH_MSG_DEBUG("m_MuonEFContainerName.value() = "<<
        // m_MuonEFContainerName.value());

        std::vector<TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>>
            features = getTrigDecisionTool()->features<xAOD::MuonContainer>(
                tagTrig.tagTrig.Data(), TrigDefs::Physics);

        for (const auto& aaa : features) {
            ATH_CHECK(aaa.isValid());
            auto trigmuon_link = aaa.link;
            auto trigmuon = *trigmuon_link;
            TVector3 trigvec;
            trigvec.SetPtEtaPhi(trigmuon->pt(), trigmuon->eta(),
                                trigmuon->phi());
            if (trigvec.DeltaR(muonvec) < m_trigMatchWindow.value())
                return StatusCode::SUCCESS;
        }
    }
    return StatusCode::FAILURE;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::extrapolate2RPC(const xAOD::TrackParticle* track,
                                           const Trk::PropDirection direction,
                                           std::vector<GasGapResult>& results,
                                           BarrelDL barrelDL) const {
    /*
      get intersections of the muon with the RPC planes

      Iterate over all valid RPCDetectorElements and RPCReadoutElements:
       1) compute DR distance between track and center of ReadoutElement
          if this distance within tolerance - proceed
       2) Next, compute:
            -- min DR distance between track and strips within this gas gap
            -- number of valid eta and phi strips within this gas gap
          if both results within their tolerances - proceed
       3) Extrapolate track to the surface of this gas gap:
            -- Check that extrapolation result valid
            -- Check that extrapolated position is in the gas gap surface bounds
          if both within checks valid - then save RPC extrapolation result
    */
    int doubletR = 1;
    if (barrelDL >= OUT) {
        return StatusCode::SUCCESS;
    } else if (barrelDL == BM2 || barrelDL == BO2) {
        doubletR = 2;
    }

    using namespace Monitored;
    auto tool = getGroup(m_packageName);

    if (!track) {
        return StatusCode::FAILURE;
    }

    std::map<BarrelDL, std::vector<int>>::const_iterator dl_vec_it =
        m_StationNames.find(barrelDL);
    if (dl_vec_it == m_StationNames.end()) {
        return StatusCode::FAILURE;
    }

    std::unique_ptr<Trk::TrackParameters> trackParamLayer{};
    double minDR = 1.0;  // A intial value

    const std::vector<int> dl_vec = dl_vec_it->second;
    std::vector<int>::const_iterator it_dl = dl_vec.begin();
    for (; it_dl != dl_vec.end(); ++it_dl) {
        int stName = *it_dl;
        std::pair<int, int> st_dbR = std::make_pair(stName, doubletR);
        std::map<std::pair<int, int>,
                 std::vector<std::shared_ptr<GasGapData>>>::const_iterator
            gasgapIt = m_gasGapData.find(st_dbR);
        if (gasgapIt == m_gasGapData.end()) {
            continue;
        }

        //
        // Iterate over RPC readout elements and compute intersections with each
        // gas gap
        //
        for (const std::shared_ptr<GasGapData>& gap : gasgapIt->second) {
            ExResult result(gap->gapid, direction);

            // Compute track distance to the gas gap surface
            gap->computeTrackDistanceToGasGap(result, *track);

            if (result.minTrackGasGapDR > m_minDRTrackToGasGap) {
                continue;
            }

            //
            // Extrapolate track to the gas gap surface and check whether the
            // track position is in bounds returns new object
            auto trackParamInGap =
                computeTrackIntersectionWithGasGap(result, track, gap);

            if (!trackParamInGap) {
                continue;
            }

            if (!result.localTrackPosInBoundsTight) {
                continue;
            }

            if (result.minTrackGasGapDR < minDR) {
                minDR = result.minTrackGasGapDR;
                // new object moved to trackParamLayer; previous trackParamLayer
                // gets destroyed
                trackParamLayer = std::move(trackParamInGap);
            }
            ATH_MSG_DEBUG(name() << " extrapolated gasgap: " << gap->gapid_str);

            results.push_back(std::make_pair(result, gap));
        }
    }

    // Go to next layer of doublet
    BarrelDL nextDL = BarrelDL(barrelDL + 1);

    // propgate the track parameter of the last doublet
    // if no track paramater, use the input track
    if (trackParamLayer != nullptr) {
        // trackParamLayer used and then goes out of scope to destroy the object
        return extrapolate2RPC(std::move(trackParamLayer), direction, results,
                               nextDL);
    } else {
        return extrapolate2RPC(track, direction, results, nextDL);
    }
}

//========================================================================================================
std::unique_ptr<Trk::TrackParameters>
RpcTrackAnaAlg::computeTrackIntersectionWithGasGap(
    ExResult& result, const xAOD::TrackParticle* track_particle,
    const std::shared_ptr<GasGapData>& gap) const {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    /*
      This function:
      - constructs Identifier for specific gasgap
      - extrapolates muon to this gas gap
    */

    // Get surface of this gas gap and extrapolate track to this surface
    const Trk::SurfaceBounds& bounds = gap->readoutEl->bounds(gap->gapid);
    const Trk::PlaneSurface& gapSurface = gap->readoutEl->surface(gap->gapid);
    std::unique_ptr<Trk::TrackParameters> detParameters{};

    if (m_useAODParticle) {
        detParameters = m_extrapolator->extrapolate(
            ctx, track_particle->perigeeParameters(), gapSurface,
            result.direction, false, Trk::muon);
    } else if (track_particle->track()) {
        detParameters = m_extrapolator->extrapolateTrack(
            ctx, *(track_particle->track()), gapSurface, result.direction, true,
            Trk::muon);
    } else {
        return detParameters;
    }

    if (!detParameters) {
        return detParameters;
    }

    //
    // Transform global extrapolated track position on surface to local
    // coordinates
    //
    const Amg::Vector3D local3dTrackPosition =
        gap->readoutEl->globalToLocalCoords(detParameters->position(),
                                            gap->gapid);
    const Amg::Vector2D local2dTrackPosition(local3dTrackPosition.y(),
                                             local3dTrackPosition.z());

    //
    // Check if the track position on surface is within tolerable bounds
    //
    const bool inbounds =
        bounds.inside(local2dTrackPosition, m_boundsToleranceReadoutElement,
                      m_boundsToleranceReadoutElement);
    const bool inbounds_tight = bounds.inside(
        local2dTrackPosition, m_boundsToleranceReadoutElementTight,
        m_boundsToleranceReadoutElementTight);

    result.localTrackPosInBounds = inbounds;
    result.localTrackPosInBoundsTight = inbounds_tight;
    result.localPos = local3dTrackPosition;
    result.globalPos = detParameters->position();

    return detParameters;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::extrapolate2RPC(
    std::unique_ptr<Trk::TrackParameters> trackParam,
    const Trk::PropDirection direction, std::vector<GasGapResult>& results,
    BarrelDL barrelDL) const {
    /*
      get intersections of the muon with the RPC planes

      Iterate over all valid RPCDetectorElements and RPCReadoutElements:
       1) compute DR distance between track and center of ReadoutElement
          if this distance within tolerance - proceed
       2) Next, compute:
            -- min DR distance between track and strips within this gas gap
            -- number of valid eta and phi strips within this gas gap
          if both results within their tolerances - proceed
       3) Extrapolate track to the surface of this gas gap:
            -- Check that extrapolation result valid
            -- Check that extrapolated position is in the gas gap surface bounds
          if both within checks valid - then save RPC extrapolation result
    */
    int doubletR = 1;
    if (barrelDL >= OUT) {
        return StatusCode::SUCCESS;
    } else if (barrelDL == BM2 || barrelDL == BO2) {
        doubletR = 2;
    }

    using namespace Monitored;
    auto tool = getGroup(m_packageName);

    if (!trackParam) {
        return StatusCode::FAILURE;
    }

    std::map<BarrelDL, std::vector<int>>::const_iterator dl_vec_it =
        m_StationNames.find(barrelDL);
    if (dl_vec_it == m_StationNames.end()) {
        return StatusCode::FAILURE;
    }

    std::unique_ptr<Trk::TrackParameters> trackParamLayer{};
    double minDR = 1.0;  // A intial value

    const std::vector<int> dl_vec = dl_vec_it->second;

    std::vector<int>::const_iterator it_dl = dl_vec.begin();
    for (; it_dl != dl_vec.end(); ++it_dl) {
        int stName = *it_dl;
        std::pair<int, int> st_dbR = std::make_pair(stName, doubletR);
        std::map<std::pair<int, int>,
                 std::vector<std::shared_ptr<GasGapData>>>::const_iterator
            gasgapIt = m_gasGapData.find(st_dbR);

        if (gasgapIt == m_gasGapData.end()) {
            continue;
        }

        //
        // Iterate over RPC readout elements and compute intersections with each
        // gas gap
        //
        for (const std::shared_ptr<GasGapData>& gap : gasgapIt->second) {
            ExResult result(gap->gapid, direction);

            // Compute track distance to the gas gap surface; doesnt take
            // ownership
            gap->computeTrackDistanceToGasGap(result, trackParam.get());

            if (result.minTrackGasGapDR > m_minDRTrackToGasGap) {
                continue;
            }

            //
            // Extrapolate track to the gas gap surface and check whether the
            // track position is in bounds; doesnt take ownership but returns a
            // new object
            auto trackParamInGap = computeTrackIntersectionWithGasGap(
                result, trackParam.get(), gap);
            if (trackParamInGap == nullptr) {
                continue;
            }

            if (!result.localTrackPosInBoundsTight) {
                continue;
            }

            ATH_MSG_DEBUG(name() << " extrapolated gasgap: " << gap->gapid_str);

            if (result.minTrackGasGapDR < minDR) {
                minDR = result.minTrackGasGapDR;
                // previously created trackParamInGap moved to trackParamLayer;
                // previous trackParamLayer deleted
                trackParamLayer = std::move(trackParamInGap);
            }

            results.push_back(std::make_pair(result, gap));
        }
    }

    if (trackParamLayer == nullptr) {
        trackParamLayer = std::move(trackParam);
    }
    BarrelDL nextDL = BarrelDL(barrelDL + 1);
    // trackParamLayer used and then goes out of scope, destroying the object
    return extrapolate2RPC(std::move(trackParamLayer), direction, results,
                           nextDL);
}

//========================================================================================================
std::unique_ptr<Trk::TrackParameters>
RpcTrackAnaAlg::computeTrackIntersectionWithGasGap(
    ExResult& result, const Trk::TrackParameters* trackParam,
    const std::shared_ptr<GasGapData>& gap) const {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    /*
      This function:
      - constructs Identifier for specific gasgap
      - extrapolates muon to this gas gap
    */

    // Get surface of this gas gap and extrapolate track to this surface
    const Trk::SurfaceBounds& bounds = gap->readoutEl->bounds(gap->gapid);
    const Trk::PlaneSurface& gapSurface = gap->readoutEl->surface(gap->gapid);
    auto detParameters = m_extrapolator->extrapolate(
        ctx, *trackParam, gapSurface, result.direction, true, Trk::muon);

    if (!detParameters) {
        return detParameters;
    }

    //
    // Transform global extrapolated track position on surface to local
    // coordinates
    //
    const Amg::Vector3D local3dTrackPosition =
        gap->readoutEl->globalToLocalCoords(detParameters->position(),
                                            gap->gapid);
    const Amg::Vector2D local2dTrackPosition(local3dTrackPosition.y(),
                                             local3dTrackPosition.z());

    //
    // Check if the track position on surface is within tolerable bounds
    //
    const bool inbounds =
        bounds.inside(local2dTrackPosition, m_boundsToleranceReadoutElement,
                      m_boundsToleranceReadoutElement);
    const bool inbounds_tight = bounds.inside(
        local2dTrackPosition, m_boundsToleranceReadoutElementTight,
        m_boundsToleranceReadoutElementTight);

    result.localTrackPosInBounds = inbounds;
    result.localTrackPosInBoundsTight = inbounds_tight;
    result.localPos = local3dTrackPosition;
    result.globalPos = detParameters->position();

    return detParameters;
}

//========================================================================================================
StatusCode RpcTrackAnaAlg::readHitsPerGasgap(const EventContext& ctx,
                                             std::vector<GasGapResult>& results,
                                             MuonSource muon_source) const {
    using namespace Monitored;
    auto tool = getGroup(m_packageName);

    int lumiBlock = ctx.eventID().lumi_block();

    SG::ReadHandle<Muon::RpcPrepDataContainer> rpcContainer(m_rpcPrdKey, ctx);
    const RpcIdHelper& rpcIdHelper = m_idHelperSvc->rpcIdHelper();

    ATH_MSG_DEBUG(" RpcPrepDataContainer size = " << rpcContainer->size());
    ATH_MSG_DEBUG(" results size = " << results.size());

    ATH_MSG_INFO(" RpcPrepDataContainer Test 1 ");
    
    auto i_hitTime_sec = Scalar<int>("hitTime_sec", 0);

    auto isOutTime_prd = Scalar<bool>("isOutTime_prd", false);
    auto isOutTime_onTrack = Scalar<bool>("isOutTime_prd_onTrack", false);
    auto i_panelIndex = Scalar<int>("panelInd_prd", 0);
    auto i_panelIndex_onTrack = Scalar<int>("panelInd_prd_onTrack", 0);

    auto res_eta = Scalar<int>("residual_eta", 0);
    auto res_phi = Scalar<int>("residual_phi", 0);
    auto closest_res_eta = Scalar<int>("closest_residual_eta", 0);
    auto closest_res_phi = Scalar<int>("closest_residual_phi", 0);

    for (GasGapResult& exr : results) {
        const std::shared_ptr<GasGapData> gap = exr.second;

        int sector = (gap->RpcPanel_eta_phi.first->getSectorLayer()).first;
        
        float clo_res_eta = 1001.; // initial value; no sense
        float clo_res_phi = 1001.;
        int NHit_perEvt_eta = 0;
        int NHit_perEvt_phi = 0;
        std::vector<const Muon::RpcPrepData*> view_hits_eta;
        std::vector<const Muon::RpcPrepData*> view_hits_phi;

        // loop on RpcPrepDataCollection
        for (const Muon::RpcPrepDataCollection* rpcCollection : *rpcContainer) {
            if (!rpcCollection) {
                continue;
            }

            // loop on RpcPrepData
            for (const Muon::RpcPrepData* rpcData : *rpcCollection) {
                if (!rpcData) {
                    continue;
                }

                const Identifier id = rpcData->identify();
                const int stationName = rpcIdHelper.stationName(id);
                const int stationEta = rpcIdHelper.stationEta(id);
                const int stationPhi = rpcIdHelper.stationPhi(id);

                const int doubletR = rpcIdHelper.doubletR(id);
                const int doubletZ = rpcIdHelper.doubletZ(id);
                const int doubletPhi = rpcIdHelper.doubletPhi(id);
                const unsigned gasGap = rpcIdHelper.gasGap(id);
                const int measuresPhi = rpcIdHelper.measuresPhi(id);

                // match hit to the gasgap
                if (stationName == gap->stationName &&
                    stationPhi == gap->stationPhi &&
                    stationEta == gap->stationEta &&
                    doubletR == gap->doubletR && gasGap == gap->gasgap &&
                    doubletPhi == gap->doubletPhi &&
                    doubletZ == gap->doubletZ) {


                    i_hitTime_sec = rpcData->time();
                    fill(m_tools[m_SectorGroup.at(
                             "sector" + std::to_string(std::abs(sector)))],
                         i_hitTime_sec);

                    // ---------------------------------
                    // Calculate distance between extrapolated muon track position and hit
                    Amg::Vector3D hitPos_global = rpcData->globalPosition();
                    const Amg::Vector3D hitPos_local =
                        gap->readoutEl->globalToLocalCoords(hitPos_global, gap->gapid);

                    float trackPos_localY = exr.first.localPos.y();
                    float trackPos_localZ = exr.first.localPos.z();

                    float residual_phi = trackPos_localY-hitPos_local.y();
                    float residual_eta = trackPos_localZ-hitPos_local.z();

                    // If hit is out-of-time
                    // bool isOutTime = (std::abs(i_hitTime_sec+50.) > m_outtime); // for run 3
                    bool isOutTime = (std::abs(i_hitTime_sec) > m_outtime); // for run 2 test

                    // Fill histograms of out-of-time hit fraction
                    if (muon_source == ZCand) {
                        
                        isOutTime_prd = isOutTime;
                        isOutTime_onTrack = isOutTime;
                        
                        if (measuresPhi) {
                            int i_panel_phi = gap->RpcPanel_eta_phi.second->panel_index;
                            i_panelIndex = i_panel_phi;
                            fill(tool, i_panelIndex, isOutTime_prd);
                            if (std::abs(residual_phi) < m_diffHitTrackPostion) {
                                i_panelIndex_onTrack = i_panel_phi;
                                fill(tool, i_panelIndex_onTrack, isOutTime_onTrack);
                            }
                        } else {
                            int i_panel_eta = gap->RpcPanel_eta_phi.first->panel_index;
                            i_panelIndex = i_panel_eta;
                            fill(tool, i_panelIndex, isOutTime_prd);
                            if (std::abs(residual_eta) < m_diffHitTrackPostion) {
                                i_panelIndex_onTrack = i_panel_eta;
                                fill(tool, i_panelIndex_onTrack, isOutTime_onTrack);
                            }
                        }
                    }

                    // process hit within |time| < 12.5 ns && on Track
                    if (isOutTime){
                        continue;
                    }

                    if (measuresPhi) {
                        res_phi = residual_phi;
                        clo_res_phi = std::min(clo_res_phi, residual_phi);

                        fill(tool, res_phi);

                        if (std::abs(residual_phi) < m_diffHitTrackPostion) {
                            NHit_perEvt_phi++;
                            view_hits_phi.push_back(rpcData);
                        }
                    } else {
                        res_eta = residual_eta;
                        clo_res_eta = std::min(clo_res_eta, residual_eta);

                        fill(tool, res_eta);

                        if (std::abs(residual_eta) < m_diffHitTrackPostion){
                            NHit_perEvt_eta++;
                            view_hits_eta.push_back(rpcData);
                        }
                    }
                }
            }
        }

        int etaPanel_ind = -1;
        int phiPanel_ind = -1;

        etaPanel_ind = gap->RpcPanel_eta_phi.first->panel_index;
        phiPanel_ind = gap->RpcPanel_eta_phi.second->panel_index;

        // Declare the quantities which should be monitored
        if (muon_source == ZCand) {
            auto hitMulti_eta = Scalar<int>("hitMulti_eta", NHit_perEvt_eta);
            auto hitMulti_phi = Scalar<int>("hitMulti_phi", NHit_perEvt_phi);
            auto hitMulti = Scalar<int>("hitMulti", 0);
            auto i_panelIndex = Scalar<int>("panelInd_hM", 0);
            auto i_passExtrap = Scalar<bool>("muon_passExtrap", false);
            auto i_LB = Scalar<int>("LB_detEff", lumiBlock);

            fill(tool, hitMulti_eta, hitMulti_phi);

            //
            // Eta panel
            hitMulti = NHit_perEvt_eta;
            i_panelIndex = etaPanel_ind;
            if (NHit_perEvt_eta > 0)
                i_passExtrap = true;

            fill(tool, hitMulti, i_panelIndex, i_passExtrap, i_LB);
            ATH_CHECK(fillClusterSize(view_hits_eta, etaPanel_ind, lumiBlock,
                                      sector, 0));  // isPhi = 0

            if (clo_res_eta < 1000.) {
                closest_res_eta = clo_res_eta;
                fill(tool, closest_res_eta);
            }

            //
            // Phi panel
            hitMulti = NHit_perEvt_phi;
            i_panelIndex = phiPanel_ind;
            if (NHit_perEvt_phi > 0)
                i_passExtrap = true;

            fill(tool, hitMulti, i_panelIndex, i_passExtrap, i_LB);
            ATH_CHECK(fillClusterSize(view_hits_phi, phiPanel_ind, lumiBlock,
                                      sector, 1));  // isPhi = 1

            if (clo_res_phi < 1000.) {
                closest_res_phi = clo_res_phi;
                fill(tool, closest_res_phi);
            }
        } else {
            auto i_panelIndex = Scalar<int>("panelInd_hM_allMu", 0);
            auto i_passExtrap = Scalar<bool>("muon_passExtrap_allMu", false);

            //
            // Eta panel
            i_panelIndex = etaPanel_ind;
            if (NHit_perEvt_eta > 0)
                i_passExtrap = true;
            fill(tool, i_panelIndex, i_passExtrap);

            //
            // Phi panel
            i_panelIndex = phiPanel_ind;
            if (NHit_perEvt_phi > 0)
                i_passExtrap = true;

            fill(tool, i_panelIndex, i_passExtrap);
        }
    }

    return StatusCode::SUCCESS;
}

//==================================================================================
StatusCode RpcTrackAnaAlg::fillClusterSize(
    std::vector<const Muon::RpcPrepData*>& view_hits, const int panel_index,
    int LB, int phiSector, int isPhi) const {
    using namespace Monitored;

    auto tool = getGroup(m_packageName);
    auto i_LB = Scalar<int>("LB_nrpchit", LB);

    // Make clusters from hits that are close together in space and time
    std::vector<const Muon::RpcPrepData*> cluster_hits;
    while (!view_hits.empty()) {
        cluster_hits.clear();

        // Seed cluster with first (random) hit
        cluster_hits.push_back(view_hits.back());

        // Erase the selected first hit from the list
        view_hits.pop_back();

        // Collect all other hits which are close to the selected hit in time
        // and space
        std::vector<const Muon::RpcPrepData*>::const_iterator hit =
            view_hits.begin();

        while (hit != view_hits.end()) {
            const Muon::RpcPrepData* hit_ptr = *hit;

            if (IsNearbyHit(cluster_hits, hit_ptr)) {
                cluster_hits.push_back(*hit);
                view_hits.erase(hit);

                // Start loop from the beginning since we have increased cluster
                // size
                hit = view_hits.begin();
            } else {
                ++hit;
            }
        }

        int cluster_size = cluster_hits.size();
        for (int i_hit = 0; i_hit < cluster_size; i_hit++) {
            auto i_phiSector = Scalar<int>("PhiSector", phiSector);
            fill(tool, i_LB, i_phiSector);
        }

        auto i_panelIndex = Scalar<int>("panelInd_clust", panel_index);
        auto i_clusterSize = Scalar<int>("clusterSize", cluster_size);
        fill(tool, i_panelIndex, i_clusterSize);

        auto i_cs_sec = Scalar<int>("cs_sec", cluster_size);
        fill(m_tools[m_SectorGroup.at("sector" +
                                      std::to_string(std::abs(phiSector)))],
             i_cs_sec);

        if (isPhi == 1) {
            auto i_clusterSize_view =
                Scalar<int>("clusterSize_phi", cluster_size);
            fill(tool, i_clusterSize_view);
        } else {
            auto i_clusterSize_view =
                Scalar<int>("clusterSize_eta", cluster_size);
            fill(tool, i_clusterSize_view);
        }
    }

    return StatusCode::SUCCESS;
}

//====================================================================================
bool RpcTrackAnaAlg::IsNearbyHit(
    const std::vector<const Muon::RpcPrepData*>& cluster_hits,
    const Muon::RpcPrepData* hit) const {
    const RpcIdHelper& rpcIdHelper = m_idHelperSvc->rpcIdHelper();

    // Check whether this hit is close to any hits in the cluster
    for (const Muon::RpcPrepData* it_hit : cluster_hits) {
        if (abs(rpcIdHelper.strip(it_hit->identify()) -
                rpcIdHelper.strip(hit->identify())) < 2 &&
            std::abs(it_hit->time() - hit->time()) < 6.5) {
            return true;
        }
    }

    return false;
}
