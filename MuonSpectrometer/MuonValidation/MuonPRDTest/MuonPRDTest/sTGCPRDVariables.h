/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_sTGCPRDVARIABLES_H
#define MuonPRDTEST_sTGCPRDVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonTesterTree/TwoVectorBranch.h"

namespace MuonPRDTest{
    class sTGCPRDVariables : public PrdTesterModule {
    public:

        /** @brief Standard constructor taking the MuonTesterTree as parent
         *         The container name under which the MicroMegaPrds can be retrieved
         *         The message level
         */

        sTGCPRDVariables(MuonTesterTree& tree,
                         const std::string& container_name,
                         MSG::Level msglvl);
    
        ~sTGCPRDVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;

        /** @brief Adds a prd to the output tree. Returns the index in the vector
         *         where the PRD is saved for output. Internal checks ensure that 
         *         the same Prd is nver pushed twice.
         */
        unsigned int push_back(const Muon::sTgcPrepData& prd);

        /** @brief Adds all hits in this particular chamber to the output n-tuple */
        void dumpAllHitsInChamber(const MuonGM::sTgcReadoutElement& detEle);
        /** @brief Dumps only hits which are marked by the dumpAllHitsInChamber method */
        void enableSeededDump();

    
    private:
        unsigned int dump(const Muon::sTgcPrepData& prd);
        SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_NSWsTGC_nPRD{parent().newScalar<unsigned int>("N_PRD_sTGC")};
        sTgcIdentifierBranch m_NSWsTGC_PRD_id{parent(), "PRD_sTGC"};
        TwoVectorBranch m_NSWsTGC_PRD_localPos {parent(), "PRD_sTGC_localPos"};
        ThreeVectorBranch m_NSWsTGC_PRD_globalPos{parent(), "PRD_sTGC_globalPos"};
        VectorBranch<float>& m_NSWsTGC_PRD_covMatrix_1_1{parent().newVector<float>("PRD_sTGC_covMatrix_1_1")};
        VectorBranch<float>& m_NSWsTGC_PRD_covMatrix_2_2{parent().newVector<float>("PRD_sTGC_covMatrix_2_2")};

        VectorBranch<int>& m_NSWsTGC_PRD_charge{parent().newVector<int>("PRD_sTGC_charge")};
        VectorBranch<short>& m_NSWsTGC_PRD_time{parent().newVector<short>("PRD_sTGC_time")};
        VectorBranch<int>& m_NSWsTGC_PRD_nStrips{parent().newVector<int>("PRD_sTGC_nStrips")};
        
        MatrixBranch<int>& m_NSWsTGC_PRD_stripCharge{parent().newMatrix<int>("PRD_sTGC_stripCharge")};
        MatrixBranch<short>& m_NSWsTGC_PRD_stripTime{parent().newMatrix<short>("PRD_sTGC_stripTime")};
        MatrixBranch<uint16_t>& m_NSWsTGC_PRD_stripChannel{parent().newMatrix<uint16_t>("PRD_sTGC_stripChannel")};

        VectorBranch<uint8_t>& m_stgcAuthor{parent().newVector<uint8_t>("stgcAuthor")};
        VectorBranch<uint8_t>& m_stgcQuality{parent().newVector<uint8_t>("stgcQuality")};
        
        
        /// Set of chambers to be dumped
        std::unordered_set<Identifier> m_filteredChamb{};
        /// Set of particular chambers to be dumped
        std::unordered_map<Identifier, unsigned int> m_filteredPRDs{};
        /// Apply a filter to dump the prds
        bool m_applyFilter{false};
        /// Flag telling whether an external prd has been pushed
        bool m_externalPush{false};
    };
};

#endif  // MuonPRDTEST_sTGCPRDVARIABLES_H