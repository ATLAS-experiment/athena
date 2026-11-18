/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFMATCHINGTRUTHTOOL_H
#define PFMATCHINGTRUTHTOOL_H

#include "PFMatchingTool.h"

struct PFData;


class PFMatchingTruthTool : public PFMatchingTool
{

public:
  using PFMatchingTool::PFMatchingTool;
  ~PFMatchingTruthTool();

  virtual StatusCode initialize() override;

private:

  unsigned int matchAndCreateEflowCaloObj(const EventContext& ctx, PFData &data) const override;

  /** Read handle key to decorate CaloCluster with threeN leading truth particle uniqueID and energy */
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer> m_caloClusterReadDecorHandleKeyNLeadingTruthParticles{this,"CaloClusterReadDecorHandleKey_NLeadingTruthParticles",""};

};

#endif
