/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  TeATIME.h
//  TopoCore
//  Based on the JIRA ticket: https://its.cern.ch/jira/browse/ATR-31097

#ifndef __TopoCore__TeATIME__
#define __TopoCore__TeATIME__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"

class TH2;

namespace TCS {
   
   class TeATIME : public DecisionAlg {
   public:
      TeATIME(const std::string & name);
      virtual ~TeATIME() = default;

      virtual StatusCode initialize() override;

      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override;

      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override;
      
   };
   
}

#endif
