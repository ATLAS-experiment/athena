/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCANALYSIS_ZDCFITWRAPPER_H
#define ZDCANALYSIS_ZDCFITWRAPPER_H

#include "CxxUtils/checker_macros.h"

// Base class that defines the interface
//
#include <TF1.h>
#include <memory>

double ZDCFermiExpFit(const double* xvec, const double* pvec);
double ZDCFermiExpFitRefl(const double* xvec, const double* pvec);

class ATLAS_NOT_THREAD_SAFE ZDCFitWrapper
{
private:
   std::shared_ptr<TF1> m_wrapperTF1;

  float m_tmin{};
  float m_tmax{};

  float m_ampMin{};
  float m_ampMax{};

  float m_t0Min{};
  float m_t0Max{};

  bool  m_adjTLimitsEvent{};
  float m_tminAdjust{};

public:
 ZDCFitWrapper(const std::shared_ptr<TF1>& wrapperTF1) : m_wrapperTF1(wrapperTF1),
    m_ampMin(0),
    m_adjTLimitsEvent(true)  // true here forces a setting of T0 par limits on first event
  {
    m_tmin = m_wrapperTF1->GetXmin();
    m_tmax = m_wrapperTF1->GetXmax();

    m_t0Min = m_tmin;
    m_t0Max = m_tmax;
  }

  virtual ~ZDCFitWrapper() {}

  void Initialize(float initialAmp, float initialT0, float ampMin, float ampMax);
  void Initialize(float initialAmp, float initialT0, float ampMin, float ampMax, float fitTmin, float fitTmax, float fitTRef);

  // Performs the class-specific event initialization
  //
  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) = 0;

  // Actually sets the t0 parameter limits in the fit function
  //
  virtual void SetT0FitLimits(float tMin, float tMax) = 0;

  void SetAmpMinMax(float minAmp, float maxAmp)
  {
    m_ampMin = minAmp;
    m_ampMax = maxAmp;
  }

  void SetT0Range(float t0Min, float t0Max)
  {
    m_t0Min = t0Min;
    m_t0Max = t0Max;

    SetT0FitLimits(t0Min, t0Max);
  }

  virtual void ConstrainFit() = 0;
  virtual void UnconstrainFit() = 0;

  virtual float GetAmplitude() const = 0;
  virtual float GetAmpError() const = 0;
  virtual float GetTime() const = 0;
  virtual float GetTau1() const = 0;
  virtual float GetTau2() const = 0;

  float GetMinAmp() const {return m_ampMin;}
  float GetMaxAmp() const {return m_ampMax;}

  float GetTMin() const {return m_tmin;}
  float GetTMax() const {return m_tmax;}

  float GetT0Min() const {return m_t0Min;}
  float GetT0Max() const {return m_t0Max;}

  float GetTMinAdjust() const {return m_tminAdjust;}

  virtual float GetBkgdMaxFraction() const = 0;

  virtual float GetShapeParameter(size_t index) const = 0;

  virtual double operator()(const double *x, const double *p) = 0;

  virtual std::shared_ptr<TF1> GetWrapperTF1() {return m_wrapperTF1;}
  virtual const TF1* GetWrapperTF1() const {return m_wrapperTF1.get();}
  virtual TF1* GetWrapperTF1RawPtr() const {return m_wrapperTF1.get();}
};

class ATLAS_NOT_THREAD_SAFE ZDCPrePulseFitWrapper : public ZDCFitWrapper
{
protected:
  float m_preT0Min;
  float m_preT0Max;
public:
  ZDCPrePulseFitWrapper(std::shared_ptr<TF1> wrapperTF1) : ZDCFitWrapper(wrapperTF1)
  {
    m_preT0Min = GetTMin();
    m_preT0Max = GetTMax();
  }

  virtual void SetInitialPrePulse(float amp, float t0, float expamp, bool fixPrePulseToZero) = 0;

  virtual void SetPrePulseT0Range(float tmin, float tmax) = 0;
  virtual void SetPostPulseT0Range(float tmin, float tmax, float initialPostT0) = 0;

  virtual unsigned int GetPreT0ParIndex() const = 0;

  virtual float GetPreT0()  const = 0;
  virtual float GetPreAmp() const = 0;

  virtual float GetPostT0()  const = 0;
  virtual float GetPostAmp() const = 0;

  virtual float GetExpAmp()  const = 0;
};

