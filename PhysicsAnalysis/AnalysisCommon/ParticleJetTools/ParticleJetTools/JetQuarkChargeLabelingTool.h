/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETQUARKCHARGELABELINGTOOL_H
#define JETQUARKCHARGELABELINGTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "JetInterface/IJetDecorator.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "ParticleJetTools/ParticleJetLabelCommon.h"

class JetQuarkChargeLabelingTool : public asg::AsgTool, public IJetDecorator {
ASG_TOOL_CLASS(JetQuarkChargeLabelingTool, IJetDecorator)
public:

  /// Constructor
  JetQuarkChargeLabelingTool(const std::string& name);

  StatusCode initialize() override;
  StatusCode decorate(const xAOD::JetContainer& jets) const override;

protected:

  Gaudi::Property<std::string> m_hadronAccessor{this, "HadronDecorationName", "HadronGhostInitialTruthLabelPdgId", "Name of attribute for hadron labeling."};
  Gaudi::Property<std::string> m_partonAccessor{this, "PartonDecorationName", "PartonExtendedTruthLabelID", "Name of attribute for parton labeling."};
  Gaudi::Property<std::string> m_chargeDecorator{this, "OutputName", "QuarkChargeTruthLabelID", "Name of the output variable containing the charge ID."};
  Gaudi::Property<std::map<int,int>> m_mapOption{this, "HadronChargeMap", std::map <int,int>(), "Map relating the PdgID of the hadrons to the charge of the original heavy quark."};

  std::map<int,int> m_map;
};


#endif
