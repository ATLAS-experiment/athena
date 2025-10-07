/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/DiTauDiscriminantTool.h"
#include "AthContainers/ConstAccessor.h"
#include "PathResolver/PathResolver.h"

using namespace DiTauRecTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauDiscriminantTool::DiTauDiscriminantTool( const std::string& name )
  : AsgTool(name) { }

//______________________________________________________________________________
DiTauDiscriminantTool::~DiTauDiscriminantTool( )
= default;

//______________________________________________________________________________
StatusCode DiTauDiscriminantTool::initialize()
{  
   ATH_MSG_INFO( "Initializing DiTauDiscriminantTool" );
   ATH_MSG_DEBUG( "path to weights file: " << m_sWeightsFile );

   ATH_CHECK(parseWeightsFile());

   // m_bIsInitialized = true;
   return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                              Wrapper functions                             //
////////////////////////////////////////////////////////////////////////////////
StatusCode DiTauDiscriminantTool::execute(const xAOD::DiTauJet& xDiTau) const{

  auto availableVariables = setIDVariables(xDiTau);

  float bdtScore = m_mvaBDT->getGradBoostMVA(availableVariables);

  const static SG::Decorator<float> decBDTScore(m_sBDTScoreName);
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

  m_mvaBDT = std::make_unique<tauRecTools::BDTHelper>();
  ATH_CHECK(m_mvaBDT->initialize(weight_file));

  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------------
std::map<TString, float> DiTauDiscriminantTool::setIDVariables(const xAOD::DiTauJet& xDiTau) const
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

  std::map<TString, float> availableVariables;
  availableVariables.emplace("f_core_lead", f_core_leadAcc(xDiTau));
  availableVariables.emplace("f_core_subl", f_core_sublAcc(xDiTau));
  availableVariables.emplace("f_subjet_subl", f_subjet_sublAcc(xDiTau));
  availableVariables.emplace("f_subjets", f_subjetsAcc(xDiTau));
  availableVariables.emplace("R_max_lead", R_max_leadAcc(xDiTau));
  availableVariables.emplace("R_max_subl", R_max_sublAcc(xDiTau));
  availableVariables.emplace("n_track", static_cast<float>(n_trackAcc(xDiTau)));
  availableVariables.emplace("n_tracks_lead", static_cast<float>(n_tracks_leadAcc(xDiTau)));
  availableVariables.emplace("R_isotrack", R_isotrackAcc(xDiTau));
  availableVariables.emplace("R_tracks_subl", R_tracks_sublAcc(xDiTau));
  availableVariables.emplace("m_core_lead", M_core_leadAcc(xDiTau));
  availableVariables.emplace("log(m_core_lead)", std::log(M_core_leadAcc(xDiTau)));
  availableVariables.emplace("m_core_subl", M_core_sublAcc(xDiTau));
  availableVariables.emplace("log(m_core_subl)", std::log(M_core_sublAcc(xDiTau)));
  availableVariables.emplace("m_tracks_lead", M_tracks_leadAcc(xDiTau));
  availableVariables.emplace("log(m_tracks_lead)", std::log(M_tracks_leadAcc(xDiTau)));
  availableVariables.emplace("m_tracks_subl", M_tracks_sublAcc(xDiTau));
  availableVariables.emplace("log(m_tracks_subl)", std::log(M_tracks_sublAcc(xDiTau)));
  availableVariables.emplace("d0_leadtrack_lead", d0_leadtrack_leadAcc(xDiTau));
  availableVariables.emplace("log(abs(d0_leadtrack_lead))", std::log(std::abs(d0_leadtrack_leadAcc(xDiTau))));
  availableVariables.emplace("d0_leadtrack_subl", d0_leadtrack_sublAcc(xDiTau));
  availableVariables.emplace("log(abs(d0_leadtrack_subl))", std::log(std::abs(d0_leadtrack_sublAcc(xDiTau))));
  availableVariables.emplace("f_isotracks", f_isotracksAcc(xDiTau));
  availableVariables.emplace("log(f_isotracks)", std::log(f_isotracksAcc(xDiTau)));

  for (const auto& [var, value]: availableVariables)
  {
    ATH_MSG_DEBUG(var << ": " << value);
  }

  return availableVariables;
}
