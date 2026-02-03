/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 *  @file   BdKstarMuMu_metadata.cxx
 *  @author Pavel Reznicek <pavel.reznicek@cern.ch>
 */

#include "DerivationFrameworkBPhys/BdKstarMuMu_metadata.h"

namespace DerivationFramework {

  BdKstarMuMu_metadata::BdKstarMuMu_metadata(const std::string& t,
                                             const std::string& n,
                                             const IInterface*  p):
    AthAlgTool(t,n,p), BPhysMetadataBase(t,n,p) {

      recordPropertyI("verbose"      , 0);          // verbose athena output
      recordPropertyB("isSimulation" , false);      // input data is MC simulation or real data
      recordPropertyS("projectTag"   , "__NONE__"); // copy reconstruction project tag
      recordPropertyB("isRelease21"  , true);       // is processed in release 21
      recordPropertyS("mcCampaign"   , "__NONE__"); // what MC campaign is used
      recordPropertyS("triggerStream", "__NONE__"); // what data stream is analyzed
      recordPropertyI("runNumber"    , -1);         // run number from the input file

      recordPropertyB("looseCuts"   , false); // apply loose cuts (debugging only)
      recordPropertyB("skimTrig"    , false); // skim data by selected triggers
      recordPropertyB("skimData"    , false); // skim data by passed B-candidates
      recordPropertyB("thinData"    , false); // thin ID tracks, muons and PVs
      recordPropertyB("slimData"    , false); // TODO: data slimming
      recordPropertyB("thinMC"      , false); // thin MC-truth (keep wide range of heavy hadrons)
      recordPropertyB("thinMCsignal", false); // thin MC-truth to signal-only (keep only signal PDG b-hadrons)
      recordPropertyB("trigObjects" , false); // store trigger objects for B-physics and muons

      recordPropertyS("version", "v1.0"); // derivation version (update with every update of the derivation)

      /*
        Constants (in sync with the JpsiUpsilonTools)
      */
      recordPropertyD("mass_mu"   ,  105.658); // PDG: 105.6583745
      recordPropertyD("mass_e"    ,    0.511); // PDG:   0.5109989461
      recordPropertyD("mass_K"    ,  493.677); // PDG: 493.677
      recordPropertyD("mass_pi"   ,  139.57 ); // PDG: 139.57039
      recordPropertyD("mass_p"    ,  938.272); // PDG: 938.272081
      recordPropertyD("mass_Jpsi" , 3096.916); // PDG:3096.900
      recordPropertyD("mass_Kstar",  895.55 ); // NOTE!! Corrected since 21.2.189.0+
      recordPropertyD("mass_Bd"   , 5279.65 ); // PDG:5279.65
      recordPropertyD("mass_Bs"   , 5366.88 ); // PDG:5366.88

      /*
     N.B.:
     -----
     isoTrackWorkingPoints and isoTrackMinPts are interpreted in
     correspondence w/ each other, i.e., k-th isoTrackWorkingPoint will
     be used with k-th isoTrackMinPt.
     */
      recordPropertyVS ( "isoTrackWorkingPoints" , { "Loose" } );
      recordPropertyVD ( "isoTrackMinPts"        , {   500.0 } );
      recordPropertyS  ( "isoTTVAWorkingPoint"   ,    "Loose"  );


     /* Isolation/Multiplicity Calculation */
     /* ---------------------------------- */

      recordPropertyB( "isoMultOnlyInVertex", false ); // CLI Flag

     /*
      isoTargetLegTypes -> Calculate isolation for:
      0: Only muon legs in the vertex
      1: Only electron legs in the vertex
      2: All legs in the vertex
      */
      recordPropertyI  ( "isoTargetLegTypes" , 2 );
      recordPropertyVD ( "isoConeSizes"      , { 0.1, 0.2, 0.3, 0.4, 0.5 } );

      // 0: NoVtx, 1: Primary, 2: Secondary, 3: Pileup, 4: Conversion
      // See: https://acode-browser.usatlas.bnl.gov/lxr/source/athena/Tracking/TrkEvent/TrkEventPrimitives/TrkEventPrimitives/VertexType.h?v=21.2#0024
      // Used as `PVTypesToConsider` in `BPhysVertexTrackBase.cxx`
      recordPropertyVD ( "isoTTVALogChi2CutValues", { 5.0, 0. } );
      recordPropertyVI ( "isoTTVAChi2CutTypes"    , {  2, 0  } );
      recordPropertyVI ( "isoPVTypesForTTVA"      , {  1, 3 } );
      recordPropertyI  ( "isoPVSVAssocType"       , 2 ); // 2 is minA0

      recordPropertyVI(
        "isoTrackTypes",
        {
          1,
          // 1: Associated w/ PV associated w/ Candidate
          
          ( 1 << 0 ) | ( 1 << 1 ) | ( 1 << 5 ),
          // 35: Associated w/ PV associated w/ Candidate
          // OR Associated w/ Dummy PV
          // OR Associated w/ PV other than Primary, Secondary, Pileup
          
          ( 1 << 0 ) | ( 1 << 1 ) | ( 1 << 2 ) |\
          ( 1 << 3 ) | ( 1 << 4 ) | ( 1 << 5 ) |\
          ( 1 << 6 ), //| (1 << 23) | (1 << 24) | (1 << 27) | (1 << 28),
          // 127: All Tracks, NO PV Association!!!
          // 1 << 23,
          // 8388608: Associated w/ Refitted PV associated w/ Candidate
          // 1 << 24,
          // 16777216: Associated w/ PV associated w/ Candidate
          // w/ minNumTracks = 0 => Equivalent to 1?
          
          1 << 27,
          // 134217728: Min chi2 PV is the same as the refitted PV associated w/
          // candidate, 3D chi2 from track perigee using uncertainties
          // from both track and vertex
          
          1 << 28
          // 268435456: Min chi2 PV is the same as the PV associated w/
          // candidate, 3D chi2 from track perigee using uncertainties
          // from both track and vertex
        }
      );




    } // AthAlgTool
} // namespace