class ATLAS_NOT_THREAD_SAFE ZDCPreExpFitWrapper : public ZDCFitWrapper
{
protected:
  float m_defaultTau;
  bool m_fixTau;

public:
  ZDCPreExpFitWrapper(std::shared_ptr<TF1> wrapperTF1, float defaultExpTau, bool fixTau) :
    ZDCFitWrapper(wrapperTF1), m_defaultTau(defaultExpTau), m_fixTau(fixTau)
    {}

  virtual void SetInitialExpPulse(float expamp) = 0;

  bool fixExpTau() const {return m_fixTau;}
  float getDefaultExpTau() const {return m_defaultTau;}
  
  virtual float GetExpAmp() const = 0;
  virtual float GetExpTau() const = 0;
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiVariableTaus : public ZDCFitWrapper
{
protected:
  bool m_fixTau1;
  bool m_fixTau2;

  float m_tau1;
  float m_tau2;

public:

  ZDCFitExpFermiVariableTaus(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2);

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return GetWrapperTF1()->GetParameter(2);}
  virtual float GetTau2() const override {return GetWrapperTF1()->GetParameter(3);}

  virtual float GetTime() const override {
    const TF1* theTF1 = GetWrapperTF1();

    float fitT0 =  theTF1->GetParameter(1);

    float tau1 = theTF1->GetParameter(2);
    float tau2 = theTF1->GetParameter(3);

    // Correct the time to the maximum
    //
    if (tau2 > tau1) fitT0 += tau1 * std::log(tau2 / tau1 - 1.0);
    return fitT0;
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return GetWrapperTF1()->GetParameter(2);
    else if (index == 1) return GetWrapperTF1()->GetParameter(3);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    double amp = theTF1->GetParameter(0);
    double constant = theTF1->GetParameter(4);

    if (amp > 0) return constant / amp;
    else return -1;
  }

  virtual double operator()(const double *x, const double *p)  override{
    return ZDCFermiExpFit(x, p);
  }

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;
};

//
// A special version of the fermi*negative exponential function that includes a term to describe
//   the "reflection" effects we see in the ZDC+LHCf joint run due to the splitting of the ZDC
//   signals to send a copy to LHCf DAQ. It's not really a reflection but an artifact introduced
//   by the bandwidth limit of the linear fan-in/fan-out module that we used.
//
class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiVariableTausLHCf : public ZDCFitWrapper
{
protected:
  bool m_fixTau1;
  bool m_fixTau2;

  float m_tau1;
  float m_tau2;

public:

