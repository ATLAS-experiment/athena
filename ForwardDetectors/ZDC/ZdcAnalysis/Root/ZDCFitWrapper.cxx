/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <numeric>
#include <memory>
#include "ZdcAnalysis/ZDCFitWrapper.h"

const std::map<std::string, std::vector<ZDCFitWrapper::FitParameter>> ZDCFitExpFermiVariableTausInduct::s_ConfigParamMap = 
  {
    {"default",
     {{{"A", 1000, false, 2, 5000, 50, false, 0}},
      {{"T0", 40, false,  0, 100, 2, false, 0}},
      {{"tau1", 0, false,  0.5, 3, 0.05, false, 0}},  // default value set via JSON
      {{"tau2", 0, false,  3.5, 8, 0.25, false, 0}},  // default value set via JSON
      {{"C", 0, false,  -50, 50, 1, true, 0}}, 
      {{"period", 13.5, true,  10, 30, 0, false, 0}}, 
      {{"IndA", 0.2, false,  -2, 2, 0.1, false, 0}}, 
      {{"IndB", 0.2, false,  -2, 2, 0.1, false, 0}}, 
      {{"bI", 0.18, false,  -1, 1, 0.025, false, 0}}, 
      {{"delta", 0.015, true,  0, 0, 0, false, 0}}}
    },
    {"PbPb2026",
     {{{"period",  16,     true,  0, 0, 0, false, 0}}, 
      {{"IndA",     0.25,  false,  0, 2, 0.1, false, 0}}, 
      {{"IndB",     0.8,   false,  0, 2, 0.1, false, 0}}, 
      {{"bI",       0.18,  false,  -1, 1, 0.025, false, 0}}, 
      {{"delta",    0.015, true,  0, 0, 0, false, 0}},
      {{"C",        0, false,  -50, 50, 1, true, 0}}, 
     }
    },
    {"PbPb2025",
     {{{"tau2", 0, false,  5, 8, 0.25, false, 0}},  // adjust the range
      {{"period",  13.5,  true,  0, 0, 0, false, 0}}, 
      {{"IndA",     0.1,  false,  -0.5, 2, 0.1, true, 0}}, 
      {{"IndB",     0.2,   false,  -0.5, 2, 0.1, false, 0}}, 
      {{"bI",     -0.02,   true,  -0.5, 0.5, 0.025, true, -0.02}}, 
     }
    },
    {"PbPb2025FreeInduct",
     {{{"period",  13,     false,  10, 30, 1, true, 13}}, 
      {{"IndA",     0.25,  false,  -1, 2, 0.1, true, 0}}, 
      {{"IndB",     0.8,   false,  -1, 2, 0.1, false, 0}}, 
      {{"bI",     -0.025,  false,  -0.5, 0.5, 0, false, 0}},
     }
    },
    {"PbPb2024",
     {{{"period",  16,     true,  0, 0, 0, false, 0}}, 
      {{"IndA",     0.25,  false,  0, 2, 0.1, false, 0}}, 
      {{"IndB",     0.8,   false,  0, 2, 0.1, false, 0}}, 
      {{"bI",       0.018, true,  0, -0.5, 0.5, false, 0}}, 
      {{"delta",    0.015, true,  0, 0, 0, false, 0}}, 
     }
    },
  };

const std::map<std::string, std::vector<ZDCFitWrapper::FitParameter>> ZDCFitExpFermiInductPreExp::s_ConfigParamMap = 
  {
    {"default",
     {{{"A", 1000, false, 2, 5000, 50, false, 0}},
      {{"T0", 40, false,  0, 100, 1, false, 0}},
      {{"Aexp", 0, false,  0.5, 3, 0.05, false, 0}},  
      {{"tauexp", 0, false,  3.5, 8, 0.25, false, 0}}, 
      {{"bsqrtexp", 0, false,  -50, 50, 1, true, 0}}, 
      {{"C", 0, false,  -50, 50, 1, true, 0}},
      {{"period",  13.5,  true,  0, 0, 0, false, 0}}, 
      {{"IndA", 0.2, false,  -2, 2, 0.1, false, 0}}, 
      {{"IndB", 0.2, false,  -2, 2, 0.1, false, 0}},
      {{"bI",  -0.02, true,  -0.5, 0.5, 0.025, true, -0.02}}
     }
    },
    {"PbPb2025",
     {{{"period",  13.5,  true,  0, 0, 0, false, 0}}, 
      {{"IndA",     0.1,  false,  -0.5, 2, 0.1, true, 0}}, 
      {{"IndB",     0.2,   false,  -0.5, 2, 0.1, false, 0}}, 
      {{"bI",     -0.02,   true,  -0.5, 0.5, 0.025, true, -0.02}}, 
     }
    },
    {"PbPb2025FreeInduct",
     {{{"period",  13,     false,  10, 30, 1, true, 13}}, 
      {{"IndA",     0.25,  false,  -1, 2, 0.1, true, 0}}, 
      {{"IndB",     0.8,   false,  -1, 2, 0.1, false, 0}}, 
      {{"bI",     -0.025,  false,  -0.5, 0.5, 0, false, 0}},
     }
    }
  };

const std::map<std::string, std::vector<ZDCFitWrapper::FitParameter>> ZDCFitExpFermiInductPrePulse::s_ConfigParamMap = 
  {
    {"default",
     {{{"A", 1000, false, 2, 5000, 50, false, 0}},
      {{"T0", 40, false,  0, 100, 1, false, 0}},
      {{"Apre", 0, false,  0.5, 3, 0.05, false, 0}},  
      {{"T0pre", 0, false,  3.5, 8, 0.25, false, 0}},  
      {{"C", 0, false,  -50, 50, 1, true, 0}},
      {{"period",  13.5,  true,  0, 0, 0, false, 0}}, 
      {{"IndA", 0.2, false,  -2, 2, 0.1, false, 0}}, 
      {{"IndB", 0.2, false,  -2, 2, 0.1, false, 0}},
      {{"bI",  -0.02, true,  -0.5, 0.5, 0.025, true, -0.02}}
     }
    },
    {"PbPb2025",
     {{{"period",  13.5,  true,  0, 0, 0, false, 0}}, 
      {{"IndA",     0.1,  false,  -0.5, 2, 0.1, true, 0}}, 
      {{"IndB",     0.2,   false,  -0.5, 2, 0.1, false, 0}}, 
      {{"bI",     -0.02,   true,  -0.5, 0.5, 0.025, true, -0.02}}, 
     }
    },
    {"PbPb2025FreeInduct",
     {{{"period",  13,     false,  10, 30, 1, true, 13}}, 
      {{"IndA",     0.25,  false,  -1, 2, 0.1, true, 0}}, 
      {{"IndB",     0.8,   false,  -1, 2, 0.1, false, 0}}, 
      {{"bI",     -0.025,  false,  -0.5, 0.5, 0, false, 0}},
     }
    }
  };

const ZDCFitWrapper::FitParameter& ZDCFitWrapper::GetParameterDescr(const std::string & name)
{
  auto iter = std::find_if(m_parameterSet.begin(), m_parameterSet.end(),
			   [name](const FitParameter& parDesc){return parDesc.name() == name;});
  if (iter == m_parameterSet.end()) throw std::logic_error("ZDCFitWrapper::GetParameterDescr: invalid parameter name: " + name );
  return *iter;
}

