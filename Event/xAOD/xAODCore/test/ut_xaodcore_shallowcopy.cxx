/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>
#include <cmath>

// EDM include(s):
#ifdef XAOD_STANDALONE
#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
#define private public
#define protected public
#   include "AthLinks/DataLink.h"
#undef protected
#undef private
#ifdef __clang__
# pragma clang diagnostic pop
#endif
#else
# include "AthLinks/DataLink.h"
#endif
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"

// Local include(s):
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/ShallowAuxContainer.h"

/// Helper macro for evaluating logical tests
#define SIMPLE_ASSERT( EXP )                                                 \
   do {                                                                      \
      const bool result = EXP;                                               \
      if( ! result ) {                                                       \
         std::cerr << "Expression \"" << #EXP << "\" failed the evaluation"  \
                   << std::endl;                                             \
         return 1;                                                           \
      }                                                                      \
   } while( 0 )


int testCopy (const DataVector<SG::AuxElement>& origVec,
               xAOD::ShallowAuxContainer& copyAux)
{
   DataVector< SG::AuxElement > copyVec;
   for( size_t i = 0; i < origVec.size(); ++i ) {
      copyVec.push_back( new SG::AuxElement() );
   }
   copyVec.setStore( &copyAux );

   // Some starting tests:
   copyAux.setShallowIO( true );
   SIMPLE_ASSERT( copyAux.getAuxIDs().size() == 4 );
   SIMPLE_ASSERT( copyAux.getDynamicAuxIDs().empty() );
   SIMPLE_ASSERT( copyAux.getSelectedAuxIDs().empty() );
   copyAux.setShallowIO( false );
   SIMPLE_ASSERT( copyAux.getAuxIDs().size() == 4 );
   SIMPLE_ASSERT( copyAux.getDynamicAuxIDs().size() == 4 );
   SIMPLE_ASSERT( copyAux.getSelectedAuxIDs().size() == 4 );

   int index = 0;
   SG::ConstAccessor< int > IntVarConst( "IntVar" );
   SG::ConstAccessor< int > Int2VarConst( "Int2Var" );
   SG::ConstAccessor< int > Int3VarConst( "Int3Var" );
   SG::ConstAccessor< float > FloatVarConst( "FloatVar" );
   SG::ConstAccessor< double > DoubleVarConst( "DoubleVar" );
   for( const SG::AuxElement* el : copyVec ) {
      SIMPLE_ASSERT( IntVarConst( *el ) == index );
      SIMPLE_ASSERT( Int2VarConst( *el ) == index );
      SIMPLE_ASSERT( Int3VarConst( *el ) == index );
      SIMPLE_ASSERT( std::abs( FloatVarConst( *el ) -
                               static_cast< float >( index + 1 ) ) < 0.0001 );
      ++index;
   }

   // Create some modifications
   SG::Accessor< int > IntVar( "IntVar" );
   SG::Accessor< int > Int2Var( "Int2Var" );
   SG::Accessor< double > DoubleVar( "DoubleVar" );
   for( size_t i = 0; i < copyVec.size(); ++i ) {
      IntVar( *copyVec[ i ] ) = i + 2;
      DoubleVar( *copyVec[ i ] ) = 3.14;
   }
   Int2Var( *copyVec.front() ) = 5;
   SG::Decorator< int > Int3Decor( "Int3Var" );
   Int3Decor( *copyVec.front() ) = 6;

   // Check what happened:
   copyAux.setShallowIO( true );
   SIMPLE_ASSERT( copyAux.getAuxIDs().size() == 5 );
   SIMPLE_ASSERT( copyAux.getDynamicAuxIDs().size() == 4 );
   SIMPLE_ASSERT( copyAux.getSelectedAuxIDs().size() == 4 );
   copyAux.setShallowIO( false );
   SIMPLE_ASSERT( copyAux.getAuxIDs().size() == 5 );
   SIMPLE_ASSERT( copyAux.getDynamicAuxIDs().size() == 5 );
   SIMPLE_ASSERT( copyAux.getSelectedAuxIDs().size() == 5 );

   index = 0;
   for( const SG::AuxElement* el : copyVec ) {
      SIMPLE_ASSERT( IntVarConst( *el ) == index + 2 );
      SIMPLE_ASSERT( std::abs( FloatVarConst( *el ) -
                               static_cast< float >( index + 1 ) ) < 0.0001 );
      SIMPLE_ASSERT( std::abs( DoubleVarConst( *el ) -
                               3.14 ) < 0.0001 );
      if( index > 0 ) {
         SIMPLE_ASSERT( Int2VarConst( *el ) == index );
         SIMPLE_ASSERT( Int3VarConst( *el ) == index );
      }
      ++index;
   }
   SIMPLE_ASSERT( Int2VarConst( *copyVec.front() ) == 5 );
   SIMPLE_ASSERT( Int3VarConst( *copyVec.front() ) == 6 );

   // Finally, test variable filtering:
   xAOD::AuxSelection sel;
   sel.selectAux (std::set< std::string >( { "FloatVar", "DoubleVar" } ) );
   copyAux.setShallowIO( true );
   SIMPLE_ASSERT( sel.getSelectedAuxIDs (copyAux.getSelectedAuxIDs()).size() == 1 );
   copyAux.setShallowIO( false );
   SIMPLE_ASSERT( sel.getSelectedAuxIDs (copyAux.getSelectedAuxIDs()).size() == 2 );

   return 0;
}


int main() {

   // Create a test container that we'll make a copy of later on:
   xAOD::AuxContainerBase origAux;
   DataVector< SG::AuxElement > origVec;
   origVec.setStore( &origAux );
   SG::Accessor< int > IntVar( "IntVar" );
   SG::Accessor< int > Int2Var( "Int2Var" );
   SG::Accessor< int > Int3Var( "Int3Var" );
   SG::Accessor< float > FloatVar( "FloatVar" );
   for( int i = 0; i < 10; ++i ) {
      SG::AuxElement* e = new SG::AuxElement();
      origVec.push_back( e );
      IntVar( *e ) = i;
      Int2Var( *e ) = i;
      Int3Var( *e ) = i;
      FloatVar( *e ) = i + 1;
   }

#ifdef XAOD_STANDALONE
   DataLink< SG::IConstAuxStore > link;
   link.m_object = &origAux;
#else
   DataLink< SG::IConstAuxStore > link (&origAux);
#endif

   // Make a shallow copy of it:
   {
     xAOD::ShallowAuxContainer copyAux;
     copyAux.setParent( link );
     if (testCopy (origVec, copyAux))
       return 1;
   }

   {
     xAOD::ShallowAuxContainer copyAux (link);
     if (testCopy (origVec, copyAux))
       return 1;
     xAOD::ShallowAuxContainer copyAux2 (copyAux);
     SIMPLE_ASSERT( copyAux2.getAuxIDs().size() == 5 );
   }

   // Tell the user that everything went okay:
   std::cout << "All tests with xAOD::ShallowAuxContainer succeeded"
             << std::endl;

   // Return gracefully:
   return 0;
}
