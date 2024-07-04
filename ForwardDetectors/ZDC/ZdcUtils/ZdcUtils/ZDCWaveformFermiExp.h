/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDCUTILS_ZDCWAVEFORMFERMIEXP_H
#define ZDCUTILS_ZDCWAVEFORMFERMIEXP_H

#include "ZDCWaveform.h"

//
// The Fermi*negative exponential that we have used since the start of Run 2 to
//   parameterize ZDC pulses
//
class ZDCWaveformFermiExp : public ZDCWaveformBase
{
protected:
  double doEvaluate(double time) const override;

  virtual std::string name() const override {return "FermiExp";}

public:
  ZDCWaveformFermiExp() = delete;

  ZDCWaveformFermiExp(const std::string& tag, double initialTauRise, double initialTauFall) :
    ZDCWaveformBase(tag, initialTauRise, initialTauFall, std::vector<std::string>(), std::vector<double>())
  {}

  explicit ZDCWaveformFermiExp(const ZDCWaveformFermiExp& instance) :
    ZDCWaveformBase(instance.getTag(), instance.getTauRise(),instance.getTauFall(), std::vector<std::string>(), std::vector<double>()) {};
};

//
// A version of the Fermi*negative exponential that has a shift in baseline before and after T0
//   to describe a small (few %) long tail seen in pulses in the Run 3 data
//
//
class ZDCWaveformFermiExpTail : public ZDCWaveformFermiExp
{
  double doEvaluate(double time) const override
  {
    double tailShift = getAddtlShapeValue(0);
    if (time <= 0) return ZDCWaveformFermiExp::doEvaluate(time);
    else return ZDCWaveformFermiExp::doEvaluate(time) + tailShift;
  }
  
  virtual std::string name() const override {return "FermiExpTail";}

public:
  ZDCWaveformFermiExpTail() = delete;

  ZDCWaveformFermiExpTail(const std::string& tag, double initialTauRise, double initialTauFall, double tailShiftFrac) :
    ZDCWaveformFermiExp(tag, initialTauRise, initialTauFall)
  {
    double tailShiftScaled = tailShiftFrac*ZDCWaveformFermiExp::doEvaluate(0);
    setAddtlShapeParameters(std::vector<std::string>({"tailFrac"}), std::vector<double>({tailShiftScaled}));
  }

  void setTailParameter(double tailShiftFrac) {
    double tailShiftScaled = tailShiftFrac*ZDCWaveformFermiExp::doEvaluate(0);
    setAddtlShapeValue(0, tailShiftScaled);
  }
  
};
#endif
