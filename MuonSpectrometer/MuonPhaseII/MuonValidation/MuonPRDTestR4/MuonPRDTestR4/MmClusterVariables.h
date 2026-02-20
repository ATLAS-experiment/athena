/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_MMCLUSTERVARIABLES_H
#define PRDTESTERR4_MMCLUSTERVARIABLES_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"

#include <unordered_map>
#include <unordered_set>
/** @brief Module to dump the basic properties of the MmCluster collection
 * 
 */
namespace MuonValR4{

    class MmClusterVariables: public TesterModuleBase {
        public:
            MmClusterVariables(MuonTesterTree& tree,
                               const std::string& inContainer,
                               MSG::Level msgLvl = MSG::Level::INFO,
                               const std::string& collName="MmPrd");

            bool declare_keys() override final;

            
            bool fill(const EventContext& ctx) override final;

            /** @brief Push back the drift circle measurement to the output. 
             *          Returns the position index to which the measurement is pushed to.
             *          Automated deduplication of multiple push_backs of the same measurement 
             *          based on the measurement identifier  */
            unsigned int push_back(const xAOD::MMCluster& cluster);
            /** @brief All hits from this particular chamber identifier are dumped to the output
             *         including the ones from the first and the second multilayer.
             *         Activates the external selection behaviour of the branch
             * */
            void dumpAllHitsInChamber(const Identifier& chamberId);
            /** @brief Activates the seeded dump of the branch. Only hits that are parsed either directly or
             *         which are included by the dumpAllHitsInChamber are added to the output
             */
            void enableSeededDump(); 
        private:
           void dump(const ActsTrk::GeometryContext& gctx,
                     const xAOD::MMCluster& dc);

           SG::ReadHandleKey<xAOD::MMClusterContainer> m_key{};

           std::string m_collName{};
           /** @brief Identifier of the Mdt */
           MmIdentifierBranch m_id{parent(), m_collName};
           /** @brief Position of the Mdt drift circle in the global frame */
           ThreeVectorBranch m_globPos{parent(), m_collName+"_globalPos"};
           /** @brief Local strip position of the measurement  */
           VectorBranch<float>& m_locPos{parent().newVector<float>(m_collName+"_localPos")};
           /** @brief local covariance of the measurement  */
           VectorBranch<float>& m_locCov{parent().newVector<float>(m_collName+"_localCov")};
           /** @brief cluster time */
           VectorBranch<uint16_t>& m_time{parent().newVector<uint16_t>(m_collName+"_time")};
           /** @brief cluster charge */
           VectorBranch<uint32_t>& m_charge{parent().newVector<uint32_t>(m_collName+"_charge")};
           /** @brief cluster author */
           VectorBranch<uint8_t>& m_author{parent().newVector<uint8_t>(m_collName+"_author")};
           /// Set of chambers to be dumped
           std::unordered_set<Identifier> m_filteredChamb{};
           /// Map of Identifiers to the position index inside the vector
           std::unordered_map<Identifier, unsigned int> m_idOutIdxMap{};
           /// Vector of PRDs parsed via the external mechanism. These measurements are parsed first
           std::vector<const xAOD::MMCluster*> m_dumpedPRDS{};
           /// Apply a filter to dump the prds
           bool m_applyFilter{false};
    };
}
#endif