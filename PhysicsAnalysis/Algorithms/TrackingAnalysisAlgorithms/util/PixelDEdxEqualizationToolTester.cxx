/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// a simple testing macro for the PixelDEdxEqualizationTool package
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
#include "TrackingAnalysisAlgorithms/PixelDEdxEqualizationTool.h"


/// Example of how to run the PixelDEdxEqualizationTool package to obtain cluster and dE/dx information
int main(int argc, char* argv[]) {

  using StatesOnTrack = std::vector<ElementLink<xAOD::TrackStateValidationContainer>>;

  // Default arguments
  bool clusterEqualize = false; // Whether or not to equalize the dE/dx at cluster level.
  bool trackEqualize = false; // Whether or not to equalize the dE/dx at track level.
  int maxEvents = 1; // Max number of events to process.

  // Path to equalization SF trees.
  // Will eventualy not need to set this property.
  // Will instead rely on default properties pointing to the file in the ASG calibration area.
  std::string localSFPath = "../athena/PhysicsAnalysis/Algorithms/TrackingAnalysisAlgorithms/share/pixeldEdxEqualizationSFs_v0.root"; // FIXME!

  // Name of link from track to MSOS.
  std::string msosLinkName = "Reco_msosLink";

  // The application's name:
  const char* APP_NAME = argv[0];

  // Check if we received a file name:
  if (argc < 2) {
    Error(APP_NAME, "No file name received!");
    Error(APP_NAME, "Usage: %s <xAOD file name> [--equalize] [--maxEvents N]", APP_NAME);
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

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--cluster") {
      clusterEqualize = true;
    } 
    else if (arg == "--track") {
      trackEqualize = true;
    }
    else if (arg == "--maxEvents") {
      if (i + 1 < argc) {
        maxEvents = std::stoll(argv[++i]);
      } else {
        std::cerr << "--maxEvents requires a number\n";
        return 1;
      }
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      return 1;
    }
  }
  
  if(clusterEqualize && trackEqualize) {
    std::cerr << "Must choose --cluster OR --track, not both!\n";
    return 1;
  }
  if(!clusterEqualize && !trackEqualize) {
    std::cerr << "Must choose --cluster OR --track!\n";
    return 1;
  }

  // Decide how many events to run over:
  Long64_t entries = event.getEntries();
  if (maxEvents>0 && maxEvents<entries) {
    entries = maxEvents;
  }

  // Get tool
  CP::PixelDEdxEqualizationTool* dEdxEqTool = new CP::PixelDEdxEqualizationTool("PixelDEdxEqualizationTool");
  dEdxEqTool->msg().setLevel(MSG::INFO);

  bool failed = false;
  failed = failed || dEdxEqTool->setProperty("EqualizeClusterMeasurements", clusterEqualize).isFailure();
  failed = failed || dEdxEqTool->setProperty("EqualizeTrackMeasurements", trackEqualize).isFailure();
  failed = failed || dEdxEqTool->setProperty("SFLocalFileName", localSFPath).isFailure();  // FIXME!  Eventually won't need.
  failed = failed || dEdxEqTool->initialize().isFailure();
  if (failed) {
    Error( APP_NAME, "Failed to set up PixelDEdxEqualizationTool!");
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

      // Print some info
      Info(APP_NAME, "===== Entry: %i, Track number: %i", static_cast<int>(entry), static_cast<int>(trkCounter));

      // Calculate dE/dx metrics & decorate tracks+clusters using tool
      int nUsedHits = -1;
      int numberOfIBLOverflowsdEdx = -1;
      float dEdx = -1;
      float stdDev = -1;
      /*
      StatusCode sc = dEdxEqTool->dEdx(*trkIt);
      if (sc.isFailure()) {
        Error( APP_NAME, "PixelDEdxEqualizationTool::dEdx failed!");
        return StatusCode::FAILURE;
      }
      */

      // Now access newly decorated dE/dx metrics and counters.
      std::string trackdEdxEqName = "pixeldEdx";
      std::string trackdEdxEqStdDevName = "pixeldEdxStdDev";
      std::string nUsedName = "numberOfUsedHitsdEdx";
      std::string iblofName = "numberOfIBLOverflowsdEdx";
      if(clusterEqualize) {
        trackdEdxEqName += "ClusterEqualized"; // hardcode?
        trackdEdxEqStdDevName += "ClusterEqualized"; // hardcode?
        nUsedName += "ClusterEqualized"; // hardcode?
        iblofName += "ClusterEqualized"; // hardcode?
      } else if(trackEqualize) {
        trackdEdxEqName += "TrackEqualized"; // hardcode?
        trackdEdxEqStdDevName += "TrackEqualized"; // hardcode?
        nUsedName += "TrackEqualized"; // hardcode?
        iblofName += "TrackEqualized"; // hardcode?
      } else { //redundant with check above.
        Error(APP_NAME, "Must choose to equalize the dE/dx measurements at cluster-level OR track-level.");
      }
      static const SG::AuxElement::ConstAccessor< float > trackdEdxEqAcc(trackdEdxEqName);
      if (trackdEdxEqAcc.isAvailable(*trkIt)) {
        dEdx = trackdEdxEqAcc(*trkIt);
      }
      // Next 3 won't be available if equalizing at track level
      // Since pixel clusters are required to redo the trunc mean & std dev calc here.
      static const SG::AuxElement::ConstAccessor< float > trackdEdxEqStdDevAcc(trackdEdxEqStdDevName);
      if (trackdEdxEqStdDevAcc.isAvailable(*trkIt)) {
        stdDev = trackdEdxEqStdDevAcc(*trkIt);
      }
      static const SG::AuxElement::ConstAccessor< int > nUsedAcc(nUsedName);
      if (nUsedAcc.isAvailable(*trkIt)) {
        nUsedHits = nUsedAcc(*trkIt);
      }
      static const SG::AuxElement::ConstAccessor< int > iblofAcc(iblofName);
      if (iblofAcc.isAvailable(*trkIt)) {
        numberOfIBLOverflowsdEdx = iblofAcc(*trkIt);
      }


      // Get summary values for comparison
      float stored_dEdx { 0 };
      unsigned char stored_numberOfUsedHitsdEdx = -1;
      unsigned char stored_numberOfIBLOverflowsdEdx = -1;
      trkIt->summaryValue(stored_dEdx, xAOD::pixeldEdx);
      stored_numberOfUsedHitsdEdx = (unsigned int) (trkIt)->auxdataConst<unsigned char>("numberOfUsedHitsdEdx");
      stored_numberOfIBLOverflowsdEdx = (unsigned int) (trkIt)->auxdataConst<unsigned char>("numberOfIBLOverflowsdEdx");

      // Print some info
      if( dEdx < 0.) {
        Info(APP_NAME, "Invalid track dE/dx found by tool.");
        if (clusterEqualize) {
          Info(APP_NAME, "Perhaps clusters were not present or thinned away for this track.");
        }
        Info(APP_NAME, "Track dE/dx from AOD: %g", stored_dEdx);
        continue;
      }
      
      // Check if the hit counters differ between now (xAOD) and reco (ESD).
      Info(APP_NAME, "Track dE/dx (orig):          %g", stored_dEdx);
      Info(APP_NAME, "Track dE/dx (EQ):        %g", dEdx);
      if( ((int) stored_numberOfUsedHitsdEdx != nUsedHits) || ((int) stored_numberOfIBLOverflowsdEdx != numberOfIBLOverflowsdEdx) ) {
        Info(APP_NAME, "Mismatch in either numberOfUsedHitsdEdx or numberOfIBLOverflowsdEdx!");
        Info(APP_NAME, "Likely from a migration in the cluster (x,y) between reco (ESD) and now (xAOD).");
        Info(APP_NAME, "Clusters too close to the edge of sensor not included in truncated mean.");
        Info(APP_NAME, "Track nUsedHits (orig):        %u ", stored_numberOfUsedHitsdEdx);
        Info(APP_NAME, "Track nUsedHits (EQ):      %d ", nUsedHits);
        Info(APP_NAME, "Track numberOfIBLOverflowsdEdx (orig):    %u ", numberOfIBLOverflowsdEdx);
        Info(APP_NAME, "Track numberOfIBLOverflowsdEdx (EQ):  %d ", stored_numberOfIBLOverflowsdEdx);
      }

      // If using track-level equalization, don't try to find linked pixel clusters.
      if( trackEqualize ) {
        continue;
      }

      // Follow links from track -> MSOSs -> clusters and check that they are decorated with the dE/dx.
      // Better to follow links since only clusters belonging to this track collection will be decorated.

      // Check for track states:
      static const SG::AuxElement::ConstAccessor< StatesOnTrack > trackStateAcc(msosLinkName);
      if( ! trackStateAcc.isAvailable( *trkIt ) ) {
        Info(APP_NAME,"Cannot find TrackState link from xAOD::TrackParticle. Skipping track.");
        return -1;
      }
      const StatesOnTrack& measurementsOnTrack = trackStateAcc(*trkIt);

      // Loop over MSOS.
      for( const ElementLink<xAOD::TrackStateValidationContainer>& msos : measurementsOnTrack) {
        if (not msos.isValid()) {
          continue; //not a valid link.  Can happen if clusters are thinned away via ThinInDetClustersAlg.
        }
        if ((int) (*msos)->detType() != 1) {
          continue; // not a pixel cluster. See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs.h
        }
        if ( (*msos)->type()!=0) {
          continue; // not fittable.  See Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/TrackStateDefs. Want this?
        }
      
        // Get the corresponding TrackMeasurementValidation object (cluster/drift tube)
        const ElementLink<xAOD::TrackMeasurementValidationContainer> pixclus = (*msos)->trackMeasurementValidationLink();
        if (not pixclus.isValid()) {
          continue; //not a valid link
        }
        if (*pixclus == nullptr) {
          continue; //not linking to a valid object -- is it necessary?
        }
      
        // Get cluster info
        int bec = -99;
        int layer = -99;
        float locx = -999.;
        float locy = -999.;
        float clusdEdxRaw = 0.;
        float clusdEdxEq = 0.;
        static const SG::AuxElement::ConstAccessor< int > becAcc("bec");
        if (becAcc.isAvailable(**pixclus)) {
          bec  = becAcc(**pixclus);
        } else {
          Error( APP_NAME,"bec auxdata is missing!");
          continue;
        }
        static const SG::AuxElement::ConstAccessor< int > layerAcc("layer");
        if (layerAcc.isAvailable(**pixclus)) {
          layer = layerAcc(**pixclus);
        } else {
          Error( APP_NAME, "layer auxdata is missing!");
          continue;
        }
        static const SG::AuxElement::ConstAccessor< float > localXAcc("localX");
        if (localXAcc.isAvailable(**pixclus)) {
          locx = localXAcc(**pixclus);
        } else {
          Error( APP_NAME,"localX auxdata is missing!");
          continue;
        }
        static const SG::AuxElement::ConstAccessor< float > localYAcc("localY");
        if (localYAcc.isAvailable(**pixclus)) {
          locy = localYAcc(**pixclus);
        } else {
          Error( APP_NAME,"localY auxdata is missing!");
          continue;
        }
        static const SG::AuxElement::ConstAccessor< float > clusdEdxRawAcc("dEdx");
        if (clusdEdxRawAcc.isAvailable(**pixclus)) {
          clusdEdxRaw = clusdEdxRawAcc(**pixclus);
        }
        else {
          Error( APP_NAME, "Could not find raw cluster dE/dx measurement!");
          return 1;
        }
        Info(APP_NAME, "cluster dEdx:  %g,     bec:  %i,    layer:  %i,    local (x,y): (%g, %g) ", clusdEdxRaw, bec, layer,  locx, locy);

        static const SG::AuxElement::ConstAccessor< float > clusdEdxEqAcc("dEdxEq");
        if (clusdEdxEqAcc.isAvailable(**pixclus)) {
          clusdEdxEq = clusdEdxEqAcc(**pixclus);
          Info(APP_NAME, "cluster dEdxEq:        %g ", clusdEdxEq);
        }

      } // end msos loop

    } // done loop over tracks
    
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
