/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

// System include(s):
#include <memory>

// ROOT include(s):
#include <TTree.h>

// EDM include(s):
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"

#include "AsgMessaging/MessageCheck.h"

// Local include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TAuxStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

/// Helper macro for evaluating logical tests
#define SIMPLE_ASSERT( EXP )                                            \
   do {                                                                 \
      const bool result = EXP;                                          \
      if( ! result ) {                                                  \
         ::Error( APP_NAME, "Expression \"%s\" failed the evaluation",  \
                  #EXP );                                               \
         return 1;                                                      \
      }                                                                 \
   } while( 0 )


// The name of the application:
const char* APP_NAME = "ut_xaodrootaccess_tauxstore_test";


StatusCode test_linked()
{
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t auxid1 = r.getAuxID<int> ("ltest1", "",
                                        SG::AuxVarFlags::Linked);
  SG::auxid_t auxid2 = r.getAuxID<float> ("ltest2", "",
                                          SG::AuxVarFlags::None,
                                          auxid1);
  TTree tree ("t", "t");
  xAOD::TAuxStore s( "fooAux." );
  RETURN_CHECK( APP_NAME, s.readFrom (tree) );
  int* vp1 = reinterpret_cast<int*> (s.getData (auxid1, 10, 10));
  float* vp2 = reinterpret_cast<float*> (s.getData (auxid2, 3, 3));

  assert (s.getVector (auxid1)->toPtr() == vp1);
  assert (s.getVector (auxid1)->size() == 10);

  auto v1 = reinterpret_cast<const std::vector<int>*> (s.getIOData (auxid1));
  auto v2 = reinterpret_cast<const std::vector<float>*> (s.getIOData (auxid2));
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);
  assert (v2->size() == 3);
  assert (v2->capacity() == 3);
  assert (s.size() == 3);

  s.resize (7);
  assert (s.size() == 7);
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);
  assert (v2->size() == 7);

  s.reserve (50);
  assert (s.size() == 7);
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);
  assert (v2->size() == 7);
  assert (v2->capacity() == 50);

  std::vector<int> vv1 { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  std::vector<float> vv2 { 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5 };

  vp2 = reinterpret_cast<float*> (s.getData (auxid2, 7, 50));

  std::copy (vv1.begin(), vv1.end(), vp1);
  std::copy (vv2.begin(), vv2.end(), vp2);

  s.shift (3, 1);
  assert (s.size() == 8);
  assert (v1->size() == 10);
  assert (v2->size() == 8);
  assert (*v1 == vv1);
  assert (*v2 == (std::vector<float> { 11.5, 12.5, 13.5, 0, 14.5, 15.5, 16.5, 17.5 }) );

  {
    SG::IAuxTypeVector* vi = s.linkedVector (auxid2);
    assert (vi != nullptr);
  }
  {
    const xAOD::TAuxStore& cs = s;
    const SG::IAuxTypeVector* vi = cs.linkedVector (auxid2);
    assert (vi != nullptr);
  }

  TTree tree2 ("t2", "t2");
  xAOD::TAuxStore s2( "fooAux." );
  RETURN_CHECK( APP_NAME, s2.readFrom (tree2) );
  (void)s2.getData (auxid2, 6, 6);
  (void)s2.getData (auxid1, 4, 4);
  auto v3 = reinterpret_cast<const std::vector<int>*> (s2.getIOData (auxid1));
  auto v4 = reinterpret_cast<const std::vector<float>*> (s2.getIOData (auxid2));
  assert (v3->size() == 4);
  assert (v4->size() == 6);

  SG::auxid_set_t ignore;
  s.insertMove (3, s2, ignore);
  assert (s.size() == 14);
  assert (v1->size() == 10);
  assert (v2->size() == 14);
  assert (*v1 == vv1);

  return StatusCode::SUCCESS;
}


