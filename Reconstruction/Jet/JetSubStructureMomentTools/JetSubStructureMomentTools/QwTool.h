// This file is -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETSUBSTRUCTURE_QWTOOL_H
#define JETSUBSTRUCTURE_QWTOOL_H
////////////////////////////////////////////
///
/// \class QwTool
/// \brief Dual-use tool wrapper to set the Qw substructure variable.
/// \author P.A. Delsart
/// \date May 2015
///
/// See JetSubStructureUtils package for the implementation.
//////////////////////////////////////////////////////////
#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

class QwTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(QwTool, IJetModifier)

  public:
    // Ctor.
    QwTool(const std::string& t);

    virtual StatusCode initialize() override;

    StatusCode modify(xAOD::JetContainer& jets) const override;

  private:
    Gaudi::Property<std::string> m_jetContainerName{
      this, "JetContainer", "", "SG key for the input jet container"};

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_Qw_Key{this, "Qw_Key", "Qw"};
};

#endif