void ZDCFitWrapper::LoadDefaultParameterSet(const std::vector<FitParameter>& parameterSet)
{
  // First step is to check consistency with number of parameters in TF1
  //
  if (int(parameterSet.size()) != m_wrapperTF1->GetNpar()) throw std::logic_error("ZDCFitWrapper::LoadDefaultParameterSet:: Incorrect number of parameters in default parameter set");

  m_parameterSet = parameterSet;
  m_haveParameterSet = true;

  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // Loop over passed descriptors
  //
  unsigned parIndex = 0;
  for (const auto & parDescr : parameterSet) {
    //
    // Load the settings for this parameter in the TF1
    //
    std::string name = parDescr.name();
    theTF1->SetParName(parIndex, name.c_str());
    
    bool fixed = parDescr.fixed();
    
    if (fixed) {
      float fixedValue = parDescr.defaultValue();
      theTF1->FixParameter(parIndex, fixedValue);
    }
    else {
      float lowerLimit = parDescr.lowerLimit();
      float upperLimit = parDescr.upperLimit();
      theTF1->SetParLimits(parIndex, lowerLimit, upperLimit);
    }

    parIndex++;
  }
}

void ZDCFitWrapper::OverrideParameterSet(const std::vector<FitParameter>& overParamSet)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // Loop over the provided parameter descriptors
  //
  for (const auto & parDescr : overParamSet) {
    std::string inputName = parDescr.name();

    // Fit the parameter name in the current list
    //
    auto iter = std::find_if(m_parameterSet.begin(), m_parameterSet.end(),
			     [inputName](const FitParameter& parDesc){return parDesc.name() == inputName;});
    if (iter == m_parameterSet.end()) throw std::logic_error("ZDCFitWrapper::OverrideParameterSet: invalid parameter name: " +inputName );

    // Update the descriptor for this parameter
    //
    (*iter) = parDescr;
    
    // Update the settings in the TF!
    //
    unsigned int parIndex = std::distance(m_parameterSet.begin(), iter);
    bool fixed = parDescr.fixed();

    if (fixed) {
      float fixedValue = parDescr.defaultValue();
      theTF1->FixParameter(parIndex, fixedValue);
    }
    else {
      float lowerLimit = parDescr.lowerLimit();
      float upperLimit = parDescr.upperLimit();
      theTF1->SetParLimits(parIndex, lowerLimit, upperLimit);
    }
  }
}

void ZDCFitWrapper::Initialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  // Clear the errors on the TF1 because retaining the errors seems to affect 
  //   the convergence of fits 
  //
  auto tf1_p = GetWrapperTF1(); 
  unsigned int npar = tf1_p->GetNpar();
  std::vector<double> zeroVec(npar, 0.0);
  GetWrapperTF1()->SetParErrors(&zeroVec[0]);

  // If we adjusted the time range on the previous event, restore to default
  //
  if (m_adjTLimitsEvent) {
    SetT0FitLimits(m_t0Min, m_t0Max);

    m_adjTLimitsEvent = false;
    m_tminAdjust = 0;
  }

  SetAmpMinMax(ampMin, ampMax);

  //
  // Do we have parameter sets?
  //
  if (m_haveParameterSet) {  
    for (unsigned int ipar = 0; ipar < m_parameterSet.size(); ipar++) {
      //
      // Yes, use parameter descriptors to initialize the TF1
      //
      const auto & fitparDescr = m_parameterSet[ipar];
      
      if (!fitparDescr.fixed()) {
	tf1_p->SetParameter(ipar, fitparDescr.defaultValue());
	tf1_p->SetParError(ipar, fitparDescr.stepSize());
      }
    }

    tf1_p->SetParameter(0, initialAmp);    
    tf1_p->SetParameter(1, initialT0);    
  }
  else {
    //
    // No, perform the wrapper-implemented initialize
    //
    DoInitialize(initialAmp, initialT0, ampMin, ampMax);

    // Clear the errors on the TF1 because retaining the errors seems to affect 
    //   the convergence of fits
    //
    unsigned int npar = GetWrapperTF1()->GetNpar();
    std::vector<double> zeroVec(npar, 0.0);
    tf1_p->SetParErrors(&zeroVec[0]);

  }
}

void ZDCFitWrapper::Initialize(float initialAmp, float initialT0, float ampMin, float ampMax, float fitTmin, float fitTmax, float fitTRef)
{

  // First do the nominal initializatioon
  //
  Initialize(initialAmp, initialT0, ampMin, ampMax);

  // Now account for the modified time range
  //
  m_adjTLimitsEvent = true;

  // Handle the specific limits on T0 
  //
  m_tminAdjust = fitTRef; // Note: this is the time corresponding to the sample used for presample subtraction 

  m_tempTmin = std::max(m_t0Min, fitTmin);
  m_tempTmax = std::min(m_t0Max, fitTmax);
  
  SetAmpMinMax(ampMin, ampMax);
  SetT0FitLimits(m_tempTmin, m_tempTmax);
}

void ZDCFitWrapper::ConstrainFit()
{
  if (m_haveParameterSet) {  
    auto tf1_p = GetWrapperTF1(); 

    for (unsigned int ipar = 2; ipar < m_parameterSet.size(); ipar++) {
      const auto & fitparDescr = m_parameterSet[ipar];
      
      if (!fitparDescr.fixed() && fitparDescr.constrainFixed()) {
	tf1_p->FixParameter(ipar, fitparDescr.constrainValue());
      }
    }
  }
  else doConstrainFit();
}

void ZDCFitWrapper::UnconstrainFit()
{
  if (m_haveParameterSet) {  
    auto tf1_p = GetWrapperTF1();
    
    for (unsigned int ipar = 2; ipar < m_parameterSet.size(); ipar++) {
      const auto & fitparDescr = m_parameterSet[ipar];
      
      if (!fitparDescr.fixed() && fitparDescr.constrainFixed()) {
	tf1_p->ReleaseParameter(ipar);
	tf1_p->SetParameter(ipar, fitparDescr.defaultValue());

	float lowerLimit = fitparDescr.lowerLimit();
	float upperLimit = fitparDescr.upperLimit();
	tf1_p->SetParLimits(ipar, lowerLimit, upperLimit);

	tf1_p->SetParError(ipar, fitparDescr.stepSize());
      }
    }
  }
  else doUnconstrainFit();
}

ZDCFitExpFermiVariableTaus::ZDCFitExpFermiVariableTaus(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiVariableTaus" + tag).c_str(), ZDCFermiExpFit, tmin, tmax, 5)),
  m_fixTau1(fixTau1), m_fixTau2(fixTau2), m_tau1(tau1), m_tau2(tau2)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "tau1");
  theTF1->SetParName(3, "tau2");
  theTF1->SetParName(4, "C");

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 3, 6);
  theTF1->SetParLimits(3, 10, 40);

  if (m_fixTau1) theTF1->FixParameter(2, m_tau1);
  if (m_fixTau2) theTF1->FixParameter(3, m_tau2);
}

void ZDCFitExpFermiVariableTaus::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParameter(0, initialAmp);
  theTF1->SetParameter(1, initialT0);
  theTF1->SetParameter(4, 0);

  theTF1->SetParLimits(0, ampMin, ampMax);

  if (!m_fixTau1) theTF1->SetParameter(2, m_tau1);
  if (!m_fixTau2) theTF1->SetParameter(3, m_tau2);
}

