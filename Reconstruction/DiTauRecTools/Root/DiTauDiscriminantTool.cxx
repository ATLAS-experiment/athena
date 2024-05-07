/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/DiTauDiscriminantTool.h"

// Core include(s):
#include "AthLinks/ElementLink.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"
#include "xAODTau/DiTauJetContainer.h"

#include "DiTauRecTools/HelperFunctions.h"
#include "PathResolver/PathResolver.h"


using namespace DiTauRecTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauDiscriminantTool::DiTauDiscriminantTool( const std::string& name )
  : AsgTool(name)
  , m_bdt()
  , m_eDecayMode(DecayMode::Default)
{
  declareProperty( "WeightsFile", m_sWeightsFile = "tauRecTools/R22_preprod/DiTau_JetBDT_winter2024.weights.root");
  declareProperty( "BDTScoreName", m_sBDTScoreName = "JetBDT");
  declareProperty( "DiTauDecayChannel", m_sDecayMode = "HadHad");
}

//______________________________________________________________________________
DiTauDiscriminantTool::~DiTauDiscriminantTool( )
= default;

//______________________________________________________________________________
StatusCode DiTauDiscriminantTool::initialize()
{  
   ATH_MSG_INFO( "Initializing DiTauDiscriminantTool" );
   ATH_MSG_DEBUG( "path to weights file: " << m_sWeightsFile );

   if(m_sDecayMode == "HadHad")
     m_eDecayMode = DecayMode::HadHad;
   if(m_eDecayMode == DecayMode::Default)
     ATH_MSG_ERROR( "No valid DecayMode initialized for DiTauDiscriminantTool. Possible Options: HadHad");

   switch(m_eDecayMode){
   case(DecayMode::HadHad):
     m_mIDSpectators = {
       {"ditau_pt", new float(0)},
       {"mu", new float(0)},
       {"pt_weight", new float(0)},
       {"isSignal", new float(0)}
     };

     m_mIDVariables = {
       {"f_core_lead", new float(0)}, // used 
       {"f_core_subl", new float(0)}, // used
       {"f_subjet_lead", new float(0)},
       {"f_subjet_subl", new float(0)}, // used 
       {"f_subjets", new float(0)}, // used
       {"f_track_lead", new float(0)},
       {"f_track_subl", new float(0)},
       {"R_max_lead", new float(0)}, // used 
       {"R_max_subl", new float(0)}, // used
       {"n_Subjets", new float(0)},
       {"n_track", new float(0)}, // used 
       {"n_tracks_lead", new float(0)}, // used
       {"n_tracks_subl", new float(0)},
       {"n_isotrack", new float(0)},
       {"R_track", new float(0)},
       {"R_track_core", new float(0)},
       {"R_track_all", new float(0)},
       {"R_isotrack", new float(0)}, // used 
       {"R_core_lead", new float(0)},
       {"R_core_subl", new float(0)},
       {"R_tracks_lead", new float(0)},
       {"R_tracks_subl", new float(0)}, // used
       {"m_track", new float(0)},
       {"m_track_core", new float(0)},
       {"m_core_lead", new float(0)},
       {"log(m_core_lead)", new float(0)}, // used
       {"m_core_subl", new float(0)},
       {"log(m_core_subl)", new float(0)}, // used
       {"m_track_all", new float(0)},
       {"m_tracks_lead", new float(0)},
       {"log(m_tracks_lead)", new float(0)}, // used
       {"m_tracks_subl", new float(0)},
       {"log(m_tracks_subl)", new float(0)}, // used
       {"E_frac_subl", new float(0)},
       {"E_frac_subsubl", new float(0)},
       {"R_subjets_subl", new float(0)},
       {"R_subjets_subsubl", new float(0)},
       {"d0_leadtrack_lead", new float(0)},
       {"log(abs(d0_leadtrack_lead))", new float(0)}, // used
       {"d0_leadtrack_subl", new float(0)},
       {"log(abs(d0_leadtrack_subl))", new float(0)}, // used 
       {"f_isotracks", new float(0)},
       {"log(f_isotracks)", new float(0)}, // used
     };
     
     break;
   default:
     ATH_MSG_ERROR( "No valid DecayMode" );
     break;
   }
   
   

   ATH_CHECK(parseWeightsFile());

   // m_bIsInitialized = true;
   return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                              Wrapper functions                             //
////////////////////////////////////////////////////////////////////////////////
/*double DiTauDiscriminantTool::getJetBDTScore(const xAOD::DiTauJet& xDiTau)
{
  //ATH_CHECK(execute(xDiTau));

  execute(xDiTau);
  const static SG::AuxElement::ConstAccessor<double> accBDTScore(m_sBDTScoreName);
  return accBDTScore(xDiTau);
} */

StatusCode DiTauDiscriminantTool::execute(const xAOD::DiTauJet& xDiTau){

  setIDVariables(xDiTau);

  double bdtScore = m_bdt->GetClassification();

  const static SG::AuxElement::Decorator<double> decBDTScore(m_sBDTScoreName);
  decBDTScore(xDiTau) = bdtScore;

  std::cout << "Jet BDT score: " << bdtScore << std::endl;

  ATH_MSG_DEBUG("Jet BDT score: " << bdtScore);
  return StatusCode::SUCCESS;
} 

std::string DiTauDiscriminantTool::getDecayMode(){
  return m_sDecayMode;
}

//=================================PRIVATE-PART=================================
//______________________________________________________________________________

StatusCode DiTauDiscriminantTool::parseWeightsFile()
{
  std::string weight_file = PathResolverFindCalibFile(m_sWeightsFile);

  ATH_MSG_DEBUG("InputWeightsPath: " << weight_file);

  m_bdt = DiTauRecTools::configureMVABDT( m_mIDVariables, weight_file.c_str() );
  if(!m_bdt) {
    ATH_MSG_FATAL("Couldn't configure MVA");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------------
void DiTauDiscriminantTool::setIDVariables(const xAOD::DiTauJet& xDiTau)
{
  switch(m_eDecayMode){
  case(DecayMode::HadHad):
    setVar("f_core_lead") = xDiTau.auxdata<float>("f_core_lead");
    setVar("f_core_subl") = xDiTau.auxdata<float>("f_core_subl");
    setVar("f_subjet_lead") = xDiTau.auxdata<float>("f_subjet_lead");
    setVar("f_subjet_subl") = xDiTau.auxdata<float>("f_subjet_subl");
    setVar("f_subjets") = xDiTau.auxdata<float>("f_subjets");
    setVar("f_track_lead") = xDiTau.auxdata<float>("f_track_lead");
    setVar("f_track_subl") = xDiTau.auxdata<float>("f_track_subl");
    setVar("R_max_lead") = xDiTau.auxdata<float>("R_max_lead");
    setVar("R_max_subl") = xDiTau.auxdata<float>("R_max_subl");
    setVar("n_Subjets") = (float) xDiTau.auxdata<int>("n_subjets");
    setVar("n_track") = (float) xDiTau.auxdata<int>("n_track");
    setVar("n_tracks_lead") = (float) xDiTau.auxdata<int>("n_tracks_lead");
    setVar("n_tracks_subl") = (float) xDiTau.auxdata<int>("n_tracks_subl");
    setVar("n_isotrack") = (float) xDiTau.auxdata<int>("n_isotrack");
    setVar("R_track") = xDiTau.auxdata<float>("R_track");
    setVar("R_track_core") = xDiTau.auxdata<float>("R_track_core");
    setVar("R_track_all") = xDiTau.auxdata<float>("R_track_all");
    setVar("R_isotrack") = xDiTau.auxdata<float>("R_isotrack");
    setVar("R_core_lead") = xDiTau.auxdata<float>("R_core_lead");
    setVar("R_core_subl") = xDiTau.auxdata<float>("R_core_subl");
    setVar("R_tracks_lead") = xDiTau.auxdata<float>("R_tracks_lead");
    setVar("R_tracks_subl") = xDiTau.auxdata<float>("R_tracks_subl");
    setVar("m_track") = xDiTau.auxdata<float>("m_track");
    setVar("m_track_core") = xDiTau.auxdata<float>("m_track_core");
    setVar("m_core_lead") = xDiTau.auxdata<float>("m_core_lead");
    setVar("log(m_core_lead)") = log(*m_mIDVariables["m_core_lead"]);
    setVar("m_core_subl") = xDiTau.auxdata<float>("m_core_subl");
    setVar("log(m_core_subl)") = log(*m_mIDVariables["m_core_subl"]);
    setVar("m_track_all") = xDiTau.auxdata<float>("m_track_all");
    setVar("m_tracks_lead") = xDiTau.auxdata<float>("m_tracks_lead");
    setVar("log(m_tracks_lead)") = log(*m_mIDVariables["m_tracks_lead"]);
    setVar("m_tracks_subl") = xDiTau.auxdata<float>("m_tracks_subl");
    setVar("log(m_tracks_subl)") = log(*m_mIDVariables["m_tracks_subl"]);
    setVar("E_frac_subl") = xDiTau.auxdata<float>("E_frac_subl");
    setVar("E_frac_subsubl") = xDiTau.auxdata<float>("E_frac_subsubl");
    setVar("R_subjets_subl") = xDiTau.auxdata<float>("R_subjets_subl");
    setVar("R_subjets_subsubl") = xDiTau.auxdata<float>("R_subjets_subsubl");
    setVar("d0_leadtrack_lead") = xDiTau.auxdata<float>("d0_leadtrack_lead");
    setVar("log(abs(d0_leadtrack_lead))") = log(fabs(*m_mIDVariables["d0_leadtrack_lead"]));
    setVar("d0_leadtrack_subl") = xDiTau.auxdata<float>("d0_leadtrack_subl");
    setVar("log(abs(d0_leadtrack_subl))") = log(fabs(*m_mIDVariables["d0_leadtrack_subl"]));
    setVar("f_isotracks") = xDiTau.auxdata<float>("f_isotracks");
    setVar("log(f_isotracks)") = log(*m_mIDVariables["f_isotracks"]);
    break;
  default:
    ATH_MSG_ERROR("Invalid DecayMode.");
    break;
  }

  for (const auto &var: m_vVarNames)
  {
    ATH_MSG_DEBUG(var << ": " << m_mIDVariables[var]);
  }
}
