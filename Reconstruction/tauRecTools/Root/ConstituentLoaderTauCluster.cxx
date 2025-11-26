/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/ConstituentsLoaderTauCluster.h"

namespace FlavorTagInference {
    using FeatureFunc_t = std::function<float(const xAOD::CaloVertexedTopoCluster&, const xAOD::TauJet&)>;
    using FeatureFuncAsReference_t = std::function<bool(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster&, float&)>;
    ConstituentLoaderTauCluster::ConstituentLoaderTauCluster(const ConstituentsInputConfig& cfg, double max_cluster_dr, bool doVertexCorrection) :
        IConstituentsLoader(cfg),
        m_max_cluster_dr(max_cluster_dr),
        m_doVertexCorrection(doVertexCorrection)
    {
        for (const InputVariableConfig& input_var : cfg.inputs) {
            m_feature_extractors.push_back(getFeatureExtractor(input_var.name));
        }
    }

    std::vector<xAOD::CaloVertexedTopoCluster> ConstituentLoaderTauCluster::getTauClusters(const xAOD::TauJet* tau) const {
        std::vector<xAOD::CaloVertexedTopoCluster> clusters;
        TLorentzVector tauAxis = tauRecTools::getTauAxis(*tau, m_doVertexCorrection);
        for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : tau->vertexedClusters()) {
            TLorentzVector clusterP4 = vertexedCluster.p4();
            if (clusterP4.DeltaR(tauAxis) > m_max_cluster_dr) continue;
            clusters.push_back(vertexedCluster);
        }
        if (m_config.order == ConstituentsSortOrder::PT_DESCENDING) {
            // Sort by descending et
            auto et_cmp = [](xAOD::CaloVertexedTopoCluster& lhs, xAOD::CaloVertexedTopoCluster& rhs) {
                return lhs.p4().Et() > rhs.p4().Et();
            };
            std::sort(clusters.begin(), clusters.end(), et_cmp);
        } else {
            // throw
            throw std::runtime_error("Unsupported sorting order");
        }
        // Truncate clusters
        if (clusters.size() > m_config.max_n_constituents) {
            clusters.resize(m_config.max_n_constituents, clusters[0]);
        }
            return clusters;
    }


    Inputs ConstituentLoaderTauCluster::getFeatures(const xAOD::TauJet* tau, const std::vector<xAOD::CaloVertexedTopoCluster>& tau_clusters) const {
        std::vector<int64_t> features_dim = {static_cast<int64_t>(tau_clusters.size()), static_cast<int64_t>(m_feature_extractors.size())};
        std::vector<float> features;
        features.reserve(tau_clusters.size() * m_feature_extractors.size());
        for (const auto& cluster : tau_clusters) {
            for (const auto& extractor : m_feature_extractors) {
                features.push_back(extractor(cluster, *tau));
            }
        }
        return Inputs{std::move(features), std::move(features_dim)};
    }

    std::tuple<Inputs, std::vector<const xAOD::IParticle*>> ConstituentLoaderTauCluster::getData(const xAOD::IParticle& i_tau) const {
        auto tau = dynamic_cast<const xAOD::TauJet*>(&i_tau);
        std::vector<xAOD::CaloVertexedTopoCluster> sorted_tau_cls = getTauClusters(tau);
        return std::make_tuple(getFeatures(tau, sorted_tau_cls), std::vector<const xAOD::IParticle*>{} );
    }

    FeatureFunc_t ConstituentLoaderTauCluster::getFeatureExtractor(const std::string& var_name) const {
        FeatureFuncAsReference_t func_as_ref = nullptr;
        try {
            func_as_ref = m_func_map.at(var_name);
        } catch (const std::out_of_range &e) {
            throw std::runtime_error("Variable '" + var_name + "' not defined");
        }
        return [func_as_ref](const xAOD::CaloVertexedTopoCluster& cls, const xAOD::TauJet& tau) {
            float out;
            bool success = func_as_ref(tau, cls, out);
            if (!success) {
                throw std::runtime_error("Error in cluster variable calculation");
            }
            return out;
        };
    }

    const std::string& ConstituentLoaderTauCluster::getName() const {
        return m_name;
    }
    const ConstituentsType& ConstituentLoaderTauCluster::getType() const {
        return m_config.type;
    }
    const FTagDataDependencyNames& ConstituentLoaderTauCluster::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& ConstituentLoaderTauCluster::getUsedRemap() const {
        return m_used_remap;
    }
}