  ZDCFitExpFermiVariableTausLHCf(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2);

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return GetWrapperTF1()->GetParameter(2);}
  virtual float GetTau2() const override {return GetWrapperTF1()->GetParameter(3);}

  virtual float GetTime() const override {
    const TF1* theTF1 = GetWrapperTF1();

    float fitT0 =  theTF1->GetParameter(1);

    float tau1 = theTF1->GetParameter(2);
    float tau2 = theTF1->GetParameter(3);

    // Correct the time to the maximum
    //
    if (tau2 > tau1) fitT0 += tau1 * std::log(tau2 / tau1 - 1.0);
    return fitT0;
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return GetWrapperTF1()->GetParameter(2);
    else if (index == 1) return GetWrapperTF1()->GetParameter(3);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    double amp = theTF1->GetParameter(0);
    double constant = theTF1->GetParameter(4);

    if (amp > 0) return constant / amp;
    else return -1;
  }

  virtual double operator()(const double *x, const double *p)  override{
    return ZDCFermiExpFitRefl(x, p);
  }

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiVariableTausRun3 : public ZDCFitExpFermiVariableTaus
{
public:
  ZDCFitExpFermiVariableTausRun3(const std::string& tag, float tmin, float tmax, bool fixTau1, bool fixTau2, float tau1, float tau2);
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiFixedTaus : public ZDCFitWrapper
{
private:
  float m_tau1;
  float m_tau2;

  float m_norm;
  float m_timeCorr;

  std::shared_ptr<TF1> m_expFermiFunc = 0;

public:

  ZDCFitExpFermiFixedTaus(const std::string& tag, float tmin, float tmax, float tau1, float tau2);

  ~ZDCFitExpFermiFixedTaus() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    double amp = theTF1->GetParameter(0);
    double C = theTF1->GetParameter(2);

    return C / amp;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double amp = p[0];
    double t0 = p[1];
    double C = p[2];

    double deltaT = x[0] - t0;

    double expFermi =  amp * m_norm * m_expFermiFunc->operator()(deltaT);

    return expFermi + C; // + bckgd;
  }
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiPrePulse : public ZDCPrePulseFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  std::shared_ptr<TF1> m_expFermiFunc = 0;

public:
  ZDCFitExpFermiPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2);
  ~ZDCFitExpFermiPrePulse() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;


  virtual void SetInitialPrePulse(float amp, float t0, float /*expamp = 0*/, bool /*fixPrePulseToZero = false*/) override {
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 1.5)); //1.5 here ensures that we're above lower limit
    GetWrapperTF1()->SetParameter(3, t0);
  }

  virtual void SetPrePulseT0Range(float tmin, float tmax) override;
  virtual void SetPostPulseT0Range(float /*tmin*/, float /*tmax*/, float /*initialPostT0*/) override {return;}

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual unsigned int GetPreT0ParIndex() const override {return 3;}

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetPreT0()  const override {return 0;}
  virtual float GetPreAmp() const override {return 0;}

  virtual float GetPostT0()  const override {return 0;}
  virtual float GetPostAmp() const override {return 0;}

  virtual float GetExpAmp() const override {return 0;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index < 5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();

    double maxTime = GetTime();

    double amp = theTF1->GetParameter(0);
    if (amp <= 0) return -1;
    
    double preAmp = theTF1->GetParameter(2);
    double preT0 = theTF1->GetParameter(3);

    double deltaTPre = maxTime - preT0;
    double deltaPresamp = GetTMinAdjust() - preT0;

    double background = preAmp * m_norm * m_expFermiFunc->operator()(deltaTPre);

    return background / (amp + background);
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp = p[0];
    double t0 = p[1];
    double preAmp = p[2];
    double preT0 = p[3];
    double C = p[4];

    double deltaT = t - t0;
    double deltaTPre = t - preT0;

    // We subtract off the  value of the pre-pulse at the minimum time (nominally 0,
    //   but can change if we exclude early samples) to account for the subtraction of the pre-sample
    //
    double deltaPresamp = GetTMinAdjust() - preT0;

    // double bckgd = linSlope*t;

    double pulse1 =  amp * m_norm * m_expFermiFunc->operator()(deltaT);
    double pulse2 =  preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) -
                                        m_expFermiFunc->operator()(deltaPresamp));

    return C + pulse1 + pulse2;// + bckgd;
  }
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiPreExp : public ZDCPreExpFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  
  std::shared_ptr<TF1> m_expFermiFunc = 0;

public:
  ZDCFitExpFermiPreExp(const std::string& tag, float tmin, float tmax, float tau1, float tau2,
			   float defExpTau, float fixExpTau);
  ~ZDCFitExpFermiPreExp() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void SetInitialExpPulse(float amp) override
  {
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 0.5)); //0.5 here ensures that we're above lower limit (0)   
  }

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetExpAmp() const override {return GetWrapperTF1()->GetParameter(2);}
  virtual float GetExpTau() const override {return GetWrapperTF1()->GetParameter(3);}

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index < 5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    // const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    // double maxTime = GetTime();
    // double amp = theTF1->GetParameter(0);

    //    return background / amp;
    return 0;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp = p[0];
    double t0 = p[1];
    double expAmp = p[2];
    double expTau = p[3];
    double expSqrtTau = p[4];
    double C = p[5];

    double deltaT = t - t0;
    double pulse =  amp * m_norm * m_expFermiFunc->operator()(deltaT);

    // We subtract off the value of the exponential pulse at the minimum time (nominally 0),
    //   because it would have been included in the baseline subtraction
    //
    double tRef = GetTMinAdjust();
    double expPre = 0;
    if (t > 0 && std::abs(expSqrtTau)>1e-6) expPre = expAmp * (std::exp(-t/expTau-expSqrtTau*std::sqrt(t)) - std::exp(-tRef/expTau));
    else expPre = expAmp * (std::exp(-t/expTau) - std::exp(-tRef/expTau));
      
    return C + pulse + expPre;
  }
};

class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiLHCfPreExp : public ZDCPreExpFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  
  std::shared_ptr<TF1> m_expFermiLHCfFunc = 0;

