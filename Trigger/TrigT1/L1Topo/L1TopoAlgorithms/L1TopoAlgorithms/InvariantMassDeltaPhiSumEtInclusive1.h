/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  InvariantMassDeltaPhiSumEtInclusive1.h
//  TopoCore
//  Based on InvariantMassDeltaPhiInclusive2 by Joerg Stelzer on 19/02/2019. For questions contact atlas-trig-l1topo-algcom@cern.ch. 

#ifndef __TopoCore__InvariantMassDeltaPhiSumEtInclusive1__
#define __TopoCore__InvariantMassDeltaPhiSumEtInclusive1__

#include "L1TopoInterfaces/DecisionAlg.h"

class TH2;

namespace TCS {
   
   class InvariantMassDeltaPhiSumEtInclusive1 : public DecisionAlg {
   public:
      InvariantMassDeltaPhiSumEtInclusive1(const std::string & name);
      virtual ~InvariantMassDeltaPhiSumEtInclusive1();

      virtual StatusCode initialize() override final;
 
      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override final;

      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison ) override final;
      

   private:

      parType_t      p_NumberLeading = { 0 };
      parType_t      p_InvMassMin[6] = { 0,0,0,0,0,0 };
      parType_t      p_InvMassMax[6] = { 0,0,0,0,0,0 };
      parType_t      p_SumEtMin[6] = { 0,0,0,0,0,0 };
      parType_t      p_SumEtMax[6] = { 0,0,0,0,0,0 };
      parType_t      p_MinET1[6] = { 0,0,0,0,0,0 };
      parType_t      p_MinET2[6] = { 0,0,0,0,0,0 };
      parType_t      p_ApplyEtaCut = { 0 };
      parType_t      p_MinEta1 = { 0 };
      parType_t      p_MaxEta1 = { 0 };
      parType_t      p_MinEta2 = { 0 };
      parType_t      p_MaxEta2 = { 0 };
      parType_t      p_DeltaPhiMin[6] = { 0,0,0,0,0,0 };
      parType_t      p_DeltaPhiMax[6] = { 0,0,0,0,0,0 };

   };
   
}

#endif
