/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCLUSTERFORMATION_RPCCLUSTERINGALG_H
#define  MUONCLUSTERFORMATION_RPCCLUSTERINGALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <xAODMuonPrepData/RpcStripContainer.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteDecorHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include <AthLinks/ElementLink.h>
namespace MuonR4{
    class RpcClusteringAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            
            StatusCode initialize() override final;
            StatusCode execute(const EventContext& ctx) const override final;
        private:
            SG::ReadHandleKey<xAOD::RpcStripContainer> m_readKey{this, "ReadKey", "xRpcStrips"};

            SG::WriteHandleKey<xAOD::RpcStripContainer> m_writeKey{this, "WriteKey", "xRpcClusters"};
            using LinkType = std::vector<ElementLink<xAOD::RpcStripContainer>>;
            SG::WriteDecorHandleKey<xAOD::RpcStripContainer> m_linkKey{this, "LinkKey", m_writeKey, "rpcConstituents"};

            Gaudi::Property<unsigned> m_maxHoles{this, "maxHoles", 1, "maximum holes between two strips"};
            Gaudi::Property<unsigned> m_maxSize{this, "maxSize", 5, "Maximum size of a cluster to grow"};    
    };
}

#endif
