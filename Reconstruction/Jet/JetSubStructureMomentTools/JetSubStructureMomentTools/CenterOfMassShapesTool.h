/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_centerofmassshapestool_header
#define jetsubstructuremomenttools_centerofmassshapestool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"

class CenterOfMassShapesTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(CenterOfMassShapesTool, IJetModifier)
    
    public:
      // Constructor and destructor
      CenterOfMassShapesTool(const std::string& name);

      virtual StatusCode initialize() override;

      StatusCode modify(xAOD::JetContainer& jets) const override;

    private:
      Gaudi::Property<std::string> m_jetContainerName{
	this, "JetContainer", "", "SG key for the input jet container"};

      SG::WriteDecorHandleKey<xAOD::JetContainer> m_ThrustMin_Key{
	this, "ThrustMin_Key", "ThrustMin"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_ThrustMaj_Key{
	this, "ThrustMaj_Key", "ThrustMaj"};

      SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_FoxWolfram_Keys{
	this, "FoxWolfram_Key", {}};

      SG::WriteDecorHandleKey<xAOD::JetContainer> m_Sphericity_Key{
	this, "Sphericity_Key", "Sphericity"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_Aplanarity_Key{
	this, "Aplanarity_Key", "Aplanarity"};

};

#endif
