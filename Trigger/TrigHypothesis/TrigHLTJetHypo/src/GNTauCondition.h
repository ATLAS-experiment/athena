/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_GNTAUCONDITION_H
#define TRIGHLTJETHYPO_GNTAUCONDITION_H

/********************************************************************
 *
 * NAME:     GNTauCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   C. Pollard, M. Valente
 *********************************************************************/

#include <string>
#include "./ICondition.h"
#include "AsgTools/AsgTool.h"
#include "xAODJet/JetContainer.h"
namespace HypoJet{
  class IJet;
}

class ITrigJetHypoInfoCollector;

class GNTauCondition: public ICondition{
 public:
   GNTauCondition(double workingPoint,
                 const std::string &decName_ptau,
                 const std::string &decName_pu,
                 const std::string &decName_isValid = "");

   float getGNTauDecValue(const pHypoJet &ip,
                         const std::unique_ptr<ITrigJetHypoInfoCollector> &collector,
                         const std::string &decName) const;

   float evaluateGNTau(const float &gntau_ptau,
                      const float &gntau_pu) const;

   bool isSatisfied(const HypoJetVector &,
                    const std::unique_ptr<ITrigJetHypoInfoCollector> &) const override;

   virtual unsigned int capacity() const override { return s_capacity; }

   std::string toString() const override;

 private:
   double      m_workingPoint;
   std::string m_decName_ptau;
   std::string m_decName_pu;
   std::string m_decName_isValid;

   bool isSatisfied(const pHypoJet &,
                    const std::unique_ptr<ITrigJetHypoInfoCollector> &) const;

   const static unsigned int s_capacity{1};

};

#endif