public:
  ZDCFitExpFermiLHCfPreExp(const std::string& tag, float tmin, float tmax, float tau1, float tau2,
			   float defExpTau, float fixExpTau);
  ~ZDCFitExpFermiLHCfPreExp() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void SetInitialExpPulse(float amp) override
  {
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 0.5)); //0.5 here ensures that we're above lower limit (0)   
  }

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetExpAmp() const override {return GetWrapperTF1()->GetParameter(2);}
  virtual float GetExpTau() const override {return GetWrapperTF1()->GetParameter(3);}

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index < 5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    // const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    // double maxTime = GetTime();
    // double amp = theTF1->GetParameter(0);

    //    return background / amp;
    return 0;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp = p[0];
    double t0 = p[1];
    double expAmp = p[2];
    double expTau = p[3];
    double expSqrtTau = p[4];
    double reflFrac = p[5];
    double C = p[6];

    m_expFermiLHCfFunc->SetParameter(6, reflFrac);
      
    double deltaT = t - t0;
    double pulse =  amp * m_norm * m_expFermiLHCfFunc->operator()(deltaT);

    // We subtract off the value of the exponential pulse at the minimum time (nominally 0),
    //   because it would have been included in the baseline subtraction
    //
    double tRef = GetTMinAdjust();
    double expPre = 0;
    /* if (t > 0 && std::abs(expSqrtTau)>1e-6) expPre = expAmp * (std::exp(-t/expTau-expSqrtTau*std::sqrt(t)) - std::exp(-tRef/expTau)); */
    /* else expPre = expAmp * (std::exp(-t/expTau) - std::exp(-tRef/expTau)); */

    expPre = expAmp * (std::exp(-t/expTau-expSqrtTau*t*t) - std::exp(-tRef/expTau - -expSqrtTau*tRef*tRef));
      
    return C + pulse + expPre;
  }
};

// ----------------------------------------------------------------------
class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiLinearFixedTaus : public ZDCFitWrapper
{
private:
  float m_tau1;
  float m_tau2;

  float m_norm;
  float m_timeCorr;

  std::shared_ptr<TF1> m_expFermiFunc;

public:

  ZDCFitExpFermiLinearFixedTaus(const std::string& tag, float tmin, float tmax, float tau1, float tau2);

  ~ZDCFitExpFermiLinearFixedTaus() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();
    double amp = theTF1->GetParameter(0);
    double slope = theTF1->GetParameter(2);

    double background = slope * GetTime();
    return background / amp;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double amp    = p[0];
    double t0     = p[1];
    double deltaT = x[0] - t0;

    double bckgd = p[2] * x[0] + p[3];

    double expFermi =  amp * m_norm * m_expFermiFunc->operator()(deltaT);

    return expFermi + bckgd;
  }
};

// ----------------------------------------------------------------------
class ATLAS_NOT_THREAD_SAFE ZDCFitExpFermiLinearPrePulse : public ZDCPrePulseFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  std::shared_ptr<TF1> m_expFermiFunc;

public:
  ZDCFitExpFermiLinearPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2);
  ~ZDCFitExpFermiLinearPrePulse() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual void SetInitialPrePulse(float amp, float t0, float /*expamp = 0*/, bool /*fixPrePulseToZero = false*/) override {
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 1.5)); //1.5 here ensures that we're above lower limit
    GetWrapperTF1()->SetParameter(3, t0);
  }

  virtual void SetPrePulseT0Range(float tmin, float tmax) override;
  virtual void SetPostPulseT0Range(float /*tmin*/, float /*tmax*/, float /*initialPostT0*/) override {return;}

  unsigned int GetPreT0ParIndex() const override {return 3;}

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetPreT0()  const override {
    float fitPreT0 =  GetWrapperTF1()->GetParameter(3);

    // Correct the time to the maximum
    //
    fitPreT0 += m_timeCorr;

    return fitPreT0;
  }
  virtual float GetPreAmp() const override {return GetWrapperTF1()->GetParameter(2);}

  virtual float GetPostT0()  const override {return 0;}
  virtual float GetPostAmp() const override {return 0;}

  virtual float GetExpAmp() const override {return 0;}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index < 5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();

    double maxTime = GetTime();

    double amp = theTF1->GetParameter(0);
    double preAmp = theTF1->GetParameter(2);
    double preT0 = theTF1->GetParameter(3);
    double slope = theTF1->GetParameter(4);

    double deltaTPre = maxTime - preT0;

    double background = slope * maxTime + preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) -
                        m_expFermiFunc->operator()(-preT0));

    return background / amp;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp = p[0];
    double t0 = p[1];
    double preAmp = p[2];
    double preT0 = p[3];
    double linSlope = p[4];

    double deltaT = t - t0;
    double deltaTPre = t - preT0;

    // We subtract off the  value of the pre-pulse at the minimum time (nominally 0,
    //   but can change if we exclude early samples) to account for the subtraction of the pre-sample
    //
    double deltaPresamp = GetTMinAdjust() - preT0;

    double pulse1 =  amp * m_norm * m_expFermiFunc->operator()(deltaT);
    double pulse2 =  preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) -
                                        m_expFermiFunc->operator()(deltaPresamp));

    double bckgd = linSlope * (t - deltaPresamp) + p[5];

    return pulse1 + pulse2 + bckgd;
  }
};