int main() {

   ANA_CHECK_SET_TYPE (int);
   using namespace asg::msgUserCode;

   // Initialise the environment:
   ANA_CHECK( xAOD::Init( APP_NAME ) );

   // Reference to the auxiliary type registry:
   SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

   // Create a TTree in memory to help with the reading tests:
   std::unique_ptr< ::TTree > itree( new ::TTree( "InputTree", "Input Tree" ) );
   itree->SetDirectory( 0 );

   // Fill this transient tree with some information:
   std::vector< float > var1{  1,  2,  3,  4,  5 };
   std::vector< float > var2{ 11, 12, 13, 14, 15 };
   std::vector< float >* var1Ptr = &var1;
   std::vector< float >* var2Ptr = &var2;
   ::TBranch* br1 = itree->Branch( "PrefixAuxDyn.var1", &var1Ptr );
   ::TBranch* br2 = itree->Branch( "PrefixAuxDyn.var2", &var2Ptr );
   if( ( ! br1 ) || ( ! br2 ) ) {
      ::Error( APP_NAME, "Couldn't create branches in transient input tree" );
      return 1;
   }
   if( itree->Fill() < 0 ) {
      ::Error( APP_NAME, "Failed to fill the transient data into the input "
               "tree" );
      return 1;
   }
   itree->LoadTree( 0 );
   itree->Print();
   ::Info( APP_NAME, "Created transient input TTree for the test" );

   // Create the object that we want to test:
   xAOD::TAuxStore store( "PrefixAux." );
   store.lock();

   // Connect it to this transient input tree:
   ANA_CHECK( store.readFrom( *itree ) );

   // Check that it found the two variables that it needed to:
   ::Info( APP_NAME, "Auxiliary variables found on the input:" );
   for( auto auxid : store.getAuxIDs() ) {
      ::Info( APP_NAME, "  - id: %i, name: %s, type: %s",
              static_cast< int >( auxid ),
              reg.getName( auxid ).c_str(),
              reg.getTypeName( auxid ).c_str() );
   }
   SIMPLE_ASSERT( store.getAuxIDs().size() == 2 );

   // Create a transient decoration in the object:
   const auto decId = reg.getAuxID< int >( "decoration" );
   SIMPLE_ASSERT( store.getDecoration( decId, 5, 5 ) != 0 );

   // Make sure that the store now knows about this variable:
   SIMPLE_ASSERT( store.getAuxIDs().size() == 3 );

   // Test the isDecoration(...) function.
   const SG::auxid_t var1Id = reg.findAuxID( "var1" );
   SIMPLE_ASSERT( var1Id != SG::null_auxid );
   const SG::auxid_t var2Id = reg.findAuxID( "var2" );
   SIMPLE_ASSERT( var2Id != SG::null_auxid );
   SIMPLE_ASSERT( ! store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( ! store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( store.isDecoration( decId ) );

   // Check that it can be cleared out:
   SIMPLE_ASSERT (store.clearDecorations() == true);
   SIMPLE_ASSERT( store.getAuxIDs().size() == 2 );
   SIMPLE_ASSERT (store.clearDecorations() == false);
   SIMPLE_ASSERT( store.getAuxIDs().size() == 2 );
   SIMPLE_ASSERT( ! store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( ! store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( ! store.isDecoration( decId ) );

   // Try to overwrite an existing variable with a decoration, to check that
   // it can't be done:
   SIMPLE_ASSERT( var1Id != SG::null_auxid );
   bool exceptionThrown = false;
   try {
      store.getDecoration( var1Id, 2, 2 );
   } catch( const SG::ExcStoreLocked& ) {
      exceptionThrown = true;
   }
   SIMPLE_ASSERT( exceptionThrown == true );

   // Set up a selection rule for the output writing:
   store.selectAux( { "var1", "decoration" } );

   // Create another transient tree to test the tree writing with:
   std::unique_ptr< ::TTree > otree( new ::TTree( "OutputTree",
                                                  "Output Tree" ) );
   otree->SetDirectory( 0 );

   // Connect the store object to the tree:
   ANA_CHECK( store.writeTo( *otree ) );

   // Create the decoration again:
   SIMPLE_ASSERT( store.getDecoration( decId, 5, 5 ) != 0 );
   SIMPLE_ASSERT( store.getAuxIDs().size() == 3 );
   SIMPLE_ASSERT( ! store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( ! store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( store.isDecoration( decId ) );

   // Try to overwrite an existing variable with a decoration, to check that
   // it can't be done:
   SIMPLE_ASSERT( var2Id != SG::null_auxid );
   exceptionThrown = false;
   try {
      store.getDecoration( var1Id, 2, 2 );
   } catch( const SG::ExcStoreLocked& ) {
      exceptionThrown = true;
   }
   SIMPLE_ASSERT( exceptionThrown == true );
   exceptionThrown = false;
   try {
      store.getDecoration( var2Id, 2, 2 );
   } catch( const SG::ExcStoreLocked& ) {
      exceptionThrown = true;
   }
   SIMPLE_ASSERT( exceptionThrown == true );

   // Check that the output tree looks as it should:
   if( otree->Fill() < 0 ) {
      ::Error( APP_NAME, "Failed to fill the transient data into the output "
               "tree" );
      return 1;
   }
   otree->Print();
   SIMPLE_ASSERT( otree->GetNbranches() == 2 );

   // Make sure that when we clear the decorations, the auxiliary ID is not
   // removed. Since this is a "persistent decoration" now:
   SIMPLE_ASSERT (store.clearDecorations() == false);
   SIMPLE_ASSERT( store.getAuxIDs().size() == 3 );
   SIMPLE_ASSERT( ! store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( ! store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( store.isDecoration( decId ) );

   SIMPLE_ASSERT( test_linked().isSuccess() );

   return 0;
}