void ZDCFitExpFermiVariableTaus::SetT0FitLimits(float t0Min, float t0Max)
{
  // Set the parameter limits accordingly on the TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

void ZDCFitExpFermiVariableTaus::doConstrainFit()
{
  // We force the constant terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(4, 0);
}
void ZDCFitExpFermiVariableTaus::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(4);
}

ZDCFitExpFermiVariableTausRun3::ZDCFitExpFermiVariableTausRun3(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2) :
  ZDCFitExpFermiVariableTaus(tag, tmin, tmax, fixTau1, fixTau2, tau1, tau2)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParLimits(2, 0.5, 3);
  theTF1->SetParLimits(3, 4, 10);

  // Since we st parameter ranges, we have to redo the fixing (or not) again here
  //
  if (m_fixTau1) theTF1->FixParameter(2, m_tau1);
  if (m_fixTau2) theTF1->FixParameter(3, m_tau2);
}


ZDCFitExpFermiVariableTausLHCf::ZDCFitExpFermiVariableTausLHCf(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiVariableTausLHCf" + tag).c_str(), ZDCFermiExpFitRefl, tmin, tmax, 9)),
  m_fixTau1(fixTau1), m_fixTau2(fixTau2), m_tau1(tau1), m_tau2(tau2)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "tau1");
  theTF1->SetParName(3, "tau2");
  theTF1->SetParName(4, "C");
  theTF1->SetParName(5, "refdelay");
  theTF1->SetParName(6, "reflAmpFrac");
  theTF1->SetParName(7, "reflWidth");
  theTF1->SetParName(8, "delta");

  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(4, -100, 100);
  theTF1->SetParLimits(6, -1e-4, 0.35);

  theTF1->FixParameter(5, 6.5);
  theTF1->FixParameter(7, 1.5);
  theTF1->FixParameter(8, 0.01);

  if (m_fixTau1) theTF1->FixParameter(2, m_tau1);
  else theTF1->SetParLimits(2, 1, 2);
    
  if (m_fixTau2) theTF1->FixParameter(3, m_tau2);
  else theTF1->SetParLimits(3, 3.5, 6);
}

void ZDCFitExpFermiVariableTausLHCf::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParameter(0, initialAmp);

  float t0 = initialT0;
  if (t0 < GetT0Min()) t0 = GetT0Min()*1.1;
  if (t0 > GetT0Max()) t0 = GetT0Max()/1.1;
  theTF1->SetParameter(1, t0);

  theTF1->SetParameter(1, initialT0);
  theTF1->SetParameter(4, 0);
  theTF1->SetParameter(6, 0.1);

  theTF1->SetParLimits(0, ampMin, ampMax);

  if (!m_fixTau1) theTF1->SetParameter(2, m_tau1);
  if (!m_fixTau2) theTF1->SetParameter(3, m_tau2);

  // Set the parameter errors which ROOT now uses <<<to set the initial step sizes>>>
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  
  theTF1->SetParError(0, ampStep);
  theTF1->SetParError(1, 1.0);

  if (!m_fixTau1) theTF1->SetParError(2, 0.05);
  if (!m_fixTau2) theTF1->SetParError(3, 0.25);

  theTF1->SetParError(4, 2);
  theTF1->SetParError(5, 0.25);
  theTF1->SetParError(6, 0.05);
}

void ZDCFitExpFermiVariableTausLHCf::SetT0FitLimits(float t0Min, float t0Max)
{
  // Set the parameter limits accordingly on the TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

void ZDCFitExpFermiVariableTausLHCf::doConstrainFit()
{
  // We force the constant and reflection terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(6, 0);
}
void ZDCFitExpFermiVariableTausLHCf::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(4);
  theTF1->SetParameter(4, 0);

  theTF1->ReleaseParameter(6);
  theTF1->SetParLimits(6, -1e-4, 0.35);
  theTF1->SetParameter(6, 0.1);
}

ZDCFitExpFermiVariableTausInduct::ZDCFitExpFermiVariableTausInduct(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiVariableTausInduct" + tag).c_str(), ZDCFermiExpFitInduct, tmin, tmax, 10)),
  m_fixTau1(fixTau1), m_fixTau2(fixTau2), m_tau1(tau1), m_tau2(tau2)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "tau1");
  theTF1->SetParName(3, "tau2");
  theTF1->SetParName(4, "C");
  theTF1->SetParName(5, "period");
  theTF1->SetParName(6, "IndA");
  theTF1->SetParName(7, "IndB");
  theTF1->SetParName(8, "tauI");
  theTF1->SetParName(9, "delta");

  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(4, -50, 50);
  theTF1->SetParLimits(5, 10, 50);
  theTF1->SetParLimits(6, 0, 2);
  theTF1->SetParLimits(7, 0, 2);
  //  theTF1->SetParLimits(8, 1, 6);
  theTF1->FixParameter(8, 5.5);
  theTF1->FixParameter(5, 16);
  //  theTF1->SetParLimits(9, 0, 0.05);

  
  FixDelta(0.015);
  
  if (m_fixTau1) theTF1->FixParameter(2, m_tau1);
  else theTF1->SetParLimits(2, 0.5, 3);
    
  if (m_fixTau2) theTF1->FixParameter(3, m_tau2);
  else theTF1->SetParLimits(3, 3.5, 8);
}


ZDCFitExpFermiVariableTausInduct::ZDCFitExpFermiVariableTausInduct(const std::string& tag, const std::string & paramSet, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiVariableTausInduct" + tag).c_str(), ZDCFermiExpFitInduct, tmin, tmax, 10),
		s_ConfigParamMap.at("default"))
{
  
  auto iter = s_ConfigParamMap.find(paramSet);
  if (iter == s_ConfigParamMap.end()) throw std::runtime_error("ZDCFitExpFermiVariableTausInduct:: invalid parameter set identifier: " + paramSet);

  std::vector<ZDCFitWrapper::FitParameter> overRideParams = iter->second;
  
  // Handle the JSON-based tau settings (this is ugly and temporary until we get the more general parameter overriding implemented)
  //
  auto tau1Iter = std::find_if(overRideParams.begin(), overRideParams.end(), [](const FitParameter& parDesc){return parDesc.name() == "tau1";});
  if (tau1Iter != overRideParams.end()) {
    if (fixTau1) tau1Iter->setFixed(tau1);
    else tau1Iter->setDefaultValue(tau1);
  }
  else {
    FitParameter tau1Descr = GetParameterDescr("tau1");
    if (fixTau1) tau1Descr.setFixed(tau1);
    else tau1Descr.setDefaultValue(tau1);
    overRideParams.push_back(std::move(tau1Descr));
  }

  auto tau2Iter = std::find_if(overRideParams.begin(), overRideParams.end(), [](const FitParameter& parDesc){return parDesc.name() == "tau2";});
  if (tau2Iter != overRideParams.end()) {
    if (fixTau2) tau2Iter->setFixed(tau2);
    else tau2Iter->setDefaultValue(tau2);
  }
  else {
    FitParameter tau2Descr = GetParameterDescr("tau2");
    if (fixTau2) tau2Descr.setFixed(tau2);
    else tau2Descr.setDefaultValue(std::move(tau2));    
    overRideParams.push_back(tau2Descr);
  }

  OverrideParameterSet(overRideParams);
}

