/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_MUONSPACEPOINTTESTMODULE_H
#define PRDTESTERR4_MUONSPACEPOINTTESTMODULE_H
#include "MuonPRDTestR4/TesterModuleBase.h"

#include "MuonSpacePoint/SpacePointContainer.h"
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

            
            unsigned int push_back(const MuonR4::SpacePointBucket& bucket);
            unsigned int push_back(const MuonR4::SpacePoint& spacePoint);


        private:
           std::string m_collName{};
           SG::ReadHandleKey<MuonR4::SpacePointContainer> m_key{};
           /** @brief Space point bucket information */
           VectorBranch<uint16_t>& m_bucketNumber{parent().newVector<uint16_t>(m_collName+"bucket_index")};
           /** @brief stationIndex / stationEta / stationPhi of the bucket chamber */
           MuonIdentifierBranch m_bucketId{parent(), m_collName+"bucket"};
           /** @brief Range of the space point bucket */
           VectorBranch<float>& m_bucketMin{parent().newVector<float>(m_collName+"bucket_xMin")};
           VectorBranch<float>& m_bucketMax{parent().newVector<float>(m_collName+"bucket_xMax")};
           /** @brief associated space points */
           MatrixBranch<uint16_t>& m_bucketPoints{parent().newMatrix<uint16_t>(m_collName+"bucket_spacePoints")};

         
           /** @brief Space point position */
           ThreeVectorBranch m_spPos{parent(), m_collName+"spacePoint_Position"};
           /** @brief Space point drift radius */
           VectorBranch<float>& m_driftR{parent().newVector<float>(m_collName+"spacePoint_driftR")};
           /** @brief Covariance of the space point */
           VectorBranch<float>& m_covX{parent().newVector<float>(m_collName+"spacePoint_covX")};
           VectorBranch<float>& m_covY{parent().newVector<float>(m_collName+"spacePoint_covY")};
           VectorBranch<float>& m_covT{parent().newVector<float>(m_collName+"spacePoint_covT")};
           /** @brief  Does the space point measure phi or eta*/
           VectorBranch<bool>& m_measEta{parent().newVector<bool>(m_collName+"spacePoint_measEta")};
           VectorBranch<bool>& m_measPhi{parent().newVector<bool>(m_collName+"spacePoint_measPhi")};
           /** @brief How many other spacepoints were built with the same eta /phi prd */
           VectorBranch<unsigned int>& m_nEtaInstances{parent().newVector<unsigned int>(m_collName+"spacePoint_nEtaInUse")};
           VectorBranch<unsigned int>& m_nPhiInstances{parent().newVector<unsigned int>(m_collName+"spacePoint_nPhiInUse")};


           /** @brief Station Identifier */
           MuonIdentifierBranch m_spacePointId{parent(), "spacePoint"};
           /** @brief Technology index of the space point */
           VectorBranch<unsigned char>& m_techIdx{parent().newVector<unsigned char>(m_collName+"spacePoint_technology")};
           /** @brief Measurement layer */
           VectorBranch<unsigned char>& m_layer{parent().newVector<unsigned char>(m_collName+"spacePoint_layer")};
           /** @brief Measurement channel */
           VectorBranch<uint16_t>& m_channel{parent().newVector<uint16_t>(m_collName+"spacePoint_channel")};
           /** @brief Channel of the secondary measurment */
           VectorBranch<int16_t>& m_phiChannel{parent().newVector<int16_t>(m_collName+"spacePoint_secChannel")};

           /** @brief: Keep track when a spacepoint is filled into the tree */
           std::unordered_map<const MuonR4::SpacePoint*, unsigned int> m_spacePointIdx{};
           /** @brief: Keep tarck when a space point bucket is filled into the tree */
           std::unordered_map<const MuonR4::SpacePointBucket*, unsigned int> m_bucketIdx{};
           /** @brief: Flag whether the module is operated in filter mode */
           bool m_applyFilter{false};
           /** @brief: Flag toggling whether the module is in internal filling mode */
           bool m_internalFill{false};
    };
}
#endif
