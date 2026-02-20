/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// A unit test for the track selection tool. Currently this compares the pre-defined cut levels
// to hard-coded versions that can be easily read. Does not check every possible cut option.

// System include(s):
#include <memory>
#include <cstdlib>
#include <cassert>

// ROOT include(s):
#include <TFile.h>
#include <TError.h>

// Infrastructure include(s):
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE

// Local include(s):
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"

using std::string;
using std::vector;
using std::map;
using std::unique_ptr;
using InDet::InDetTrackSelectionTool;
using xAOD::TrackParticle;
using tool_ptr = unique_ptr<InDetTrackSelectionTool>;

bool passNoCut( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passLoose( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passLoosePrimary( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passTightPrimary( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passLooseMuon( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passLooseElectron( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passLooseTau( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passMinBias( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passHILoose( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passHITight( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passHILooseOptimized( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passHITightOptimized( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
bool passExpPix( const TrackParticle& trk, const xAOD::Vertex* vtx = nullptr );
uint8_t getSum(const TrackParticle&, xAOD::SummaryType);
void dumpTrack( const TrackParticle& );

int main( int argc, char* argv[] ) {

   // The application's name:
   const char* APP_NAME = argv[ 0 ];
#define CHECK( ARG ) do {ASG_CHECK_SA( APP_NAME, ARG );} while (false)


   string filename;
   filename = getenv("ROOTCORE_TEST_FILE");
   if (filename.empty()) {
     Error( APP_NAME, "Could not find $ROOTCORE_TEST_FILE." );
     return 1;
   }

  // fail on an unchecked StatusCode
   StatusCode::enableFailure();

   // Initialise the application:
   ASG_CHECK_SA( APP_NAME, static_cast<StatusCode>(xAOD::Init( APP_NAME )) );

   map<string, tool_ptr> selTools;
   map<string, bool (*)(const TrackParticle&, const xAOD::Vertex*)> cutFuncs;

#define FUNC_HELP( CUT ) do {cutFuncs[ #CUT ] = pass##CUT;} while (false)
   FUNC_HELP( NoCut );
   FUNC_HELP( Loose );
   FUNC_HELP( LoosePrimary );
   FUNC_HELP( TightPrimary );
   FUNC_HELP( LooseMuon );
   FUNC_HELP( LooseElectron );
   FUNC_HELP( LooseTau );
   FUNC_HELP( MinBias );
   FUNC_HELP( HILoose );
   FUNC_HELP( HITight );
   FUNC_HELP( HILooseOptimized );
   FUNC_HELP( HITightOptimized );
#undef FUNC_HELP

   for (const auto& cutLevelPair : cutFuncs) {
     const auto& cutLevel = cutLevelPair.first;
     string toolName = "TrackSel";
     toolName += cutLevel;
     selTools[cutLevel] = tool_ptr( new InDetTrackSelectionTool(toolName) );
     CHECK( selTools[cutLevel]->setProperty( "CutLevel", cutLevel) );
     CHECK( selTools[cutLevel]->initialize() );
   }
   // handle the experimental one differently: add the map entry after initializing the others because it is not a selection level
   cutFuncs["ExpPix"] = passExpPix;
   selTools["ExpPix"] = tool_ptr( new InDetTrackSelectionTool("TrackSelExpPix") );
   CHECK( selTools["ExpPix"]->setProperty( "useExperimentalInnermostLayersCut", true ) );
   CHECK( selTools["ExpPix"]->initialize() );

   // Open the input file:
   Info( APP_NAME, "Opening file: %s", filename.data() );
   unique_ptr< TFile > ifile( TFile::Open( filename.data(), "READ" ) );
   StatusCode gotFile = ifile.get()!=nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE;
   CHECK( gotFile );

   // Create a TEvent object:
   // xAOD::TEvent event( static_cast<TFile*>(nullptr), xAOD::TEvent::kClassAccess );
   // ASG_CHECK_SA( APP_NAME, static_cast<StatusCode>(event.readFrom( ifile.get() )) );
   xAOD::TEvent event( ifile.get(), xAOD::TEvent::kAthenaAccess );
   Info( APP_NAME, "Number of events in the file: %llu", event.getEntries() );

   // Decide how many events to run over:
   Long64_t entries = event.getEntries();
   if( argc > 2 ) {
      const Long64_t e = atoll( argv[ 2 ] );
      if( e < entries ) {
         entries = e;
      }
   } else if( entries > 100 ) {
      entries = 100;
   }

   for (Long64_t entry = 0; entry < entries; ++entry) {
     CHECK( !event.getEntry(entry) );

     // the MinBias cut has IP cuts w.r.t. the primary vertex
     const xAOD::VertexContainer* vertices = nullptr;
     if ( !event.retrieve( vertices, "PrimaryVertices" ) ) {
       Error( APP_NAME, "Could not retrieve primary vertices." );
       return 2;
     }
     const xAOD::Vertex* primaryVertex = nullptr;
     auto find_vtx = std::find_if(vertices->cbegin(), vertices->cend(),
				  [](const xAOD::Vertex* vtx)
				  {return vtx->vertexType() == xAOD::VxType::PriVtx;} );
     if (find_vtx == vertices->cend()) Warning( APP_NAME, "No primary vertex." );
     else primaryVertex = *find_vtx;

     const xAOD::TrackParticleContainer* tracks = nullptr;
     if ( !event.retrieve( tracks, "InDetTrackParticles" ) ) {
       Error( APP_NAME, "Could not retrieve track particles." );
       return 2;
     }

     for (const auto track : *tracks) {
       for (const auto& cutLevelPair : cutFuncs) {
	 const auto& cutLevel = cutLevelPair.first;
         asg::AcceptData taccept = selTools[cutLevel]->accept( *track, primaryVertex );
	 if ( bool (taccept) != cutFuncs[cutLevel]( *track, primaryVertex ) ) {
	   Error( APP_NAME, "Track selection tool at %s cut level does not", cutLevel.data() );
	   Error( APP_NAME, "  match hard-coded test function." );
	   if (taccept) Error( APP_NAME, "Passes tool but not function" );
	   else Error( APP_NAME, "Passes function but not tool" );

	   for (size_t i_cut=0; i_cut < taccept.getNCuts(); ++i_cut) {
	     const auto& cutName = taccept.getCutName(i_cut);
	     Info( APP_NAME, "Result of %s = %i", cutName.c_str(), taccept.getCutResult(i_cut) );
	   }

	   dumpTrack( *track ); // output track info

	   assert(false);
	 }
       }
     }

   } // end loop over events

   // finalize all the tools
   for (const auto& cutLevelPair : cutFuncs) {
     CHECK( selTools[cutLevelPair.first]->finalize() );
   }

   // Return gracefully:
   return 0;
}

// The functions are implemented in an easy-to-read form
bool passNoCut( const TrackParticle&, const xAOD::Vertex* )
{
  return true;
}

bool passLoose( const TrackParticle& trk, const xAOD::Vertex* )
{
  if (std::fabs(trk.eta()) > 2.5) return false;

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  if (nPixHits + nSctHits < 7) return false;

  uint8_t nPixShared = getSum(trk, xAOD::numberOfPixelSharedHits);
  uint8_t nSctShared = getSum(trk, xAOD::numberOfSCTSharedHits); // need two shared SCT hits to count for one
  if (nPixShared + nSctShared/2 > 1) return false;

  uint8_t nPixHoles = getSum(trk, xAOD::numberOfPixelHoles);
  uint8_t nSiHoles = nPixHoles + getSum(trk, xAOD::numberOfSCTHoles);
  if (nSiHoles > 2) return false;
  if (nPixHoles > 1) return false;

  return true;
}

bool passLoosePrimary( const TrackParticle& trk, const xAOD::Vertex* )
{
  if (!passLoose(trk)) return false;
  uint8_t nPixShared = getSum(trk, xAOD::numberOfPixelSharedHits);
  uint8_t nSctShared = getSum(trk, xAOD::numberOfSCTSharedHits); // need two shared SCT hits to count for one
  if (nPixShared + nSctShared > 0) {
    uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits)
      + getSum(trk, xAOD::numberOfPixelDeadSensors);
    uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits)
      + getSum(trk, xAOD::numberOfSCTDeadSensors);
    if (nPixHits + nSctHits < 10) return false;
  }

  return true;
}

bool passTightPrimary( const TrackParticle& trk, const xAOD::Vertex* )
{
  if (!passLoose(trk)) return false;

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  uint8_t minSiHits = (std::fabs(trk.eta()) <= 1.65) ? 9 : 11;
  if (nPixHits + nSctHits < minSiHits) return false;

  bool expectInnermost = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectNextToInnermost = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  if (expectInnermost && expectNextToInnermost) {
    uint8_t nInnermost = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
    uint8_t nNextToInnermost = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
    if (nInnermost + nNextToInnermost < 1) return false;
  }

  uint8_t nPixHoles = getSum(trk, xAOD::numberOfPixelHoles);
  if (nPixHoles > 0) return false;

  return true;
}

bool passLooseMuon( const TrackParticle& trk, const xAOD::Vertex* )
{
  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 1) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  if (nSctHits < 5) return false;
  uint8_t nSiHoles = getSum(trk, xAOD::numberOfPixelHoles) + getSum(trk, xAOD::numberOfSCTHoles);
  if (nSiHoles > 2) return false;
  auto absEta = std::fabs(trk.eta());
  if (0.1 < absEta && absEta < 1.9) {
    uint8_t nTrtHits = getSum(trk, xAOD::numberOfTRTHits);
    uint8_t nTrtOutliers = getSum(trk, xAOD::numberOfTRTOutliers);
    if (nTrtHits + nTrtOutliers < 6) return false;
    if (nTrtOutliers >= 0.9*(nTrtHits + nTrtOutliers)) return false;
  }

  return true;
}

bool passLooseElectron( const TrackParticle& trk, const xAOD::Vertex* )
{
  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 1) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  if (nPixHits + nSctHits < 7) return false;

  return true;
}

bool passLooseTau( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  if (trk.pt() < 1000.0) return false; // pT cut at 1 GeV

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 2) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  if (nPixHits + nSctHits < 7) return false;

  if (std::fabs(trk.d0()) > 1.0) return false;
  if (vtx != nullptr) {
    if (std::fabs(trk.z0() + trk.vz() - vtx->z()) > 1.5) return false;
  }

  return true;
}

bool passMinBias( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  if (std::fabs(trk.eta()) > 2.5) return false;
  if (trk.pt() < 500.) return false;

  bool expectIBL = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nIBL = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t nBL = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
  if (expectIBL) {
    if (nIBL < 1) return false;
  } else {
    if (expectBL && nBL < 1) return false;
  }

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 1) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  if (nSctHits < 6) return false;

  if (trk.pt() > 10.0*1e3 && TMath::Prob(trk.chiSquared(), trk.numberDoF()) < 0.01) return false;

  if (std::fabs(trk.d0()) > 1.5) return false;
  if (vtx != nullptr) {
    if (std::fabs(trk.z0() + trk.vz() - vtx->z())*std::sin(trk.theta()) > 1.5) return false;
  }

  return true;
}

bool passHILoose( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  if (std::fabs(trk.eta()) > 2.5) return false;
  bool expectIBL = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nIBL = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t nBL = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
  if (expectIBL) {
    if (nIBL < 1) return false;
  } else {
    if (expectBL && nBL < 1) return false;
  }

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 1) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  auto pt = trk.pt()*1e-3;
  if (pt >= 0.4 && nSctHits < 6) return false;
  else if (pt >= 0.3 && nSctHits < 4) return false;
  else if (nSctHits < 2) return false;

  if (std::fabs(trk.d0()) > 1.5) return false;
  if (vtx != nullptr) {
    if (std::fabs(trk.z0() + trk.vz() - vtx->z())*std::sin(trk.theta()) > 1.5) return false;
  }

  return true;
}

bool passHITight( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  if (std::fabs(trk.eta()) > 2.5) return false;
  bool expectIBL = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nIBL = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t nBL = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
  if (expectIBL) {
    if (nIBL < 1) return false;
  } else {
    if (expectBL && nBL < 1) return false;
  }

  uint8_t nPixHits = getSum(trk, xAOD::numberOfPixelHits) + getSum(trk, xAOD::numberOfPixelDeadSensors);
  if (nPixHits < 2) return false;
  uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
  auto pt = trk.pt()*1e-3;
  if (pt >= 0.4 && nSctHits < 8) return false;
  else if (pt >= 0.3 && nSctHits < 6) return false;
  else if (nSctHits < 4) return false;

  if (std::fabs(trk.d0()) > 1.0) return false;
  if (vtx != nullptr) {
    if (std::fabs(trk.z0() + trk.vz() - vtx->z())*std::sin(trk.theta()) > 1.0) return false;
  }

  if (trk.chiSquared() / trk.numberDoF() > 6.0) return false;

  return true;
}

bool passHILooseOptimized( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  auto eta=std::fabs(trk.eta());
  if (eta > 2.5) return false;

  uint8_t nPixHoles = getSum(trk, xAOD::numberOfPixelHoles);
  if (nPixHoles > 0) return false;

  bool expectIBL = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nIBL = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t nBL = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
  if (expectIBL) {
    if (nIBL < 1) return false;
  } else {
    if (expectBL && nBL < 1) return false;
  }

  std::vector<double> vEta = {0.0, 1.1,1.6,2.0};
  std::vector<double> vPt  = {500, 600, 700, 800, 900, 1000, 1500,
                              2000,2500,3000,5000,8000,12000};
  std::vector<std::vector<double>> vvMaxZ0sT = {{2.10,2.15,6.00,5.00,3.10,2.00,1.75,1.60,1.43,1.40,1.05,0.65,0.60},
                                                {1.44,1.47,1.50,1.55,1.62,1.45,1.45,1.78,1.73,1.50,1.20,0.97,0.53},
                                                {1.40,1.45,1.50,1.46,1.41,1.37,1.25,1.50,1.50,1.36,1.10,0.85,0.52},
                                                {1.51,1.70,1.70,1.71,1.71,1.53,1.54,1.49,1.36,1.20,0.95,0.60,0.55}};
  std::vector<std::vector<double>> vvMaxD0 = {{0.81,0.90,0.94,0.92,0.90,0.75,0.65,0.63,0.62,0.60,0.63,0.50,0.55},
                                              {1.00,0.98,0.98,0.92,0.90,0.69,0.67,0.86,0.88,0.88,0.88,0.87,1.06},
                                              {1.19,1.15,1.10,1.08,1.03,0.94,0.85,0.97,0.97,0.96,0.95,0.92,1.04},
                                              {1.33,1.23,1.21,1.15,1.15,1.07,0.94,0.97,0.97,0.97,0.98,1.10,1.10}};
  auto pt=std::fabs(trk.pt());
  if(eta>vEta.at(0) && pt>vPt.at(0))
  {
    uint8_t eta_bin=0;
    for(;eta_bin<vEta.size()-1;++eta_bin)
    {
      if(vEta.at(eta_bin)<=eta && eta<vEta.at(eta_bin+1))
      {
        break;
      }
    }
    uint8_t pt_bin=0;
    for(;pt_bin<vPt.size()-1;++pt_bin)
    {
      if(vPt.at(pt_bin)<=pt && pt<vPt.at(pt_bin+1))
      {
        break;
      }
    }

    if (vtx != nullptr) {
      if (std::fabs(trk.z0()+trk.vz()-vtx->z())*std::sin(trk.theta()) > vvMaxZ0sT.at(eta_bin).at(pt_bin)) return false;
    }
    if(std::abs(trk.d0())>vvMaxD0.at(eta_bin).at(pt_bin)) return false;
  }

  return true;
}

bool passHITightOptimized( const TrackParticle& trk, const xAOD::Vertex* vtx )
{
  auto eta=std::fabs(trk.eta());
  if (eta > 2.5) return false;

  uint8_t nPixHoles = getSum(trk, xAOD::numberOfPixelHoles);
  if (nPixHoles > 0) return false;

  bool expectIBL = getSum(trk, xAOD::expectInnermostPixelLayerHit);
  bool expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nIBL = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t nBL = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);
  if (expectIBL) {
    if (nIBL < 1) return false;
  } else {
    if (expectBL && nBL < 1) return false;
  }

  std::vector<double> vEta = {0.0,1.1,1.6,2.0};
  std::vector<double> vPt  = {500, 600, 700, 800, 900, 1000, 1500,
                              2000,2500,3000,5000,8000,12000};
  std::vector<std::vector<double>> vvMaxZ0sT = {{0.62,0.70,0.82,0.87,0.74,0.61,0.50,0.48,0.46,0.45,0.30,0.24,0.23},
                                                {0.51,0.53,0.53,0.53,0.52,0.43,0.28,0.27,0.28,0.30,0.24,0.22,0.13},
                                                {0.91,0.89,0.87,0.55,0.59,0.37,0.39,0.31,0.34,0.35,0.30,0.30,0.20},
                                                {0.76,0.71,0.69,0.48,0.48,0.47,0.46,0.42,0.38,0.32,0.28,0.20,0.15}};
  std::vector<std::vector<double>> vvMaxD0 = {{0.34,0.39,0.47,0.49,0.55,0.47,0.44,0.21,0.19,0.17,0.12,0.14,0.15},
                                              {0.32,0.32,0.33,0.33,0.33,0.27,0.16,0.15,0.13,0.15,0.13,0.16,0.20},
                                              {0.95,0.91,0.88,0.35,0.37,0.24,0.26,0.22,0.23,0.24,0.19,0.19,0.23},
                                              {0.68,0.67,0.65,0.42,0.42,0.36,0.35,0.31,0.27,0.26,0.27,0.28,0.30}};
  std::vector<std::vector<double>> vvMaxSctHoles = {{0,0,0,0,0,0,0,1,1,1,1,1,1},
                                                    {0,0,0,0,0,0,1,1,1,1,1,1,1},
                                                    {1,1,1,1,1,1,1,1,1,1,1,1,1},
                                                    {1,1,1,1,1,1,1,1,1,1,1,1,1}};                           
  std::vector<std::vector<double>> vvMinSctHits = {{0,0,0,0,0,0,0,0,0,0,0,0,0},
                                                   {0,0,0,0,0,0,6,6,6,6,0,0,0},
                                                   {8,8,8,7,7,6,6,6,6,6,0,0,0},
                                                   {7,7,7,0,0,0,0,0,0,0,0,0,0}};
  auto pt=std::fabs(trk.pt());
  if(eta>vEta.at(0) && pt>vPt.at(0))
  {
    uint8_t eta_bin=0;
    for(;eta_bin<vEta.size()-1;++eta_bin)
    {
      if(vEta.at(eta_bin)<=eta && eta<vEta.at(eta_bin+1))
      {
        break;
      }
    }
    uint8_t pt_bin=0;
    for(;pt_bin<vPt.size()-1;++pt_bin)
    {
      if(vPt.at(pt_bin)<=pt && pt<vPt.at(pt_bin+1))
      {
        break;
      }
    }

    if (vtx != nullptr) {
      if (std::fabs(trk.z0()+trk.vz()-vtx->z())*std::sin(trk.theta()) > vvMaxZ0sT.at(eta_bin).at(pt_bin)) return false;
    }
    if(std::abs(trk.d0())>vvMaxD0.at(eta_bin).at(pt_bin)) return false;
    uint8_t nSctHoles = getSum(trk, xAOD::numberOfSCTHoles);
    if (nSctHoles > vvMaxSctHoles.at(eta_bin).at(pt_bin)) return false;
    uint8_t nSctHits = getSum(trk, xAOD::numberOfSCTHits) + getSum(trk, xAOD::numberOfSCTDeadSensors);
    if (nSctHits < vvMinSctHits.at(eta_bin).at(pt_bin)) return false;
  }

  return true;
}

// whether the track passes the experimental pixel cut
bool passExpPix( const TrackParticle& trk, const xAOD::Vertex* )
{
  uint8_t nPixHoles = getSum(trk, xAOD::numberOfPixelHoles);
  if (nPixHoles == 0) return true; // if there are no pixel holes, then the track passes regardless
  if (nPixHoles > 1) return false; // if there is more than 1 hole, the track fails regardless

  uint8_t nIBLHits = getSum(trk, xAOD::numberOfInnermostPixelLayerHits);
  uint8_t expectBL = getSum(trk, xAOD::expectNextToInnermostPixelLayerHit);
  uint8_t nBLHits = getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits);

  // make an exception is there is an IBL hit, and the hole is in the BLayer
  if (nIBLHits >= 1) {
    if (expectBL && nBLHits ==0) return true;
  }

  return false;
}

uint8_t getSum( const TrackParticle& trk, xAOD::SummaryType sumType )
{
  uint8_t sumVal=0;
  if (!trk.summaryValue(sumVal, sumType)) {
    Error( "getSum()", "Could not get summary type %i", sumType );
  }
  return sumVal;
}

void dumpTrack( const TrackParticle& trk )
{
  Info( "dumpTrack()", "pt = %f", trk.pt() );
  Info( "dumpTrack()", "eta = %f", trk.eta() );
  Info( "dumpTrack()", "expect ibl = %u", getSum(trk, xAOD::expectInnermostPixelLayerHit) );
  Info( "dumpTrack()", "ibl hit = %u", getSum(trk, xAOD::numberOfInnermostPixelLayerHits) );
  Info( "dumpTrack()", "expect bl = %u", getSum(trk, xAOD::expectNextToInnermostPixelLayerHit) );
  Info( "dumpTrack()", "bl hit = %u", getSum(trk, xAOD::numberOfNextToInnermostPixelLayerHits) );
  Info( "dumpTrack()", "pix hit = %u", getSum(trk, xAOD::numberOfPixelHits) );
  Info( "dumpTrack()", "pix dead = %u", getSum(trk, xAOD::numberOfPixelDeadSensors) );
  Info( "dumpTrack()", "pix holes = %u", getSum(trk, xAOD::numberOfPixelHoles) );
  Info( "dumpTrack()", "sct hit = %u", getSum(trk, xAOD::numberOfSCTHits) );
  Info( "dumpTrack()", "sct dead = %u", getSum(trk, xAOD::numberOfSCTDeadSensors) );
  Info( "dumpTrack()", "sct holes = %u", getSum(trk, xAOD::numberOfSCTHoles) );
  Info( "dumpTrack()", "trt hits = %u", getSum(trk, xAOD::numberOfTRTHits) );
  Info( "dumpTrack()", "trt outliers = %u", getSum(trk, xAOD::numberOfTRTOutliers) );
}
