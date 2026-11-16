/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//#####################################################
//#                                                   #
//# File   : TBBeamQualityDoubleHitsTool.h            #
//# Package: TBRec                                    #
//#                                                   #
//# Author : Marco Bieri <mbieri@sfu.ca>              #
//#                                                   #
//# Last Modified: Feb 7/2005                         #
//#####################################################

#ifndef TBREC_TBBEAMQUALITYDOUBLEHITSTOOL_H
#define TBREC_TBBEAMQUALITYDOUBLEHITSTOOL_H

#include "TBBeamQualityTool.h"

#include <string>
#include <vector>

class TBBeamQualityDoubleHitsTool : public TBBeamQualityTool
{
 public:
  TBBeamQualityDoubleHitsTool(const std::string& type,
			      const std::string& name,
			      const IInterface* parent);
  
  virtual ~TBBeamQualityDoubleHitsTool();
  
  virtual StatusCode initializeTool() override;
  
  virtual StatusCode accept(const std::vector<std::string>&) override;
  
 protected:
  
  std::string m_SGScintkey;
  
  //Properties
  Gaudi::Property<std::vector<std::string>> m_scint_names{this, "Scintnames", {}, "scintillator names"};

  Gaudi::Property<bool> m_h6cuts{this, "h6cuts", true, "use H6 cut values"};
  
  Gaudi::Property<int> m_ScintCut1{this, "ScintCut1", 1, "index of first scintillator in Scintnames"};
  Gaudi::Property<int> m_ScintCut2{this, "ScintCut2", 2, "index of second scintillator in Scintnames"};
  
  Gaudi::Property<float> m_ScintCut1ADC{this, "ScintCut1ADC", 0, "ADC cut on first scintillator"};
  Gaudi::Property<float> m_ScintCut2ADC{this, "ScintCut2ADC", 0, "ADC cut on second scintillator"};
  
  float m_Scint1ADC{0};
  float m_Scint2ADC{0};

};

#endif
