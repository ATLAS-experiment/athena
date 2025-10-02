/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// jTENoSort.cxx

#include "L1TopoAlgorithms/jTENoSort.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jTETOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jTENoSort)


// constructor
TCS::jTENoSort::jTENoSort(const std::string & name) : SortingAlg(name) {

   defineParameter( "InputWidth", 1 ); // for FW
   defineParameter( "OutputWidth", 1 ); // for FW
   defineParameter( "NumRegisters", 1); // for FW

}

// destructor
TCS::jTENoSort::~jTENoSort() {}

TCS::StatusCode
TCS::jTENoSort::initialize() {
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::jTENoSort::sort(const InputTOBArray & input, TOBArray & output) {

   if(input.size()!=1) {
     TCS_EXCEPTION("jTE sort alg expects exactly single jTE TOB, got " << input.size());
   }

   const jTETOBArray & jtes = dynamic_cast<const jTETOBArray&>(input);

   for(jTETOBArray::const_iterator jte = jtes.begin(); jte!= jtes.end(); ++jte ) { 
     output.push_back( GenericTOB(**jte) );
   }
	
   return TCS::StatusCode::SUCCESS;

}
