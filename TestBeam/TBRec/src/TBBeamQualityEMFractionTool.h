/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//#####################################################
//#                                                   #
//# File   : TBBeamQualityEMFractionTool.h            #
//# Package: TBRec                                    #
//#                                                   #
//# Author : Marco Bieri <mbieri@sfu.ca>              #
//#                                                   #
//# Created: July 19/2005                             #
//#####################################################

#ifndef TBBEAMQUALITYEMFRACTIONTOOL_H
#define TBBEAMQUALITYEMFRACTIONTOOL_H

#include "TBBeamQualityTool.h"

//#include "LArDetDescr/LArDetDescrManager.h"

#include "CaloIdentifier/CaloID.h"

class TBBeamQualityEMFractionTool : public TBBeamQualityTool
{
 public:
  TBBeamQualityEMFractionTool(const std::string& type,
			      const std::string& name,
				const IInterface* parent);
  
  virtual ~TBBeamQualityEMFractionTool();
  
  virtual StatusCode initializeTool() override;
  
  virtual StatusCode accept(const std::vector<std::string>&) override;
  
 protected:
  
  //LArDetDescrManager* m_larMgr;

  // Identifier Pointers
  const LArEM_ID* m_emecID_help{nullptr};
  const LArHEC_ID* m_hecID_help{nullptr};
  const LArFCAL_ID* m_fcalID_help{nullptr};
  
  // Properties
  
  Gaudi::Property<float> m_em_fraccut{this, "EM_fraccut", 0.2, "cut on EM fraction"}; // cut ration
  Gaudi::Property<std::vector<double>> m_em_fracnumsamp{this, "EM_fracnumsamp", {}, "samplings in numerator"}; // sample: numerator
  Gaudi::Property<std::vector<double>> m_em_fracdensamp{this, "EM_fracdensamp", {}, "samplings in denominator"}; // sample: denominator
  Gaudi::Property<std::string> m_em_fracnumdet{this, "EM_fracnumdet", "none", "detector in numerator"};          // detector: numerator
  Gaudi::Property<std::string> m_em_fracdendet{this, "EM_fracdendet", "none", "detector in denominator"};          // detector: denominator
  std::vector<float> m_Mlayer;
  
  float m_em_frac{0};
  
};
#endif
