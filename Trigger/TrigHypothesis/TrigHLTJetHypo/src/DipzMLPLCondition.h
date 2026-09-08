/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_DipzMLPLCONDITION_H
#define TRIGHLTJETHYPO_DipzMLPLCONDITION_H

/********************************************************************
 *
 * NAME:     DipzMLPLCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   I. Ochoa
 *********************************************************************/

#include "./ICondition.h"
#include "./DipzLikelihoodCmp.h"

#include <string>


namespace HypoJet{
  class IJet;
}


class ITrigJetHypoInfoCollector;

class DipzMLPLCondition: public ICondition{
 public:
  DipzMLPLCondition(double wp, 
                unsigned int capacity, // this is the number of jets to be considered in each combination 
                const std::string &decName_z, 
                const std::string &decName_sigma,
                bool sigmaIsStdDev = false);

  ~DipzMLPLCondition() override {}

  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  std::string toString() const override;
  virtual unsigned int capacity() const override {return m_capacity;}

 private:
  
  double m_workingPoint;
  const unsigned int m_capacity;
  DipzLikelihood m_likelihoodCalculator;           
  
};

#endif
