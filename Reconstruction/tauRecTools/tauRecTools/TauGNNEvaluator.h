/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNNEVALUATOR_H
#define TAURECTOOLS_TAUGNNEVALUATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/TauGNN.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

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
    // Getter for the underlying RNN implementation
    inline const TauGNN* get_gnn_inclusive() const { return m_net_inclusive.get(); }
    inline const TauGNN* get_gnn_0p() const { return m_net_0p.get(); }
    inline const TauGNN* get_gnn_1p() const { return m_net_1p.get(); }
    inline const TauGNN* get_gnn_2p() const { return m_net_2p.get(); }
    inline const TauGNN* get_gnn_3p() const { return m_net_3p.get(); }

    // Selects tracks to be used as input to the network
    StatusCode get_tracks(const xAOD::TauJet &tau,
                          std::vector<const xAOD::TauTrack *> &out) const;

    // Selects clusters to be used as input to the network
    StatusCode get_clusters(const xAOD::TauJet &tau,
                            std::vector<xAOD::CaloVertexedTopoCluster> &out) const;

    // Selects hits to be used as input to the network
    StatusCode get_hits(const xAOD::TauJet &tau,
                            std::vector<const xAOD::TrackMeasurementValidation*> &out) const;

    enum Discriminant {
        Disabled = -1,
        NegLogPJet = 0,
        PTau = 1
    };

private:

    Gaudi::Property<std::string> m_tauContainerName{this, "TauContainerName", "", "Name of TauJetContainer, must be set when using "};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_scoreHandleKey{this, "ScoreHandleKey","","Output Score"};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_pTauHandleKey{this, "PTauHandleKey","","Output pTau"};
    SG::WriteDecorHandleKey<xAOD::TauJetContainer> m_pJetHandleKey{this, "PJetHandleKey","","Output pJet"};

    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_hitsHandleKey{this, "HitsHandleKey", "", "Hits decoration key"};
   
    std::string m_output_varname;
    std::string m_output_ptau;
    std::string m_output_pjet;
    int m_output_discriminant;

    std::string m_weightfile_inclusive;
    std::string m_weightfile_0p;
    std::string m_weightfile_1p;
    std::string m_weightfile_2p;
    std::string m_weightfile_3p;
    float m_min_prong_track_pt;

    int m_max_tracks;
    int m_max_clusters;
    float m_max_cluster_dr;
    int m_max_hits;
    float m_minTauPt;
    bool m_applyTrackSel;
    bool m_doVertexCorrection;
    bool m_doTrackClassification;
    bool m_decorateTracks;

    // Configuration of the network file
    std::string m_input_layer_scalar;
    std::string m_input_layer_tracks;
    std::string m_input_layer_clusters;
    std::string m_input_layer_hits;
    std::string m_outnode_tau;
    std::string m_outnode_jet;

    // Wrappers for lwtnn
    std::unique_ptr<TauGNN> m_net_inclusive;
    std::unique_ptr<TauGNN> m_net_0p;
    std::unique_ptr<TauGNN> m_net_1p;
    std::unique_ptr<TauGNN> m_net_2p;
    std::unique_ptr<TauGNN> m_net_3p;

    std::unique_ptr<TauGNN> load_network(const std::string& network_file, const TauGNN::Config& config) const;
};

#endif // TAURECTOOLS_TAUGNNEVALUATOR_H
