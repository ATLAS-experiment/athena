/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// 
// 
//  Created by Marco Montella on 15/01/2024

#ifndef __TopoCore__ZeroBias__
#define __TopoCore__ZeroBias__
#include <iostream>
#include <vector>
#include "L1TopoInterfaces/CountingAlg.h"
#include "L1TopoEvent/TOBArray.h"

#include "TrigConfData/L1Threshold.h"

class TH2;

namespace TCS { 
  
   class ZeroBias: public CountingAlg {
   public:
      ZeroBias(const std::string & name);
      virtual ~ZeroBias();

      virtual StatusCode initialize() override;

      virtual StatusCode processBitCorrect( const TCS::InputTOBArray & input,
					    Count & count ) override final ;

      virtual StatusCode process( const TCS::InputTOBArray & input,
				  Count & count ) override final ;

     
    
   private:
     
      TrigConf::L1Threshold const * m_threshold{nullptr};

   };

}

#endif
