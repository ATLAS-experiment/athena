/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_TGCSTRIPVARIABLES_H
#define PRDTESTERR4_TGCSTRIPVARIABLES_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"
#include "MuonTesterTree/TwoVectorBranch.h"

#include <unordered_map>
#include <unordered_set>
/** @brief Module to dump the basic properties of the TgcStrip collection
 * 
 */
namespace MuonValR4{

    class TgcStripVariables: public TesterModuleBase {
        public:
            TgcStripVariables(MuonTesterTree& tree,
                             const std::string& inContainer,
                             MSG::Level msgLvl = MSG::Level::INFO,
                             const std::string& collName="TgcPrd");

            bool declare_keys() override final;

            
            bool fill(const EventContext& ctx) override final;

            /** @brief Push back the drift circle measurement to the output. 
             *          Returns the position index to which the measurement is pushed to.
             *          Automated deduplication of multiple push_backs of the same measurement 
             *          based on the measurement identifier  */
            unsigned int push_back(const xAOD::TgcStrip& strip);
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
           void dump(const ActsGeometryContext& gctx,
                     const xAOD::TgcStrip& dc);           

           SG::ReadHandleKey<xAOD::TgcStripContainer> m_key{};

           std::string m_collName{};
           /** @brief Identifier of the Mdt */
           TgcIdentifierBranch m_id{parent(), m_collName};
           /** @brief Position of the Mdt drift circle in the global frame */
           ThreeVectorBranch m_globPos{parent(), m_collName+"_globalPos"};
           /** @brief Local strip position of the measurement  */
           VectorBranch<float>& m_locPos{parent().newVector<float>(m_collName+"_localPos")};
           /** @brief local covariance of the measurement  */
           VectorBranch<float>& m_locCov{parent().newVector<float>(m_collName+"_localCov")};

           /// Set of chambers to be dumped
           std::unordered_set<Identifier> m_filteredChamb{};
           /// Map of Identifiers to the position index inside the vector
           std::unordered_map<Identifier, unsigned int> m_idOutIdxMap{};
           /// Vector of PRDs parsed via the external mechanism. These measurements are parsed first
           std::vector<const xAOD::TgcStrip*> m_dumpedPRDS{};
           /// Apply a filter to dump the prds
           bool m_applyFilter{false};
    };
}
#endif