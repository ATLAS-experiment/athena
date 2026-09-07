/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_ACCEPTALL_H
#define TRIGHLTJETHYPO_ACCEPTALL_H

/********************************************************************
 *
 * NAME:     AcceptAllCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   P. Sherwood
 *********************************************************************/


#include "./ICondition.h"
#include <string>
#include <memory>

namespace HypoJet{
  class IJet;
}

class ITrigJetHypoInfoCollector;

class AcceptAllCondition: public ICondition{
 public:
  ~AcceptAllCondition() override {}

  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  virtual unsigned int capacity() const override{return 0;}
  std::string toString() const override;
  
 private:
  
};

#endif
