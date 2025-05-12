/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_MDTTWINDRITFCIRCLEVARIABLES_H
#define PRDTESTERR4_MDTTWINDRITFCIRCLEVARIABLES_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"

#include <unordered_map>
#include <unordered_set>
/** @brief Module to dump the basic properties of the MdtTwinDriftCircle collection
 * 
 */
namespace MuonValR4{

    class MdtTwinDriftCircleVariables: public TesterModuleBase {
        public:
            MdtTwinDriftCircleVariables(MuonTesterTree& tree,
                                        const std::string& inContainer,
                                        MSG::Level msgLvl = MSG::Level::INFO,
                                        const std::string& collName="MdtTwinPrd");

            bool declare_keys() override final;

            
            bool fill(const EventContext& ctx) override final;

            /** @brief Push back the drift circle measurement to the output. 
             *          Returns the position index to which the measurement is pushed to.
             *          Automated deduplication of multiple push_backs of the same measurement 
             *          based on the measurement identifier  */
            unsigned int push_back(const xAOD::MdtTwinDriftCircle& dc);
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
                     const xAOD::MdtTwinDriftCircle& dc);

           SG::ReadHandleKey<xAOD::MdtTwinDriftCircleContainer> m_key{};

           std::string m_collName{};
           /** @brief Identifier of the Mdt */
           MdtIdentifierBranch m_id{parent(), m_collName};
           /** @brief Position of the Mdt drift circle in the global frame */
           ThreeVectorBranch m_globPos{parent(), m_collName+"_globalPos"};
           /** @brief Dirft radius of the associated drit circle */
           VectorBranch<float>& m_driftRadius{parent().newVector<float>(m_collName+"_driftRadius")};
           /** @brief Uncertainty on the drift radius measurement */
           VectorBranch<float>& m_driftRadiusUncert{parent().newVector<float>(m_collName+"_uncertDriftRadius")};
           /** @brief Local z coordinate along the drift tube */
           VectorBranch<float>& m_twinLocZ{parent().newVector<float>(m_collName+"_posAlongWire")};
           /** @brief Uncertainty on the local z measurement along the wire */
           VectorBranch<float>& m_twinUncertLocZ{parent().newVector<float>(m_collName+"_uncertAlongWire")};
           /** @brief tdc counts of the measurement */
           VectorBranch<uint16_t>& m_tdcCounts{parent().newVector<uint16_t>(m_collName+"_tdc")};
           /** @brief Adc counts of the measurement */
           VectorBranch<uint16_t>& m_adcCounts{parent().newVector<uint16_t>(m_collName+"_adc")};
           /** @brief Tdc counts of the twin measurement */
           VectorBranch<uint16_t>& m_twinTdcCounts{parent().newVector<uint16_t>(m_collName+"_twinTdc")};
           /** @brief Adc counts of the twin measurement */
           VectorBranch<uint16_t>& m_twinAdcCounts{parent().newVector<uint16_t>(m_collName+"_twinAdc")};
           /** @brief Tube number of the twin */
           VectorBranch<uint16_t>& m_twinTube{parent().newVector<uint16_t>(m_collName+"_twinTube")};
           /** @brief Tube layer of the twin */
           VectorBranch<uint8_t>& m_twinLayer{parent().newVector<uint8_t>(m_collName+"_twinLayer")};

            /// Set of chambers to be dumped
           std::unordered_set<Identifier> m_filteredChamb{};
           /// Map of Identifiers to the position index inside the vector
           std::unordered_map<Identifier, unsigned int> m_idOutIdxMap{};
           /// Vector of PRDs parsed via the external mechanism. These measurements are parsed first
           std::vector<const xAOD::MdtTwinDriftCircle*> m_dumpedPRDS{};
           /// Apply a filter to dump the prds
           bool m_applyFilter{false};
    };
}
#endif