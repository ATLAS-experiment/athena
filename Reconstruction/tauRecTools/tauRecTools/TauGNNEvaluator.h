/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNNEVALUATOR_H
#define TAURECTOOLS_TAUGNNEVALUATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/TauGNN.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include <memory>

/**
 * @brief Tool to calculate tau identification score from .onnx inputs
 *
 *   The network configuration is supplied in .onnx format. 
 *   Currently runs on a prongness-inclusive model 
 *   Based off of TauJetRNNEvaluator.h format!
 * @author N.M. Tamir
 *
 */
class TauGNNEvaluator : public TauRecToolBase {
public:
    ASG_TOOL_CLASS2(TauGNNEvaluator, TauRecToolBase, ITauToolBase)

    TauGNNEvaluator(const std::string &name = "TauGNNEvaluator");
    virtual ~TauGNNEvaluator();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(xAOD::TauJet &tau) const override;

    enum Discriminant {
        NegLogPJet = 0,
        PTau = 1
    };

private:

    Gaudi::Property<std::string> m_tauContainerName{this, "TauContainerName", "", "Name of TauJetContainer, must be set when using "};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_scoreHandleKey{this, "ScoreHandleKey","","Output Score"};
   
    // properties
    Gaudi::Property<std::string> m_weightfile_inclusive{this, "NetworkFileInclusive", ""};
    Gaudi::Property<std::string> m_weightfile_0p{this, "NetworkFile0P", ""};
    Gaudi::Property<std::string> m_weightfile_1p{this, "NetworkFile1P", ""};
    Gaudi::Property<std::string> m_weightfile_2p{this, "NetworkFile2P", ""};
    Gaudi::Property<std::string> m_weightfile_3p{this, "NetworkFile3P", ""}; 
    Gaudi::Property<std::string> m_input_layer_scalar{this, "InputLayerScalar","tau_vars"};
    Gaudi::Property<std::string> m_input_layer_tracks{this, "InputLayerTracks","track_vars"};
    Gaudi::Property<std::string> m_input_layer_clusters{this, "InputLayerClusters","cluster_vars"};
    Gaudi::Property<std::string> m_output_varname{this, "OutputVarname", "GNTauScore"}; 
    Gaudi::Property<std::string> m_output_ptau{this, "OutputPTau", "GNTauProbTau"};
    Gaudi::Property<std::string> m_output_pjet{this, "OutputPJet", "GNTauProbJet"};
    Gaudi::Property<unsigned int> m_output_discriminant{this, "OutputDiscriminant", Discriminant::NegLogPJet, 
    "Discriminant used to calculate the output score: 0 -> -log(PJet), 1 -> PTau"};
    Gaudi::Property<int> m_max_tracks{this, "MaxTracks", 30};
    Gaudi::Property<int> m_max_clusters{this, "MaxClusters", 20};
    Gaudi::Property<float> m_max_cluster_dr{this, "MaxClusterDR", 1.0f};
    Gaudi::Property<bool> m_doVertexCorrection{this, "VertexCorrection", true};
    Gaudi::Property<bool> m_doTrackClassification{this, "TrackClassification", true};
    Gaudi::Property<bool> m_useTRT{this, "useTRT", true};
    Gaudi::Property<float> m_minTauPt{this, "MinTauPt", 0.};
    Gaudi::Property<bool> m_applyLooseTrackSel{this, "ApplyLooseTrackSel", false};
    Gaudi::Property<bool> m_applyTightTrackSel{this, "ApplyTightTrackSel", false};
    Gaudi::Property<std::string> m_outnode_tau{this, "NodeNameTau", "GN2TauNoAux_pb"};
    Gaudi::Property<std::string> m_outnode_jet{this, "NodeNameJet", "GN2TauNoAux_pu"};  
    Gaudi::Property<float> m_min_prong_track_pt{this, "MinProngTrackPt", 0.};

    // Wrappers for lwtnn
    std::unique_ptr<TauGNN> m_net_inclusive;
    std::unique_ptr<TauGNN> m_net_0p;
    std::unique_ptr<TauGNN> m_net_1p;
    std::unique_ptr<TauGNN> m_net_2p;
    std::unique_ptr<TauGNN> m_net_3p;

    std::unique_ptr<TauGNN> load_network(const std::string& network_file) const;
};

#endif // TAURECTOOLS_TAUGNNEVALUATOR_H
