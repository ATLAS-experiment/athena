/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once
#include "xAODBase/IParticle.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"
#include "FlavorTagInference/ISaltModel.h"

#include "FlavorTagInference/TracksLoader.h"
#include "FlavorTagInference/FlowElementsLoader.h"
#include "FlavorTagInference/HitsLoader.h"
#include "FlavorTagInference/ElectronsLoader.h"

#include <map>
#include <vector>
#include <utility>
#include <functional>

namespace FlavorTagInference {

    using Inputs = std::pair<std::vector<float>, std::vector<int64_t>>;
    using SaltModelInputs = std::map<std::string, Inputs>;

    struct SaltModelData {
        SaltModelInputs gnn_inputs;
        size_t num_inputs = 0;
        std::map<std::string, std::vector<const xAOD::IParticle*>> constituents;
    };

    class SaltModelEDMLoaderBase {
    public:
        SaltModelEDMLoaderBase(ISaltModelPtr salt_model) :
            graph_config(salt_model->getGraphConfig()) {};
        SaltModelGraphConfig::GraphConfig graph_config;
        std::string scalarInputName;
        std::vector<std::pair<std::string /* varName */, std::function<float(const xAOD::IParticle* /* parent */)>>> scalarVarLoaders;
        std::map<std::string /* vecInputName */, std::shared_ptr<IConstituentsLoader>> vectorVarLoaders;


        void addScalarLoader(const std::string& varName, std::function<float(const xAOD::IParticle*)> loader) {
            scalarVarLoaders.emplace_back(varName, loader);
        }

        void addVectorLoader(const std::string& vecName, std::shared_ptr<IConstituentsLoader> loader) {
            vectorVarLoaders.try_emplace(vecName, std::move(loader));
        }

        virtual SaltModelData loadInputs(const xAOD::IParticle* p) const final {
            SaltModelData salt_model_data;
            // loading scalar inputs.
            std::vector<float> scalar_feat;
            for (const auto& varLoader : scalarVarLoaders) {
                std::string varName = varLoader.first;
                scalar_feat.push_back(varLoader.second(p));
            }
            std::vector<int64_t> scalar_feat_dim = {1, static_cast<int64_t>(scalar_feat.size())};
            Inputs scalar_inputs(scalar_feat, scalar_feat_dim);
            salt_model_data.gnn_inputs.insert({scalarInputName, scalar_inputs});

            //load vector inputs.
            for (auto loader : vectorVarLoaders) {
                std::string input_name = loader.first;
                auto [input_data, input_objects] = loader.second->getData(*p);

                salt_model_data.gnn_inputs.insert({input_name, input_data});
                salt_model_data.num_inputs += input_data.first.size();
                salt_model_data.constituents[input_name] = input_objects;
            }
            return salt_model_data;
        }

        void DumpGnnInputs(const SaltModelInputs& gnn_inputs) const {
            // Implementation for dumping GNN input data
            std::cout << "-------- Dumping GNN Input Data --------" << std::endl;

            for (const auto& [name, inputs] : gnn_inputs) {
                std::cout << "Input Name: " << name << std::endl;
                std::cout << "  vec floats: ";
                for (const auto& feature : inputs.first) {
                    std::cout << feature << " ";
                }
                std::cout << std::endl;
                std::cout << "  vec ints  : ";
                for (const auto& id : inputs.second) {
                    std::cout << id << " ";
                }
                std::cout << std::endl;
            }
            std::cout << "---------- END GNN Input Data ----------" << std::endl;
        }
    }; // class SaltModelEDMLoaderBase
} // namespace FlavorTagInference
