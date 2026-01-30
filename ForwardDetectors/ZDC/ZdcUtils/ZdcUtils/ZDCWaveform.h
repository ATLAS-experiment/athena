/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCUTILS_ZDCWAVEFORM_H
#define ZDCUTILS_ZDCWAVEFORM_H

#include <string>
#include <vector>
#include <map>
#include "TF1.h"

// Base class that defines the interface to all waveform classes
//
//   The waveforms describe possible shapes of pulses seen in the ZDC or RPD
//
//   They should be defined such that the maximum occurs at t = 0.
//     The base class provides a rescaling to provide unity value at maximum (t = 0).
//
//   Users of the waveform then produce "real" pulses by evaluating the pulse at
//   at (t-t0) and multiplying by an amplitude.
//   
//   The waveforms are assumed to have at least two shape parameters, tau_1 and tau_2.
//   They may also have additional shape parameters that are described by strings and
//     are given default values in the constructor.
//
//   The waveform should be able to be evaluated at any time, positive or negative
// 
//
class ZDCWaveformBase
{
  std::string m_tag;
  std::map<std::string, unsigned int> m_addtlShapeNames;

  double m_initialTauRise;
  double m_initialTauFall;
  unsigned int m_numAddtlShapePars;
  std::vector<double> m_addtlShapeInitialValues;

protected:
  
  double m_tauRise;
  double m_tauFall;
  
  std::vector<double> m_addtlShapeValues;

  void setAddtlShapeParameters(const std::vector<std::string> &addtlShapeNames, 
			       const std::vector<double> &addtlShapeValues);  
  void setAddtlShapeValues(const double* values);

protected:
    
  void setAddtlShapeValues(const std::vector<double> &values) {
    setAddtlShapeValues(&values[0]);
  }

  // The actual implementation classes must override doEvaluate to produce the corresponding waveform
  //
  virtual double doEvaluate(double time) const = 0;
 
  // The implementation classes must provide a name
  //
  virtual std::string name() const = 0;

  ZDCWaveformBase() = delete;

 public:
  ZDCWaveformBase(std::string tag, double initialTauRise, double initialTauFall, const std::vector<std::string> &addtlShapeNames, 
		  const std::vector<double> &addtlShapeValues);
		  
  virtual ~ZDCWaveformBase() = default;

  ZDCWaveformBase(const ZDCWaveformBase& instance);

  // Duplicate this object 
  //
  ZDCWaveformBase* Duplicate();
  
  // This method provides the actual value of the waveform at the provided time
  //
  double evaluate(double time) const
  {
    return doEvaluate(time)/doEvaluate(0);
  }

  double evaluateRoot (double *x, double *p) {
    return this->operator()(x,p);
  }
  
  double evaluateRootNoTF1Par (double *x, double *p) {
    (void)p;
    return evaluate(x[0]);
  }

  double operator() (double *x, double *p) {
    setTaus(p[0], p[1]);
    if (getNumAddtlShapeValues() > 0) setAddtlShapeValues(p + 2);
    return evaluate(x[0]);
  }

  const std::string& getTag() const {return m_tag;}
  std::string getNameTag() const {return name() + "_" + m_tag;}
  const std::string getName() const {return name();}
  double getTauRise() const {return m_tauRise;}
  double getTauFall() const {return m_tauFall;}

  unsigned int getNumAddtlShapeValues() const {return m_numAddtlShapePars;}
  double getAddtlShapeValue(const std::string& name) const {return m_addtlShapeValues.at(m_addtlShapeNames.find(name)->second);}
  double getAddtlShapeValue(unsigned int index) const {return m_addtlShapeValues.at(index);}
  
  void setAddtlShapeValue(std::string name, double value);

   void setAddtlShapeValue(unsigned int index, double value) {
    m_addtlShapeValues.at(index) = value;
   }

  void setTaus(double tauRise, double tauFall)
  {
    m_tauRise = tauRise;
    m_tauFall = tauFall;
  }

  // Restore the initial values for all parameters
  //
  void restoreInitial();
  

// Make a TF1 that can draw the waveform. Depending on useTF1Params, the parameters
//   can be controlled via the TF1 or via the ZDCWaveform object. In the latter case
//   the TF1 has no parameters
// 
//
TF1* makeWaveformTF1(ZDCWaveformBase* ptr, double xmin, double xmax, bool useTF1Params = true)
{
  std::string name = ptr->getNameTag() + "_TF1";

  TF1* newTF1 = 0;

  if (useTF1Params) {
    unsigned int numPar = 2 + ptr->getNumAddtlShapeValues();

    newTF1= new TF1(name.c_str(), ptr, &ZDCWaveformBase::evaluateRoot, xmin, xmax, numPar);

    newTF1->SetParameter(0, ptr->getTauRise());
    newTF1->SetParameter(1, ptr->getTauFall());
    
    if (numPar > 2) {
      for (unsigned int idxpar = 2; idxpar < numPar; idxpar++) {
	newTF1->SetParameter(idxpar, ptr->getAddtlShapeValue(idxpar - 2));
      }
    }
  }
  else {
    newTF1= new TF1(name.c_str(), ptr, &ZDCWaveformBase::evaluateRootNoTF1Par, xmin, xmax, 0);
  }
  
  newTF1->SetNpx(1000);
  return newTF1;
}

TF1* makeWaveformTF1(ZDCWaveformBase& instance, double xmin, double xmax, bool useTF1Params = true)
{
  return makeWaveformTF1(&instance, xmin, xmax, useTF1Params);
}

};
#endif