// ----------------------------------------------------------------------
class ATLAS_NOT_THREAD_SAFE ZDCFitComplexPrePulse : public ZDCPrePulseFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  std::shared_ptr<TF1> m_expFermiFunc;

public:
  ZDCFitComplexPrePulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2);
  ~ZDCFitComplexPrePulse() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void SetInitialPrePulse(float amp, float t0, float expamp, bool /*fixPrePulseToZero = false*/) override {
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 1.5)); //1.5 here ensures that we're above lower limit
    GetWrapperTF1()->SetParameter(3, t0);
    GetWrapperTF1()->SetParameter(6, std::max(std::abs(expamp), (float) 1.5));
  }
  
  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual void SetPrePulseT0Range(float tmin, float tmax) override;
  virtual void SetPostPulseT0Range(float /*tmin*/, float /*tmax*/, float /*initialPostT0*/) override {return;}

  unsigned int GetPreT0ParIndex() const override {return 3;}

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError() const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetPreT0()  const override {
    float fitPreT0 =  GetWrapperTF1()->GetParameter(3);

    // Correct the time to the maximum
    //
    fitPreT0 += m_timeCorr;

    return fitPreT0;
  }
  virtual float GetPreAmp() const override {return GetWrapperTF1()->GetParameter(2);}

  virtual float GetPostT0()  const override {return 0;}
  virtual float GetPostAmp() const override {return 0;}

  virtual float GetExpAmp() const override {return GetWrapperTF1()->GetParameter(6);}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if      (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index <  5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();

    double maxTime = GetTime();

    double amp    = theTF1->GetParameter(0);
    double preAmp = theTF1->GetParameter(2);
    double preT0  = theTF1->GetParameter(3);
    double slope  = theTF1->GetParameter(4);

    double deltaTPre = maxTime - preT0;

    double background = slope * maxTime + preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) -
                        m_expFermiFunc->operator()(-preT0));

    return background / amp;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp      = p[0];
    double t0       = p[1];
    double preAmp   = p[2];
    double preT0    = p[3];
    double linSlope = p[4];
    double linConst = p[5];
    double expamp   = p[6];

    double deltaT    = t - t0;
    double deltaTPre = t - preT0;

    // We subtract off the  value of the pre-pulse at the minimum time (nominally 0,
    //   but can change if we exclude early samples) to account for the subtraction of the pre-sample
    //
    double deltaPresamp = GetTMinAdjust() - preT0;

    double pulse1 =  amp    * m_norm *  m_expFermiFunc->operator()(deltaT);
    double pulse2 =  preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) - m_expFermiFunc->operator()(deltaPresamp));

    double linBG = linSlope * (t - deltaPresamp) + linConst;
    double expBG = expamp * std::exp(-(t) / m_tau2) - expamp;  // deltaPresamp

    return pulse1 + pulse2 + linBG + expBG;
  }
};


// ----------------------------------------------------------------------
class ATLAS_NOT_THREAD_SAFE ZDCFitGeneralPulse : public ZDCPrePulseFitWrapper
{
private:
  float m_tau1;
  float m_tau2;
  float m_norm;
  float m_timeCorr;
  std::shared_ptr<TF1> m_expFermiFunc;

public:
  ZDCFitGeneralPulse(const std::string& tag, float tmin, float tmax, float tau1, float tau2);
  ~ZDCFitGeneralPulse() {}

  virtual void DoInitialize(float initialAmp, float initialT0, float ampMin, float ampMax) override;
  virtual void SetT0FitLimits(float tMin, float tMax) override;

