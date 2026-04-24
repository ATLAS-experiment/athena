/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef EFLOWREC_EFLOWEETABINBASE_H
#define EFLOWREC_EFLOWEETABINBASE_H

/********************************************************************

NAME:     eflowEEtaBinBase.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHOR:   R Duxfield
CREATED:  17th May, 2006

********************************************************************/

#include <vector>

class eflowParameters;

/**
Base class which sets up some of the infrastructure to store the e/p reference values. The base class has methods to return bin indices, and has a bool to toggle whether to use linear or log interpolation between bins (such that the e/p reference can smoothly vary as a function of kinematics).
*/
class eflowEEtaBinBase {
 public:

  eflowEEtaBinBase() = default;
  virtual ~eflowEEtaBinBase();
  
  bool binExists(double e, double eta) const  { return (getEBinIndex(e) >= 0 && getEtaBinIndex(eta) >= 0); }

  static double getErrorReturnValue()  {return m_errorReturnValue;}

 protected:

  int getNumEBins() const { return m_eBinBounds.size(); }
  int getNumEtaBins() const { return m_etaBinBounds.size(); }

  int getEBinIndexLegacy(double e) const;
  int getEBinIndex(double e) const;
  int getEtaBinIndex(double eta) const;
  static int getBinIndex(double x, const std::vector<double>& binBounds) ;

  static const double m_errorReturnValue;

  bool m_useAbsEta{true};
  std::vector<double> m_eBinBounds;
  std::vector<double> m_etaBinBounds;
};
#endif
