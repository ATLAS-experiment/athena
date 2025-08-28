/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  AnomDetVAE.h
//  Created by Sagar Addepalli on 25/11/2024.
//  addepalli.sagar@CERNSPAMNOT.CH

#ifndef __TopoCore__ADVAE_2A__
#define __TopoCore__ADVAE_2A__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"

namespace TCS {
   
   class ADVAE_2A : public DecisionAlg {
   public:
      ADVAE_2A(const std::string & name);
      virtual ~ADVAE_2A();

      virtual StatusCode initialize();

      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );
      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );


   private:

      parType_t      p_NumberLeading1 = { 0 };
      parType_t      p_NumberLeading2 = { 0 };
      parType_t      p_NumberLeading3 = { 0 };
      parType_t      p_NumberLeading4 = { 0 };
      parType_t      p_minEt1 = { 0 };
      parType_t      p_minEt2 = { 0 };
      parType_t      p_minEt3 = { 0 };
      parType_t      p_minEt4 = { 0 };
      parType_t      p_ScaleSqr1[2] = { 0, 0 };
      parType_t      p_ScaleSqr2[2] = { 0, 0 };
      parType_t      p_ScaleSqr3[2] = { 0, 0 };
      parType_t      p_AnomalyScoreThresh[2] = { 0, 0 };

      unsigned int   p_ScaleSqr_DropBits = 7;

   };
   
}

#endif
