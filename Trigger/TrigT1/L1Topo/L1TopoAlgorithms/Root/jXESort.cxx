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

   const jXETOBArray & mets = dynamic_cast<const jXETOBArray&>(input);
   int missingET = quadraticSumBW(mets[0].Ex(), mets[0].Ey());
   //clamp missingET value as in FW since GenericTOBs only use 13 bits for ET values
   //"sorted" missingET is thus far the only exception that can exceed this based on 
   //the corresponding input TOBs' possible value ranges
   missingET = std::clamp(missingET, 0, (1<<13) - 1 );
   
   int metphi = TSU::Trigo::atan2(mets[0].Ex(),mets[0].Ey());
    
   TRG_MSG_DEBUG("MET phi values " << metphi << " from x/y = " << mets[0].Ex() << "/" << mets[0].Ey() << "( ET = " << missingET << ")"  );
   output.push_back( GenericTOB( missingET, 0, metphi ) );

   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::jXESort::sort(const InputTOBArray & input, TOBArray & output) {
    //we have a bitCorrect version, so use it
    return this->sortBitCorrect(input, output);
    
}

