// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "DerivationFrameworkBPhys/BeeKstMetaData.h"

namespace DerivationFramework {
  //--------------------------------------------------------------------------
  BeeKstMetaData::BeeKstMetaData(const std::string& t,
				 const std::string& n,
				 const IInterface*  p) : 
    AthAlgTool(t,n,p), BPhysMetadataBase(t,n,p) {

    // Configuration for Vertexing
    recordPropertyB( "runGSFCalo"          , true      ); // CLI Flag
    recordPropertyS( "JPsiFinderLegAndLeg" , "elAndEl" ); // CLI Flag
    /*
     N.B: 
     ----
     1. The following default for BeeKstUseElMass changes the default 
     behavior wrt p6413/p6407 (or older) DAODs of BPHY18!
     2. The BeeKstUseElMass and GSFCaloRefitXXXX flags are kept as CLI
     flags for local/private experimentation by "experts". They shouldn't 
     be utilized in production preExec.
    */
    recordPropertyB( "BeeKstUseElMass"         , true     ); // CLI Flag but Don't Use!
    recordPropertyB( "GSFCaloRefitUsePhi"      , false    ); // CLI Flag but Don't Use!
    recordPropertyB( "GSFCaloRefitUseEta"      , false    ); // CLI Flag but Don't Use!
    recordPropertyB( "GSFCaloRefitUsePosition" , true     ); // CLI Flag but Don't Use!
    recordPropertyB( "GSFCaloRefitUseEnergy"   , true     ); // CLI Flag but Don't Use!
    recordPropertyS( "GSFCaloRefitDepthChoice" , "middle" ); // CLI Flag but Don't Use!

    /* Global Constants, in MeV if Relevant */
    /* ------------------------------------ */

    /* 
    N.B.:
    ----- 
    In the following, the values from PDG (2015) slightly modified 
    to be consistent with the values used up until the introduction of 
    this config file:
    Charged Pion Mass    : 139.57061 -> 139.570
    J/Psi Mass           : 3096.92   -> 3096.916
    Neutral B-Meson Mass : 5279.61   -> 5279.6
    */
    
    recordPropertyD( "GlobalElectronMass" , 0.511    );
    recordPropertyD( "GlobalPionMass"     , 139.570  ); // PDG: 139.57061
    recordPropertyD( "GlobalKaonMass"     , 493.677  );
    recordPropertyD( "GlobalJPsiMass"     , 3096.916 ); // PDG: 3096.92
    recordPropertyD( "GlobalKstMass"      , 895.55   ); // NOTE!! Corrected since 21.2.189.0+
    recordPropertyD( "GlobalB0Mass"       , 5279.6   ); // PDG: 5279.61

    /* Kinematic Cuts, in MeV if Relevant */
    /* ---------------------------------- */

    // Kinematic Cuts -> Pre-Fit Cuts
    // ------------------------------
    recordPropertyD( "JPsiPreFitElPtCut"            , 4000.0 ); // CLI Flag
    recordPropertyD( "JPsiPreFitDiElMassLowerCut"   ,    1.0 );
    recordPropertyD( "JPsiPreFitDiElMassUpperCut"   , 7000.0 );
    recordPropertyD( "BeeKstPreFitMesonTrackPtCut"  ,  500.0 );
    recordPropertyD( "BeeKstPreFitMesonTrackEtaCut" ,    3.0 );
    recordPropertyD( "BeeKstPreFitDiMesonPtCut"     ,  500.0 );
    
    /*
    N.B.: 
    -----
    Cuts on KPi/PiK conjugation dependent masses are implemented in a way
    that requires at least one conjugation to pass BOTH the lower and the 
    higher cuts.
    */ 
    
    recordPropertyD( "BeeKstPreFitDiMesonMassLowerCut" ,   690.0 ); 
    recordPropertyD( "BeeKstPreFitDiMesonMassUpperCut" ,  1110.0 ); 
    recordPropertyD( "BeeKstPreFitBMassLowerCut"       ,  1000.0 ); 
    recordPropertyD( "BeeKstPreFitBMassUpperCut"       , 10000.0 ); 
    
    // Kinematic Cuts -> Post-Fit Cuts
    // -------------------------------
    recordPropertyD( "JPsiPostFitDiElMassLowerCut" ,    1.0 );
    recordPropertyD( "JPsiPostFitDiElMassUpperCut" , 7000.0 );
    recordPropertyD( "BeeKstPostFitDiMesonPtCut"   ,  500.0 );
    
    /*
    N.B.:
    -----
    Cuts on KPi/PiK conjugation dependent masses are implemented in a way
    that requires at least one conjugation to pass BOTH the lower and the 
    higher cuts.
    */ 

    recordPropertyD( "BeeKstPostFitDiMesonMassLowerCut" ,  690.0 );
    recordPropertyD( "BeeKstPostFitDiMesonMassUpperCut" , 1100.0 );
    recordPropertyD( "BeeKstPostFitBPtCut"              , 1000.0 );
    recordPropertyD( "BeeKstPostFitBMassLowerCut"       , 3000.0 );
    recordPropertyD( "BeeKstPostFitBMassUpperCut"       , 6500.0 );

    /* 
    N.B.:
    -----
    "Container Cuts" are deliberately set to trivial values so as to 
    retain both conjugations -- as long as it's already checked that at 
    least one conjugation has passed the non-trivial cuts.
    */
    
    recordPropertyD( "BeeKstPostFitBMassLowerCutContainer" ,      1.0 );
    recordPropertyD( "BeeKstPostFitBMassUpperCutContainer" ,  10000.0 );
    recordPropertyD( "KstPostFitKstMassLowerCutContainer"  ,      1.0 );
    recordPropertyD( "KstPostFitKstMassUpperCutContainer"  , 100000.0 );

    /* Vertex-Fit Cuts */
    /* --------------- */
    recordPropertyD( "JPsiChi2Cut"       ,  30.0 );
    recordPropertyD( "BeeKstChi2NDoFCut" ,  15.0 );
    recordPropertyD( "BeeKstChi2Cut"     ,  30.0 );
    recordPropertyD( "KstChi2Cut"        , 100.0 );

    /* Isolation/Multiplicity Calculation */
    /* ---------------------------------- */

    recordPropertyB( "isoMultOnlyInVertex", false ); // CLI Flag 

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
    recordPropertyVI ( "isoTTVAChi2CutTypes"    , {   2, 0  } );
    recordPropertyVI ( "isoPVTypesForTTVA"      , {   1, 3  } ); 
    recordPropertyI  ( "isoPVSVAssocType"       , 2           ); // 2 is minA0
    
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
        ( 1 << 6 ), 
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

  }
  //--------------------------------------------------------------------------
} // namespace
