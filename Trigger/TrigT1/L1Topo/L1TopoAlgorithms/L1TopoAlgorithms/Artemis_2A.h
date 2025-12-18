/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  Artemis_2A.h
//  Created by Paula Martinez Suarez, Ralf Gugel, Sagar Addepalli on 09/12/2025.
//  {paula.martinez.suarez, ralf.gugel, addepalli.sagar}@CERNSPAMNOT.CH

#ifndef __TopoCore__ARTEMIS_2A__
#define __TopoCore__ARTEMIS_2A__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"

namespace TCS {
   
   class ARTEMIS_2A : public DecisionAlg {
   public:
      ARTEMIS_2A(const std::string & name);
      virtual ~ARTEMIS_2A();

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
      parType_t      p_maxEt1 = { 0 };
      parType_t      p_maxEt2 = { 0 };
      parType_t      p_maxEt3 = { 0 };
      parType_t      p_maxEt4 = { 0 };
      parType_t      p_AnomalyScoreThresh[2] = { 0, 0 };

      unsigned int   p_ScaleSqr_DropBits = 7;

   };
   
}

#endif
