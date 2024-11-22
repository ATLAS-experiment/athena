/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/DiTauDiscriminantTool.h"

// Core include(s):
#include "AthLinks/ElementLink.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"

#include "DiTauRecTools/HelperFunctions.h"
#include "PathResolver/PathResolver.h"
#include "AthContainers/ConstAccessor.h"


using namespace DiTauRecTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauDiscriminantTool::DiTauDiscriminantTool( const std::string& name )
  : AsgTool(name)
  , m_bdt()
{
  declareProperty( "WeightsFile", m_sWeightsFile = "tauRecTools/R22_preprod/DiTau_JetBDT_winter2024.weights.root");
  declareProperty( "BDTScoreName", m_sBDTScoreName = "JetBDT");
}

//______________________________________________________________________________
DiTauDiscriminantTool::~DiTauDiscriminantTool( )
= default;

//______________________________________________________________________________
StatusCode DiTauDiscriminantTool::initialize()
{  
   ATH_MSG_INFO( "Initializing DiTauDiscriminantTool" );
   ATH_MSG_DEBUG( "path to weights file: " << m_sWeightsFile );

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

   ATH_CHECK(parseWeightsFile());

   // m_bIsInitialized = true;
   return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                              Wrapper functions                             //
////////////////////////////////////////////////////////////////////////////////
StatusCode DiTauDiscriminantTool::execute(const xAOD::DiTauJet& xDiTau){

  setIDVariables(xDiTau);

  double bdtScore = m_bdt->GetClassification();

  const static SG::Decorator<double> decBDTScore(m_sBDTScoreName);
  decBDTScore(xDiTau) = bdtScore;

  ATH_MSG_DEBUG("Jet BDT score: " << bdtScore);
  return StatusCode::SUCCESS;
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
  SG::ConstAccessor<float> f_core_leadAcc("f_core_lead");
  SG::ConstAccessor<float> f_core_sublAcc("f_core_subl");
  SG::ConstAccessor<float> f_subjet_leadAcc("f_subjet_lead");
  SG::ConstAccessor<float> f_subjet_sublAcc("f_subjet_subl");
  SG::ConstAccessor<float> f_subjetsAcc("f_subjets");
  SG::ConstAccessor<float> f_track_leadAcc("f_track_lead");
  SG::ConstAccessor<float> f_track_sublAcc("f_track_subl");
  SG::ConstAccessor<float> R_max_leadAcc("R_max_lead");
  SG::ConstAccessor<float> R_max_sublAcc("R_max_subl");
  SG::ConstAccessor<int>   n_subjetsAcc("n_subjets");
  SG::ConstAccessor<int>   n_trackAcc("n_track");
  SG::ConstAccessor<int>   n_tracks_leadAcc("n_tracks_lead");
  SG::ConstAccessor<int>   n_tracks_sublAcc("n_tracks_subl");
  SG::ConstAccessor<int>   n_isotrackAcc("n_isotrack");
  SG::ConstAccessor<float> R_trackAcc("R_track");
  SG::ConstAccessor<float> R_track_coreAcc("R_track_core");
  SG::ConstAccessor<float> R_track_allAcc("R_track_all");
  SG::ConstAccessor<float> R_isotrackAcc("R_isotrack");
  SG::ConstAccessor<float> R_core_leadAcc("R_core_lead");
  SG::ConstAccessor<float> R_core_sublAcc("R_core_subl");
  SG::ConstAccessor<float> R_tracks_leadAcc("R_tracks_lead");
  SG::ConstAccessor<float> R_tracks_sublAcc("R_tracks_subl");
  SG::ConstAccessor<float> M_trackAcc("m_track");
  SG::ConstAccessor<float> M_track_coreAcc("m_track_core");
  SG::ConstAccessor<float> M_core_leadAcc("m_core_lead");
  SG::ConstAccessor<float> M_core_sublAcc("m_core_subl");
  SG::ConstAccessor<float> M_track_allAcc("m_track_all");
  SG::ConstAccessor<float> M_tracks_leadAcc("m_tracks_lead");
  SG::ConstAccessor<float> M_tracks_sublAcc("m_tracks_subl");
  SG::ConstAccessor<float> E_frac_sublAcc("E_frac_subl");
  SG::ConstAccessor<float> E_frac_subsublAcc("E_frac_subsubl");
  SG::ConstAccessor<float> R_subjets_sublAcc("R_subjets_subl");
  SG::ConstAccessor<float> R_subjets_subsublAcc("R_subjets_subsubl");
  SG::ConstAccessor<float> d0_leadtrack_leadAcc("d0_leadtrack_lead");
  SG::ConstAccessor<float> d0_leadtrack_sublAcc("d0_leadtrack_subl");
  SG::ConstAccessor<float> f_isotracksAcc("f_isotracks");

  setVar("f_core_lead") = f_core_leadAcc(xDiTau);
  setVar("f_core_subl") = f_core_sublAcc(xDiTau);
  setVar("f_subjet_lead") = f_subjet_leadAcc(xDiTau);
  setVar("f_subjet_subl") = f_subjet_sublAcc(xDiTau);
  setVar("f_subjets") = f_subjetsAcc(xDiTau);
  setVar("f_track_lead") = f_track_leadAcc(xDiTau);
  setVar("f_track_subl") = f_track_sublAcc(xDiTau);
  setVar("R_max_lead") = R_max_leadAcc(xDiTau);
  setVar("R_max_subl") = R_max_sublAcc(xDiTau);
  setVar("n_Subjets") = (float) n_subjetsAcc(xDiTau);
  setVar("n_track") = (float) n_trackAcc(xDiTau);
  setVar("n_tracks_lead") = (float) n_tracks_leadAcc(xDiTau);
  setVar("n_tracks_subl") = (float) n_tracks_sublAcc(xDiTau);
  setVar("n_isotrack") = (float) n_isotrackAcc(xDiTau);
  setVar("R_track") = R_trackAcc(xDiTau);
  setVar("R_track_core") = R_track_coreAcc(xDiTau);
  setVar("R_track_all") = R_track_allAcc(xDiTau);
  setVar("R_isotrack") = R_isotrackAcc(xDiTau);
  setVar("R_core_lead") = R_core_leadAcc(xDiTau);
  setVar("R_core_subl") = R_core_sublAcc(xDiTau);
  setVar("R_tracks_lead") = R_tracks_leadAcc(xDiTau);
  setVar("R_tracks_subl") = R_tracks_sublAcc(xDiTau);
  setVar("m_track") = M_trackAcc(xDiTau);
  setVar("m_track_core") = M_track_coreAcc(xDiTau);
  setVar("m_core_lead") = M_core_leadAcc(xDiTau);
  setVar("log(m_core_lead)") = log(*m_mIDVariables["m_core_lead"]);
  setVar("m_core_subl") = M_core_sublAcc(xDiTau);
  setVar("log(m_core_subl)") = log(*m_mIDVariables["m_core_subl"]);
  setVar("m_track_all") = M_track_allAcc(xDiTau);
  setVar("m_tracks_lead") = M_tracks_leadAcc(xDiTau);
  setVar("log(m_tracks_lead)") = log(*m_mIDVariables["m_tracks_lead"]);
  setVar("m_tracks_subl") = M_tracks_sublAcc(xDiTau);
  setVar("log(m_tracks_subl)") = log(*m_mIDVariables["m_tracks_subl"]);
  setVar("E_frac_subl") = E_frac_sublAcc(xDiTau);
  setVar("E_frac_subsubl") = E_frac_subsublAcc(xDiTau);
  setVar("R_subjets_subl") = R_subjets_sublAcc(xDiTau);
  setVar("R_subjets_subsubl") = R_subjets_subsublAcc(xDiTau);
  setVar("d0_leadtrack_lead") = d0_leadtrack_leadAcc(xDiTau);
  setVar("log(abs(d0_leadtrack_lead))") = log(fabs(*m_mIDVariables["d0_leadtrack_lead"]));
  setVar("d0_leadtrack_subl") = d0_leadtrack_sublAcc(xDiTau);
  setVar("log(abs(d0_leadtrack_subl))") = log(fabs(*m_mIDVariables["d0_leadtrack_subl"]));
  setVar("f_isotracks") = f_isotracksAcc(xDiTau);
  setVar("log(f_isotracks)") = log(*m_mIDVariables["f_isotracks"]);

  for (const auto &var: m_vVarNames)
  {
    ATH_MSG_DEBUG(var << ": " << m_mIDVariables[var]);
  }
}