  virtual void SetInitialPrePulse(float amp, float t0, float expamp, bool fixPrePulseToZero) override {
    GetWrapperTF1()->ReleaseParameter(2);
    GetWrapperTF1()->ReleaseParameter(3);
    GetWrapperTF1()->SetParameter(2, std::max(amp, (float) 1.5)); //1.5 here ensures that we're above lower limit
    GetWrapperTF1()->SetParameter(3, std::max(t0, (float) 20.0));
    GetWrapperTF1()->SetParameter(6, std::max(std::abs(expamp), (float) 1.5));

    if (fixPrePulseToZero) {
      GetWrapperTF1()->FixParameter(2, 0. );
      GetWrapperTF1()->FixParameter(3, 20.);
    }
  }

  virtual void ConstrainFit() override;
  virtual void UnconstrainFit() override;

  virtual void SetPrePulseT0Range(float tmin, float tmax) override;
  virtual void SetPostPulseT0Range(float tmin, float tmax, float initialPostT0) override;

  unsigned int GetPreT0ParIndex() const override {return 3;}

  virtual float GetAmplitude() const override {return GetWrapperTF1()->GetParameter(0); }
  virtual float GetAmpError()  const override {return GetWrapperTF1()->GetParError(0); }

  virtual float GetTau1() const override {return m_tau1;}
  virtual float GetTau2() const override {return m_tau2;}

  virtual float GetPreT0()  const override {
    float fitPreT0 =  GetWrapperTF1()->GetParameter(3);

    // Correct the time to the maximum
    //
    fitPreT0 += m_timeCorr;

    return fitPreT0;
  }
  virtual float GetPreAmp() const override {return GetWrapperTF1()->GetParameter(2);}

  virtual float GetPostT0()  const override {
    float fitPostT0 =  GetWrapperTF1()->GetParameter(8);

    // Correct the time to the maximum
    //
    fitPostT0 += m_timeCorr;

    return fitPostT0;
  }
  virtual float GetPostAmp() const override {return GetWrapperTF1()->GetParameter(7);}

  virtual float GetExpAmp() const override {return GetWrapperTF1()->GetParameter(6);}

  virtual float GetTime() const override {
    return GetWrapperTF1()->GetParameter(1) + m_timeCorr; // Correct the time to the maximum
  }

  virtual float GetShapeParameter(size_t index) const override
  {
    if      (index == 0) return m_tau1;
    else if (index == 1) return m_tau2;
    else if (index <  5) return GetWrapperTF1()->GetParameter(index);
    else throw std::runtime_error("Fit parameter does not exist.");
  }

  virtual float GetBkgdMaxFraction() const override
  {
    const TF1* theTF1 = ZDCFitWrapper::GetWrapperTF1();

    double maxTime = GetTime();

    double amp    = theTF1->GetParameter(0);
    double preAmp = theTF1->GetParameter(2);
    double preT0  = theTF1->GetParameter(3);
    double slope  = theTF1->GetParameter(4);

    double deltaTPre = maxTime - preT0;

    double background = slope * maxTime + preAmp * m_norm * (m_expFermiFunc->operator()(deltaTPre) -
                        m_expFermiFunc->operator()(-preT0));

    return background / amp;
  }

  virtual double operator() (const double *x, const double *p) override
  {
    double t = x[0];

    double amp      = p[0];
    double t0       = p[1];
    double preAmp   = p[2];
    double preT0    = p[3];
    double linSlope = p[4];
    double linConst = p[5];
    double expamp   = p[6];
    double postAmp  = p[7];
    double postT0   = p[8];

    double deltaT     = t - t0;
    double deltaTPre  = t - preT0;
    double deltaTPost = t - postT0;

    // We subtract off the  value of the pre-pulse at the minimum time (nominally 0,
    //   but can change if we exclude early samples) to account for the subtraction of the pre-sample
    //
    double deltaPresamp = GetTMinAdjust() - preT0;

    double pulse1 =  amp     * m_norm *  m_expFermiFunc->operator()(deltaT);
    double pulse2 =  preAmp  * m_norm * (m_expFermiFunc->operator()(deltaTPre) - m_expFermiFunc->operator()(deltaPresamp));
    double pulse3 =  postAmp * m_norm *  m_expFermiFunc->operator()(deltaTPost);

    double linBG = linSlope * t + linConst;
    double expBG = expamp * std::exp(-(t) / m_tau2) - expamp * std::exp(-(GetTMinAdjust()) / m_tau2);  // deltaPresamp

    return pulse1 + pulse2 + pulse3 + linBG + expBG;
  }
};
#endif
