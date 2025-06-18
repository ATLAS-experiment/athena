/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/
//  TeAsymmetry.h
//  TopoCore
//  Based on the JIRA ticket: https://its.cern.ch/jira/browse/ATR-31097

#ifndef __TopoCore__TeAsymmetry__
#define __TopoCore__TeAsymmetry__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"
#include "L1TopoEvent/TOBArray.h"

class TH2;

namespace TCS {
   
   class TeAsymmetry : public DecisionAlg {
   public:
      TeAsymmetry(const std::string & name);
      virtual ~TeAsymmetry();

      virtual StatusCode initialize();

      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );

      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );
      

   private:

      parType_t      p_MinSidejTE = { 0 };
      parType_t      p_MaxSidejTE = { 0 };
      parType_t      p_deltaAbsMin[4] = {0, 0, 0, 0};
      parType_t      p_asymFactor[4] = { 0, 0, 0, 0 };
      parType_t      p_asymOffset[4] = { 0, 0, 0, 0 };
      parType_t      p_maxTeProduct[4] = { 0, 0, 0, 0 };
     
   };
   
}

#endif
