/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_MDTDRITFCIRCLEVARIABLES_H
#define PRDTESTERR4_MDTDRITFCIRCLEVARIABLES_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include <unordered_map>
#include <unordered_set>
/** @brief Module to dump the basic properties of the MdtDriftCircle collection
 * 
 */
namespace MuonValR4{

    class MdtDriftCircleVariables: public TesterModuleBase {
        public:
            MdtDriftCircleVariables(MuonTesterTree& tree,
                                    const std::string& inContainer,
                                    MSG::Level msgLvl = MSG::Level::INFO,
                                    const std::string& collName="MdtPrd");

            bool declare_keys() override final;

            
            bool fill(const EventContext& ctx) override final;

            /** @brief Push back the drift circle measurement to the output. 
             *          Returns the position index to which the measurement is pushed to.
             *          Automated deduplication of multiple push_backs of the same measurement 
             *          based on the measurement identifier  */
            unsigned int push_back(const xAOD::MdtDriftCircle& dc);
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
                     const xAOD::MdtDriftCircle& dc);

           SG::ReadHandleKey<xAOD::MdtDriftCircleContainer> m_key{};

           std::string m_collName{};
           /** @brief Identifier of the Mdt */
           MdtIdentifierBranch m_id{parent(), m_collName};
           /** @brief Position of the Mdt drift circle in the global frame */
           ThreeVectorBranch m_globPos{parent(), m_collName+"_globalPos"};
           /** @brief Dirft radius of the associated drit circle */
           VectorBranch<float>& m_driftRadius{parent().newVector<float>(m_collName+"_driftRadius")};
           /** @brief Uncertainty on the drift radius measurement */
           VectorBranch<float>& m_driftRadiusUncert{parent().newVector<float>(m_collName+"_uncertDriftRadius")};
           /** @brief tdc counts of the measurement */
           VectorBranch<uint16_t>& m_tdcCounts{parent().newVector<uint16_t>(m_collName+"_tdc")};
           /** @brief Adc counts of the measurement */
           VectorBranch<uint16_t>& m_adcCounts{parent().newVector<uint16_t>(m_collName+"_adc")};
    
            /// Set of chambers to be dumped
           std::unordered_set<Identifier> m_filteredChamb{};
           /// Map of Identifiers to the position index inside the vector
           std::unordered_map<Identifier, unsigned int> m_idOutIdxMap{};
           /// Vector of PRDs parsed via the external mechanism. These measurements are parsed first
           std::vector<const xAOD::MdtDriftCircle*> m_dumpedPRDS{};
           /// Apply a filter to dump the prds
           bool m_applyFilter{false};
    };
}
#endif