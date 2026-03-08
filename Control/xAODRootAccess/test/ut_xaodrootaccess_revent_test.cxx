/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <algorithm>
#include <memory>
#include <vector>
#include <string>

// ROOT include(s):
#include <TError.h>
#include <TFile.h>
#include <TSystem.h>

// EDM that the package uses anyway:
#include "AthContainers/DataVector.h"
#include "AthContainers/ConstDataVector.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "xAODCore/AuxContainerBase.h"

// Local include(s):
#include "xAODRootAccess/Init.h"

#include "xAODRootAccess/Event.h"
#include "xAODRootAccess/REvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/RAuxStore.h"
#include "xAODRootAccess/TAuxStore.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/Utils.h"
#include "xAODRootAccessInterfaces/TActiveEvent.h"

/// Helper macro
#define RETURN_CHECK( CONTEXT, EXP )                                 \
   do {                                                              \
      const StatusCode result = EXP;                                 \
      if( ! result.isSuccess() ) {                                   \
         ::Error( CONTEXT, XAOD_MESSAGE( "Failed to execute: %s" ),  \
                  #EXP );                                            \
         return 1;                                                   \
      }                                                              \
   } while( false )

/// Another little helper macro
#define SIMPLE_ASSERT( EXP )                                                \
   do {                                                                     \
      if( ! ( EXP ) ) {                                                     \
         ::Error( APP_NAME, XAOD_MESSAGE( "Error evaluating: %s" ), #EXP ); \
         return 1;                                                          \
      }                                                                     \
   } while( 0 )

/// Type used in the event/store test
class ClassA {
   
public:
   int m_var1;
   float m_var2;
   
}; // class ClassA

/// Type used in the event/store test
class ClassB : public ClassA {
   
public:
   int m_var3;
   float m_var4;
   
}; // class ClassB

// some dummz definitions to test REvent::keys
namespace xAOD {
   // a test for metadata
   class FileMetaData_v1 { };
   typedef FileMetaData_v1 FileMetaData;

   // a test for event payload
   class TrackParticle_v1 { };
   typedef TrackParticle_v1 TrackParticle;
}
//coverity[UNCAUGHT_EXCEPT]
int main() {

   // Get the name of the application:
   const char* APP_NAME = "ut_xaodrootaccess_revent_test";

   // Initialise the environment:
   RETURN_CHECK( APP_NAME, xAOD::Init() );
   
   // Create the tested object(s):
   
   // xAOD::Experimental::REvent event;
   xAOD::TStore store;

   // Read from test file
   ::Info( APP_NAME, XAOD_MESSAGE( "run readFrom for file %s" ), gSystem->Getenv( "ASG_TEST_FILE_RUN2_LITE_RNTUPLE_MC" )); 

   // Connect an input file to the event:
   std::unique_ptr< ::TFile > ifile( ::TFile::Open( "$ASG_TEST_FILE_RUN2_LITE_RNTUPLE_MC", "READ" ) );
   if( ! ifile ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "File %s couldn't be opened..." ),
               gSystem->Getenv( "ASG_TEST_FILE_RUN2_LITE_RNTUPLE_MC" ) );
      return 1;
   }

   std::unique_ptr<xAOD::Event> event = xAOD::Event::createAndReadFrom(*ifile);

   // Try resetting the ifile pointer to make sure things still work
   ifile.reset();

   // // Set debug level 
   // event->msg().setLevel(MSG::DEBUG);

   // Read in the first event:
   if( event->getEntry( 0 ) < 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't load entry 0 from file %s" ),
               gSystem->Getenv( "ASG_TEST_FILE_RUN2_LITE_RNTUPLE_MC" ) );
      return 1;
   }

   // Try to retrieve some objects:
   const xAOD::AuxContainerBase* c = 0;
   RETURN_CHECK( APP_NAME, event->retrieve( c, "AnalysisElectronsAux." ) );
   RETURN_CHECK( APP_NAME, event->retrieve( c, "AnalysisMuonsAux." ) );
   
   // Record some objects into TStore:
   ClassA* objA = new ClassA();
   RETURN_CHECK( APP_NAME, store.record( objA, "MyObjA" ) );
   ClassB* objB = new ClassB();
   RETURN_CHECK( APP_NAME, store.record( objB, "MyObjB" ) );

   // Print what's in the event now:
   Info( APP_NAME, "Event contents:\n\n%s\n\n", event->dump().c_str() );

   // They should now be accessible through Event:
   const ClassA* dummy1 = 0;
   RETURN_CHECK( APP_NAME, event->retrieve( dummy1, "MyObjA" ) );
   const ClassB* dummy2 = 0;
   RETURN_CHECK( APP_NAME, event->retrieve( dummy2, "MyObjB" ) );

   // Try to get them through the TVirtualEvent interface:
   xAOD::TVirtualEvent* vevent = xAOD::TActiveEvent::event();

   SIMPLE_ASSERT( vevent != 0 );
   SIMPLE_ASSERT( vevent->retrieve( dummy1,
                                    xAOD::Utils::hash( "MyObjA" ) ) == true );
   SIMPLE_ASSERT( vevent->retrieve( dummy2,
                                    xAOD::Utils::hash( "MyObjB" ) ) == true );
   SIMPLE_ASSERT( vevent->retrieve( dummy1, 0x1234 ) == false );

   // Test the functions used by the smart pointers:
   if( event->getName( objA ) != "MyObjA" ) {
      ::Error( APP_NAME,
               XAOD_MESSAGE( "getName(objA) = \"%s\" (!=\"MyObjA\")" ),
               event->getName( objA ).c_str() );
      return 1;
   }

   if( event->getName( xAOD::Utils::hash( "MyObjB" ) ) != "MyObjB" ) {
      ::Error( APP_NAME,
               XAOD_MESSAGE( "Couldn't retrieve the name for hashed "
                             "\"MyObjB\"" ) );
      return 1;
   }

   if( event->getName( ( void* ) 0x12345678 ) != "" ) {
      ::Error( APP_NAME,
               XAOD_MESSAGE( "Found a name for an imaginary pointer?!?" ) );
      return 1;
   }

   if( event->getName( 0x12345678 ) != "" ) {
      ::Error( APP_NAME,
               XAOD_MESSAGE( "Found a name for an imaginary hash?!?" ) );
      return 1;
   }

   // This is not supposed to work:
   if( event->retrieve( dummy1, "MyObjB" ).isSuccess() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Something strange happened" ) );
      return 1;
   }

   // Clear the container:
   store.clear();

   // Now this is not supposed to work either:
   if( event->retrieve( dummy2, "MyObjB" ).isSuccess() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Something strange happened" ) );
      return 1;
   }

   // Record a constant DV into the store:
   DataVector< ClassA > dv;
   dv.push_back( new ClassA() );
   dv.push_back( new ClassA() );
   ConstDataVector< DataVector< ClassA > >* cdv =
      new ConstDataVector< DataVector< ClassA > >( SG::VIEW_ELEMENTS );
   RETURN_CHECK( APP_NAME, store.record( cdv, "ConstDataVector" ) );

   store.print();

   // Try to retrieve it in all possible ways:
   ConstDataVector< DataVector< ClassA > >* cdv1 = 0;
   if ( event->retrieve( cdv1, "ConstDataVector" ).isSuccess() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Non-const retrieval of ConstDataVector should not work" ) );
      return 1;
   }
   const ConstDataVector< DataVector< ClassA > >* cdv2 = 0;
   RETURN_CHECK( APP_NAME, event->retrieve( cdv2, "ConstDataVector" ) );
   const DataVector< ClassA >* cdv3 = 0;
   RETURN_CHECK( APP_NAME, event->retrieve( cdv3, "ConstDataVector" ) );

   // But this should not work:
   DataVector< ClassA >* cdv4 = 0;
   if( event->retrieve( cdv4, "ConstDataVector" ).isSuccess() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Non-const retrieval of ConstDataVector should not work" ) );
      return 1;
   }


   // test listing object keys
   {
      std::vector<std::string> keys;
      RETURN_CHECK( APP_NAME, event->keys<xAOD::FileMetaData>(keys, true) );
      Info( APP_NAME, "Looking for keys for FileMetaData. Found: " );
      int ikey = 0;
      for ( auto& key : keys ) {
         Info( APP_NAME, "%i - %s", ikey, key.c_str() );
         ++ikey;
      }
      if (keys.size() != 1) {
         ::Error( APP_NAME,
               XAOD_MESSAGE( "keys<xAOD::FileMetaData>(true).size = %u (!=1)" ),
               static_cast<unsigned>(keys.size()) );
         return 1;
      }
      keys.clear();
      keys.reserve(5); // Only 5 containers for TrackParticles in OpenData, whereas there are 6 for std DAODs
      RETURN_CHECK( APP_NAME, event->keys<DataVector< xAOD::TrackParticle > >(keys, false) );
      Info( APP_NAME, "Looking for keys for TrackParticle. Found: " );
      ikey = 0;
      for ( auto& key : keys ) {
         Info( APP_NAME, "%i - %s", ikey, key.c_str() );
         ++ikey;
      }
      if (keys.size() != 5) {
         ::Error( APP_NAME,
               XAOD_MESSAGE( "keys<xAOD::TrackParticle >().size = %u (!=6)" ),
               static_cast<unsigned>(keys.size()) );
         return 1;
      }
      auto begin = keys.begin();
      auto end = keys.end();
      if (std::find(begin, end, "InDetTrackParticles") == end) {
         ::Error( APP_NAME,
               XAOD_MESSAGE( "keys<xAOD::TrackParticle >() did not find "
                             "\"InDetTrackParticles\"" ) );
         return 1;
      }

      // $TODO: test scanning through output

   }


   // Create another REvent instance to test the file writing capabilities of
   // the code:
   xAOD::Experimental::REvent wevent;

   // // Set debug level 
   // wevent.msg().setLevel(MSG::DEBUG);

   // And connect it to an output file:
   static const char* const OFNAME = "test_rntuple.xAOD.root";
   std::unique_ptr< ::TFile > ofile( ::TFile::Open( OFNAME, "RECREATE" ) );
   if( ! ofile.get() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't create test output file: %s" ),
               OFNAME );
      return 1;
   }
   RETURN_CHECK( APP_NAME, wevent.writeTo( *ofile ) );


   // Construct some xAOD store objects 
   //   - for the first fill, keep the last container, c3, empty for the first fill
   //   - the the add objects for the next fill
   SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();
   std::unique_ptr< xAOD::AuxContainerBase > c1( new xAOD::AuxContainerBase() );
   c1->resize( 5 );
   c1->getData( reg.getAuxID< int >( "IntVar" ), 5, 5 );
   c1->getData( reg.getAuxID< float >( "FloatVar" ), 5, 5 );
   std::unique_ptr< xAOD::AuxContainerBase > c2( new xAOD::AuxContainerBase() );
   c2->resize( 10 );
   c2->getData( reg.getAuxID< int >( "IntVar" ), 10, 10 );
   c2->getData( reg.getAuxID< float >( "FloatVar" ), 10, 10 );
   std::unique_ptr< xAOD::AuxContainerBase > c3( new xAOD::AuxContainerBase() );
   // c2->resize( 10 );
   // c2->getData( reg.getAuxID< int >( "IntVar" ), 10, 10 );
   // c2->getData( reg.getAuxID< float >( "FloatVar" ), 10, 10 );
   // And some STL ones as well:
   std::unique_ptr< std::vector< float > >
      fvector( new std::vector< float >() );
   fvector->insert( fvector->begin(), 5, 1.23 );
   std::unique_ptr< std::vector< std::string > >
      svector( new std::vector< std::string >() );
   svector->insert( svector->begin(), 3, "foo" );
   // It should be possible to record these into the output:
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c1 ), "Container1Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c2 ), "Container2Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c3 ), "Container3Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( fvector ), "FloatVec" ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( svector ), "StringVec" ) );

   ::Int_t nbytes = wevent.fill();
   if( nbytes <= 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't write event to output - first fill" ) );
      return 1;
   }

   // Now refill with c3 not empty
   c1.reset( new xAOD::AuxContainerBase() );
   c1->resize( 6 );
   c1->getData( reg.getAuxID< int >( "IntVar" ), 6, 6 );
   c1->getData( reg.getAuxID< float >( "FloatVar" ), 6, 6 );
   c2.reset( new xAOD::AuxContainerBase() );
   c2->resize( 11 );
   c2->getData( reg.getAuxID< int >( "IntVar" ), 11, 11 );
   c2->getData( reg.getAuxID< float >( "FloatVar" ), 11, 11 );
   c3.reset( new xAOD::AuxContainerBase() );
   c3->resize( 16 );
   c3->getData( reg.getAuxID< int >( "IntVar" ), 16, 16 );
   c3->getData( reg.getAuxID< float >( "FloatVar" ), 16, 16 );
   fvector.reset( new std::vector< float >() );
   fvector->insert( fvector->begin(), 5, 1.33 );
   svector.reset( new std::vector< std::string >() );
   svector->insert( svector->begin(), 4, "bar1" );
   // It should be possible to record these into the output:
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c1 ), "Container1Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c2 ), "Container2Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c3 ), "Container3Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( fvector ), "FloatVec" ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( svector ), "StringVec" ) );

   nbytes = wevent.fill();
   if( nbytes <= 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't write event to output - second fill" ) );
      return 1;
   }

   // Now repeat another time with container 2 empty
   c1.reset( new xAOD::AuxContainerBase() );
   c1->resize( 7 );
   c1->getData( reg.getAuxID< int >( "IntVar" ), 7, 7 );
   c1->getData( reg.getAuxID< float >( "FloatVar" ), 7, 7 );
   c2.reset( new xAOD::AuxContainerBase() );
   // c2->resize( 11 );
   c2->getData( reg.getAuxID< int >( "IntVar" ), 0, 0 );
   c2->getData( reg.getAuxID< float >( "FloatVar" ), 0, 0 );
   c3.reset( new xAOD::AuxContainerBase() );
   c3->resize( 17 );
   c3->getData( reg.getAuxID< int >( "IntVar" ), 17, 17 );
   c3->getData( reg.getAuxID< float >( "FloatVar" ), 17, 17 );
   fvector.reset( new std::vector< float >() );
   fvector->insert( fvector->begin(), 5, 1.33 );
   svector.reset( new std::vector< std::string >() );
   svector->insert( svector->begin(), 4, "bar1" );
   // It should be possible to record these into the output:
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c1 ), "Container1Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c2 ), "Container2Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c3 ), "Container3Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( fvector ), "FloatVec" ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( svector ), "StringVec" ) );

   nbytes = wevent.fill();
   if( nbytes <= 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't write event to output - third fill" ) );
      return 1;
   }


   // Now update just some of them, and see what happens. The fill should fail.
   // Remember that the old
   // objects are now owned by REvent, so we can re-use these pointers.
   c1.reset( new xAOD::AuxContainerBase() );
   c1->resize( 5 );
   c1->getData( reg.getAuxID< int >( "IntVar" ), 5, 5 );
   c1->getData( reg.getAuxID< float >( "FloatVar" ), 5, 5 );
   svector.reset( new std::vector< std::string >() );
   svector->insert( svector->begin(), 4, "bar" );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( c1 ), "Container1Aux." ) );
   RETURN_CHECK( APP_NAME, wevent.record( std::move( svector ), "StringVec" ) );

   if( wevent.fill() > 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "This writing should've failed. nbytes %i" ), nbytes );
      return 1;
   }

   // And now try to record an object without a dictionary. Which should
   // of course fail.
   std::unique_ptr< ClassA > c4( new ClassA() );
   if( wevent.record( std::move( c4 ), "ClassA" ).isSuccess() ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "This record should've failed. o.O" ) );
      return 1;
   }

   // Finish writing to the file:
   RETURN_CHECK( APP_NAME, wevent.finishWritingTo(*ofile) );

   xAOD::Experimental::REvent rwevent;
   RETURN_CHECK( APP_NAME, rwevent.readFrom( OFNAME ) );

   // Read in the first event:
   if( rwevent.getEntry( 0 ) < 0 ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't load entry 0 from test.xAOD.root" ) );
      return 1;
   }

   // And delete the file from disk:
   if( gSystem->Unlink( OFNAME ) ) {
      ::Error( APP_NAME, XAOD_MESSAGE( "Couldn't remove temporary file: %s" ),
               OFNAME );
      return 1;
   }

   // Return gracefully:
   return 0;
}
