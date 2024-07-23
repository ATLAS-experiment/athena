/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
// jTauSelect.cxx
// TopoCore
//  Algorithm to select the abbreviated list of jTaus , no order is applied
//

#include "L1TopoAlgorithms/jTauSelect.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jTauTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jTauSelect)

// constructor
TCS::jTauSelect::jTauSelect(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120); // for fw
   defineParameter( "OutputWidth", 6);    
   defineParameter( "MinET", 0 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
   defineParameter( "Isolation", 1024);
   defineParameter( "passIsolation", false);
}


// destructor
TCS::jTauSelect::~jTauSelect() {}

TCS::StatusCode
TCS::jTauSelect::initialize() {

   m_numberOfjTaus = parameter("OutputWidth").value();
   m_minET         = parameter("MinET").value();
   m_minEta        = parameter("MinEta").value();
   m_maxEta        = parameter("MaxEta").value();
   m_iso           = parameter("Isolation").value();
   m_passIsolation = parameter("passIsolation").value();

   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::jTauSelect::sort(const InputTOBArray & input, TOBArray & output) {

   const jTauTOBArray & clusters = dynamic_cast<const jTauTOBArray&>(input);

   // fill output array with GenericTOB built from clusters
   for(jTauTOBArray::const_iterator jtau = clusters.begin(); jtau!= clusters.end(); ++jtau ) {
     if ( parType_t((*jtau)-> Et())  < m_minET  ) continue; 
     if ( parType_t(std::abs((*jtau)-> eta())) < m_minEta ) continue; 
     if ( parType_t(std::abs((*jtau)-> eta())) > m_maxEta ) continue;      	
     if (! ( m_passIsolation || checkIsolation(*jtau)  )  ) continue;

      const GenericTOB gtob(**jtau);
      output.push_back( gtob );
   }


   // keep only max number of jTaus
   int par = m_numberOfjTaus ;
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
TCS::jTauSelect::checkIsolation(const TCS::jTauTOB* jtau) const {
  if(m_passIsolation) return true;
  return jtau->EtIso()*1024 < jtau->Et()*m_iso;
}

