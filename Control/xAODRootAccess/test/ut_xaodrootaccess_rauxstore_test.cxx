/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

// System include(s):
#include <filesystem>
#include <memory>

// EDM include(s):
#include "AsgMessaging/MessageCheck.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"

// Local include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/RAuxStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// ROOT include(s):
#include <TFile.h>

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleWriter.hxx>

using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleReader;
using ROOT::Experimental::RNTupleWriter;

/// Helper macro for evaluating logical tests
#define SIMPLE_ASSERT( EXP )                                                   \
   do {                                                                        \
      const bool result = EXP;                                                 \
      if( !result ) {                                                          \
         ::Error( APP_NAME, "Expression \"%s\" failed the evaluation", #EXP ); \
         return 1;                                                             \
      }                                                                        \
   } while( 0 )

// The name of the application:
const char* APP_NAME = "ut_xaodrootaccess_rauxstore_test";
const char* INPUT_FILE_NAME = "InputNtuple.root";
const char* INPUT_NTUPLE_NAME = "InputNtuple";
const char* OUTPUT_FILE_NAME = "OutputNtuple.root";
const char* OUTPUT_NTUPLE_NAME = "OutputNtuple";

StatusCode test_linked() {

   const char* ntupleName = "name";
   const char* fileName = "file.root";
   const char* fileName2 = "file2.root";

   SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
   SG::auxid_t auxid1 =
       r.getAuxID< int >( "ltest1", "", SG::AuxVarFlags::Linked );
   SG::auxid_t auxid2 =
       r.getAuxID< float >( "ltest2", "", SG::AuxVarFlags::None, auxid1 );
   // Inside it's own scope to ensure it is not open when we try to read from it
   {
      auto model = RNTupleModel::Create();
      auto ntuple =
         RNTupleWriter::Recreate( std::move( model ), ntupleName, fileName );
   }

   xAOD::RAuxStore s( "fooAux:" );
   RETURN_CHECK( APP_NAME, s.readFrom( fileName, ntupleName ) );
   int* vp1 = reinterpret_cast< int* >( s.getData( auxid1, 10, 10 ) );
   float* vp2 = reinterpret_cast< float* >( s.getData( auxid2, 3, 3 ) );

   assert( s.getVector( auxid1 )->toPtr() == vp1 );
   assert( s.getVector( auxid1 )->size() == 10 );

   auto v1 =
       reinterpret_cast< const std::vector< int >* >( s.getIOData( auxid1 ) );
   auto v2 =
       reinterpret_cast< const std::vector< float >* >( s.getIOData( auxid2 ) );
   assert( v1->size() == 10 );
   assert( v1->capacity() == 10 );
   assert( v2->size() == 3 );
   assert( v2->capacity() == 3 );
   assert( s.size() == 3 );

   s.resize( 7 );
   assert( s.size() == 7 );
   assert( v1->size() == 10 );
   assert( v1->capacity() == 10 );
   assert( v2->size() == 7 );

   s.reserve( 50 );
   assert( s.size() == 7 );
   assert( v1->size() == 10 );
   assert( v1->capacity() == 10 );
   assert( v2->size() == 7 );
   assert( v2->capacity() == 50 );

   std::vector< int > vv1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
   std::vector< float > vv2{ 11.5, 12.5, 13.5, 14.5, 15.5, 16.5, 17.5 };

   vp2 = reinterpret_cast< float* >( s.getData( auxid2, 7, 50 ) );

   std::copy( vv1.begin(), vv1.end(), vp1 );
   std::copy( vv2.begin(), vv2.end(), vp2 );

   s.shift( 3, 1 );
   assert( s.size() == 8 );
   assert( v1->size() == 10 );
   assert( v2->size() == 8 );
   assert( *v1 == vv1 );
   assert( *v2 == ( std::vector< float >{ 11.5, 12.5, 13.5, 0, 14.5, 15.5, 16.5,
                                          17.5 } ) );

   {
      SG::IAuxTypeVector* vi = s.linkedVector( auxid2 );
      assert( vi != nullptr );
   }
   {
      const xAOD::RAuxStore& cs = s;
      const SG::IAuxTypeVector* vi = cs.linkedVector( auxid2 );
      assert( vi != nullptr );
   }
   // Inside it's own scope to ensure it is not open when we try to read from it
   {
      auto model2 = RNTupleModel::Create();
      auto ntuple2 =
         RNTupleWriter::Recreate( std::move( model2 ), ntupleName, fileName2 );
   }
   xAOD::RAuxStore s2( "fooAux:" );
   RETURN_CHECK( APP_NAME, s2.readFrom( fileName2, ntupleName ); );

   (void)s2.getData( auxid2, 6, 6 );
   (void)s2.getData( auxid1, 4, 4 );
   auto v3 =
       reinterpret_cast< const std::vector< int >* >( s2.getIOData( auxid1 ) );
   auto v4 = reinterpret_cast< const std::vector< float >* >(
       s2.getIOData( auxid2 ) );
   assert( v3->size() == 4 );
   assert( v4->size() == 6 );

   SG::auxid_set_t ignore;
   s.insertMove( 3, s2, ignore );
   assert( s.size() == 14 );
   assert( v1->size() == 10 );
   assert( v2->size() == 14 );
   assert( *v1 == vv1 );

   return StatusCode::SUCCESS;
}

struct MoveTest {
   MoveTest( int x = 0 ) : m_v( x ) {}
   MoveTest( const MoveTest& other ) : m_v( other.m_v ) {}
   MoveTest( MoveTest&& other ) : m_v( std::move( other.m_v ) ) {}
   MoveTest& operator=( const MoveTest& other ) {
      if( this != &other ) m_v = other.m_v;
      return *this;
   }
   MoveTest& operator=( MoveTest&& other ) {
      if( this != &other ) m_v = std::move( other.m_v );
      return *this;
   }
   std::vector< int > m_v;
   bool operator==( const MoveTest& other ) const {
      return m_v.size() == other.m_v.size();
   }
};

bool wasMoved( const MoveTest& x ) {
   return x.m_v.empty();
}

StatusCode test_insertmove() {

   const char* fileName = "file.root";
   const char* ntupleName = "name";

   SG::auxid_t ityp1 =
       SG::AuxTypeRegistry::instance().getAuxID< int >( "anInt" );
   SG::auxid_t ityp2 =
       SG::AuxTypeRegistry::instance().getAuxID< int >( "anotherInt" );
   SG::auxid_t ityp3 =
       SG::AuxTypeRegistry::instance().getAuxID< int >( "anInt3" );
   SG::auxid_t ityp4 =
       SG::AuxTypeRegistry::instance().getAuxID< int >( "anInt4" );
   SG::auxid_t mtyp1 =
       SG::AuxTypeRegistry::instance().getAuxID< MoveTest >( "moveTest" );
   // Inside it's own scope to ensure it is not open when we try to read from it
   {
      auto model = RNTupleModel::Create();
      auto ntuple =
         RNTupleWriter::Recreate( std::move( model ), ntupleName, fileName );
   }
   xAOD::RAuxStore s1( "fooAux:" );

   RETURN_CHECK( APP_NAME, s1.readFrom( fileName, ntupleName ) );
   s1.resize( 5 );

   int* i1 = reinterpret_cast< int* >( s1.getData( ityp1, 5, 20 ) );

   int* i2 = reinterpret_cast< int* >( s1.getData( ityp2, 5, 20 ) );

   MoveTest* m1 = reinterpret_cast< MoveTest* >( s1.getData( mtyp1, 5, 20 ) );

   for( int i = 0; i < 5; i++ ) {
      i1[i] = i;
      i2[i] = i + 100;
      m1[i] = MoveTest( i );
   }

   SG::AuxStoreInternal s2;
   s2.resize( 5 );

   int* i1_2 = reinterpret_cast< int* >( s2.getData( ityp1, 5, 5 ) );
   int* i3_2 = reinterpret_cast< int* >( s2.getData( ityp3, 5, 5 ) );
   int* i4_2 = reinterpret_cast< int* >( s2.getData( ityp4, 5, 5 ) );
   MoveTest* m1_2 = reinterpret_cast< MoveTest* >( s2.getData( mtyp1, 5, 5 ) );
   for( int i = 0; i < 5; i++ ) {
      i1_2[i] = i + 10;
      i3_2[i] = i + 110;
      i4_2[i] = i + 210;
      m1_2[i] = MoveTest( i + 10 );
   }

   SG::auxid_set_t ignore{ ityp4 };

   s1.insertMove( 3, s2, ignore );

   assert( s1.size() == 10 );

   s1.reserve( 20 );

   assert( s1.getData( ityp4 ) == nullptr );

   const int* i3 = reinterpret_cast< const int* >( s1.getData( ityp3 ) );

   assert( i3 != 0 );
   i1 = reinterpret_cast< int* >( s1.getData( ityp1, 5, 20 ) );
   i2 = reinterpret_cast< int* >( s1.getData( ityp2, 5, 20 ) );
   m1 = reinterpret_cast< MoveTest* >( s1.getData( mtyp1, 5, 20 ) );
   for( int i = 0; i < 3; i++ ) {
      assert( i1[i] == i );
      assert( i2[i] == i + 100 );
      assert( i3[i] == 0 );
      assert( m1[i] == MoveTest( i ) );
   }

   for( int i = 0; i < 5; i++ ) {
      assert( i1[3 + i] == i + 10 );
      assert( i2[3 + i] == 0 );
      assert( i3[3 + i] == i + 110 );
      assert( m1[3 + i] == MoveTest( i + 10 ) );
   }

   for( int i = 3; i < 5; i++ ) {
      assert( i1[5 + i] == i );
      assert( i2[5 + i] == i + 100 );
      assert( i3[5 + i] == 0 );
      assert( m1[5 + i] == MoveTest( i ) );
   }
   for( int i = 0; i < 5; i++ ) {
      assert( wasMoved( m1_2[i] ) );
   }

   for( int i = 0; i < 5; i++ ) {
      i1_2[i] = i + 20;
      i3_2[i] = i + 120;
      m1_2[i] = MoveTest( i + 20 );
   }
   s1.insertMove( 10, s2, ignore );
   assert( s1.size() == 15 );
   i1 = reinterpret_cast< int* >( s1.getData( ityp1, 5, 20 ) );
   i2 = reinterpret_cast< int* >( s1.getData( ityp2, 5, 20 ) );
   i3 = reinterpret_cast< int* >( s1.getData( ityp3, 5, 20 ) );
   m1 = reinterpret_cast< MoveTest* >( s1.getData( mtyp1, 5, 20 ) );
   for( int i = 0; i < 3; i++ ) {
      assert( i1[i] == i );
      assert( i2[i] == i + 100 );
      assert( i3[i] == 0 );
      assert( m1[i] == MoveTest( i ) );
   }

   for( int i = 0; i < 5; i++ ) {
      assert( i1[3 + i] == i + 10 );
      assert( i2[3 + i] == 0 );
      assert( i3[3 + i] == i + 110 );
      assert( m1[3 + i] == MoveTest( i + 10 ) );
   }
   for( int i = 3; i < 5; i++ ) {
      assert( i1[5 + i] == i );
      assert( i2[5 + i] == i + 100 );
      assert( i3[5 + i] == 0 );
      assert( m1[5 + i] == MoveTest( i ) );
   }

   for( int i = 0; i < 5; i++ ) {
      assert( i1[10 + i] == i + 20 );
      assert( i2[10 + i] == 0 );
      assert( i3[10 + i] == i + 120 );
      assert( m1[10 + i] == MoveTest( i + 20 ) );
   }
   for( int i = 0; i < 5; i++ ) {
      assert( wasMoved( m1_2[i] ) );
   }

   return StatusCode::SUCCESS;
}

void createAndFillNtuple( const char* ntupleName, const char* fileName ) {
   // Create an RNTuple model
   auto model = RNTupleModel::Create();

   // Create fields for the RNTuple
   auto var1Field =
       model->MakeField< std::vector< float > >( "PrefixAuxDyn:var1" );
   auto var2Field =
       model->MakeField< std::vector< float > >( "PrefixAuxDyn:var2" );

   // Create an RNTuple writer
   auto ntuple =
       RNTupleWriter::Recreate( std::move( model ), ntupleName, fileName );

   // Fill the RNTuple with some information
   std::vector< float > var1{ 1, 2, 3, 4, 5 };
   std::vector< float > var2{ 11, 12, 13, 14, 15 };

   *var1Field = var1;
   *var2Field = var2;
   ntuple->Fill();
}

int main() {
   // attempt to remove the files if they exist (othewise all is fine)
   std::filesystem::remove( INPUT_FILE_NAME );
   std::filesystem::remove( OUTPUT_FILE_NAME );

   ANA_CHECK_SET_TYPE( int );
   using namespace asg::msgUserCode;

   // Initialise the environment:
   ANA_CHECK( xAOD::Init( APP_NAME ) );

   // Reference to the auxiliary type registry:
   SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

   createAndFillNtuple( INPUT_NTUPLE_NAME, INPUT_FILE_NAME );

   // Read the RNTuple
   auto inputNtuple = ROOT::Experimental::RNTupleReader::Open(
       INPUT_NTUPLE_NAME, INPUT_FILE_NAME );
   inputNtuple->PrintInfo();

   std::unique_ptr< ::TFile > ifile( ::TFile::Open( INPUT_FILE_NAME, "READ" ) );
   ::Info( APP_NAME, "Created transient input RNTuple for the test" );

   // Create the store and tell it to load entry 0
   xAOD::RAuxStore store( "PrefixAux:",0 );
   store.lock();
   // Create the object that we want to test:

   // Connect it to this transient RNTuple:
   ANA_CHECK( store.readFrom( INPUT_FILE_NAME, INPUT_NTUPLE_NAME ) );

   // Check that it found the two variables that it needed to:
   ::Info( APP_NAME, "Auxiliary variables found on the input:" );
   for( auto auxid : store.getAuxIDs() ) {
      ::Info( APP_NAME, "  - id: %i, name: %s, type: %s",
              static_cast< int >( auxid ), reg.getName( auxid ).c_str(),
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

   //check that we can access the data
   const void* var1Ptr = store.getData( var1Id );
   SIMPLE_ASSERT( var1Ptr != 0 );
   const float* var1 = static_cast<const float*>(var1Ptr);

   const void* var2Ptr = store.getData( var2Id );
   SIMPLE_ASSERT( var2Ptr != 0 );
   const float* var2 = reinterpret_cast< const float* >( var2Ptr );

   //check that the values match those added in createAndFillNtuple
   SIMPLE_ASSERT( ( var1 )[ 0 ] == 1.0f );
   SIMPLE_ASSERT( ( var1 )[ 1 ] == 2.0f );
   SIMPLE_ASSERT( ( var1 )[ 2 ] == 3.0f );
   SIMPLE_ASSERT( ( var1 )[ 3 ] == 4.0f );
   SIMPLE_ASSERT( ( var1 )[ 4 ] == 5.0f );

   SIMPLE_ASSERT( ( var2 )[ 0 ] == 11.0f );
   SIMPLE_ASSERT( ( var2 )[ 1 ] == 12.0f );
   SIMPLE_ASSERT( ( var2 )[ 2 ] == 13.0f );
   SIMPLE_ASSERT( ( var2 )[ 3 ] == 14.0f );
   SIMPLE_ASSERT( ( var2 )[ 4 ] == 15.0f );

   SIMPLE_ASSERT( !store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( !store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( store.isDecoration( decId ) );

   // Check that it can be cleared out:
   SIMPLE_ASSERT( store.clearDecorations() == true );
   SIMPLE_ASSERT( store.getAuxIDs().size() == 2 );
   SIMPLE_ASSERT( store.clearDecorations() == false );
   SIMPLE_ASSERT( store.getAuxIDs().size() == 2 );
   SIMPLE_ASSERT( !store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( !store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( !store.isDecoration( decId ) );

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

   // Create another transient ntuple to test the ntuple writing with:
   // Inside it's own scope to ensure it is not open when we try to read from it
   {
      auto model = RNTupleModel::Create();
      auto ntuple = RNTupleWriter::Recreate(
          std::move( model ), OUTPUT_NTUPLE_NAME, OUTPUT_FILE_NAME );
   }
   // Connect the store object to the ntuple:
   ANA_CHECK( store.writeTo( OUTPUT_FILE_NAME, OUTPUT_NTUPLE_NAME ) );

   // Create the decoration again:
   SIMPLE_ASSERT( store.getDecoration( decId, 5, 5 ) != 0 );
   SIMPLE_ASSERT( store.getAuxIDs().size() == 3 );
   SIMPLE_ASSERT( !store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( !store.isDecoration( var2Id ) );
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
   auto outputNtuple = ROOT::Experimental::RNTupleReader::Open(
       OUTPUT_NTUPLE_NAME, OUTPUT_FILE_NAME );
   outputNtuple->PrintInfo();

   // It should have 2 top level fields (var1 and decoration).
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
   SIMPLE_ASSERT(
       outputNtuple->GetModel().GetConstFieldZero().GetSubFields().size() ==
       2 );
#else
   SIMPLE_ASSERT(
       outputNtuple->GetModel().GetFieldZero().GetSubFields().size() ==
       2 );
#endif
   // It should have 5 fields (the artificial/implicit "zero field" and the two
   // top level fields that each have 1 subfield as they are vectors)
   SIMPLE_ASSERT( outputNtuple->GetDescriptor().GetNFields() == 5 );

   // Make sure that when we clear the decorations, the auxiliary ID is not
   // removed. Since this is a "persistent decoration" now:
   SIMPLE_ASSERT( store.clearDecorations() == false );
   SIMPLE_ASSERT( store.getAuxIDs().size() == 3 );
   SIMPLE_ASSERT( !store.isDecoration( var1Id ) );
   SIMPLE_ASSERT( !store.isDecoration( var2Id ) );
   SIMPLE_ASSERT( store.isDecoration( decId ) );

   SIMPLE_ASSERT( test_linked().isSuccess() );
   SIMPLE_ASSERT( test_insertmove().isSuccess() );

   if( !std::filesystem::remove( INPUT_FILE_NAME ) ) {
      ::Error( APP_NAME, "Failed to remove file: %s", INPUT_FILE_NAME );
   }
   if( !std::filesystem::remove( OUTPUT_FILE_NAME ) ) {
      ::Error( APP_NAME, "Failed to remove file: %s", OUTPUT_FILE_NAME );
   }

   return 0;
}