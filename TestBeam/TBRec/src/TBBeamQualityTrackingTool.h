/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//#####################################################
//#                                                   #
//# File   : TBBeamQualityTrackingTool.h              #
//# Package: TBRec                                    #
//#                                                   #
//# Author : P.Krieger <krieger@physics.utoronto.ca>  #
//#                                                   #
//# Last Modified: Jun 30/2005                        #
//##################################################### 

#ifndef TBREC_TBBEAMQUALITYTRACKINGTOOL_H
#define TBREC_TBBEAMQUALITYTRACKINGTOOL_H

#include "TBBeamQualityTool.h"

#include <string>
#include <vector>

class TBBeamQualityTrackingTool : public TBBeamQualityTool
{
 public:
  TBBeamQualityTrackingTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
  
  virtual ~TBBeamQualityTrackingTool();
  
  virtual StatusCode initializeTool() override;
  
  virtual StatusCode accept(const std::vector<std::string>&) override;
  
 protected:
  
  // Keys
  std::string m_SGTrigkey;
  
  // TRIGGERING
  std::vector<unsigned int> m_triggflag;
  
  // defaults are 0, which means no cut; if cutting, must specify both.
  Gaudi::Property<float> m_chi2cut_u{this, "Chi2Cut_x", 0., "chi2 cut on x (u) track fit; 0 means no cut"};
  Gaudi::Property<float> m_chi2cut_v{this, "Chi2Cut_y", 0., "chi2 cut on y (v) track fit; 0 means no cut"};
};
#endif