namespace TauClusterVars {
using MomentType = xAOD::CaloCluster::MomentType;

bool et_log(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    out = std::log10(cluster.p4().Et());
    return true;
}

bool pt_tau_log(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster& /*cluster*/, float &out) {
    out = std::log10(std::max(tau.pt(), 1e-6));
    return true;
}

bool pt_jetseed_log(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster& /*cluster*/, float &out) {
    out = std::log10(tau.ptJetSeed());
    return true;
}

bool dEta(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    out = cluster.eta() - tau.eta();
    return true;
}

bool dPhi(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    out = cluster.p4().DeltaPhi(tau.p4());
    return true;
}

bool SECOND_R(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double double_out;
    const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_R, double_out);
    out = static_cast<float>(double_out);
    return success;
}

bool SECOND_LAMBDA(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double double_out;
    const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_LAMBDA, double_out);
    out = static_cast<float>(double_out);
    return success;
}

bool CENTER_LAMBDA(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double double_out;
    const auto success = cluster.clust().retrieveMoment(MomentType::CENTER_LAMBDA, double_out);
    out = static_cast<float>(double_out);
    return success;
}

bool SECOND_LAMBDAOverClustersMeanSecondLambda(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
  static const SG::ConstAccessor<float> acc_ClustersMeanSecondLambda("ClustersMeanSecondLambda");
  float ClustersMeanSecondLambda = acc_ClustersMeanSecondLambda(tau);
  double secondLambda(0);
  const auto success = cluster.clust().retrieveMoment(MomentType::SECOND_LAMBDA, secondLambda);
  out = (ClustersMeanSecondLambda != 0.) ? secondLambda/ClustersMeanSecondLambda : 0.;
  return success;
}

bool CENTER_LAMBDAOverClustersMeanCenterLambda(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
  static const SG::ConstAccessor<float> acc_ClustersMeanCenterLambda("ClustersMeanCenterLambda");
  float ClustersMeanCenterLambda = acc_ClustersMeanCenterLambda(tau);
  double centerLambda(0);
  const auto success = cluster.clust().retrieveMoment(MomentType::CENTER_LAMBDA, centerLambda);
  if (ClustersMeanCenterLambda == 0.){
    out = 250.;
  }else {
    out = centerLambda/ClustersMeanCenterLambda;
  }

  out = std::min(out, 250.0f);

  return success;
}


bool FirstEngDensOverClustersMeanFirstEngDens(const xAOD::TauJet &tau, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
  // the ClustersMeanFirstEngDens is the log10 of the energy weighted average of the First_ENG_DENS 
  // divided by ETot to make it dimension-less, 
  // so we need to evaluate the difference of log10(clusterFirstEngDens/clusterTotalEnergy) and the ClustersMeanFirstEngDens
  double clusterFirstEngDens = 0.0;
  bool status = cluster.clust().retrieveMoment(MomentType::FIRST_ENG_DENS, clusterFirstEngDens);
  if (clusterFirstEngDens < 1e-6) clusterFirstEngDens = 1e-6;

  static const SG::ConstAccessor<float> acc_ClusterTotalEnergy("ClusterTotalEnergy");
  float clusterTotalEnergy = acc_ClusterTotalEnergy(tau);
  if (clusterTotalEnergy < 1e-6) clusterTotalEnergy = 1e-6;

  static const SG::ConstAccessor<float> acc_ClustersMeanFirstEngDens("ClustersMeanFirstEngDens");
  float clustersMeanFirstEngDens = acc_ClustersMeanFirstEngDens(tau);

  out = std::log10(clusterFirstEngDens/clusterTotalEnergy) - clustersMeanFirstEngDens;
  
  return status;
}

//Extension - Variables for GNTau
bool e(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    out = cluster.p4().E();
    return true;
}

bool et(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    out = cluster.p4().Et();
    return true;
}

bool FIRST_ENG_DENS(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double clusterFirstEngDens = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::FIRST_ENG_DENS, clusterFirstEngDens);
    out = clusterFirstEngDens;
    return status;
}

bool EM_PROBABILITY(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double clusterEMprob = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::EM_PROBABILITY, clusterEMprob);
    out = clusterEMprob;
    return status;
}

bool CENTER_MAG(const xAOD::TauJet& /*tau*/, const xAOD::CaloVertexedTopoCluster &cluster, float &out) {
    double clusterCenterMag = 0.0;
    bool status = cluster.clust().retrieveMoment(MomentType::CENTER_MAG, clusterCenterMag);
    out = clusterCenterMag;
    return status;
}

} // namespace Cluster
