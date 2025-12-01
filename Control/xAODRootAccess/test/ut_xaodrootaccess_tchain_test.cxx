/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ROOT include(s):
#include <TChain.h>
#include <TError.h>

// EDM that the package uses anyway:
#include "AthContainers/AuxVectorBase.h"
#include "xAODCore/AuxContainerBase.h"

#include "AsgMessaging/MessageCheck.h"

// Local include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Utils.h"

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

/// Helper function, "processing" a TChain
StatusCode process( xAOD::TEvent& event, xAOD::TStore& store );

//coverity[root_function]
int main() {

   ANA_CHECK_SET_TYPE (int);
   using namespace asg::msgUserCode;

   // Get the name of the application:
   const char* APP_NAME = "ut_xaodrootaccess_tchain_test";

   // Initialise the environment:
   ANA_CHECK( xAOD::Init( APP_NAME ) );

   // Create the tested object(s):
   xAOD::TEvent event( xAOD::TEvent::kClassAccess );
   xAOD::TStore store;

   const char* ref = getenv( "ATLAS_REFERENCE_DATA" );
   const std::string FPATH =
      ref ? ref : "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art";
   const std::string FNAME1 = FPATH + "/CampaignInputs/mc23/AOD/"
      "mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon."
      "AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1";
   const std::string FNAME2 = FPATH + "/CampaignInputs/mc23/AOD/"
      "mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon."
      "AOD.e8514_s4162_r14622/1000events.AOD.33799166._000073.pool.root.1";

   // Set up a TChain with some mc23_13p6TeV input files:
   ::TChain chain1( "CollectionTree" );
   chain1.Add( FNAME1.c_str() );
   chain1.Add( FNAME2.c_str() );

   // Connect the TEvent object to it:
   ANA_CHECK( event.readFrom( &chain1 ) );

   // Run the processing:
   ::Info( APP_NAME, "Processing mc23_13p6TeV chain..." );
   ANA_CHECK( process( event, store ) );

   // Return gracefully:
   return 0;
}

StatusCode process( xAOD::TEvent& event, xAOD::TStore& store ) {

   // Loop over all events:
   const ::Long64_t entries = event.getEntries();
   for( ::Long64_t entry = 0; entry < entries; ++entry ) {

      // Clear the store:
      store.clear();

      // Read in the event:
      if( event.getEntry( entry ) < 0 ) {
         ::Error( "process", "Couldn't load entry %i",
                  static_cast< int >( entry ) );
         return StatusCode::FAILURE;
      }
      if( ! ( entry % 100 ) ) {
         ::Info( "process", "Processed %i / %i events",
                 static_cast< int >( entry ),
                 static_cast< int >( entries ) );
      }

      // Try to retrieve some objects:
      const xAOD::AuxContainerBase* c = 0;
      RETURN_CHECK( "process", event.retrieve( c, "ElectronsAux." ) );
      RETURN_CHECK( "process", event.retrieve( c, "MuonsAux." ) );
      const SG::AuxVectorBase* b = 0;
      RETURN_CHECK( "process", event.retrieve( b, "Electrons" ) );
      RETURN_CHECK( "process", event.retrieve( b, "Muons" ) );

      // Record some objects into TStore:
      ClassA* objA = new ClassA();
      RETURN_CHECK( "process", store.record( objA, "MyObjA" ) );
      ClassB* objB = new ClassB();
      RETURN_CHECK( "process", store.record( objB, "MyObjB" ) );

      // They should now be accessible through TEvent:
      const ClassA* dummy1 = 0;
      RETURN_CHECK( "process", event.retrieve( dummy1, "MyObjA" ) );
      const ClassB* dummy2 = 0;
      RETURN_CHECK( "process", event.retrieve( dummy2, "MyObjB" ) );
   }

   // Return gracefully:
   return StatusCode::SUCCESS;
}
