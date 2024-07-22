/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_MULTIPLICITYCONDITION_H
#define TRIGHLTJETHYPO_MULTIPLICITYCONDITION_H

/********************************************************************
 *
 * NAME:     MultiplicityCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   P. Sherwood
 *********************************************************************/

#include "./ICondition.h"

#include <string>


namespace HypoJet{
  class IJet;
}


class ITrigJetHypoInfoCollector;

class MultiplicityCondition: public ICondition{
 public:

  
  MultiplicityCondition(std::size_t multMin, std::size_t multMax);

  
  ~MultiplicityCondition() override {}

  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  std::string toString() const override;
  virtual unsigned int capacity() const override {return s_capacity;}

 private:

  // test is min<= mult < max, so max = last accepted value+ 1
  std::size_t m_multMin;
  std::size_t m_multMax;   

  // number of jets unspecified - signalled by 0.
  const static  unsigned int s_capacity{0};
  
  
};

#endif
