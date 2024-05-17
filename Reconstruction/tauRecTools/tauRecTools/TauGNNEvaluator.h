/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUGNNEVALUATOR_H
#define TAURECTOOLS_TAUGNNEVALUATOR_H

#include "tauRecTools/TauRecToolBase.h"

#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include <memory>

class TauGNN;

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
    const TauGNN* get_gnn() const;

    // Selects tracks to be used as input to the network
    StatusCode get_tracks(const xAOD::TauJet &tau,
                          std::vector<const xAOD::TauTrack *> &out) const;

    // Selects clusters to be used as input to the network
    StatusCode get_clusters(const xAOD::TauJet &tau,
                            std::vector<xAOD::CaloVertexedTopoCluster> &out) const;

private:
    std::string m_output_varname;
    std::string m_output_ptau;
    std::string m_output_pjet;
    std::string m_weightfile;
    std::size_t m_max_tracks;
    std::size_t m_max_clusters;
    float m_max_cluster_dr;
    float m_minTauPt;
    bool m_doVertexCorrection;
    bool m_doTrackClassification;
    bool m_decorateTracks;

    // Configuration of the network file
    std::string m_input_layer_scalar;
    std::string m_input_layer_tracks;
    std::string m_input_layer_clusters;
    std::string m_outnode_tau;
    std::string m_outnode_jet;

    // Wrappers for lwtnn
    std::unique_ptr<TauGNN> m_net; //!
};

#endif // TAURECTOOLS_TAUGNNEVALUATOR_H
