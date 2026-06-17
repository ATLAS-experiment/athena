/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFUNIFIEDMATCHINGTRUTHTOOL_H
#define PFUNIFIEDMATCHINGTRUTHTOOL_H

#include "PFUnifiedMatchingTool.h"

struct PFData;


class PFUnifiedMatchingTruthTool : public PFUnifiedMatchingTool
{

public:
  using PFUnifiedMatchingTool::PFUnifiedMatchingTool;
  ~PFUnifiedMatchingTruthTool();

  virtual StatusCode initialize() override;

private:

  unsigned int matchAndCreateEflowCaloObj(const EventContext& ctx, PFData &data) const override;

  /** Read handle key to decorate CaloCluster with threeN leading truth particle uniqueID and energy */
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer> m_caloClusterReadDecorHandleKeyNLeadingTruthParticles{this,"CaloClusterReadDecorHandleKey_NLeadingTruthParticles",""};

};

#endif
