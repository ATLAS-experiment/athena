/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTERTREE_IPARTICLEFOURMOMBRANCH_H
#define MUONTESTERTREE_IPARTICLEFOURMOMBRANCH_H

#include <MuonTesterTree/IMuonTesterBranch.h>
#include <MuonTesterTree/MuonTesterTree.h>
#include <MuonTesterTree/AuxElementBranch.h>
#include <AthenaBaseComps/AthMessaging.h>

namespace MuonVal {
    /** @brief  Helper class to easily to add xAOD::IParticles and associated decorator variables to the 
     *          MuonTesterTree. The branch saves, the particle 4-momentum (E,Pt, Eta, Phi). Energy and Pt 
     *          are saved in units of GeV. Extra information decorated to the particle can also easily appended
     *          to the tree output. 
     * 
     *   Potential setup:          
     *         auto part_br = std::make_shared<IParticleFourMomBranch>(m_testerTree, "TheCakeIsALie");
     *         /// Adds a branch ptcone30 and fetches the infromation from ptcone30
     *         part_br->addVariable<float>("ptcone30");
     *         /// Adds a branch caloIso and fetches the information from etcone20
     *         part_br->addVariable<float>("caloIso", "etcone20");
     *        
     *         m_testerTree.addBranch(part_br);
     *   Then fill the branches:
     *        for (const xAOD::IParticle* part : *container) {
     *            part_br->push_back(part);
     *        }
     */
    class IParticleFourMomBranch : public AthMessaging, public IParticleDecorationBranch {
        public:
            /** @brief Construct an IParticleFourMomBranch
             *  @param tree: Instance of the parental MuonTesterTree to which the IParticleFourMom branch is added
             *  @param particle: Name of the IParticleFourMomBranch inside the TTree 
             */
            IParticleFourMomBranch(MuonTesterTree& tree, const std::string& particle);
            /** @brief Fill the branch */
            bool fill(const EventContext& ctx) override;
            /** @brief Initialize the branches and all children */
            bool init() override;
            /** @brief Name of the four momentum branch */
            std::string name() const override;
            /** @brief Return the underyling TTree pointer */
            const TTree* tree() const override;
            TTree* tree() override;

            /** @brief Access to the MuonTrestTree parent*/
            MuonTesterTree& getTree();
            /** @brief Has the init method been called */
            bool initialized() const;
            /** @brief Returns a list of all Read(Coond)HandleKeys needed by the branch */
            std::vector<DataDependency> data_dependencies() override;
            /** @brief How many particles have been pushed back already */
            size_t size() const;
   
            void push_back(const xAOD::IParticle* p) override;
            void push_back(const xAOD::IParticle& p) override;
            void operator+=(const xAOD::IParticle* p) override;
            void operator+=(const xAOD::IParticle& p) override;

    
            /** @brief Returns the position at which the particle has already be inserted in the chain
                -- If the particle is not present then the size is returned
                -- In cases of nullptr -1 is returned
            */
            size_t find(const xAOD::IParticle& p) const;
            size_t find(const xAOD::IParticle* p) const;
            /** @brief Find the first particle satisfying the selection criteria given by the client.
             *         If none of the particle is actually matching the current size of the branch is returned
            */
            size_t find(std::function<bool(const xAOD::IParticle*)> func) const;
            /** @brief Returns the list of currently cached particles */
            const std::vector<const xAOD::IParticle*>& getCached() const;
            /** @brief Writes a variable of type <T> which can be directly accessed via an SG::AuxElement e.g.
             *             SG::AuxElement::Accessor<float> acc_ptCone30{"pt_cone30"};
             *             const float ptCone30 = acc_ptCone30(*partPtr);
             *         to the TTree. If the variable is not available the filling of the TTree fails
             * @param variable: Name of the branch in the output tree
             * @param accName: Name of the accessor to obtain the information. If left empty, it's assumed 
             *                 that the accesor is the same as the variable name */
            template <typename T>
                bool addVariable(const std::string& variable, 
                                 const std::string& accName = "");
            /** @brief Write a variable of type <T> to the TTree. If the information is not decorated to the
             *         particle a default value is used
             * @param defValue: Default value to use as a backup option
             * @param variable: Name of the branch in the output tree
             * @param accName: Name of the accessor to obtain the information. If left empty, it's assumed 
             *                 that the accesor is the same as the variable name */
            template <typename T>
                bool addVariable(T defaultValue, 
                                 const std::string& variable,
                                 const std::string& accName = "");
            
            /** @brief Write a variable of type <T> to the TTree, but divide it by 1k before dumping. 
             *         Useful for the conversion from MeV to GeV
             * @param variable: Name of the branch in the output tree
|            * @param accName: Name of the accessor to obtain the information. If left empty, it's assumed 
             *                 that the accesor is the same as the variable name */
            template <typename T>
                bool addVariableGeV(const std::string& variable, 
                                    const std::string& accName = "");
             /** @brief Write a variable of type <T> to the TTree, but divide it by 1k before dumping. 
             *         Useful for the conversion from MeV to GeV
             * @param defValue: Default value to use as a backup option
             * @param variable: Name of the branch in the output tree
             * @param accName: Name of the accessor to obtain the information. If left empty, it's assumed 
             *                 that the accesor is the same as the variable name */
            template <typename T>
                bool addVariableGeV(T defaultValue,
                                    const std::string& variable,
                                    const std::string& accName = "");

            /** @brief Returns the point to the underlying ParticleVariableBranch translating the 
             *         IParticle Accessors into TTree branches
             * @param variable: Name of the branch as parsed in addVariable(GeV)
             */
            template <typename T>
                std::shared_ptr<ParticleVariableBranch<T>> getBranch(const std::string& var) const;
            
            bool addVariable(std::shared_ptr<IParticleDecorationBranch> branch);

    
        private:
            MuonTesterTree& m_parent;
            std::string m_name{};
            bool m_init{false};
            bool m_updated{false};
            VectorBranch<float> m_pt{m_parent.tree(), m_name +"_pt"};
            VectorBranch<float> m_eta{m_parent.tree(), m_name + "_eta"};
            VectorBranch<float> m_phi{m_parent.tree(), m_name + "_phi"};
            VectorBranch<float> m_e{m_parent.tree(), m_name + "_e"};
            VectorBranch<int> m_q{m_parent.tree(), m_name+"_q"};

            std::vector<std::shared_ptr<IParticleDecorationBranch>> m_variables{};
            std::vector<const xAOD::IParticle*> m_cached_particles{};
};

}
#include <MuonTesterTree/IParticleFourMomBranch.icc>
#endif
