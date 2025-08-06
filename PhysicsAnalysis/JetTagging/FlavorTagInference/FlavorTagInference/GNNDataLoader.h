/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once
#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/DataPrepUtilities.h"

#include "FlavorTagInference/TracksLoader.h"
#include "FlavorTagInference/FlowElementsLoader.h"
#include "FlavorTagInference/HitsLoader.h"
#include "FlavorTagInference/ElectronsLoader.h"

namespace FlavorTagInference {
    
    using Inputs = std::pair<std::vector<float>, std::vector<int64_t>>;
    using SaltModelInputs = std::map<std::string, Inputs>;

    struct SaltModelData {
        SaltModelInputs gnn_inputs;
        size_t num_inputs = 0;
        std::map<std::string, std::vector<const xAOD::IParticle*>> constituents;
    };

    class GNNDataLoader {
    public:
        GNNDataLoader(std::shared_ptr<const SaltModel> salt_model, const GNNOptions& opts);
        SaltModelData loadInputs(const xAOD::IParticle* p) const;
        std::shared_ptr<const SaltModel> salt_model;
        SaltModelGraphConfig::GraphConfig graph_config;
        FTagOptions ftag_options;
        std::vector<internal::VarFromJet> vars_from_jet;
        std::vector<std::shared_ptr<IConstituentsLoader>> constituents_loaders;
        FTagDataDependencyNames data_dependency_names;
    private:
        GNNOptions m_gnn_options;
    };
} // namespace FlavorTagInference