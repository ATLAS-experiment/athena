/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_MMRDOVARIABLES_H
#define MuonPRDTEST_MMRDOVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonRDO/MM_RawDataContainer.h"

namespace MuonPRDTest{
    class MMRDOVariables : public PrdTesterModule {
    public:
        MMRDOVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl);
    
        ~MMRDOVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;
        
        /** @brief Adds a RDO to the output tree. Returns the index in the vector
         *         where the PRD is saved for output. Internal checks ensure that 
         *         the same Prd is nver pushed twice.
         */
        unsigned int push_back(const EventContext& ctx, const Muon::MM_RawData& rdo);
        unsigned int push_back(const EventContext& ctx, const Identifier& id);

        /** @brief Adds all hits in this particular chamber to the output n-tuple */
        void dumpAllHitsInChamber(const MuonGM::MMReadoutElement& detEle);
        /** @brief Dumps only hits which are marked by the dumpAllHitsInChamber method */
        void enableSeededDump();
    
    private:
        unsigned int dump(const EventContext& ctx, const Muon::MM_RawData& rdo);
        SG::ReadHandleKey<Muon::MM_RawDataContainer> m_rdokey{};
        ScalarBranch<unsigned int>& m_NSWMM_nRDO{parent().newScalar<unsigned int>("N_RDO_MM")};
        VectorBranch<int>& m_NSWMM_rdo_time{parent().newVector<int>("RDO_MM_time")};
        VectorBranch<int>& m_NSWMM_rdo_charge{parent().newVector<int>("RDO_MM_charge")};
        VectorBranch<uint16_t>& m_NSWMM_rdo_relBcid{parent().newVector<uint16_t>("RDO_MM_relBcid")};
        VectorBranch<double>& m_NSWMM_rdo_localPosX{parent().newVector<double>("RDO_MM_localPosX")};
        VectorBranch<double>& m_NSWMM_rdo_localPosY{parent().newVector<double>("RDO_MM_localPosY")};
        ThreeVectorBranch m_NSWMM_rdo_globalPos{parent(), "RDO_MM_globalPos"};
        MmIdentifierBranch m_NSWMM_rdo_id{parent(), "RDO_MM"};
        
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

#endif  // MuonPRDTEST_MMRDOVARIABLES_H