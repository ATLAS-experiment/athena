/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//  InvariantMassInclusive1Disambiguation2.h
//  TopoCore
//  Created by Gabriel Oliveira Correa on 10/2025.

#ifndef __TopoCore__InvariantMassInclusive1Disambiguation2__
#define __TopoCore__InvariantMassInclusive1Disambiguation2__

#include <iostream>
#include "L1TopoInterfaces/DecisionAlg.h"

namespace TCS {
   
   class InvariantMassInclusive1Disambiguation2 : public DecisionAlg {
   public:
      InvariantMassInclusive1Disambiguation2(const std::string & name);
      virtual ~InvariantMassInclusive1Disambiguation2();

      virtual StatusCode initialize();

      virtual StatusCode processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );
      
      virtual StatusCode process( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decison );
      

   private:

      parType_t      p_NumberLeading1a = { 0 };
      parType_t      p_NumberLeading1b = { 0 };
      parType_t      p_NumberLeading2 = { 0 };
      parType_t      p_InvMassMin[6] = {0,0,0,0,0,0};
      parType_t      p_InvMassMax[6] = {0,0,0,0,0,0};
      parType_t      p_MinET1a[6] = {0,0,0,0,0,0};
      parType_t      p_MinET1b[6] = {0,0,0,0,0,0};
      parType_t      p_MinET2[6] = {0,0,0,0,0,0};
      parType_t      p_DisambDR[6] = {0,0,0,0,0,0};

   };
   
}

#endif
