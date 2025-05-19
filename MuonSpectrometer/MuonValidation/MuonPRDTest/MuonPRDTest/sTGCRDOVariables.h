/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_sTGCRDOVARIABLES_H
#define MuonPRDTEST_sTGCRDOVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonRDO/STGC_RawDataContainer.h"

namespace MuonPRDTest{
    class sTGCRDOVariables : public PrdTesterModule {
    public:
        sTGCRDOVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~sTGCRDOVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
        
        /** @brief Adds a RDO to the output tree. Returns the index in the vector
         *         where the PRD is saved for output. Internal checks ensure that 
         *         the same Prd is nver pushed twice.
         */
        unsigned int push_back(const EventContext& ctx, const Muon::STGC_RawData& rdo);
        unsigned int push_back(const EventContext& ctx, const Identifier& id);

        /** @brief Adds all hits in this particular chamber to the output n-tuple */
        void dumpAllHitsInChamber(const MuonGM::sTgcReadoutElement& detEle);
        /** @brief Dumps only hits which are marked by the dumpAllHitsInChamber method */
        void enableSeededDump();
    
    private:
        unsigned int dump(const EventContext& ctx, const Muon::STGC_RawData& rdo);
        SG::ReadHandleKey<Muon::STGC_RawDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_NSWsTGC_nRDO{parent().newScalar<unsigned int>("N_RDO_sTGC")};
        VectorBranch<double>& m_NSWsTGC_rdo_time{parent().newVector<double>("RDO_sTGC_time")};
        VectorBranch<uint16_t>& m_NSWsTGC_rdo_charge{parent().newVector<uint16_t>("RDO_sTGC_charge")};
        VectorBranch<uint16_t>& m_NSWsTGC_rdo_tdo{parent().newVector<uint16_t>("RDO_sTGC_tdo")};
        VectorBranch<uint16_t>& m_NSWsTGC_rdo_bcTag{parent().newVector<uint16_t>("RDO_sTGC_bcTag")};
        VectorBranch<bool>& m_NSWsTGC_rdo_isDead{parent().newVector<bool>("RDO_sTGC_isDead")};
        VectorBranch<double>& m_NSWsTGC_rdo_localPosX{parent().newVector<double>("RDO_sTGC_localPosX")};
        VectorBranch<double>& m_NSWsTGC_rdo_localPosY{parent().newVector<double>("RDO_sTGC_localPosY")};
        ThreeVectorBranch m_NSWsTGC_rdo_globalPos{parent(), "RDO_sTGC_globalPos"};
        sTgcIdentifierBranch m_NSWsTGC_rdo_id{parent(), "RDO_sTGC"};
        
        /// Set of chambers to be dumped
        std::unordered_set<IdentifierHash> m_filteredChamb{};
        /// Set of particular chambers to be dumped
        std::unordered_map<Identifier, unsigned int> m_filteredRDOs{};
        /// Apply a filter to dump the prds
        bool m_applyFilter{false};
        /// Flag telling whether an external prd has been pushed
        bool m_externalPush{false};
    };
};

#endif  // MuonPRDTEST_sTGCRDOVARIABLES_H