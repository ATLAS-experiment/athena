/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonPRDTEST_MMPRDVARIABLES_H
#define MuonPRDTEST_MMPRDVARIABLES_H

#include "MuonPRDTest/PrdTesterModule.h"
#include "MuonTesterTree/TwoVectorBranch.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonRDO/MM_RawDataContainer.h"


namespace MuonPRDTest{
    class MMPRDVariables : public PrdTesterModule {
    public:
        /** @brief Standard constructor taking the MuonTesterTree as parent
         *         The container name under which the MicroMegaPrds can be retrieved
         *         The message level
         */
        MMPRDVariables(MuonTesterTree& tree, 
                       const std::string& prd_container_name, 
                       MSG::Level msglvl);
    
        ~MMPRDVariables() = default;
    
        bool fill(const EventContext& ctx) override final;
    
        bool declare_keys() override final;

        /** @brief Adds a prd to the output tree. Returns the index in the vector
         *         where the PRD is saved for output. Internal checks ensure that 
         *         the same Prd is nver pushed twice.
         */
        unsigned int push_back(const Muon::MMPrepData& prd);

        /** @brief Adds all hits in this particular chamber to the output n-tuple */
        void dumpAllHitsInChamber(const MuonGM::MMReadoutElement& detEle);
        /** @brief Dumps only hits which are marked by the dumpAllHitsInChamber method */
        void enableSeededDump();
    private:
        unsigned int dump(const Muon::MMPrepData& prd);
        SG::ReadHandleKey<Muon::MMPrepDataContainer> m_key{};
        ScalarBranch<unsigned int>& m_NSWMM_nPRD{parent().newScalar<unsigned int>("N_PRD_MM")};
        VectorBranch<int>& m_NSWMM_PRD_time{parent().newVector<int>("PRD_MM_time")};
        VectorBranch<float>& m_NSWMM_PRD_covMatrix_1_1{parent().newVector<float>("PRD_MM_covMatrix_1_1")};
        VectorBranch<int>& m_NSWMM_PRD_nStrips{parent().newVector<int>("PRD_MM_nStrips")};
        TwoVectorBranch m_NSWMM_PRD_localPos{parent(), "PRD_MM_localPos"};
        ThreeVectorBranch m_NSWMM_PRD_globalPos{parent(), "PRD_MM_globalPos"};
        MatrixBranch<short unsigned>& m_NSWMM_PRD_stripNumbers{parent().newMatrix<short unsigned>("PRD_MM_stripNumbers")}; //numbers of strips associated to the PRD cluster
        MatrixBranch<short int>& m_NSWMM_PRD_stripTimes{parent().newMatrix<short int>("PRD_MM_stripTimes")};                       //times of strips associated to the PRD cluster
        MatrixBranch<int>& m_NSWMM_PRD_stripCharges{parent().newMatrix<int>("PRD_MM_stripCharges")};                   //times of strips associated to the PRD cluster
        MmIdentifierBranch m_NSWMM_PRD_id{parent(), "PRD_MM"};
        VectorBranch<short>& m_NSWMM_PRD_author{parent().newVector<short>("PRD_MM_author")};
        VectorBranch<uint8_t>& m_NSWMM_PRD_quality{parent().newVector<uint8_t>("PRD_MM_quality")};

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

#endif  // MuonPRDTEST_MMPRDVARIABLES_H
