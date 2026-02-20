/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/ConstituentsLoaderTauHit.h"

namespace FlavorTagInference {
    ConstituentLoaderTauHit::ConstituentLoaderTauHit(const ConstituentsInputConfig& cfg, const std::string& hits_key) :
        IConstituentsLoader(cfg),
        m_hits_key{hits_key}
    {
        for(const InputVariableConfig& input_var : cfg.inputs) {
            m_feature_extractors.push_back(getFeatureExtractor(input_var.name));
        }
    }


    std::tuple<Inputs, std::vector<const xAOD::IParticle*>> ConstituentLoaderTauHit::getData(const xAOD::IParticle& p) const
    {
        const std::vector<const xAOD::TrackMeasurementValidation*> hits = getParticleHits(p);
        return std::make_tuple(getFeatures(p, hits), std::vector<const xAOD::IParticle*>{} );
    }


    const std::vector<const xAOD::TrackMeasurementValidation*> ConstituentLoaderTauHit::getParticleHits(const xAOD::IParticle& p) const
    {
        static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::TrackMeasurementValidationContainer>>> hitsHandle(m_hits_key);

        std::vector<const xAOD::TrackMeasurementValidation*> hits;
        for(const ElementLink<xAOD::TrackMeasurementValidationContainer>& el : hitsHandle(p)) {
            if(el.isValid()) hits.push_back(*el);

            // Truncate hits input (already sorted by the upstream JetHitAssociationAlg)
            if(hits.size() == m_config.max_n_constituents) break;
        }

        return hits;
    }


    const Eigen::Matrix3d ConstituentLoaderTauHit::getJABInvMatrix(const xAOD::IParticle& p) const
    {
        // We often work with hits associated to a particle in the following coordinates:
        //  - jet (particle axis) projection, 
        //  - adjacent projection (orthogonal to the jet axis and the beam), 
        //  - beamline projection.
        // Since the jet and the beam aren't perpendicular for eta != 0, this isn't a fully-orthogonal basis.
        //
        // We also approximate the eta-phi coordinates of the jet to be approximately constant w.r.t. the event vertex,
        // considering the very-small displacements of the beamspot in the x-y plane, and the relatively small difference 
        // in eta with small (+/- 50mm) changes in z. We might need to revisit this approximation, but we'll use it for now.
        
        // Use zhat as the beamline
        Eigen::Vector3d bhat(0, 0, 1);

        const TLorentzVector p4 = p.p4();
        Eigen::Vector3d jhat = Eigen::Vector3d(p4.X(), p4.Y(), p4.Z()).normalized();

        Eigen::Vector3d ahat = bhat.cross(jhat).normalized();

        // Build the matrix m that maps the jab displacement such that m*jab = detector
        Eigen::Matrix3d m;
        m << jhat, ahat, bhat;

        return m.inverse();
    }


    const Inputs ConstituentLoaderTauHit::getFeatures(const xAOD::IParticle& p, const std::vector<const xAOD::TrackMeasurementValidation*>& hits) const
    {
        std::vector<int64_t> features_dim = {static_cast<int64_t>(hits.size()), static_cast<int64_t>(m_feature_extractors.size())};
        
        const Eigen::Matrix3d jav_inv = getJABInvMatrix(p);

        std::vector<float> features;
        features.reserve(hits.size() * m_feature_extractors.size());
        for(const xAOD::TrackMeasurementValidation* hit : hits) {
            for(const FeatureFunc_t& extractor : m_feature_extractors) {
                features.push_back(extractor(p, *hit, jav_inv));
            }
        }

        return Inputs{std::move(features), std::move(features_dim)};
    }


    ConstituentLoaderTauHit::FeatureFunc_t ConstituentLoaderTauHit::getFeatureExtractor(const std::string& var_name) const
    {
        try {
            return m_func_map.at(var_name);
        } catch (const std::out_of_range &e) {
            throw std::runtime_error("Variable '" + var_name + "' not defined");
        }
    }

} // namespace FlavorTagInference



namespace TauHitVars {

float j(const xAOD::IParticle& /*p*/, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d& jab_inv) {
    static const SG::ConstAccessor<float> acc_localX("HitsXRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localY("HitsYRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localZ("HitsZRelToBeamspot");

    return jab_inv(0, 0)*acc_localX(hit) + jab_inv(0, 1)*acc_localY(hit) + jab_inv(0, 2)*acc_localZ(hit);
}


float a(const xAOD::IParticle& /*p*/, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d& jab_inv) {
    static const SG::ConstAccessor<float> acc_localX("HitsXRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localY("HitsYRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localZ("HitsZRelToBeamspot");

    return jab_inv(1, 0)*acc_localX(hit) + jab_inv(1, 1)*acc_localY(hit) + jab_inv(1, 2)*acc_localZ(hit);
}


float b(const xAOD::IParticle& /*p*/, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d& jab_inv) {
    static const SG::ConstAccessor<float> acc_localX("HitsXRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localY("HitsYRelToBeamspot");
    static const SG::ConstAccessor<float> acc_localZ("HitsZRelToBeamspot");

    return jab_inv(2, 0)*acc_localX(hit) + jab_inv(2, 1)*acc_localY(hit) + jab_inv(2, 2)*acc_localZ(hit);
}


float layer(const xAOD::IParticle& /*p*/, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d& /*jab_inv*/) {
    static const SG::AuxElement::ConstAccessor<int> acc_layer("layer");
    return acc_layer(hit);
}

} // namespace TauHitVars
