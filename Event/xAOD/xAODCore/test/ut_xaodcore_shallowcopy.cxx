/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/AuxInfoBase.h"
#include "xAODCore/ShallowCopy.h"

// EDM include(s)
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"

// System include(s).
#include <iostream>
#include <cmath>
#include <type_traits>

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


int testCopy (DataVector<SG::AuxElement>& copyVec,
              xAOD::ShallowAuxContainer& copyAux)
{
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

int testCopy(SG::AuxElement& copyObj, xAOD::ShallowAuxInfo& copyAux) {

  // Some starting tests.
  copyAux.setShallowIO(true);
  SIMPLE_ASSERT(copyAux.getAuxIDs().size() == 4);
  SIMPLE_ASSERT(copyAux.getDynamicAuxIDs().empty());
  SIMPLE_ASSERT(copyAux.getSelectedAuxIDs().empty());
  copyAux.setShallowIO(false);
  SIMPLE_ASSERT(copyAux.getAuxIDs().size() == 4);
  SIMPLE_ASSERT(copyAux.getDynamicAuxIDs().size() == 4);
  SIMPLE_ASSERT(copyAux.getSelectedAuxIDs().size() == 4);

  SG::ConstAccessor<int> IntVarConst("IntVar");
  SG::ConstAccessor<int> Int2VarConst("Int2Var");
  SG::ConstAccessor<int> Int3VarConst("Int3Var");
  SG::ConstAccessor<float> FloatVarConst("FloatVar");
  SG::ConstAccessor<double> DoubleVarConst("DoubleVar");
  SIMPLE_ASSERT(IntVarConst(copyObj) == 1);
  SIMPLE_ASSERT(Int2VarConst(copyObj) == 2);
  SIMPLE_ASSERT(Int3VarConst(copyObj) == 3);
  SIMPLE_ASSERT(std::abs(FloatVarConst(copyObj) - 4.f) < 0.001f);

  // Create some modifications.
  SG::Accessor<int> IntVar("IntVar");
  SG::Accessor<int> Int2Var("Int2Var");
  SG::Accessor<double> DoubleVar("DoubleVar");
  IntVar(copyObj) = 2;
  DoubleVar(copyObj) = 3.14;

  SG::Decorator<int> Int3Decor("Int3Var");
  Int3Decor(copyObj) = 6;

  // Check what happened.
  copyAux.setShallowIO(true);
  SIMPLE_ASSERT(copyAux.getAuxIDs().size() == 5);
  SIMPLE_ASSERT(copyAux.getDynamicAuxIDs().size() == 3);
  SIMPLE_ASSERT(copyAux.getSelectedAuxIDs().size() == 3);
  copyAux.setShallowIO(false);
  SIMPLE_ASSERT(copyAux.getAuxIDs().size() == 5);
  SIMPLE_ASSERT(copyAux.getDynamicAuxIDs().size() == 5);
  SIMPLE_ASSERT(copyAux.getSelectedAuxIDs().size() == 5);

  SIMPLE_ASSERT(IntVarConst(copyObj) == 2);
  SIMPLE_ASSERT(std::abs(FloatVarConst(copyObj) - static_cast<float>(4.f)) <
                0.001f);
  SIMPLE_ASSERT(std::abs(DoubleVarConst(copyObj) - 3.14) < 0.0001);
  SIMPLE_ASSERT(Int3VarConst(copyObj) == 6);

  // Finally, test variable filtering:
  xAOD::AuxSelection sel;
  sel.selectAux(std::set<std::string>({"FloatVar", "DoubleVar"}));
  copyAux.setShallowIO(true);
  SIMPLE_ASSERT(sel.getSelectedAuxIDs(copyAux.getSelectedAuxIDs()).size() == 1);
  copyAux.setShallowIO(false);
  SIMPLE_ASSERT(sel.getSelectedAuxIDs(copyAux.getSelectedAuxIDs()).size() == 2);

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

   // Make a shallow copy of it:
   {
     auto [copyVec, copyAux] = xAOD::shallowCopy(origVec);
     static_assert(std::is_same_v<decltype(copyVec),
                                  std::unique_ptr<DataVector<SG::AuxElement>>>);
     static_assert(std::is_same_v<decltype(copyAux),
                                  std::unique_ptr<xAOD::ShallowAuxContainer>>);
     if (testCopy (*copyVec, *copyAux)) {
       return 1;
     }
   }

   {
     auto [copyVec, copyAux] = xAOD::shallowCopy(origVec);
     if (testCopy (*copyVec, *copyAux)) {
       return 1;
     }
     auto [copyVec2, copyAux2] = xAOD::shallowCopy(*copyVec);
     SIMPLE_ASSERT( copyAux2->getAuxIDs().size() == 5 );
   }

   // Tell the user that everything went okay:
   std::cout << "All tests with xAOD::ShallowAuxContainer succeeded"
             << std::endl;

   // Create a test object that we'll make a copy of later on.
   xAOD::AuxInfoBase origAuxInfo;
   SG::AuxElement origObj;
   origObj.setStore(&origAuxInfo);
   IntVar(origObj) = 1;
   Int2Var(origObj) = 2;
   Int3Var(origObj) = 3;
   FloatVar(origObj) = 4.f;

   // Make a shallow copy of it.
   {
     auto [copyObj, copyAux] = xAOD::shallowCopy(origObj);
     static_assert(
         std::is_same_v<decltype(copyObj), std::unique_ptr<SG::AuxElement>>);
     static_assert(std::is_same_v<decltype(copyAux),
                                  std::unique_ptr<xAOD::ShallowAuxInfo>>);
     if (testCopy(*copyObj, *copyAux)) {
       return 1;
     }
   }

   {
     auto [copyObj, copyAux] = xAOD::shallowCopy(origObj);
     if (testCopy(*copyObj, *copyAux)) {
       return 1;
     }
     auto [copyObj2, copyAux2] = xAOD::shallowCopy(*copyObj);
     SIMPLE_ASSERT(copyAux2->getAuxIDs().size() == 5);
   }

   // Tell the user that everything went okay:
   std::cout << "All tests with xAOD::ShallowAuxInfo succeeded" << std::endl;

   // Return gracefully:
   return 0;
}