void ZDCFitExpFermiVariableTausInduct::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  // Note: if we  use parameter sets, this code never gets called as the base class executes the initialize
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  theTF1->SetParameter(0, initialAmp);
  theTF1->SetParLimits(0, ampMin, ampMax);

  float t0 = initialT0;
  if (t0 < GetT0Min()) t0 = GetT0Min()*1.1;
  if (t0 > GetT0Max()) t0 = GetT0Max()/1.1;
  theTF1->SetParameter(1, t0);

  theTF1->SetParameter(6, 0.25);
  theTF1->SetParameter(7, 0.8);
  if (!m_fixDelta) theTF1->SetParameter(9, 0.);

  if (!m_fixTau1) theTF1->SetParameter(2, m_tau1);
  if (!m_fixTau2) theTF1->SetParameter(3, m_tau2);

  // Set the parameter errors which ROOT now uses <<<to set the initial step sizes>>>
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  
  theTF1->SetParError(0, ampStep);
  theTF1->SetParError(1, 1.0);

  if (!m_fixTau1) theTF1->SetParError(2, 0.05);
  if (!m_fixTau2) theTF1->SetParError(3, 0.25);

  theTF1->SetParError(4, 1);
  theTF1->SetParError(5, 5);
  theTF1->SetParError(6, 0.1);
  theTF1->SetParError(7, 0.1);
}

void ZDCFitExpFermiVariableTausInduct::SetT0FitLimits(float t0Min, float t0Max)
{
  // Set the parameter limits accordingly on the TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

void ZDCFitExpFermiVariableTausInduct::doConstrainFit()
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->FixParameter(6, 0.25);
  theTF1->FixParameter(4, 0);
  // theTF1->FixParameter(7, 0.1);
}

void ZDCFitExpFermiVariableTausInduct::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->ReleaseParameter(6);
  theTF1->ReleaseParameter(4);
  theTF1->SetParLimits(6, 0, 2);
  theTF1->SetParLimits(4, -50, 50);

}

ZDCFitExpFermiFixedTaus::ZDCFitExpFermiFixedTaus(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiFixedTaus" + tag).c_str(), this, tmin, tmax, 3)),
  m_tau1(tau1), m_tau2(tau2)
{
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "C");

  // Now create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiFixedTausRefFunc" + tag;
  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 5);

  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);
  m_expFermiFunc->SetParameter(4, 0);

  m_norm = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);
}

void ZDCFitExpFermiFixedTaus::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);
}
void ZDCFitExpFermiFixedTaus::doConstrainFit()
{
  // We force the constant tersm to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
}
void ZDCFitExpFermiFixedTaus::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(2);
}

void ZDCFitExpFermiFixedTaus::SetT0FitLimits(float t0Min, float t0Max)
{
  GetWrapperTF1()->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiPrePulse::ZDCFitExpFermiPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiPrePulse" + tag).c_str(), this, tmin, tmax, 5)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiPrePulseRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 5);

  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);
  m_expFermiFunc->SetParameter(4, 0);

  m_norm = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);

  // Now set up the actual TF1
  //
   std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(3, -20, 10);

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Amp_{pre}");
  theTF1->SetParName(3, "T0_{pre}");
  theTF1->SetParName(4, "C");
}

void ZDCFitExpFermiPrePulse::doConstrainFit()
{
  // We force the constant term and per-pulse amplitude to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
  theTF1->FixParameter(4, 0);
}
void ZDCFitExpFermiPrePulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(2);
  theTF1->ReleaseParameter(4);

  theTF1->SetParLimits(2, 1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
}

void ZDCFitExpFermiPrePulse::SetPrePulseT0Range(float tmin, float tmax)
{
  if (tmin > GetTMin()) {
    m_preT0Min = tmin;
    GetWrapperTF1()->ReleaseParameter(3);
    GetWrapperTF1()->SetParLimits(3, tmin, tmax);
  }
  else {
    m_preT0Min = -25;
    GetWrapperTF1()->SetParLimits(3, -25, tmax);
  }
  m_preT0Max = tmax;
}

void ZDCFitExpFermiPrePulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, 5);
  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);
  
  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 2);
  GetWrapperTF1()->SetParError(3, 1.0);
  GetWrapperTF1()->SetParError(4, 1.0);
}

void ZDCFitExpFermiPrePulse::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiPreExp::ZDCFitExpFermiPreExp(const std::string& tag, float tmin, float tmax, float tau1, float tau2,
						   float defExpTau, float fixExpTau) :
  ZDCPreExpFitWrapper(std::make_shared<TF1>(("ExpFermiPreExp" + tag).c_str(), this, tmin, tmax, 6), defExpTau, fixExpTau),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiPreExpRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, tmin, tmax, 8);

  // The parameters for the FermiExp
  //
  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);
  m_expFermiFunc->SetParameter(4, 0);

  m_norm = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Amp_{pre}");
  theTF1->SetParName(3, "tau_{pre}");
  theTF1->SetParName(4, "tausqrt_{pre}");
  theTF1->SetParName(5, "C");

  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, -1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(3, 6, 30);
  theTF1->SetParLimits(4, -0.2, 0.2);
  theTF1->SetParLimits(5, -50, 50);
}

void ZDCFitExpFermiPreExp::doConstrainFit()
{
  // We force the constant term and per-pulse amplitude to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
  theTF1->FixParameter(3, getDefaultExpTau());
  theTF1->FixParameter(5, 0);
}
void ZDCFitExpFermiPreExp::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->ReleaseParameter(2);
  theTF1->SetParLimits(2, -1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.

  theTF1->ReleaseParameter(3);
  theTF1->SetParLimits(3, 6, 30);
  theTF1->SetParameter(3, getDefaultExpTau());
  
  theTF1->ReleaseParameter(5);
  theTF1->SetParLimits(5, -50, 50);
  theTF1->SetParameter(5, 0);
}

void ZDCFitExpFermiPreExp::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, 0);
  GetWrapperTF1()->SetParameter(3, getDefaultExpTau());
  GetWrapperTF1()->SetParameter(4, 0.);
  GetWrapperTF1()->SetParameter(5, 0);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, ampStep/2);
  GetWrapperTF1()->SetParError(3, 1.0);
  GetWrapperTF1()->SetParError(4, 0.025);
  GetWrapperTF1()->SetParError(5, 1.0);
}

