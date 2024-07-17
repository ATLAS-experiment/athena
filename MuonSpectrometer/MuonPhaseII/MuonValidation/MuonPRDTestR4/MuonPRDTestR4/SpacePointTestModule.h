/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_MUONSPACEPOINTTESTMODULE_H
#define PRDTESTERR4_MUONSPACEPOINTTESTMODULE_H
#include "MuonPRDTestR4/TesterModuleBase.h"

#include "MuonSpacePoint/MuonSpacePointContainer.h"
#include "MuonTesterTree/IdentifierBranch.h"
namespace MuonValR4{
    class SpacePointTesterModule: public TesterModuleBase {
        public:
            SpacePointTesterModule(MuonTesterTree& tree,
                                   const std::string& inContainer,
                                   MSG::Level msgLvl = MSG::Level::INFO,
                                   const std::string& collName="");

            bool declare_keys() override final;

            bool fill(const EventContext& ctx) override final;

            
            unsigned int push_back(const MuonR4::MuonSpacePointBucket& bucket);
            unsigned int push_back(const MuonR4::MuonSpacePoint& spacePoint);


        private:
           std::string m_collName{};
           SG::ReadHandleKey<MuonR4::MuonSpacePointContainer> m_key{};
           /** @brief Space point bucket information */
           VectorBranch<uint16_t>& m_bucketNumber{parent().newVector<uint16_t>(m_collName+"bucketIndex")};
           /** @brief stationIndex / stationEta / stationPhi of the bucket chamber */
           MuonIdentifierBranch m_bucketId{parent(), m_collName+"bucket"};
           /** @brief Range of the space point bucket */
           VectorBranch<float>& m_bucketMin{parent().newVector<float>(m_collName+"bucketXMin")};
           VectorBranch<float>& m_bucketMax{parent().newVector<float>(m_collName+"bucketXMax")};
           /** @brief associated space points */
           MatrixBranch<uint16_t>& m_bucketPoints{parent().newMatrix<uint16_t>(m_collName+"bucketSpacePoints")};

         
           /** @brief Space point position */
           ThreeVectorBranch m_globPos{parent(), m_collName+"spacePointPosition"};
           /** @brief Space point drift radius */
           VectorBranch<float>& m_driftR{parent().newVector<float>(m_collName+"spacePointDriftR")};
           /** @brief Covariance of the space point */
           VectorBranch<float>& m_covXX{parent().newVector<float>(m_collName+"spacePointCovXX")};
           VectorBranch<float>& m_covXY{parent().newVector<float>(m_collName+"spacePointCovYX")};
           VectorBranch<float>& m_covYX{parent().newVector<float>(m_collName+"spacePointCovXY")};
           VectorBranch<float>& m_covYY{parent().newVector<float>(m_collName+"spacePointCovYY")};

           /** @brief  Does the space point measure phi or eta*/
           VectorBranch<bool>& m_measEta{parent().newVector<bool>(m_collName+"spacePointMeasEta")};
           VectorBranch<bool>& m_measPhi{parent().newVector<bool>(m_collName+"spacePointMeasPhi")};
           /** @brief How many other spacepoints were built with the same eta /phi prd */
           VectorBranch<unsigned int>& m_nEtaInstances{parent().newVector<unsigned int>(m_collName+"spacePointNumEtaInUse")};
           VectorBranch<unsigned int>& m_nPhiInstances{parent().newVector<unsigned int>(m_collName+"spacePointNumPhiInUse")};


           /** @brief Station Identifier */
           MuonIdentifierBranch m_spacePointId{parent(), "spacePoint"};
           /** @brief Technology index of the space point */
           VectorBranch<unsigned char>& m_techIdx{parent().newVector<unsigned char>(m_collName+"spacePointTechnology")};
           /** @brief Measurement layer */
           VectorBranch<unsigned char>& m_layer{parent().newVector<unsigned char>(m_collName+"spacePointLayer")};
           /** @brief Measurement channel */
           VectorBranch<uint16_t>& m_channel{parent().newVector<uint16_t>(m_collName+"spacePointChannel")};

           /** @brief: Keep track when a spacepoint is filled into the tree */
           std::unordered_map<const MuonR4::MuonSpacePoint*, unsigned int> m_spacePointIdx{};
           /** @brief: Keep tarck when a space point bucket is filled into the tree */
           std::unordered_map<const MuonR4::MuonSpacePointBucket*, unsigned int> m_bucketIdx{};
    };
}
#endif