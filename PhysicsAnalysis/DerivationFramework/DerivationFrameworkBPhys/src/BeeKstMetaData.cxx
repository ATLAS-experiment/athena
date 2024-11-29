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
    recordPropertyB( "BeeKstUseElMass"     , true      ); // CLI Flag // This default changes behavior wrt 21.2.181.0-!

    // Global Constants, in MeV if Relevant.
    /* 
    N.B.: In the following, the values from PDG (2015) slightly modified 
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
    recordPropertyD( "GlobalKstMass"      , 891.66   );
    recordPropertyD( "GlobalB0Mass"       , 5279.6   ); // PDG: 5279.61

    /* Kinematic Cuts, in MeV if Relevant */
    /* ---------------------------------- */

    // Kinematic Cuts -> Pre-Fit Cuts
    // ------------------------------
    recordPropertyD( "JPsiPreFitElPtCut"                   ,   4000.0 );
    recordPropertyD( "JPsiPreFitDiElMassLowerCut"          ,      1.0 );
    recordPropertyD( "JPsiPreFitDiElMassUpperCut"          ,   7000.0 );
    recordPropertyD( "BeeKstPreFitMesonTrackPtCut"         ,    500.0 );
    recordPropertyD( "BeeKstPreFitMesonTrackEtaCut"        ,      3.0 );
    recordPropertyD( "BeeKstPreFitDiMesonPtCut"            ,    500.0 );
    /*
    Cuts on KPi/PiK conjugation dependent masses are implemented in a way
    that requires at least one conjugation to pass BOTH the lower and the 
    higher cuts.
    */ 
    recordPropertyD( "BeeKstPreFitDiMesonMassLowerCut"     ,    690.0 ); 
    recordPropertyD( "BeeKstPreFitDiMesonMassUpperCut"     ,   1110.0 ); 
    recordPropertyD( "BeeKstPreFitBMassLowerCut"           ,   1000.0 ); 
    recordPropertyD( "BeeKstPreFitBMassUpperCut"           ,  10000.0 ); 
    
    // Kinematic Cuts -> Post-Fit Cuts
    // -------------------------------
    recordPropertyD( "JPsiPostFitDiElMassLowerCut"         ,      1.0 );
    recordPropertyD( "JPsiPostFitDiElMassUpperCut"         ,   7000.0 );
    recordPropertyD( "BeeKstPostFitDiMesonPtCut"           ,    500.0 );
    /*
    Cuts on KPi/PiK conjugation dependent masses are implemented in a way
    that requires at least one conjugation to pass BOTH the lower and the 
    higher cuts.
    */ 
    recordPropertyD( "BeeKstPostFitDiMesonMassLowerCut"    ,    690.0 );
    recordPropertyD( "BeeKstPostFitDiMesonMassUpperCut"    ,   1100.0 );
    recordPropertyD( "BeeKstPostFitBPtCut"                 ,   1000.0 );
    recordPropertyD( "BeeKstPostFitBMassLowerCut"          ,   3000.0 );
    recordPropertyD( "BeeKstPostFitBMassUpperCut"          ,   6500.0 );
    /* 
    "Container Cuts" are deliberately set to trivial values so as to 
    retain both conjugations -- as long as it's already checked that at 
    least one conjugation has passed the non-trivial cuts.
    */
    recordPropertyD( "BeeKstPostFitBMassLowerCutContainer" ,      1.0 );
    recordPropertyD( "BeeKstPostFitBMassUpperCutContainer" ,  10000.0 );
    recordPropertyD( "KstPostFitKstMassLowerCutContainer"  ,      1.0 );
    recordPropertyD( "KstPostFitKstMassUpperCutContainer"  , 100000.0 );

    // Vertex-Fit Cuts
    recordPropertyD( "JPsiChi2Cut"       ,  30.0 );
    recordPropertyD( "BeeKstChi2NDoFCut" ,  15.0 );
    recordPropertyD( "BeeKstChi2Cut"     ,  30.0 );
    recordPropertyD( "KstChi2Cut"        , 100.0 );
  }
  //--------------------------------------------------------------------------
} // namespace
