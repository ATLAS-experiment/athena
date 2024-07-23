/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  jTauSort.cxx
//  TopoCore
//  algorithm to make sorted jTaus lists
//
#include "L1TopoAlgorithms/jTauSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jTauTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include "L1TopoAlgorithms/jTauMultiplicity.h"
#include <algorithm>

REGISTER_ALG_TCS(jTauSort)

bool SortByEtLargestjTau(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
   return tob1->Et() > tob2->Et();
}


// constructor
TCS::jTauSort::jTauSort(const std::string & name) :
   SortingAlg(name)
{
   defineParameter( "InputWidth", 120); // for fw
   defineParameter( "OutputWidth", 6 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
   defineParameter( "Isolation", 1024);
   defineParameter( "passIsolation", false);
}


TCS::jTauSort::~jTauSort()
{}


TCS::StatusCode
TCS::jTauSort::initialize() {

   m_numberOfjTaus = parameter("OutputWidth").value();
   m_minEta        = parameter("MinEta").value();
   m_maxEta        = parameter("MaxEta").value();
   m_iso           = parameter("Isolation").value();
   m_passIsolation = parameter("passIsolation").value();

   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::jTauSort::sort(const InputTOBArray & input, TOBArray & output) {
  
   const jTauTOBArray & jtaus = dynamic_cast<const jTauTOBArray&>(input);
  
   // loop over input TOBs && fill output array with GenericTOBs builds from jTaus
   for(jTauTOBArray::const_iterator jtau = jtaus.begin(); jtau != jtaus.end(); ++jtau ) {
     if ( parType_t(std::abs((*jtau)-> eta())) < m_minEta ) continue; 
     if ( parType_t(std::abs((*jtau)-> eta())) > m_maxEta ) continue;    
     if (! ( m_passIsolation || checkIsolation(*jtau)  )  ) continue;
     output.push_back( GenericTOB(**jtau)  );
   }

   // sort
   output.sort(SortByEtLargestjTau);
   
   // keep only max number of jTaus
   int par = m_numberOfjTaus;
   unsigned int maxNumberOfJTaus = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfJTaus>0) {
      while( output.size()> maxNumberOfJTaus ) {
         if (output.size() == (maxNumberOfJTaus+1)) {
            bool isAmbiguous = output[maxNumberOfJTaus-1].Et() == output[maxNumberOfJTaus].Et();
            if (isAmbiguous) { output.setAmbiguityFlag(true); }
         }
         output.pop_back();
      }
   }   
   return TCS::StatusCode::SUCCESS;
}


bool
TCS::jTauSort::checkIsolation(const TCS::jTauTOB* jtau) const {
  if(m_passIsolation) return true;
  return jtau->EtIso()*1024 < jtau->Et()*m_iso;
}




