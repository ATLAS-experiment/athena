/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file DerivationFrameworkTop/JetMSVAugmentation.h
 * @author Georges Aad
 * @date Nov. 2016
 * @brief tool to add some MSV variables to jets
*/


#ifndef DerivationFramework_JetMSVAugmentation_H
#define DerivationFramework_JetMSVAugmentation_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace DerivationFramework {


  class JetMSVAugmentation : public extends<AthAlgTool, IAugmentationTool> {


  public:
    JetMSVAugmentation(const std::string& t, const std::string& n, const IInterface* p);
    ~JetMSVAugmentation();
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches() const;

  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetCollectionName{this, "JetCollectionName", "AntiKt4EMTopoJets"};
    // NB WriteDecordHandleKey names and vertexAlgName need to be configured consistently
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxmass{this, "vtxmassDecorKey", m_jetCollectionName, "MSV_vtxmass"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxpt{this, "vtxptDecorKey", m_jetCollectionName, "MSV_vtxpt"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxeta{this, "vtxetaDecorKey", m_jetCollectionName, "MSV_vtxeta"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxphi{this, "vtxphiDecorKey", m_jetCollectionName, "MSV_vtxphi"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxefrac{this, "vtxefracDecorKey", m_jetCollectionName, "MSV_vtxefrac"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxx{this, "vtxxDecorKey", m_jetCollectionName, "MSV_vtxx"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxy{this, "vtxyDecorKey", m_jetCollectionName, "MSV_vtxy"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxz{this, "vtxzDecorKey", m_jetCollectionName, "MSV_vtxz"};

    SG::WriteDecorHandleKey<xAOD::JetContainer > m_dec_vtxntrk{this, "vtxntrkDecorKey", m_jetCollectionName, "MSV_vtxntrk"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_vtxdls{this, "vtxdlsDecorKey", m_jetCollectionName, "MSV_vtxdls"};

    Gaudi::Property<std::string> m_vtxAlgName{this, "vertexAlgName", "MSV"};
  }; /// class

} /// namespace


#endif
