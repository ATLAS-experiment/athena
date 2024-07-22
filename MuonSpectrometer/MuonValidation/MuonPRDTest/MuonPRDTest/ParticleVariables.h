/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_PARTICLEVARIABLES_H
#define MUONPRDTEST_PARTICLEVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"

#include "xAODBase/IParticleContainer.h"

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
        
            
            template <typename T>
                bool addVariable(const std::string& variable, const std::string& accName ="" ) {
                    return m_branch->addVariable<T>(variable, accName);
                }
            template <typename T>
                bool addVariable(T defaultValue, const std::string& variable, const std::string& accName = ""){
                    return m_branch->addVariable<T>(defaultValue, variable, accName);
                }
            
            template <typename T>
                bool addVariableGeV(const std::string& variable, const std::string& accName = "") {
                    return m_branch->addVariableGeV<T>(variable, accName);
                }
            template <typename T>
                bool addVariableGeV(T defaultValue, const std::string& variable, const std::string& accName = "") {
                    return m_branch->addVariableGeV<T>(defaultValue, variable, accName);
                }

        private:
            SG::ReadHandleKey<xAOD::IParticleContainer> m_key{};
            std::shared_ptr<IParticleFourMomBranch> m_branch{};
    };
}
#endif