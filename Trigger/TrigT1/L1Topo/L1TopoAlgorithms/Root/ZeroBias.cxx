/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * ZeroBias.cxx
 * Created by Marco Montella on 15/01/24.
 *
 * @brief ZeroBias topo algorithm, for now only an empty shell
 * 
 *
 * 
**********************************/

#include <cmath>

#include "L1TopoAlgorithms/ZeroBias.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Count.h"

#include "L1TopoEvent/TOBArray.h"


REGISTER_ALG_TCS(ZeroBias)

using namespace std;


TCS::ZeroBias::ZeroBias(const std::string & name) : CountingAlg(name)
{
   
   defineParameter("InputWidth", 0 );  
   defineParameter("NumMultiplicityBits", 0 );  
   
   defineParameter("DelayBC", 0);  
   defineParameter("Topo1Opt0Mask1", 0);    
   defineParameter("Topo1Opt0Mask2", 0);  
   defineParameter("Topo1Opt0Mask3", 0);  
   defineParameter("Topo1Opt1Mask1", 0);  
   defineParameter("Topo1Opt1Mask2", 0);  
   defineParameter("Topo1Opt1Mask3", 0);  

   
   setNumberOutputBits(12); 

}

TCS::ZeroBias::~ZeroBias(){}


TCS::StatusCode
TCS::ZeroBias::initialize() { 

  m_threshold = getThreshold();
  // book histograms

  return StatusCode::SUCCESS;
     
}

TCS::StatusCode TCS::ZeroBias::processBitCorrect(const TCS::InputTOBArray & input,
					 Count & count){
                        return process(input, count);
}


TCS::StatusCode TCS::ZeroBias::process( const TCS::InputTOBArray & input,
			       Count & count )
{
  const TOBArray & tobArray = dynamic_cast<const TOBArray&>(input);
  
  count.setSizeCount(tobArray.size());

  return TCS::StatusCode::SUCCESS;
}
