/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_PARTICLEVARIABLES_H
#define MUONPRDTEST_PARTICLEVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"

#include "xAODBase/IParticleContainer.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

namespace MuonPRDTest{
    /** @brief module to blindly dump a particle container to the Tree */
    class ParticleVariables : public PrdTesterModule {
        public:
            ParticleVariables(MuonTesterTree& tree, 
                              const std::string& containerKey,
                              const std::string& outName,
                              MSG::Level msglvl);

            bool fill(const EventContext& ctx) override final;
            bool declare_keys() override final;
            /** @brief Adds a variable of the primitive data type that can be directly read-off
             *         from the xAOD::IParticle as decoration
             *  @param variable: Name of the variable in the output tree. The output variable 
             *                   matches the name of the decorator used to read it
             *  @param declareDep: Switch whether the module shall declare a dependency on the decorator */
            template <typename T>
                bool addVariable(const std::string& variable, const bool declareDep = false) {
                    if (declareDep) {
                        declare_decorator(variable);
                    } 
                    return m_branch->addVariable<T>(variable);
                }
            /** @brief Adds a variable of the primitive data type that can be directly read-off
             *         from the xAOD::IParticle as decoration
             *  @param variable: Name of the variable in the output tree. 
             *  @param accName: Use a different decorator name than for the output
             *  @param declareDep: Switch whether the module shall declare a dependency on the decorator */
            template <typename T>
            bool addVariable(const std::string& variable, const std::string& accName, 
                             const bool declareDep = false) {
                if (declareDep){
                    declare_decorator(variable);
                }
                return m_branch->addVariable<T>(variable, accName);
            }
            /** @brief Adds a variable of the primitive data type that can be directly read-off
             *         from the xAOD::IParticle as decoration. Before storing the value to the tree
             *         it's divided by 1000.
             *  @param variable: Name of the variable in the output tree. The output variable 
             *                   matches the name of the decorator used to read it
             *  @param declareDep: Switch whether the module shall declare a dependency on the decorator */
            template <typename T>
                bool addVariableGeV(const std::string& variable, const bool declareDep = false) {
                    if (declareDep) {
                        declare_decorator(variable);
                    } 
                    return m_branch->addVariableGeV<T>(variable);
                }
            /** @brief Adds a variable of the primitive data type that can be directly read-off
             *         from the xAOD::IParticle as decoration. Before storing the value to the tree
             *         it's divided by 1000.
             *  @param variable: Name of the variable in the output tree. 
             *  @param accName: Use a different decorator name than for the output
             *  @param declareDep: Switch whether the module shall declare a dependency on the decorator */
            template <typename T>
            bool addVariableGeV(const std::string& variable, const std::string& accName, 
                                const bool declareDep = false) {
                if (declareDep){
                    declare_decorator(variable);
                }
                return m_branch->addVariableGeV<T>(variable, accName);
            }
            /** @brief Adds a generic IParticle decoration branch to the collection */
            bool addVariable(std::shared_ptr<IParticleDecorationBranch> branch);
            size_t push_back(const xAOD::IParticle* p);

            /** @brief Declares the dependency on a decorator
             *  @param decorName: Name of the decorator to declare. May not be empty */
            void declare_decorator(const std::string& decorName);

        private:
            SG::ReadHandleKey<xAOD::IParticleContainer> m_key{};
            using DecorKey_t = SG::ReadDecorHandleKey<xAOD::IParticleContainer>;
            SG::ReadDecorHandleKeyArray<xAOD::IParticleContainer> m_decorKeys{};
            std::shared_ptr<IParticleFourMomBranch> m_branch{};
    };
}
#endif