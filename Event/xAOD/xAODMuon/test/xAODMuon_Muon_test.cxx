/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"

template< typename T >
std::ostream& operator<< ( std::ostream& out,
                           const std::vector< T >& vec ) {

   out << "[";
   for( size_t i = 0; i < vec.size(); ++i ) {
      out << vec[ i ];
      if( i < vec.size() - 1 ) {
         out << ", ";
      }
   }
   out << "]";
   return out;
}

/// Function filling one Muon with information
void fill( xAOD::Muon& muon ) {

   muon.setP4( 1.0, 2.0, 3.0 );
   muon.setAuthor(xAOD::Muon::Author::MuidCo);
   muon.setAllAuthors(0x10);
   muon.setMuonType(xAOD::Muon::MuonType::Combined);
   float value=1.0;
   muon.setParameter(value, xAOD::Muon::ParamDef::spectrometerFieldIntegral);
   muon.setParameter(value, xAOD::Muon::ParamDef::momentumBalanceSignificance);
   muon.setQuality(xAOD::Muon::Quality::Medium);
   
   muon.setParameter(1, xAOD::Muon::ParamDef::msInnerMatchDOF);
   
   return;
}

/// Function printing the properties of a TrackParticle
void print( const xAOD::Muon& muon ) {

   std::cout << "pt = " << muon.pt() << ", eta = " << muon.eta()
             << ", phi = " << muon.phi() << ", m = " << muon.m()<< ", e = " << muon.e()<<std::endl;
   

   return;
}

int main() {

   // Create the main containers to test:
   xAOD::MuonAuxContainer aux;
   xAOD::MuonContainer tpc;
   tpc.setStore( &aux );

   // Add one track particle to the container:
   xAOD::Muon* p = new xAOD::Muon();
   tpc.push_back( p );

   // Fill it with information:
   fill( *p );

   // Print the information:
   print( *p );

   // // Print the contents of the auxiliary store:
   // aux.dump();

   // Return gracefully:
   return 0;
}