void ZDCFitExpFermiPreExp::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiLHCfPreExp::ZDCFitExpFermiLHCfPreExp(const std::string& tag, float tmin, float tmax, float tau1, float tau2,
						   float defExpTau, float fixExpTau) :
  ZDCPreExpFitWrapper(std::make_shared<TF1>(("ExpFermiLHCfPreExp" + tag).c_str(), this, tmin, tmax, 8), defExpTau, fixExpTau),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiLHCfPreExpRefFunc" + tag;

  m_expFermiLHCfFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitRefl, tmin, tmax, 9);

  // The parameters for the FermiExpRefl
  //
  m_expFermiLHCfFunc->SetParameter(0, 1);
  m_expFermiLHCfFunc->SetParameter(1, 0);
  m_expFermiLHCfFunc->SetParameter(2, m_tau1);
  m_expFermiLHCfFunc->SetParameter(3, m_tau2);
  m_expFermiLHCfFunc->SetParameter(4, 0);
  m_expFermiLHCfFunc->SetParameter(5, 6.5);
  m_expFermiLHCfFunc->SetParameter(6, 0.1);
  m_expFermiLHCfFunc->SetParameter(7, 1.5);
  m_expFermiLHCfFunc->SetParameter(8, 0.01);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0"); 
  theTF1->SetParName(2, "Tau1");
  theTF1->SetParName(3, "Tau2");
  theTF1->SetParName(4, "Amp_{pre}");
  theTF1->SetParName(5, "tau_{pre}");
  theTF1->SetParName(6, "bsqrt_{pre}");
  theTF1->SetParName(7, "ReflFrac");

  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 1.0, 2);
  theTF1->SetParLimits(3, 3.5, 5.5);
  theTF1->SetParLimits(4, -1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(5, 4, 12);
  theTF1->SetParLimits(6, -0.001, 0.001);
  theTF1->SetParLimits(7, -1.0e-4, 0.3);
}

void ZDCFitExpFermiLHCfPreExp::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, std::max(initialAmp, ampMin));

  float t0 = initialT0;
  if (t0 < GetT0Min()) t0 = GetT0Min()*1.1;
  if (t0 > GetT0Max()) t0 = GetT0Max()/1.1;
  GetWrapperTF1()->SetParameter(1, t0);

  GetWrapperTF1()->SetParameter(2, std::max(m_tau1, (float) 1.01));
  GetWrapperTF1()->SetParameter(3, m_tau2);
  GetWrapperTF1()->SetParameter(4, 0.);
  GetWrapperTF1()->SetParameter(5, std::max(getDefaultExpTau(), (float) 6.01));
  GetWrapperTF1()->SetParameter(6, 0);
  GetWrapperTF1()->SetParameter(7, 0.2);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 0.1);
  GetWrapperTF1()->SetParError(3, 0.1);
  GetWrapperTF1()->SetParError(4, 1.0);
  GetWrapperTF1()->SetParError(5, 0.5);
  GetWrapperTF1()->SetParError(6, 0.001);
  GetWrapperTF1()->SetParError(7, 0.01);
}

void ZDCFitExpFermiLHCfPreExp::doConstrainFit()
{
  // We force the constant term and per-pulse amplitude to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(5, std::max(getDefaultExpTau(), (float) 6.01));
  theTF1->FixParameter(6, 0);
  theTF1->FixParameter(7, 0.20);
}
void ZDCFitExpFermiLHCfPreExp::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->ReleaseParameter(5);
  theTF1->SetParLimits(5, 6, 12);
  
  theTF1->ReleaseParameter(6);
  theTF1->SetParLimits(6, -0.001, 0.001);
  GetWrapperTF1()->SetParameter(6, 0);

  theTF1->ReleaseParameter(7);
  theTF1->SetParLimits(7, -1.0e-4, 0.3);
  GetWrapperTF1()->SetParameter(7, 0.2);

}

void ZDCFitExpFermiLHCfPreExp::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiLHCfPrePulse::ZDCFitExpFermiLHCfPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiLHCfPrePulse" + tag).c_str(), this, tmin, tmax, 7)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiLHCfPrePulseRefFunc" + tag;
  std::string funcNamePreFunc = "ExpFermiLHCfPrePulsePreFunc" + tag;

  m_expFermiLHCfFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitRefl, tmin, tmax, 9);
  m_expFermiLHCfFunc->SetParameter(0, 1);
  m_expFermiLHCfFunc->SetParameter(1, 0);
  m_expFermiLHCfFunc->SetParameter(2, m_tau1);
  m_expFermiLHCfFunc->SetParameter(3, m_tau2);
  m_expFermiLHCfFunc->SetParameter(4, 0);
  m_expFermiLHCfFunc->SetParameter(5, 6.5);
  m_expFermiLHCfFunc->SetParameter(6, 0.2);
  m_expFermiLHCfFunc->SetParameter(7, 1.5);
  m_expFermiLHCfFunc->SetParameter(8, 0.01);

  m_expFermiPreFunc = std::make_shared<TF1>(funcNamePreFunc.c_str(), ZDCFermiExpFit, tmin, tmax, 9);

  m_expFermiPreFunc->SetParameter(0, 1);
  m_expFermiPreFunc->SetParameter(1, 0);
  m_expFermiPreFunc->SetParameter(2, m_tau1);
  m_expFermiPreFunc->SetParameter(3, m_tau2);
  m_expFermiPreFunc->SetParameter(4, 0);
  m_preNorm = 1. / m_expFermiPreFunc->GetMaximum();

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // The actual paerameters we fit
  //
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "tau1");
  theTF1->SetParName(3, "tau2");
  theTF1->SetParName(4, "Amp_{pre}");
  theTF1->SetParName(5, "T0_{pre}");
  theTF1->SetParName(6, "C");

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 1.0, 2);
  theTF1->SetParLimits(3, 3.5, 5.5);
  theTF1->SetParLimits(4, 1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(5, -20, 10);
  theTF1->SetParLimits(6, -50, 50);

}

void ZDCFitExpFermiLHCfPrePulse::doConstrainFit()
{
  // We force the constant term and per-pulse amplitude to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(6, 0);
}
void ZDCFitExpFermiLHCfPrePulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(4);
  theTF1->ReleaseParameter(6);

  theTF1->SetParLimits(4, 1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(6, -50, 50);
}

void ZDCFitExpFermiLHCfPrePulse::SetPrePulseT0Range(float tmin, float tmax)
{
  if (tmin > GetTMin()) {
    m_preT0Min = tmin;
    GetWrapperTF1()->ReleaseParameter(5);
    GetWrapperTF1()->SetParLimits(5, tmin, tmax);
  }
  else {
    m_preT0Min = -25;
    GetWrapperTF1()->SetParLimits(5, -25, tmax);
  }
  m_preT0Max = tmax;
}

void ZDCFitExpFermiLHCfPrePulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, m_tau1);
  GetWrapperTF1()->SetParameter(3, m_tau2);
  GetWrapperTF1()->SetParameter(4, 5);
  GetWrapperTF1()->SetParameter(5, -10);
  GetWrapperTF1()->SetParameter(6, 0);
  
  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);
  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 0.05);
  GetWrapperTF1()->SetParError(3, 1);
  GetWrapperTF1()->SetParError(4, 5.0);
  GetWrapperTF1()->SetParError(5, 1.0);
  GetWrapperTF1()->SetParError(6, 2.0);
}

