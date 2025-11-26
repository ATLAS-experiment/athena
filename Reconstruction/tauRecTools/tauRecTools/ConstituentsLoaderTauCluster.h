/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the CaloVertexedTopoClusters from the tau 
  and extract their features for the NN evaluation.
*/

#pragma once

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include <xAODTau/TauJet.h>
#include <tauRecTools/HelperFunctions.h>
#include <xAODCaloEvent/CaloVertexedTopoCluster.h>

// STL includes
#include <string>
#include <vector>
#include <functional>


namespace TauClusterVars {

bool et_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool pt_tau_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool pt_jetseed_log(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool dEta(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool dPhi(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_R(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_LAMBDA(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool SECOND_LAMBDAOverClustersMeanSecondLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_LAMBDAOverClustersMeanCenterLambda(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool FirstEngDensOverClustersMeanFirstEngDens(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

//Extension - Variables for GNTau
bool e(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool et(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool FIRST_ENG_DENS(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool EM_PROBABILITY(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);

bool CENTER_MAG(
    const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out);
} // namespace TauClusterVars


namespace FlavorTagInference {
    // Subclass for IParticles loader inherited from abstract IConstituentsLoader class
    class ConstituentLoaderTauCluster : public IConstituentsLoader {
      public:
        ConstituentLoaderTauCluster(const ConstituentsInputConfig& cfg, double max_cluster_dr, bool doVertexCorrection);
        std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(const xAOD::IParticle& p) const override ;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
      private:
        double m_max_cluster_dr;
        bool m_doVertexCorrection = false;
        using FeatureFunc_t = std::function<float(const xAOD::CaloVertexedTopoCluster&, const xAOD::TauJet&)>;
        using FeatureFuncAsReference_t = std::function<bool(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster&, float&)>;
        std::vector<FeatureFunc_t> m_feature_extractors;
        FeatureFunc_t getFeatureExtractor(const std::string& var_name) const;
        std::vector<xAOD::CaloVertexedTopoCluster> getTauClusters(const xAOD::TauJet* tau) const;
        Inputs getFeatures(const xAOD::TauJet* tau, const std::vector<xAOD::CaloVertexedTopoCluster>& tau_clusters) const;
        inline static const std::unordered_map<std::string, FeatureFuncAsReference_t> m_func_map = {
            {"dEta",                      TauClusterVars::dEta},
            {"dPhi",                      TauClusterVars::dPhi},
            {"SECOND_R",                  TauClusterVars::SECOND_R},
            {"SECOND_LAMBDA",             TauClusterVars::SECOND_LAMBDA},
            {"CENTER_LAMBDA",             TauClusterVars::CENTER_LAMBDA},
            {"et",                        TauClusterVars::et}
        };
    };
}

