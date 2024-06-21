/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PARTICLEJETGHOSTLABELTOOL_H
#define PARTICLEJETGHOSTLABELTOOL_H

#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetDecorator.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"

#include "ParticleJetTools/ParticleJetLabelCommon.h"

class ParticleJetGhostLabelTool : public asg::AsgTool, public IJetDecorator {
ASG_TOOL_CLASS(ParticleJetGhostLabelTool, IJetDecorator)
public:

  /// Constructor
  ParticleJetGhostLabelTool(const std::string& name);

  StatusCode initialize() override;
  StatusCode decorate(const xAOD::JetContainer& jets) const override;

protected:

  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truthEventsKey{
    this, "TruthEventCollection", "TruthEvents",
    "Truth event collection"
  };

  std::vector<const xAOD::TruthParticle*> match(
    const xAOD::Jet&, const std::string& ghostname
    ) const;

  /// Name of jet label attributes
  ParticleJetTools::LabelNames m_labelnames;
  std::string m_ghosttauname;
  std::string m_ghostcname;
  std::string m_ghostbname;

  /// Minimum pT for particle selection (in MeV)
  double m_partptmin;
};


#endif