void ZDCFitExpFermiLHCfPrePulse::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiInductPreExp::ZDCFitExpFermiInductPreExp(const std::string& tag, float tmin, float tmax, float tau1, float tau2,
						   float defExpTau, float fixExpTau) :
  ZDCPreExpFitWrapper(std::make_shared<TF1>(("ExpFermiInductPreExp" + tag).c_str(), this, tmin, tmax, 10), defExpTau, fixExpTau),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiInductPreExpRefFunc" + tag;

  m_expFermiInductFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitInduct, tmin, tmax, 10);

  // The parameters for the FermiExpInduct
  //
  m_expFermiInductFunc->SetParameter(0, 1);
  m_expFermiInductFunc->SetParameter(1, 0);
  m_expFermiInductFunc->SetParameter(2, m_tau1); // tau1 is fixed
  m_expFermiInductFunc->SetParameter(3, m_tau2); // tau2 is fixed
  m_expFermiInductFunc->SetParameter(4, 0);      // the constant term will be handled directly not here
  m_expFermiInductFunc->SetParameter(5, 14); // the period will be fixed
  m_expFermiInductFunc->SetParameter(6, 0.25);  // A will be varied as part of the fit
  m_expFermiInductFunc->SetParameter(7, 0.5);  // B will be varied as part of the fit
  m_expFermiInductFunc->SetParameter(8, 0.18);
  m_expFermiInductFunc->SetParameter(9, 0.015);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Aexp");
  theTF1->SetParName(3, "tauexp");
  theTF1->SetParName(4, "bsqrtexp");
  theTF1->SetParName(5, "C");
  theTF1->SetParName(6, "Period");
  theTF1->SetParName(7, "IndA");
  theTF1->SetParName(8, "IndB");
  theTF1->SetParName(9, "bI");

  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, -1, 8196); // Increase the upper range to 2 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(3, 4, 12);
  theTF1->SetParLimits(4, -0.001, 0.001);
  theTF1->SetParLimits(5, -10, 10);
  theTF1->FixParameter(6, 13);
  theTF1->SetParLimits(7, -0.1, 2);
  theTF1->SetParLimits(8, -0.1, 2);
  theTF1->SetParLimits(9, -0.05, 0.5);
  
}

ZDCFitExpFermiInductPreExp::ZDCFitExpFermiInductPreExp(const std::string& tag, const std::string & paramSet, float tmin, float tmax, float tau1, float tau2) :
  ZDCPreExpFitWrapper(std::make_shared<TF1>(("ExpFermiInductPreExp" + tag).c_str(), this, tmin, tmax, 10), s_ConfigParamMap.at("default")),
  m_tau1(tau1), m_tau2(tau2)
{
  auto iter = s_ConfigParamMap.find(paramSet);
  if (iter == s_ConfigParamMap.end()) throw std::runtime_error("ZDCFitExpFermiVariableTausInduct:: invalid parameter set identifier: " + paramSet);

  std::vector<ZDCFitWrapper::FitParameter> overRideParams = iter->second;

  OverrideParameterSet(overRideParams);

  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiInductPreExpRefFunc" + tag;

  m_expFermiInductFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitInduct, tmin, tmax, 10);

  // The parameters for the FermiExpInduct
  //
  m_expFermiInductFunc->SetParameter(0, 1);
  m_expFermiInductFunc->SetParameter(1, 0);
  m_expFermiInductFunc->SetParameter(2, m_tau1); // tau1 is fixed
  m_expFermiInductFunc->SetParameter(3, m_tau2); // tau2 is fixed
  m_expFermiInductFunc->SetParameter(4, 0);      // the constant term will be handled directly not here
  m_expFermiInductFunc->SetParameter(5, 14); // the period will be fixed
  m_expFermiInductFunc->SetParameter(6, 0.25);  // A will be varied as part of the fit
  m_expFermiInductFunc->SetParameter(7, 0.5);  // B will be varied as part of the fit
  m_expFermiInductFunc->SetParameter(8, 0.18);
  m_expFermiInductFunc->SetParameter(9, 0.015);
}

void ZDCFitExpFermiInductPreExp::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, std::max(initialAmp, ampMin));

  float t0 = initialT0;
  if (t0 < GetT0Min()) t0 = GetT0Min()*1.1;
  if (t0 > GetT0Max()) t0 = GetT0Max()/1.1;
  GetWrapperTF1()->SetParameter(1, t0);

  GetWrapperTF1()->SetParameter(2, 1);
  GetWrapperTF1()->SetParameter(3, std::max(getDefaultExpTau(), (float) 6.01));
  GetWrapperTF1()->SetParameter(6, 14);
  GetWrapperTF1()->SetParameter(7, 0.2);
  GetWrapperTF1()->SetParameter(8, 0.2);
  GetWrapperTF1()->SetParameter(9, 0.01);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 5.0);
  GetWrapperTF1()->SetParError(3, 0.5);
  GetWrapperTF1()->SetParError(4, 0.01);
  GetWrapperTF1()->SetParError(5, 1);
  GetWrapperTF1()->SetParError(6, 1);
  GetWrapperTF1()->SetParError(7, 0.05);
  GetWrapperTF1()->SetParError(8, 0.05);
  GetWrapperTF1()->SetParError(9, 0.01);
}

void ZDCFitExpFermiInductPreExp::doConstrainFit()
{
}
void ZDCFitExpFermiInductPreExp::doUnconstrainFit()
{
}

void ZDCFitExpFermiInductPreExp::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

ZDCFitExpFermiInductPrePulse::ZDCFitExpFermiInductPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiInductPrePulse" + tag).c_str(), this, tmin, tmax, 9)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiInductPrePulseRefFunc" + tag;
  std::string funcNamePreFunc = "ExpFermiInductPrePulsePreFunc" + tag;

  m_expFermiInductFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitInduct, tmin, tmax, 9);
  m_expFermiInductFunc->SetParameter(0, 1);
  m_expFermiInductFunc->SetParameter(1, 0);
  m_expFermiInductFunc->SetParameter(2, m_tau1);
  m_expFermiInductFunc->SetParameter(3, m_tau2);
  m_expFermiInductFunc->SetParameter(4, 0);
  m_expFermiInductFunc->SetParameter(5, 14);
  m_expFermiInductFunc->SetParameter(6, 0.25);
  m_expFermiInductFunc->SetParameter(7, 0.15);
  m_expFermiInductFunc->SetParameter(8, 0.18);
  m_expFermiInductFunc->SetParameter(9, 0.015);

  m_expFermiPreFunc = std::make_shared<TF1>(funcNamePreFunc.c_str(), ZDCFermiExpFit, tmin, tmax, 5);

  m_expFermiPreFunc->SetParameter(0, 1);
  m_expFermiPreFunc->SetParameter(1, 0);
  m_expFermiPreFunc->SetParameter(2, m_tau1);
  m_expFermiPreFunc->SetParameter(3, m_tau2);
  m_expFermiPreFunc->SetParameter(4, 0);
  m_preNorm = 1. / m_expFermiPreFunc->GetMaximum();
  
  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();

  // The actual paerameters we fit
  //
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Apre");
  theTF1->SetParName(3, "T0pre");
  theTF1->SetParName(4, "C");
  theTF1->SetParName(5, "Period");
  theTF1->SetParName(6, "IndA");
  theTF1->SetParName(7, "IndB");
  theTF1->SetParName(8, "bI");

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 0, 5000);
  theTF1->SetParLimits(3, 0, 30);
  theTF1->SetParLimits(4, 0, 2);
  theTF1->SetParLimits(3, 4, 12);
  theTF1->SetParLimits(4, -50, 50);
  theTF1->SetParLimits(5, 10, 20);
  theTF1->SetParLimits(6, -0.1, 1);
  theTF1->SetParLimits(7, -0.1, 1);
  theTF1->SetParLimits(9, -0.2, 0.2);

  GetWrapperTF1()->FixParameter(5, 14);
  GetWrapperTF1()->FixParameter(8, 0.02);
}

