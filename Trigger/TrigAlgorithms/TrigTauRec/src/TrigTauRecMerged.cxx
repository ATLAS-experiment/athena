/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauRecMerged.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/JetContainer.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"

#include "xAODTau/TauDefs.h"
#include "xAODTau/TauTrack.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"

#include <iterator>
#include <algorithm>


TrigTauRecMerged::TrigTauRecMerged(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{
    
}


StatusCode TrigTauRecMerged::initialize()
{
    ATH_MSG_DEBUG("Initialize");

    if(m_commonTools.begin() == m_commonTools.end()) {
        ATH_MSG_ERROR("No tools given for this algorithm");
        return StatusCode::FAILURE;
    }

    for(const auto& tool : m_commonTools) ATH_CHECK(tool.retrieve());
    for(const auto& tool : m_commonToolsBeforeTF) ATH_CHECK(tool.retrieve());
    for(const auto& tool : m_vertexFinderTools) ATH_CHECK(tool.retrieve());
    for(const auto& tool : m_trackFinderTools) ATH_CHECK(tool.retrieve());
    for(const auto& tool : m_vertexVarsTools) ATH_CHECK(tool.retrieve());
    for(const auto& tool : m_idTools) ATH_CHECK(tool.retrieve());

    if(!m_monTool.name().empty()) ATH_CHECK(m_monTool.retrieve());
  
    ATH_MSG_DEBUG("Initialising handle keys");
    ATH_CHECK(m_roiInputKey.initialize());
    ATH_CHECK(m_clustersInputKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_vertexInputKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_tauJetInputKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_tauTrackInputKey.initialize(SG::AllowEmpty));

    ATH_CHECK(m_tauSeedOutputKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_tauJetOutputKey.initialize());
    ATH_CHECK(m_tauTrackOutputKey.initialize());

    for(const auto& [key, p] : m_monitoredIdScores) {
        m_monitoredIdAccessors.emplace(
            key,
            std::make_pair(SG::ConstAccessor<float>(p.first), SG::ConstAccessor<float>(p.second))
        );
    }


    return StatusCode::SUCCESS;
}


StatusCode TrigTauRecMerged::execute(const EventContext& ctx) const
{
    //===============================================================================
    // Initialize monitoring variables
    //===============================================================================

    // Common CaloOnly and Precision monitored variables:
    auto n_taus                     = Monitored::Scalar<int>("NTauCandidates", 0);

    auto pT                         = Monitored::Scalar<float>("Pt", 0);
    auto eta                        = Monitored::Scalar<float>("Eta", -99.9);
    auto phi                        = Monitored::Scalar<float>("Phi", -99.9);

    auto etaRoI                     = Monitored::Scalar<float>("EtaRoI", -99.9);
    auto phiRoI                     = Monitored::Scalar<float>("PhiRoI", -99.9);
    auto dEta_RoI                   = Monitored::Scalar<float>("dEtaTau_RoI", -10);
    auto dPhi_RoI                   = Monitored::Scalar<float>("dPhiTau_RoI", -10);

    auto mEflowApprox               = Monitored::Scalar<float>("mEflowApprox", -99.9);
    auto ptRatioEflowApprox         = Monitored::Scalar<float>("ptRatioEflowApprox", -99.9);
    auto pt_jetseed_log             = Monitored::Scalar<float>("pt_jetseed_log", -99.9);
    auto etaDetectorAxis            = Monitored::Scalar<float>("etaDetectorAxis", -99.9);
    auto ptDetectorAxis             = Monitored::Scalar<float>("ptDetectorAxis", -99.9);
    auto ptDetectorAxis_log         = Monitored::Scalar<float>("ptDetectorAxis_log", -99.9);

    auto n_cells                    = Monitored::Scalar<int>("NCaloCells", 0);
    auto EMRadius                   = Monitored::Scalar<float>("EMRadius", -0.099);
    auto HadRadius                  = Monitored::Scalar<float>("HadRadius", -0.099);
    auto EtHad                      = Monitored::Scalar<float>("EtHad", -10);
    auto EtEm                       = Monitored::Scalar<float>("EtEm", -10);
    auto EMFrac                     = Monitored::Scalar<float>("EMFrac", -10);
    auto IsoFrac                    = Monitored::Scalar<float>("IsoFrac", -1);
    auto CentFrac                   = Monitored::Scalar<float>("CentFrac", -10);

    auto clustersMeanCenterLambda   = Monitored::Scalar<float>("clustersMeanCenterLambda", 0);
    auto clustersMeanFirstEngDens   = Monitored::Scalar<float>("clustersMeanFirstEngDens", 0);
    auto clustersMeanEMProbability  = Monitored::Scalar<float>("clustersMeanEMProbability", 0);
    auto clustersMeanSecondLambda   = Monitored::Scalar<float>("clustersMeanSecondLambda", 0);
    auto clustersMeanPresamplerFrac = Monitored::Scalar<float>("clustersMeanPresamplerFrac", 0);

    auto n_clusters                     = Monitored::Scalar<int>("NClusters", 0); 
    std::vector<float> cluster_et_log, cluster_dEta, cluster_dPhi;
    std::vector<float> cluster_log_SECOND_R, cluster_SECOND_LAMBDA, cluster_CENTER_LAMBDA;
    auto mon_cluster_et_log             = Monitored::Collection("cluster_et_log", cluster_et_log);
    auto mon_cluster_dEta               = Monitored::Collection("cluster_dEta", cluster_dEta);
    auto mon_cluster_dPhi               = Monitored::Collection("cluster_dPhi", cluster_dPhi);
    auto mon_cluster_log_SECOND_R       = Monitored::Collection("cluster_log_SECOND_R", cluster_log_SECOND_R);
    auto mon_cluster_SECOND_LAMBDA      = Monitored::Collection("cluster_SECOND_LAMBDA", cluster_SECOND_LAMBDA);
    auto mon_cluster_CENTER_LAMBDA      = Monitored::Collection("cluster_CENTER_LAMBDA", cluster_CENTER_LAMBDA);
    std::vector<unsigned char> calo_errors;
    auto mon_calo_errors                = Monitored::Collection("calo_errors", calo_errors);

    // Precision monitored variables
    auto n_tracks                   = Monitored::Scalar<int>("NTracks", -10);
    auto n_iso_tracks               = Monitored::Scalar<int>("NIsoTracks", -10);

    auto ipSigLeadTrk               = Monitored::Scalar<float>("ipSigLeadTrk", -1000);
    auto trFlightPathSig            = Monitored::Scalar<float>("trFlightPathSig", -10);
    auto massTrkSys                 = Monitored::Scalar<float>("massTrkSys", -10);
    auto dRmax                      = Monitored::Scalar<float>("dRmax", -10);
    auto trkAvgDist                 = Monitored::Scalar<float>("TrkAvgDist", -1);
    auto innerTrkAvgDist            = Monitored::Scalar<float>("innerTrkAvgDist", -1);
    auto etovPtLead                 = Monitored::Scalar<float>("EtovPtLead", -10);
    auto PSSFraction                = Monitored::Scalar<float>("PSSFraction", -999.9);
    auto EMPOverTrkSysP             = Monitored::Scalar<float>("EMPOverTrkSysP", -999.9);
    auto ChPiEMEOverCaloEME         = Monitored::Scalar<float>("ChPiEMEOverCaloEME", -999.9);
    auto vertex_x                   = Monitored::Scalar<float>("vertex_x", -999.9);
    auto vertex_y                   = Monitored::Scalar<float>("vertex_y", -999.9); 
    auto vertex_z                   = Monitored::Scalar<float>("vertex_z", -999.9);

    auto n_all_tracks                           = Monitored::Scalar<int>("NAllTracks", 0);
    std::vector<float> track_pt_log, track_dEta, track_dPhi;
    std::vector<float> track_d0_abs_log, track_z0sinthetaTJVA_abs_log;
    std::vector<float> track_nPixelHitsPlusDeadSensors, track_nSCTHitsPlusDeadSensors;
    auto mon_track_pt_log                       = Monitored::Collection("track_pt_log", track_pt_log);
    auto mon_track_dEta                         = Monitored::Collection("track_dEta", track_dEta);
    auto mon_track_dPhi                         = Monitored::Collection("track_dPhi", track_dPhi);
    auto mon_track_d0_abs_log                   = Monitored::Collection("track_d0_abs_log", track_d0_abs_log);
    auto mon_track_z0sinthetaTJVA_abs_log       = Monitored::Collection("track_z0sinthetaTJVA_abs_log", track_z0sinthetaTJVA_abs_log); 
    auto mon_track_nPixelHitsPlusDeadSensors    = Monitored::Collection("track_nPixelHitsPlusDeadSensors", track_nPixelHitsPlusDeadSensors);
    auto mon_track_nSCTHitsPlusDeadSensors      = Monitored::Collection("track_nSCTHitsPlusDeadSensors", track_nSCTHitsPlusDeadSensors);
    std::vector<unsigned char> track_errors;
    auto mon_track_errors                       = Monitored::Collection("track_errors", track_errors);

    std::map<std::string, Monitored::Scalar<float>> monitoredIdVariables;
    for(const auto& [key, p] : m_monitoredIdScores) {
        monitoredIdVariables.emplace(key + "_TauJetScore_0p", Monitored::Scalar<float>(key + "_TauJetScore_0p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTrans_0p", Monitored::Scalar<float>(key + "_TauJetScoreTrans_0p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScore_1p", Monitored::Scalar<float>(key + "_TauJetScore_1p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTrans_1p", Monitored::Scalar<float>(key + "_TauJetScoreTrans_1p", -1));
        monitoredIdVariables.emplace(key + "_TauJetScore_mp", Monitored::Scalar<float>(key + "_TauJetScore_mp", -1));
        monitoredIdVariables.emplace(key + "_TauJetScoreTrans_mp", Monitored::Scalar<float>(key + "_TauJetScoreTrans_mp", -1));
    }

    std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> monVars = {
        std::ref(n_taus),
        std::ref(pT), std::ref(eta), std::ref(phi),
        std::ref(etaRoI), std::ref(phiRoI), std::ref(dEta_RoI), std::ref(dPhi_RoI),
        std::ref(mEflowApprox), std::ref(ptRatioEflowApprox), std::ref(pt_jetseed_log),
        std::ref(etaDetectorAxis), std::ref(ptDetectorAxis), std::ref(ptDetectorAxis_log),
        std::ref(n_cells),
        std::ref(EMRadius), std::ref(HadRadius), std::ref(EtHad), std::ref(EtEm), std::ref(EMFrac), std::ref(IsoFrac), std::ref(CentFrac),
        std::ref(clustersMeanCenterLambda), std::ref(clustersMeanFirstEngDens), std::ref(clustersMeanEMProbability),
        std::ref(clustersMeanSecondLambda), std::ref(clustersMeanPresamplerFrac),
        std::ref(n_clusters), std::ref(mon_cluster_et_log), std::ref(mon_cluster_dEta), std::ref(mon_cluster_dPhi),
        std::ref(mon_cluster_log_SECOND_R), std::ref(mon_cluster_SECOND_LAMBDA), std::ref(mon_cluster_CENTER_LAMBDA),
        std::ref(mon_calo_errors),
        std::ref(n_tracks), std::ref(n_iso_tracks),
        std::ref(ipSigLeadTrk), std::ref(trFlightPathSig), std::ref(massTrkSys), std::ref(dRmax), std::ref(trkAvgDist), std::ref(innerTrkAvgDist),
        std::ref(etovPtLead), std::ref(PSSFraction), std::ref(EMPOverTrkSysP), std::ref(ChPiEMEOverCaloEME),
        std::ref(vertex_x), std::ref(vertex_y), std::ref(vertex_z),
        std::ref(n_all_tracks), std::ref(mon_track_pt_log), std::ref(mon_track_dEta), std::ref(mon_track_dPhi),
        std::ref(mon_track_d0_abs_log), std::ref(mon_track_z0sinthetaTJVA_abs_log),
        std::ref(mon_track_nPixelHitsPlusDeadSensors), std::ref(mon_track_nSCTHitsPlusDeadSensors),
        std::ref(mon_track_errors)
    };
    for(auto& [key, var] : monitoredIdVariables) monVars.push_back(std::ref(var));
    auto monitorIt = Monitored::Group(m_monTool, monVars);


    ATH_MSG_DEBUG("Executing TrigTauRecMerged");

    //===============================================================================
    // Main TauJet object:
    //===============================================================================
    xAOD::TauJet* tau = nullptr;


    //===============================================================================
    // Retrieve RoI
    //===============================================================================
    SG::ReadHandle<TrigRoiDescriptorCollection> roisHandle = SG::makeHandle(m_roiInputKey, ctx);
    if(!roisHandle.isValid()) {
        ATH_MSG_ERROR("No RoIHandle found");
        return StatusCode::FAILURE;
    }

    if(roisHandle->empty()) {
        ATH_MSG_ERROR("Empty RoIHandle");
        return StatusCode::FAILURE;
    }

    const TrigRoiDescriptor *roiDescriptor = roisHandle->at(0);
    if(roiDescriptor) {
        ATH_MSG_DEBUG("RoI: " << *roiDescriptor);
    } else {
        ATH_MSG_ERROR("Failed to find TrigRoiDescriptor");
        calo_errors.push_back(NoROIDescr);
        return StatusCode::FAILURE;
    }


    //===============================================================================
    // Prepare Output TauJet and TauTrack containers
    //===============================================================================
    
    // Create and register the output TauJetContainer
    std::unique_ptr<xAOD::TauJetContainer> outputContainer = std::make_unique<xAOD::TauJetContainer>();
    std::unique_ptr<xAOD::TauJetAuxContainer> outputAuxContainer = std::make_unique<xAOD::TauJetAuxContainer>();
    outputContainer->setStore(outputAuxContainer.get());

    SG::WriteHandle<xAOD::TauJetContainer> outputTauHandle(m_tauJetOutputKey, ctx);
    ATH_CHECK(outputTauHandle.record(std::move(outputContainer), std::move(outputAuxContainer)));

    // Create and register the output TauTrackContainer
    std::unique_ptr<xAOD::TauTrackContainer> outputTrackContainer = std::make_unique<xAOD::TauTrackContainer>();
    std::unique_ptr<xAOD::TauTrackAuxContainer> outputTrackAuxContainer = std::make_unique<xAOD::TauTrackAuxContainer>();
    outputTrackContainer->setStore(outputTrackAuxContainer.get());

    SG::WriteHandle<xAOD::TauTrackContainer> tauTrackHandle(m_tauTrackOutputKey, ctx);
    ATH_CHECK(tauTrackHandle.record(std::move(outputTrackContainer), std::move(outputTrackAuxContainer)));
    

    //===============================================================================
    // Initial TauJet calo-reco / input-retrieval
    //===============================================================================
    // We now have two options for the TauJet reconstruction:
    //  1) Reconstruct the TauJet from scratch, using the bare calo-clusters (1st trigger step)
    //  2) Fetch a preceding TauJet (and dummy TauTracks), and use them to seed the current reconstruction
    
    //-------------------------------------------------------------------------------
    // Option 1: Calorimeter-only reconstruction from clusters (CaloMVA step)
    //-------------------------------------------------------------------------------
    if(doCaloReconstruction()) {
        // Retrieve Calocluster container
        SG::ReadHandle<xAOD::CaloClusterContainer> CCContainerHandle = SG::makeHandle(m_clustersInputKey, ctx);
        ATH_CHECK(CCContainerHandle.isValid());

        const xAOD::CaloClusterContainer *RoICaloClusterContainer = CCContainerHandle.get();
        if(RoICaloClusterContainer) {
            ATH_MSG_DEBUG("CaloCluster container found of size: " << RoICaloClusterContainer->size());
            
            // If size is zero, don't stop just continue to produce empty TauJetCollection
            if(RoICaloClusterContainer->empty()) calo_errors.push_back(NoClustCont);
        } else {
            ATH_MSG_ERROR("No CaloCluster container found");
            calo_errors.push_back(NoClustCont);
            return StatusCode::FAILURE;
        }

        // Also create the seed-jet containers
        std::unique_ptr<xAOD::JetContainer> jetContainer{std::make_unique<xAOD::JetContainer>()};
        std::unique_ptr<xAOD::JetAuxContainer> jetAuxContainer{std::make_unique<xAOD::JetAuxContainer>()};
        jetContainer->setStore(jetAuxContainer.get());

        SG::WriteHandle<xAOD::JetContainer> outputTauSeedJetHandle = SG::makeHandle(m_tauSeedOutputKey, ctx);
        ATH_CHECK(outputTauSeedJetHandle.record(std::move(jetContainer), std::move(jetAuxContainer)));

        // And create the seed-jet object itself
        outputTauSeedJetHandle->push_back(std::make_unique<xAOD::Jet>());
        xAOD::Jet *jet = outputTauSeedJetHandle->back();
        

        // Build the jet, also keep track of the kinematics by hand
        // Eventually, want to use FastJet here?
        // We are using calibrated clusters, we need to keep track of this
        jet->setConstituentsSignalState(xAOD::JetConstitScale::CalibratedJetConstituent);
        TLorentzVector cluster_p4, barycenter;
        for(const xAOD::CaloCluster* cluster : *RoICaloClusterContainer) {
            ATH_MSG_DEBUG("Cluster (e, eta, phi): (" << cluster->e() << ", " << cluster->eta() << ", " << cluster->phi() << ")");
        
            if(cluster->e() < 0) {
                ATH_MSG_DEBUG("Negative energy cluster is rejected");
                continue;
            }
 
            cluster_p4.SetPtEtaPhiE(cluster->pt(), cluster->eta(), cluster->phi(), cluster->e());
            jet->addConstituent(cluster);

            barycenter += cluster_p4;
        }
         
        jet->setJetP4(xAOD::JetFourMom_t(barycenter.Pt(), barycenter.Eta(), barycenter.Phi(), barycenter.M())); 
        ATH_MSG_DEBUG("Built jet with eta: " << jet->eta() << ", phi: " << jet->phi() << ", pT: " << jet->pt() << ", E: "<< jet->e() );

        
        // If we're running calo-clustering, that means we just started the HLT reco,
        // and there's no input TauJet container to this step. Create one instead!
        outputTauHandle->push_back(std::make_unique<xAOD::TauJet>());
        tau = outputTauHandle->back();
        tau->setROIWord(roiDescriptor->roiWord());

        // Using the new Jet collection, setup the tau candidate structure
        tau->setJet(outputTauSeedJetHandle.ptr(), jet);

        // Fix eta, phi in case the jet's energy is negative
        if(jet->e() <= 0) {
            ATH_MSG_DEBUG("Changing (eta, phi) back to the RoI center due to negative energy: " << jet->e());
            tau->setP4(tau->pt(), roiDescriptor->eta(), roiDescriptor->phi(), tau->m());		
            ATH_MSG_DEBUG("Roi: " << roiDescriptor->roiId() << ", tau eta: " << tau->eta() << ", tau phi: " << tau->phi() );
        }
    }


    //-------------------------------------------------------------------------------
    // Option 2: Use input TauJet (and TauTracks) as seeds (non calo-only reco)
    //-------------------------------------------------------------------------------
    if(!doCaloReconstruction()) {
        // Retrieve input TauJet container
        if(!m_tauJetInputKey.key().empty()) {
            SG::ReadHandle<xAOD::TauJetContainer> tauInputHandle(m_tauJetInputKey, ctx);
            const xAOD::TauJetContainer* inputTauContainer = tauInputHandle.cptr();
            ATH_MSG_DEBUG("Input TauJet Container size: " << inputTauContainer->size());

            // Copy the input TauJets to the output container
            ATH_CHECK(deepCopy(outputTauHandle, inputTauContainer));
        }

        // Retrieve input TauTrack container
        if(!m_tauTrackInputKey.key().empty()) {
            SG::ReadHandle<xAOD::TauTrackContainer> tauTrackInputHandle(m_tauTrackInputKey, ctx);
            const xAOD::TauTrackContainer* inputTauTrackContainer = tauTrackInputHandle.cptr();
            ATH_MSG_DEBUG("Tau Track Container Size " << inputTauTrackContainer->size());

            // Copy the input TauTracks to the output container
            ATH_CHECK(deepCopy(tauTrackHandle, inputTauTrackContainer));
        }


        // Now retrieve the main TauJet object, if available (the recently created copy of the input TauJet)
        if(!outputTauHandle->empty()) {
            tau = outputTauHandle->back();
            
            // Check if the tau has a valid jetLink
            ATH_CHECK(tau->jetLink().isValid());

            // Clear all previous TauTrack links (if any), since we will run the
            // TauTrackFinder tool (and the associated InDet helpers) in the
            // Presel and Precision steps, refilling the tracks
            if(!m_tauTrackInputKey.key().empty()) tau->clearTauTrackLinks();
        }
    }
   

    //===============================================================================
    // Get Vertex Container (optional)
    //===============================================================================
    const xAOD::VertexContainer* RoIVxContainer = nullptr;
    if(!m_vertexInputKey.key().empty()){
        SG::ReadHandle<xAOD::VertexContainer> VertexContainerHandle = SG::makeHandle(m_vertexInputKey, ctx);

        if(!VertexContainerHandle.isValid()) {
            ATH_MSG_DEBUG("No VertexContainer retrieved for the trigger element");
            track_errors.push_back(NoVtxCont);
        } else {
            RoIVxContainer = VertexContainerHandle.get();
            ATH_MSG_DEBUG("Size of VertexContainer: " << RoIVxContainer->size());
        }
    }


    ATH_MSG_DEBUG("roiDescriptor roiWord: " << roiDescriptor->roiWord() << ", saved in TauJet: " << tau->ROIWord());


    //===============================================================================
    // Loop over all booked tau tools:
    //===============================================================================
    StatusCode processStatus = StatusCode::SUCCESS;

    // Sequence: VertexFinderTools -> CommonToolsBeforeTF -> TrackFinderTools -> CommonTools -> VertexVarsTools -> IDTools
  
    for(const auto& tool : m_vertexFinderTools) {
        ATH_MSG_DEBUG("Starting Tool: " << tool->name());

        processStatus = tool->executeVertexFinder(*tau, RoIVxContainer);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    for(const auto& tool : m_commonToolsBeforeTF) {
        if(!processStatus.isFailure()) ATH_MSG_DEBUG("Starting Tool: " << tool->name());
        else break;

        processStatus = tool->execute(*tau);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    for(const auto& tool : m_trackFinderTools) {
        if(!processStatus.isFailure()) ATH_MSG_DEBUG("Starting Tool: " << tool->name());
        else break;

        processStatus = tool->executeTrackFinder(*tau, *tauTrackHandle);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    for(const auto& tool : m_commonTools) {
        if(!processStatus.isFailure()) ATH_MSG_DEBUG("Starting Tool: " << tool->name());
        else break;

        processStatus = tool->execute(*tau);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    // Dummy container passed to TauVertexVariables, not used in trigger though
    xAOD::VertexContainer dummyVxCont; 
    for(const auto& tool : m_vertexVarsTools) {
        if(!processStatus.isFailure()) ATH_MSG_DEBUG("Starting Tool: " << tool->name());
        else break;

        processStatus = tool->executeVertexVariables(*tau, dummyVxCont);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    for(const auto& tool : m_idTools) {
        if(!processStatus.isFailure()) ATH_MSG_DEBUG("Starting Tool: " << tool->name());
        else break;

        processStatus = tool->execute(*tau);

        if(!processStatus.isFailure()) {
            ATH_MSG_DEBUG(" " << tool->name() << " executed successfully");
        } else {
            ATH_MSG_DEBUG(" " << tool->name() << " execution failed");
            break;
        }
    }

    ATH_MSG_DEBUG("This tau has " << tau->allTracks() << " tracks linked");


    // Cleanup in case any of the tools failed (rejected Tau)
    if(!processStatus.isSuccess()) {
        ATH_MSG_DEBUG("The tau object has NOT been registered in the tau container");

        xAOD::TauJet* bad_tau = outputTauHandle->back();
        ATH_MSG_DEBUG("Deleting " << bad_tau->nAllTracks() << " tracks associated with tau");
        tauTrackHandle->erase(tauTrackHandle->end() - bad_tau->nAllTracks(), tauTrackHandle->end());

        outputTauHandle->pop_back();

        ATH_MSG_DEBUG("Clean up done after jet seed");  

    } else {
        // Check that the seed-jet energy is positive
        // Otherwise, try to salvage it by replacing the tau position with the RoI's
        float fJetEnergy = (*tau->jetLink())->e();
        ATH_MSG_DEBUG("Seed jet E: " << fJetEnergy);
	      
        if(fJetEnergy < 0.00001) {
            ATH_MSG_DEBUG("Changing tau's (eta,phi) to RoI ones due to negative energy (PxPyPzE flips eta and phi)");
            ATH_MSG_DEBUG("This is probably not needed anymore, method PxPyPzE has been corrected");
            // TODO: Do we still need this??
	        
            tau->setP4(tau->pt(), roiDescriptor->eta(), roiDescriptor->phi(), tau->m());
	        
            ATH_MSG_DEBUG("Roi: " << roiDescriptor->roiId() << ", Tau eta: " << tau->eta() << ", phi: " << tau->phi() << ", pT: " << tau->pt());
        }


        //===============================================================================
        // Monitor tau variables
        //===============================================================================
        
        pT = tau->pt()/Gaudi::Units::GeV;
        eta = tau->eta();
        phi = tau->phi();


        etaRoI = roiDescriptor->eta();
        phiRoI = roiDescriptor->phi();	  
        dEta_RoI = eta - roiDescriptor->eta();
        dPhi_RoI = phi - roiDescriptor->phi();
        if(dPhi_RoI < -M_PI) dPhi_RoI += 2.0*M_PI;
        if(dPhi_RoI > M_PI) dPhi_RoI -= 2.0*M_PI;


        float pre_mEflowApprox;
        tau->detail(xAOD::TauJetParameters::mEflowApprox, pre_mEflowApprox);  
        mEflowApprox = std::log10(std::max(pre_mEflowApprox, 140.0f));

        float pre_ptRatioEflowApprox;
        tau->detail(xAOD::TauJetParameters::ptRatioEflowApprox, pre_ptRatioEflowApprox);
        ptRatioEflowApprox = std::min(pre_ptRatioEflowApprox, 4.0f);
        
        pt_jetseed_log = std::log10(tau->ptJetSeed());
        etaDetectorAxis = tau->etaDetectorAxis();   
        ptDetectorAxis = std::min(tau->ptDetectorAxis()/Gaudi::Units::GeV, 10000.0);
        ptDetectorAxis_log = std::log10(std::min(tau->ptDetectorAxis()/Gaudi::Units::GeV, 10000.0));


        tau->detail(xAOD::TauJetParameters::numCells, n_cells);
        tau->detail(xAOD::TauJetParameters::EMRadius, EMRadius);
        tau->detail(xAOD::TauJetParameters::hadRadius, HadRadius);
        tau->detail(xAOD::TauJetParameters::etHadAtEMScale, EtHad);
        EtHad /= Gaudi::Units::GeV;
        tau->detail(xAOD::TauJetParameters::etEMAtEMScale, EtEm);
        EtEm /= Gaudi::Units::GeV;

        float Et_raw = EtEm + EtHad;
        if(Et_raw != 0) EMFrac = EtEm / Et_raw;

        tau->detail(xAOD::TauJetParameters::isolFrac, IsoFrac);
        tau->detail(xAOD::TauJetParameters::centFrac, CentFrac);


        // Monitor BRT variables
        float tmp = 0;
        bool test = tau->detail(xAOD::TauJetParameters::ClustersMeanCenterLambda, tmp);
        if(test) clustersMeanCenterLambda = tmp;
        test = tau->detail(xAOD::TauJetParameters::ClustersMeanFirstEngDens, tmp);
        if(test) clustersMeanFirstEngDens = tmp;
        test = tau->detail(xAOD::TauJetParameters::ClustersMeanEMProbability, tmp);
        if(test) clustersMeanEMProbability = tmp;
        test = tau->detail(xAOD::TauJetParameters::ClustersMeanSecondLambda, tmp);
        if(test) clustersMeanSecondLambda = tmp;
        test = tau->detail(xAOD::TauJetParameters::ClustersMeanPresamplerFrac, tmp);
        if(test) clustersMeanPresamplerFrac = tmp;

        
        // Cluster variables monitoring
        n_clusters = tau->clusters().size();
        for(const auto& cluster : tau->clusters()) {
            const xAOD::CaloCluster* cls = dynamic_cast<const xAOD::CaloCluster*>(cluster);

            cluster_et_log.push_back(std::log10(cls->et()));
            cluster_dEta.push_back(cls->eta() - tau->eta());
            cluster_dPhi.push_back(cls->p4().DeltaPhi(tau->p4()));
         
            double log_second_R = -999;
            const bool success_SECOND_R = cls->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_R, log_second_R);
            if(success_SECOND_R) log_second_R = std::log10(log_second_R + 0.1);
            cluster_log_SECOND_R.push_back(log_second_R);

            double second_lambda = -999;
            const bool success_SECOND_LAMBDA = cls->retrieveMoment(xAOD::CaloCluster::MomentType::SECOND_LAMBDA, second_lambda);
            if(success_SECOND_LAMBDA) second_lambda = std::log10(second_lambda + 0.1);
            cluster_SECOND_LAMBDA.push_back(second_lambda);

            double center_lambda = -999;
            const bool success_CENTER_LAMBDA = cls->retrieveMoment(xAOD::CaloCluster::MomentType::CENTER_LAMBDA, center_lambda);
            if(success_CENTER_LAMBDA) center_lambda = std::log10(center_lambda + 1e-6);
            cluster_CENTER_LAMBDA.push_back(center_lambda);     
        }


        // Tracks summary monitoring
        n_tracks = tau->nTracks();
        n_iso_tracks = tau->nTracksIsolation();
        tau->detail(xAOD::TauJetParameters::ipSigLeadTrk, ipSigLeadTrk);
        if(tau->nTracks() > 0) ipSigLeadTrk = std::abs(tau->track(0)->d0SigTJVA()); // TODO: Is this needed?
        tau->detail(xAOD::TauJetParameters::trFlightPathSig, trFlightPathSig);
        tau->detail(xAOD::TauJetParameters::massTrkSys, massTrkSys);
        massTrkSys /= Gaudi::Units::GeV;
        tau->detail(xAOD::TauJetParameters::dRmax, dRmax);
        tau->detail(xAOD::TauJetParameters::trkAvgDist, trkAvgDist);
        tau->detail(xAOD::TauJetParameters::innerTrkAvgDist, innerTrkAvgDist);	 
        tau->detail(xAOD::TauJetParameters::etOverPtLeadTrk, etovPtLead);
        tau->detail(xAOD::TauJetParameters::PSSFraction, PSSFraction);
        tau->detail(xAOD::TauJetParameters::EMPOverTrkSysP, EMPOverTrkSysP);
        tau->detail(xAOD::TauJetParameters::ChPiEMEOverCaloEME, ChPiEMEOverCaloEME);

        if(tau->vertexLink().isValid() && tau->vertex() && tau->vertex()->vertexType() != xAOD::VxType::NoVtx) {
            vertex_x = tau->vertex()->x();
            vertex_y = tau->vertex()->y();       
            vertex_z = tau->vertex()->z();
        }


        // Track variables monitoring 
        n_all_tracks = tau->allTracks().size();
        for(const xAOD::TauTrack* track : tau->allTracks()) {
            track_pt_log.push_back(std::log10(track->pt()));
            track_dEta.push_back(track->eta() - tau->eta()); 
            track_dPhi.push_back(track->p4().DeltaPhi(tau->p4()));
            track_d0_abs_log.push_back(std::log10(std::abs(track->track()->d0()) + 1e-6));
            track_z0sinthetaTJVA_abs_log.push_back(track->z0sinthetaTJVA());

            uint8_t pixel_hits, pixel_dead;
            const bool success1_pixel_hits = track->track()->summaryValue(pixel_hits, xAOD::numberOfPixelHits);
            const bool success2_pixel_dead = track->track()->summaryValue(pixel_dead, xAOD::numberOfPixelDeadSensors);
            float nPixelHitsPlusDeadSensor = -999;
            if(success1_pixel_hits && success2_pixel_dead) nPixelHitsPlusDeadSensor = pixel_hits + pixel_dead;
            track_nPixelHitsPlusDeadSensors.push_back(nPixelHitsPlusDeadSensor);

            uint8_t sct_hits, sct_dead;
            const bool success1_sct_hits = track->track()->summaryValue(sct_hits, xAOD::numberOfSCTHits);
            const bool success2_sct_dead = track->track()->summaryValue(sct_dead, xAOD::numberOfSCTDeadSensors);
            float nSCTHitsPlusDeadSensors = -999;
            if(success1_sct_hits && success2_sct_dead) nSCTHitsPlusDeadSensors = sct_hits + sct_dead;
            track_nSCTHitsPlusDeadSensors.push_back(nSCTHitsPlusDeadSensors);
        }


        // TauID Score monitoring
        for(const auto& [key, p] : m_monitoredIdAccessors) {
            if(p.first.isAvailable(*tau)) {
                if(tau->nTracks() == 0) {
                    monitoredIdVariables.at(key + "_TauJetScore_0p") = p.first(*tau);
                } else if(tau->nTracks() == 1) {
                    monitoredIdVariables.at(key + "_TauJetScore_1p") = p.first(*tau);
                } else { // MP tau
                    monitoredIdVariables.at(key + "_TauJetScore_mp") = p.first(*tau);
                }
            }

            if(p.second.isAvailable(*tau)) {
                if(tau->nTracks() == 0) {
                    monitoredIdVariables.at(key + "_TauJetScoreTrans_0p") = p.second(*tau);
                } else if(tau->nTracks() == 1) {
                    monitoredIdVariables.at(key + "_TauJetScoreTrans_1p") = p.second(*tau);
                } else { // MP tau
                    monitoredIdVariables.at(key + "_TauJetScoreTrans_mp") = p.second(*tau);
                }
            }
        }


        ++n_taus;


        ATH_MSG_DEBUG("RoI: " << roiDescriptor->roiId()
	    	  << ", Tau pT (GeV): " << pT << ", Tau eta: " << eta << ", Tau phi: " << phi
	    	  << ", wrt RoI dEta: " << dEta_RoI << ", dPhi: " << dPhi_RoI);
    }

    //-------------------------------------------------------------------------------
    // All done!
    //-------------------------------------------------------------------------------
    
    ATH_MSG_DEBUG("Output TauJetContainer size: " << outputTauHandle->size());
    ATH_MSG_DEBUG("Output TauJetTrackContainer size: " << tauTrackHandle->size());
    
    return StatusCode::SUCCESS;
}
