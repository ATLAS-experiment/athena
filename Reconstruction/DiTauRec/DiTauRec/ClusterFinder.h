/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_CLUSTERFINDER_H
#define DITAUREC_CLUSTERFINDER_H

#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"


class ClusterFinder : public DiTauToolBase {
 public:

  ClusterFinder(const std::string& type,
	     const std::string& name,
	     const IInterface * parent);

  virtual ~ClusterFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:
  float m_Rsubjet;

};

#endif  // DITAUREC_CLUSTERFINDER_H

