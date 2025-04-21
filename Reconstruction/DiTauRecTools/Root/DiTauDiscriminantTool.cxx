/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
     {"f_subjet_subl", new float(0)}, // used 
     {"f_subjets", new float(0)}, // used
     {"R_max_lead", new float(0)}, // used 
     {"R_max_subl", new float(0)}, // used
     {"n_track", new float(0)}, // used 
     {"n_tracks_lead", new float(0)}, // used
     {"R_isotrack", new float(0)}, // used 
     {"R_tracks_subl", new float(0)}, // used
     {"m_core_lead", new float(0)},
     {"log(m_core_lead)", new float(0)}, // used
     {"m_core_subl", new float(0)},
     {"log(m_core_subl)", new float(0)}, // used
     {"m_tracks_lead", new float(0)},
     {"log(m_tracks_lead)", new float(0)}, // used
     {"m_tracks_subl", new float(0)},
     {"log(m_tracks_subl)", new float(0)}, // used
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
  SG::ConstAccessor<float> f_subjet_sublAcc("f_subjet_subl");
  SG::ConstAccessor<float> f_subjetsAcc("f_subjets");
  SG::ConstAccessor<float> R_max_leadAcc("R_max_lead");
  SG::ConstAccessor<float> R_max_sublAcc("R_max_subl");
  SG::ConstAccessor<int>   n_trackAcc("n_track");
  SG::ConstAccessor<int>   n_tracks_leadAcc("n_tracks_lead");
  SG::ConstAccessor<float> R_isotrackAcc("R_isotrack");
  SG::ConstAccessor<float> R_tracks_sublAcc("R_tracks_subl");
  SG::ConstAccessor<float> M_core_leadAcc("m_core_lead");
  SG::ConstAccessor<float> M_core_sublAcc("m_core_subl");
  SG::ConstAccessor<float> M_tracks_leadAcc("m_tracks_lead");
  SG::ConstAccessor<float> M_tracks_sublAcc("m_tracks_subl");
  SG::ConstAccessor<float> d0_leadtrack_leadAcc("d0_leadtrack_lead");
  SG::ConstAccessor<float> d0_leadtrack_sublAcc("d0_leadtrack_subl");
  SG::ConstAccessor<float> f_isotracksAcc("f_isotracks");

  setVar("f_core_lead") = f_core_leadAcc(xDiTau);
  setVar("f_core_subl") = f_core_sublAcc(xDiTau);
  setVar("f_subjet_subl") = f_subjet_sublAcc(xDiTau);
  setVar("f_subjets") = f_subjetsAcc(xDiTau);
  setVar("R_max_lead") = R_max_leadAcc(xDiTau);
  setVar("R_max_subl") = R_max_sublAcc(xDiTau);
  setVar("n_track") = (float) n_trackAcc(xDiTau);
  setVar("n_tracks_lead") = (float) n_tracks_leadAcc(xDiTau);
  setVar("R_isotrack") = R_isotrackAcc(xDiTau);
  setVar("R_tracks_subl") = R_tracks_sublAcc(xDiTau);
  setVar("m_core_lead") = M_core_leadAcc(xDiTau);
  setVar("log(m_core_lead)") = log(*m_mIDVariables["m_core_lead"]);
  setVar("m_core_subl") = M_core_sublAcc(xDiTau);
  setVar("log(m_core_subl)") = log(*m_mIDVariables["m_core_subl"]);
  setVar("m_tracks_lead") = M_tracks_leadAcc(xDiTau);
  setVar("log(m_tracks_lead)") = log(*m_mIDVariables["m_tracks_lead"]);
  setVar("m_tracks_subl") = M_tracks_sublAcc(xDiTau);
  setVar("log(m_tracks_subl)") = log(*m_mIDVariables["m_tracks_subl"]);
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
