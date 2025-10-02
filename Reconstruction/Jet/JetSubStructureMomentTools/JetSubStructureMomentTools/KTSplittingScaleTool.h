/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_ktsplittingscaletool_header
#define jetsubstructuremomenttools_ktsplittingscaletool_header

#include "JetSubStructureMomentTools/JetSubStructureMomentToolsBase.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

class KTSplittingScaleTool :
  public JetSubStructureMomentToolsBase {
    ASG_TOOL_CLASS(KTSplittingScaleTool, IJetModifier)
    
    public:
      // Constructor and destructor
      KTSplittingScaleTool(const std::string& name);

      virtual StatusCode initialize() override;

      StatusCode modify(xAOD::JetContainer& jets) const override;

    private:
      Gaudi::Property<std::string> m_jetContainerName{
	this, "JetContainer", "", "SG key for the input jet container"};

      SG::WriteDecorHandleKey<xAOD::JetContainer> m_Split12_Key{
	this, "Split12_Key", "Split12"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_Split23_Key{
	this, "Split23_Key", "Split23"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_Split34_Key{
	this, "Split34_Key", "Split34"};

      SG::WriteDecorHandleKey<xAOD::JetContainer> m_ZCut12_Key{
	this, "ZCut12_Key", "ZCut12"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_ZCut23_Key{
	this, "ZCut23_Key", "ZCut23"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_ZCut34_Key{
	this, "ZCut34_Key", "ZCut34"};

};

#endif
