/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the TrackParticleValidation attached
  to a tau by the JetHitAssociationAlg, and extract their features for the NN evaluation.
*/

#pragma once

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include <xAODBase/IParticle.h>
#include <xAODTracking/TrackMeasurementValidationContainer.h>

// Eigen is needed for Vector3D
#include "GeoPrimitives/GeoPrimitives.h"

// STL includes
#include <string>
#include <vector>
#include <functional>


namespace TauHitVars {

float j(const xAOD::IParticle &p, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d &jab_inv);

float a(const xAOD::IParticle &p, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d &jab_inv);

float b(const xAOD::IParticle &p, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d &jab_inv);

float layer(const xAOD::IParticle &p, const xAOD::TrackMeasurementValidation &hit, const Eigen::Matrix3d &jab_inv);

} // namespace TauHitVars


namespace FlavorTagInference {
    // Subclass for IParticles loader inherited from abstract IConstituentsLoader class
    class ConstituentLoaderTauHit : public IConstituentsLoader {
      public:
        ConstituentLoaderTauHit(const ConstituentsInputConfig& cfg, const std::string& hits_key);

        std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(const xAOD::IParticle& p) const override ;

        inline const std::string& getName() const override { return m_name; }
        inline const ConstituentsType& getType() const override { return m_config.type; }
        inline const FTagDataDependencyNames& getDependencies() const override { return m_deps; }
        inline const std::set<std::string>& getUsedRemap() const override { return m_used_remap; }

        using FeatureFunc_t = std::function<float(const xAOD::IParticle&, const xAOD::TrackMeasurementValidation&, const Eigen::Matrix3d&)>;
      private:
        const std::string m_hits_key;
        const std::vector<const xAOD::TrackMeasurementValidation*> getParticleHits(const xAOD::IParticle& p) const;

        const Eigen::Matrix3d getJABInvMatrix(const xAOD::IParticle& p) const;

        const Inputs getFeatures(const xAOD::IParticle& p, const std::vector<const xAOD::TrackMeasurementValidation*>& hits) const;

        std::vector<FeatureFunc_t> m_feature_extractors;
        FeatureFunc_t getFeatureExtractor(const std::string& var_name) const;
        inline static const std::unordered_map<std::string, FeatureFunc_t> m_func_map = {
            {"j",                         TauHitVars::j},
            {"a",                         TauHitVars::a},
            {"b",                         TauHitVars::b},
            {"layer",                     TauHitVars::layer}
        };
    };
}