ZDCFitExpFermiInductPrePulse::ZDCFitExpFermiInductPrePulse(const std::string& tag, const std::string & paramSet, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiInductPrePulse" + tag).c_str(), this, tmin, tmax, 9), s_ConfigParamMap.at("default")),  
  m_tau1(tau1), m_tau2(tau2)
{
  auto iter = s_ConfigParamMap.find(paramSet);
  if (iter == s_ConfigParamMap.end()) throw std::runtime_error("ZDCFitExpFermiVariableTausInduct:: invalid parameter set identifier: " + paramSet);

  std::vector<ZDCFitWrapper::FitParameter> overRideParams = iter->second;

  OverrideParameterSet(overRideParams);

  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiInductPrePulseRefFunc" + tag;
  std::string funcNamePreFunc = "ExpFermiInductPrePulsePreFunc" + tag;

  m_expFermiPreFunc = std::make_shared<TF1>(funcNamePreFunc.c_str(), ZDCFermiExpFit, tmin, tmax, 5);

  m_expFermiPreFunc->SetParameter(0, 1);
  m_expFermiPreFunc->SetParameter(1, 0);
  m_expFermiPreFunc->SetParameter(2, m_tau1);
  m_expFermiPreFunc->SetParameter(3, m_tau2);
  m_expFermiPreFunc->SetParameter(4, 0);

  // Make the base function that we use
  //
  m_expFermiInductFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFitInduct, tmin, tmax, 9);
  m_expFermiInductFunc->SetParameter(0, 1);
  m_expFermiInductFunc->SetParameter(1, 0);
  m_expFermiInductFunc->SetParameter(2, m_tau1);
  m_expFermiInductFunc->SetParameter(3, m_tau2);
  m_expFermiInductFunc->SetParameter(4, 0);
  m_expFermiInductFunc->SetParameter(5, 14);
  m_expFermiInductFunc->SetParameter(6, 0.25);
  m_expFermiInductFunc->SetParameter(7, 0.15);
  m_expFermiInductFunc->SetParameter(8, 0.18);
  m_expFermiInductFunc->SetParameter(9, 0.015);

}

void ZDCFitExpFermiInductPrePulse::doConstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(5, 0);
  theTF1->FixParameter(7, 0);
}
void ZDCFitExpFermiInductPrePulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(5);
  theTF1->ReleaseParameter(7);
  theTF1->SetParLimits(4, -0.001, 0.001);
  theTF1->SetParLimits(5, -10, 10);
  theTF1->SetParLimits(7, -0.1, 2);
}

void ZDCFitExpFermiInductPrePulse::SetPrePulseT0Range(float tmin, float tmax)
{
  if (tmin > GetTMin()) {
    m_preT0Min = tmin;
    GetWrapperTF1()->ReleaseParameter(3);
    GetWrapperTF1()->SetParLimits(3, tmin, tmax);
  }
  else {
    m_preT0Min = -25;
    GetWrapperTF1()->SetParLimits(3, -25, tmax);
  }
  m_preT0Max = tmax;
}

void ZDCFitExpFermiInductPrePulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, 5);
  GetWrapperTF1()->SetParameter(3, 10);
  GetWrapperTF1()->SetParameter(4, 0);
  GetWrapperTF1()->SetParameter(5, 0);
  GetWrapperTF1()->SetParameter(6, 0.1);
  GetWrapperTF1()->SetParameter(7, 0.1);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);
  // Set parameter errors for fit step size
  //
  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);
  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 0.05);
  GetWrapperTF1()->SetParError(3, 1);
  GetWrapperTF1()->SetParError(4, 5.0);
  GetWrapperTF1()->SetParError(5, 5);
  GetWrapperTF1()->SetParError(6, 0.05);
  GetWrapperTF1()->SetParError(7, 0.05);
}

void ZDCFitExpFermiInductPrePulse::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
ZDCFitExpFermiLinearFixedTaus::ZDCFitExpFermiLinearFixedTaus(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCFitWrapper(std::make_shared<TF1>(("ExpFermiFixedTaus" + tag).c_str(), this, tmin, tmax, 4)),
  m_tau1(tau1), m_tau2(tau2)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "s_{b}");
  theTF1->SetParName(3, "c_{b}");

  // Now create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiFixedTausRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 5);

  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);
  m_expFermiFunc->FixParameter(4, 0);

  m_norm = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);
}

void ZDCFitExpFermiLinearFixedTaus::doConstrainFit()
{
  // We force the linear terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
  theTF1->FixParameter(3, 0);
}

void ZDCFitExpFermiLinearFixedTaus::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1(); 
  theTF1->ReleaseParameter(2);
  theTF1->ReleaseParameter(3);
}

void ZDCFitExpFermiLinearFixedTaus::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  float slope     = std::abs(0.1 * initialAmp / initialT0);
  float intercept = std::abs(0.1 * initialAmp);
  GetWrapperTF1()->SetParLimits(2, -slope    , slope    );
  GetWrapperTF1()->SetParLimits(3, -intercept, intercept);

  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, 0);
  GetWrapperTF1()->SetParameter(3, 0);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);

  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1);
  GetWrapperTF1()->SetParError(2, 1);
  GetWrapperTF1()->SetParError(3, 0.5);
}

void ZDCFitExpFermiLinearFixedTaus::SetT0FitLimits(float t0Min, float t0Max)
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->SetParLimits(1, t0Min, t0Max);
}

// --------------------------------------------------------------------------------------------------------------------------------------------
//
ZDCFitExpFermiLinearPrePulse::ZDCFitExpFermiLinearPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiLinearPrePulse" + tag).c_str(), this, tmin, tmax, 6)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiPerPulseRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 5);

  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);
  m_expFermiFunc->FixParameter(4, 0);

  m_norm = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = ZDCFitWrapper::GetWrapperTF1();
  
  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Amp_{pre}");
  theTF1->SetParName(3, "T0_{pre}");
  theTF1->SetParName(4, "s_{b}");
  theTF1->SetParName(5, "c_{b}");

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2, 1, 4096); // Increase the upper range to 4 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(3, -20, 10);
}

void ZDCFitExpFermiLinearPrePulse::doConstrainFit()
{
  // We force the linear terms and prepulse terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(5, 0);
}
void ZDCFitExpFermiLinearPrePulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->ReleaseParameter(4);
  theTF1->ReleaseParameter(5);
}

void ZDCFitExpFermiLinearPrePulse::SetPrePulseT0Range(float tmin, float tmax)
{
  GetWrapperTF1()->SetParLimits(3, tmin, tmax);
}

void ZDCFitExpFermiLinearPrePulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  float slope     = std::abs(initialAmp / initialT0);  // to be studied more ??? limit 0.1 0.05
  float intercept = std::abs(0.5 * initialAmp);
  GetWrapperTF1()->SetParLimits(4, -slope    , slope    );
  GetWrapperTF1()->SetParLimits(5, -intercept, intercept);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2, 5);
  GetWrapperTF1()->SetParameter(5, 0);

  double ampStep = std::min(0.05*initialAmp, std::abs(ampMax - initialAmp)/2.);

  GetWrapperTF1()->SetParError(0, ampStep);
  GetWrapperTF1()->SetParError(1, 1.0);
  GetWrapperTF1()->SetParError(2, 1);
  GetWrapperTF1()->SetParError(3, 1);
  GetWrapperTF1()->SetParError(4, 0.1);
  GetWrapperTF1()->SetParError(5, 1);
}

