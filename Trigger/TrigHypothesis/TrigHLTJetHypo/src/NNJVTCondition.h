/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_NNJVTCONDITION_H
#define TRIGHLTJETHYPO_NNJVTCONDITION_H

/********************************************************************
 *
 * NAME:     NNJVTCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR: Teng Jian Khoo
 *           
 *********************************************************************/

#include "./ICondition.h"
#include "AthContainers/ConstAccessor.h"
#include <string>
#include<memory>

class ITrigJetHypoInfoCollector;

class NNJVTCondition: public ICondition{
 public:
  NNJVTCondition(const std::string& nnjvt_name);

  bool isSatisfied(const HypoJetVector&, const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;
  
  std::string toString() const override;

  virtual unsigned int capacity() const override{return s_capacity;}

 private:

  std::string m_nnjvt_name;
  
  std::unique_ptr<SG::ConstAccessor<float> > m_nnjvt_acc;
  std::unique_ptr<SG::ConstAccessor<char> > m_nnjvtpass_acc;
  
  const static unsigned int s_capacity{1};

};

#endif
