/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
// jXESort.cxx

#include "L1TopoAlgorithms/jXESort.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoEvent/jXETOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>
#include <cmath>

// Bitwise implementation utils
#include "L1TopoSimulationUtils/Trigo.h"

REGISTER_ALG_TCS(jXESort)


// constructor
TCS::jXESort::jXESort(const std::string & name) : SortingAlg(name) {

   defineParameter( "InputWidth", 2 ); // for FW
   defineParameter( "OutputWidth", 2 ); // for FW

}

// destructor
TCS::jXESort::~jXESort() {}


TCS::StatusCode
TCS::jXESort::initialize() {
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::jXESort::sortBitCorrect(const InputTOBArray & input, TOBArray & output) {

   if(input.size()!=1) {
      TCS_EXCEPTION("jXE sort alg expects exactly single jXE TOB, got " << input.size());
   }

   const jXETOBArray & jxes = dynamic_cast<const jXETOBArray&>(input);

   for(jXETOBArray::const_iterator jxe = jxes.begin(); jxe!= jxes.end(); ++jxe ) { 
     output.push_back( GenericTOB(**jxe) );
   }
	
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::jXESort::sort(const InputTOBArray & input, TOBArray & output) {

   if(input.size()!=1) {
      TCS_EXCEPTION("jXE sort alg expects exactly single jXE TOB, got " << input.size());
   }

   const jXETOBArray & jxes = dynamic_cast<const jXETOBArray&>(input);

   for(jXETOBArray::const_iterator jxe = jxes.begin(); jxe!= jxes.end(); ++jxe ) { 
     output.push_back( GenericTOB(**jxe) );
   }
	
   return TCS::StatusCode::SUCCESS;
}

