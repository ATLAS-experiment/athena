/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// a simple testing macro for the PixelToTPIDDualTool package
/// shamelessly stolen from MuonSelectorToolsTester.cxx

// System include(s):
#include <cstdlib>
#include <iomanip>
#include <map>
#include <memory>
#include <string>

// ROOT include(s):
#include <TError.h>
#include <TFile.h>
#include <TStopwatch.h>
#include <TString.h>

// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "AthContainers/ConstAccessor.h"

// Local include(s):
#include "PixelToTPIDDualTool/PixelToTPIDDualTool.h"


/// Example of how to run the PixelToTPIDDualTool package to obtain cluster and dE/dx information
int main(int argc, char* argv[]) {

  // Whether or not to equalize.  Maybe make this an argument.
  bool equalize = true;

  // The application's name:
  const char* APP_NAME = argv[0];

  // Check if we received a file name:
  if (argc < 2) {
    Error(APP_NAME, "No file name received!");
    Error(APP_NAME, "  Usage: %s [xAOD file name] [Nevts to process]", APP_NAME);
    return 1;
  }

  // Create a TEvent object:
#ifdef XAOD_STANDALONE
  if ( xAOD::Init( APP_NAME ).isFailure() ) {
    Error( APP_NAME, "Failed to do xAOD::Init!" );
    return 1;
  }
  xAOD::TEvent event( xAOD::TEvent::kClassAccess );
#else
  POOL::TEvent event( POOL::TEvent::kClassAccess );
#endif

  // Open the input file:
  const TString fileName = argv[1];
  Info(APP_NAME, "Opening file: %s", fileName.Data());
  std::unique_ptr<TFile> ifile(TFile::Open(fileName, "READ"));
  if ( !ifile.get() ) {
    Error( APP_NAME, "Failed to open input file!" );
    return 1;
  }

  //Read from input file with TEvent
  if ( event.readFrom( ifile.get() ).isFailure() ) {
    Error( APP_NAME, "Failed to read from input file!" );
    return 1;
  }
  Info(APP_NAME, "Number of events in the file: %i", static_cast<int>(event.getEntries()));

  // Decide how many events to run over:
  Long64_t entries = event.getEntries();
  if (argc > 2) {
    const Long64_t e = atoll(argv[2]);
    if (e < entries) { entries = e; }
  }

  // Get tool
  CP::PixelToTPIDDualTool* pidTool = new CP::PixelToTPIDDualTool("PixelToTPIDDualTool");
  pidTool->msg().setLevel(MSG::INFO);

  bool failed = false;
  failed = failed || pidTool->setProperty("EqualizeClusterMeasurements", equalize).isFailure();
  failed = failed || pidTool->initialize().isFailure();
  if (failed) {
    Error( APP_NAME, "Failed to set up PixelToTPIDDualTool!");
    return 1;
  }
  
  for (Long64_t entry = 0; entry < entries; ++entry) {
    // Tell the object which entry to look at:
    event.getEntry(entry);

    // Print some event information for fun:
    const xAOD::EventInfo* ei = 0;
    if ( event.retrieve( ei, "EventInfo" ).isFailure() ) {
      Error( APP_NAME, "Failed to read EventInfo!" );
      return 1;
    }

    // Get tracks
    const xAOD::TrackParticleContainer* tracks = 0;
    if ( event.retrieve( tracks, "InDetTrackParticles" ).isFailure() ) {
      Error( APP_NAME, "Failed to read track container!" );
      return 1;
    }
    Info(APP_NAME, "Number of tracks: %i", static_cast<int>(tracks->size()));

    int trkCounter = -1;

    for (const xAOD::TrackParticle* trkIt : *tracks  ) { 
      
      trkCounter ++;

      // Calculate dE/dx from clusters using tool
      int nUsedHits = -1;
      int nUsedIBLOverflowHits = -1;
      float dEdx = pidTool->dEdx(*trkIt, nUsedHits, nUsedIBLOverflowHits);

      // Get summary values for comparison
      float track_dEdx { 0 };
      unsigned char numberOfUsedHitsdEdx = -1;
      trkIt->summaryValue(track_dEdx, xAOD::pixeldEdx);
      numberOfUsedHitsdEdx = (unsigned int) (trkIt)->auxdataConst<unsigned char>("numberOfUsedHitsdEdx");

      // Check if the recalculated dE/dx matches the stored dE/dx
      // Only makes sense if pidTool is configured to return the raw dE/dx, not the equalized.
      float epsilon = 1e-3;
      if ( std::fabs(track_dEdx - dEdx) > epsilon ) {
        if( dEdx < 0.) {
          Info(APP_NAME, "===== Entry: %i, Track number: %i", static_cast<int>(entry), static_cast<int>(trkCounter));
          Info(APP_NAME, "Could not calculate truncated mean dE/dx from clusters.");
          Info(APP_NAME, "Clusters were likely not present or thinned away for this track.");
        }
        else if (!equalize) { // expect differences if equalizing.
          Info(APP_NAME, "===== Entry: %i, Track number: %i", static_cast<int>(entry), static_cast<int>(trkCounter));
          Info(APP_NAME, "Mismatch between recalculated track dE/dx and value stored in AOD.");
          Info(APP_NAME, "Likely from a migration in the cluster (x,y) between reco (ESD) and now (xAOD).");
          Info(APP_NAME, "Clusters too close to the edge of sensor not included in truncated mean.");
          Info(APP_NAME, "Track dE/dx (orig):        %g ", track_dEdx);
          Info(APP_NAME, "Track dE/dx (recalc):        %g ", dEdx);
          Info(APP_NAME, "Track nUsedHits:        %d ", nUsedHits);
          Info(APP_NAME, "Track nUsedHits (orig):        %u ", numberOfUsedHitsdEdx);
          Info(APP_NAME, "Track nUsedIBLOverflowHits:        %d ", nUsedIBLOverflowHits);
        }
      }
    } // done loop over tracks
    
    // Get clusters
    const xAOD::TrackMeasurementValidationContainer* clusters = 0;
    if ( event.retrieve( clusters, "PixelClusters" ).isFailure() ) {
      Error( APP_NAME, "Failed to read pixel cluster container!" );
      return 1;
    }
    Info(APP_NAME, "Number of clusters: %i", static_cast<int>(clusters->size()));

    for (const xAOD::TrackMeasurementValidation* clusIt : *clusters  ) { 
      float clusdEdxRaw = 0.;
      static const SG::AuxElement::ConstAccessor< float > clusdEdxRawAcc("dEdx");
      if (clusdEdxRawAcc.isAvailable(*clusIt)) {
        clusdEdxRaw = clusdEdxRawAcc(*clusIt);
        Info(APP_NAME, "cluster dEdx:        %g ", clusdEdxRaw);
      }
      else {
        Error( APP_NAME, "Could not find raw cluster dE/dx measurement!");
        return 1;
      }
      float clusdEdxEq = 0.;
      static const SG::AuxElement::ConstAccessor< float > clusdEdxEqAcc("dEdxEq");
      if (clusdEdxEqAcc.isAvailable(*clusIt)) {
        clusdEdxEq = clusdEdxEqAcc(*clusIt);
        Info(APP_NAME, "cluster dEdxEq:        %g ", clusdEdxEq);
      }
    }

    // Close with a message:
    Info(APP_NAME,
         "===>>>  done processing event #%i, "
         "run #%i %i events processed so far  <<<===",
         static_cast<int>(ei->eventNumber()), static_cast<int>(ei->runNumber()), static_cast<int>(entry + 1));

  } // done loop over events

  Info(APP_NAME, "======================================");
  Info(APP_NAME, "========= Full run summary ===========");
  Info(APP_NAME, "======================================");

  Info(APP_NAME, "Processed %i events", static_cast<int>(entries) );



  // Return gracefully:
  return 0;
}