void ZDCFitExpFermiLinearPrePulse::SetT0FitLimits(float t0Min, float t0Max)
{
  GetWrapperTF1()->SetParLimits(1, t0Min, t0Max);
}


// --------------------------------------------------------------------------------------------------------------------------------------------
//
ZDCFitComplexPrePulse::ZDCFitComplexPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiPrePulse" + tag).c_str(), this, tmin, tmax, 7)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiPerPulseRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 4);
  
  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);

  m_norm     = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2,    0, 2048);  // Pre-pulse upper bound should not be greater 2 times of ADC range with overflow constrains.
  theTF1->SetParLimits(3,    0,   40);
  theTF1->SetParLimits(6,    0, 4096);  // Increase the upper range to 4 times of ADC range to deal with large exponential tail case of pre-pulse.

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Amp_{pre}");
  theTF1->SetParName(3, "T0_{pre}");
  theTF1->SetParName(4, "s_{b}");
  theTF1->SetParName(5, "c_{b}");
  theTF1->SetParName(6, "Amp_{exp}");
}

void ZDCFitComplexPrePulse::doConstrainFit()
{
  // We force the linear terms and prepulse terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(5, 0);
}
void ZDCFitComplexPrePulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(2);
  theTF1->ReleaseParameter(4);
  theTF1->ReleaseParameter(5);
}

void ZDCFitComplexPrePulse::SetPrePulseT0Range(float tmin, float tmax)
{
  if (tmin > GetTMin()) {
    GetWrapperTF1()->ReleaseParameter(3);
    GetWrapperTF1()->SetParLimits(3, tmin, tmax);
  }
  else {
    GetWrapperTF1()->SetParLimits(3, 0, tmax);
  }
}

void ZDCFitComplexPrePulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  float slope     = std::abs(initialAmp / initialT0);  // to be studied more ??? limit 0.1 0.05
  float intercept = std::abs(0.1 * initialAmp);       // reduce from 0.25 to 0.1 fix some fail issue
  GetWrapperTF1()->SetParLimits(4, -slope    , slope    );  // if the lower limit is set to 0, there will be some fit fail issue...
  GetWrapperTF1()->SetParLimits(5, -intercept, intercept);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(2,  5);
  GetWrapperTF1()->SetParameter(3, 10);
  GetWrapperTF1()->SetParameter(4,  0);
  GetWrapperTF1()->SetParameter(5,  0);
  GetWrapperTF1()->SetParameter(6,  1);
}

void ZDCFitComplexPrePulse::SetT0FitLimits(float t0Min, float t0Max)
{
  GetWrapperTF1()->SetParLimits(1, t0Min, t0Max);
}



// --------------------------------------------------------------------------------------------------------------------------------------------
//
ZDCFitGeneralPulse::ZDCFitGeneralPulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2) :
  ZDCPrePulseFitWrapper(std::make_shared<TF1>(("ExpFermiPrePulse" + tag).c_str(), this, tmin, tmax, 9)),
  m_tau1(tau1), m_tau2(tau2)
{
  // Create the reference function that we use to evaluate ExpFermiFit more efficiently
  //
  std::string funcNameRefFunc = "ExpFermiPerPulseRefFunc" + tag;

  m_expFermiFunc = std::make_shared<TF1>(funcNameRefFunc.c_str(), ZDCFermiExpFit, -50, 100, 4);

  m_expFermiFunc->SetParameter(0, 1);
  m_expFermiFunc->SetParameter(1, 0);
  m_expFermiFunc->SetParameter(2, m_tau1);
  m_expFermiFunc->SetParameter(3, m_tau2);

  m_norm     = 1. / m_expFermiFunc->GetMaximum();
  m_timeCorr = m_tau1 * std::log(m_tau2 / m_tau1 - 1.0);

  // Now set up the actual TF1
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  // BAC, parameter 0 limits now is set in DoInitialize
  theTF1->SetParLimits(1, tmin, tmax);
  theTF1->SetParLimits(2,    0, 2048);  // Pre-pulse upper bound should not be greater 2 times of ADC range with overflow constrains.
  theTF1->SetParLimits(3,    0,   40);
  theTF1->SetParLimits(6,    0, 4096);  // Increase the upper range to 4 times of ADC range to deal with large exponential tail case of pre-pulse.
  theTF1->SetParLimits(7,    0, 2048);  // Post-pulse upper bound should not be greater 2 times of ADC range with overflow constrains.
  theTF1->SetParLimits(8,  100,  163);

  theTF1->SetParName(0, "Amp");
  theTF1->SetParName(1, "T0");
  theTF1->SetParName(2, "Amp_{pre}");
  theTF1->SetParName(3, "T0_{pre}");
  theTF1->SetParName(4, "s_{b}");
  theTF1->SetParName(5, "c_{b}");
  theTF1->SetParName(6, "Amp_{exp}");
  theTF1->SetParName(7, "Amp_{post}");
  theTF1->SetParName(8, "T0_{post}");
}

void ZDCFitGeneralPulse::doConstrainFit()
{
  // We force the linear terms and prepulse terms to zero
  //
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();

  theTF1->FixParameter(2, 0);
  theTF1->FixParameter(4, 0);
  theTF1->FixParameter(5, 0);
}
void ZDCFitGeneralPulse::doUnconstrainFit()
{
  std::shared_ptr<TF1> theTF1 = GetWrapperTF1();
  theTF1->ReleaseParameter(2);
  theTF1->ReleaseParameter(4);
  theTF1->ReleaseParameter(5);
}

void ZDCFitGeneralPulse::SetPrePulseT0Range(float tmin, float tmax)
{
  if (tmin > GetTMin()) {
    if (tmin < 0) tmin = 0;
    GetWrapperTF1()->SetParLimits(3, tmin, tmax);
  }
  else {
    GetWrapperTF1()->SetParLimits(3, 0, tmax);
  }
}

void ZDCFitGeneralPulse::SetPostPulseT0Range(float tmin, float tmax, float initialPostT0)
{
  GetWrapperTF1()->SetParLimits(8, tmin, tmax);
  float iniPostT0 = initialPostT0;
  GetWrapperTF1()->SetParameter(8, iniPostT0);
}

void ZDCFitGeneralPulse::DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax)
{
  float slope     = std::abs(initialAmp / initialT0);  // to be studied more ??? limit 0.1 0.05
  float intercept = std::abs(0.1 * initialAmp);       // reduce from 0.25 to 0.1 fix some fail issue
  GetWrapperTF1()->SetParLimits(4, -slope    , slope    );  // if the lower limit is set to 0, there will be some fit fail issue...
  GetWrapperTF1()->SetParLimits(5, -intercept, intercept);

  GetWrapperTF1()->SetParLimits(0, ampMin, ampMax);

  GetWrapperTF1()->SetParameter(0, initialAmp);
  GetWrapperTF1()->SetParameter(1, initialT0);
  GetWrapperTF1()->SetParameter(4,   0);
  GetWrapperTF1()->SetParameter(5,   0);
  GetWrapperTF1()->SetParameter(7,   5);
}

void ZDCFitGeneralPulse::SetT0FitLimits(float t0Min, float t0Max)
{
  GetWrapperTF1()->SetParLimits(1, t0Min, t0Max);
}
