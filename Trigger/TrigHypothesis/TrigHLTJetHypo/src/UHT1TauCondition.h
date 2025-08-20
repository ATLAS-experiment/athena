/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_UHT1TAUCONDITION_H
#define TRIGHLTJETHYPO_UHT1TAUCONDITION_H

/********************************************************************
 *
 * NAME:     UHT1TauCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   C. Pollard
 *********************************************************************/

#include <string>
#include "./ICondition.h"
#include "AsgTools/AsgTool.h"
#include "xAODJet/JetContainer.h"
namespace HypoJet{
  class IJet;
}

class ITrigJetHypoInfoCollector;

class UHT1TauCondition: public ICondition{
 public:
   UHT1TauCondition(float workingPoint,
                 const std::string &decName_ptau,
                 const std::string &decName_pu,
                 const std::string &decName_isValid = "");

   float getUHT1TauDecValue(const pHypoJet &ip,
                         const std::unique_ptr<ITrigJetHypoInfoCollector> &collector,
                         const std::string &decName) const;

   float evaluateUHT1Tau(const float &uht1tau_ptau,
                      const float &uht1tau_pu) const;

   bool isSatisfied(const HypoJetVector &,
                    const std::unique_ptr<ITrigJetHypoInfoCollector> &) const override;

   virtual unsigned int capacity() const override { return s_capacity; }

   std::string toString() const override;

 private:
   float      m_workingPoint;
   std::string m_decName_ptau;
   std::string m_decName_pu;
   std::string m_decName_isValid;

   bool isSatisfied(const pHypoJet &,
                    const std::unique_ptr<ITrigJetHypoInfoCollector> &) const;

   const static unsigned int s_capacity{1};

};

#endif
