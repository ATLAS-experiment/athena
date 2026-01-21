/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUJETRNNEVALUATOR_H
#define TAURECTOOLS_TAUJETRNNEVALUATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include <memory>

class TauJetRNN;

/**
 * @brief Tool to calculate a tau identification score based on neural networks
 *
 *   The network configuration is supplied in .json format for both 1- and
 *   3-prong taus separately.
 *
 * @author C. Deutsch
 * @author W. Davey
 *
 */
class TauJetRNNEvaluator : public TauRecToolBase {
public:
    ASG_TOOL_CLASS2(TauJetRNNEvaluator, TauRecToolBase, ITauToolBase)

    TauJetRNNEvaluator(const std::string &name = "TauJetRNNEvaluator");
    virtual ~TauJetRNNEvaluator();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(xAOD::TauJet &tau) const override;

    // Selects tracks to be used as input to the network
    StatusCode get_tracks(const xAOD::TauJet &tau,
                          std::vector<const xAOD::TauTrack *> &out) const;

    // Selects clusters to be used as input to the network
    StatusCode get_clusters(const xAOD::TauJet &tau,
                            std::vector<xAOD::CaloVertexedTopoCluster> &out) const;

private:

    // properties
    Gaudi::Property<std::string> m_weightfile_0p{this, "NetworkFile0P", ""};
    Gaudi::Property<std::string> m_weightfile_1p{this, "NetworkFile1P", ""};
    Gaudi::Property<std::string> m_weightfile_2p{this, "NetworkFile2P", ""};
    Gaudi::Property<std::string> m_weightfile_3p{this, "NetworkFile3P", ""};  
    Gaudi::Property<std::string> m_output_varname{this, "OutputVarname", "RNNJetScore"};
    Gaudi::Property<std::size_t> m_max_tracks{this, "MaxTracks", 10};
    Gaudi::Property<std::size_t> m_max_clusters{this, "MaxClusters", 6};
    Gaudi::Property<float> m_max_cluster_dr{this, "MaxClusterDR", 1.0f};
    Gaudi::Property<bool> m_doVertexCorrection{this, "VertexCorrection", true};
    Gaudi::Property<bool> m_doTrackClassification{this, "TrackClassification", true};
    Gaudi::Property<bool> m_useTRT{this, "useTRT", true};
    Gaudi::Property<std::string> m_input_layer_scalar{this, "InputLayerScalar", "scalar"};
    Gaudi::Property<std::string> m_input_layer_tracks{this, "InputLayerTracks", "tracks"};
    Gaudi::Property<std::string> m_input_layer_clusters{this, "InputLayerClusters", "clusters"};
    Gaudi::Property<std::string> m_output_layer{this, "OutputLayer", "rnnid_output"};
    Gaudi::Property<std::string> m_output_node{this, "OutputNode", "sig_prob"};
    Gaudi::Property<bool> m_applyLooseTrackSel{this, "ApplyLooseTrackSel", false};

    // Wrappers for lwtnn
    std::unique_ptr<TauJetRNN> m_net_0p; //!
    std::unique_ptr<TauJetRNN> m_net_1p; //!
    std::unique_ptr<TauJetRNN> m_net_2p; //!
    std::unique_ptr<TauJetRNN> m_net_3p; //!
};

#endif // TAURECTOOLS_TAUJETRNNEVALUATOR_H
