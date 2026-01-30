/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// vim: ts=2 sw=2
// Local include(s)
#include "DiTauMassTools/MissingMassTool.h"

// EDM include(s):
#include "xAODTau/TauJet.h"

using namespace DiTauMassTools;
using ROOT::Math::PtEtaPhiMVector;

// Default constructor
MissingMassTool::MissingMassTool(const std::string& name) : asg::AsgTool(name)
{
}

// Copy constructor
MissingMassTool::MissingMassTool(const MissingMassTool& other): asg::AsgTool(other.name() + "_copy") {}


StatusCode MissingMassTool::initialize()

{
  ATH_MSG_INFO("Initialize MissingMassTool");

  MMCCalibrationSet::e aset;

  if (m_calib_set == "2015HIGHMASS") {
    aset = MMCCalibrationSet::MMC2015HIGHMASS;
  } else if (m_calib_set == "UPGRADE") {
    aset = MMCCalibrationSet::UPGRADE;
  } else if (m_calib_set == "LFV") {
    aset = MMCCalibrationSet::LFVMMC2012;
  } else if (m_calib_set == "2019") {
    aset = MMCCalibrationSet::MMC2019;
  } else if (m_calib_set == "2024") {
    aset = MMCCalibrationSet::MMC2024;
  } else {
    return StatusCode::FAILURE;
  }


  if(aset == MMCCalibrationSet::MMC2019 &&  m_param_file_path.value().find("v051224") != std::string::npos){
		ATH_MSG_WARNING( m_param_file_path << " param file not suitable for 2019 calibration set; please use MMC_params_v1_fixed.root");
	}
 
	if(aset == MMCCalibrationSet::MMC2024 && m_param_file_path.value().find("MMC_params_v1") != std::string::npos){
    ATH_MSG_WARNING( m_param_file_path << " param file not suitable for 2024 calibration set; please use MMC_params_v051224_angle_likelihoodFit.root or MMC_params_v051224_angle_noLikelihoodFit.root"); 
	}

	m_MMC = new MissingMassCalculator(aset, m_param_file_path);
  m_MMC->SetUseFloatStopping(m_float_stop);
  m_MMC->SetFloatStoppingMinIter(m_float_stop_miniter);
  m_MMC->SetFloatStoppingCheckFreq(m_float_stop_checkfreq);
  m_MMC->SetFloatStoppingComp(m_float_stop_comp);
	// set properties if non negative
  if (m_n_sigma_met>=0) m_MMC->SetNsigmaMETscan(m_n_sigma_met);
  if (m_tail_cleanup>=0) m_MMC->preparedInput.SetUseTailCleanup(m_tail_cleanup);
  if (m_use_verbose>=0) m_MMC->preparedInput.SetUseVerbose(m_use_verbose);
  if (m_niter_fit_2>=0) m_MMC->SetNiterFit2(m_niter_fit_2);
  if (m_niter_fit_3>=0) m_MMC->SetNiterFit3(m_niter_fit_3);
  if (m_use_tau_probability>=0) m_MMC->Prob->SetUseTauProbability(m_use_tau_probability);
  if (m_use_defaults>=0) m_MMC->preparedInput.SetUseDefaults(m_use_defaults);
  if (m_use_efficiency_recovery>=0) m_MMC->SetUseEfficiencyRecovery(m_use_efficiency_recovery);
  if (m_use_met_param_dphiLL) m_MMC->Prob->SetUseDphiLL(m_use_met_param_dphiLL);
  if (m_use_mnu_probability) m_MMC->Prob->SetUseMnuProbability(m_use_mnu_probability);
  if (m_beam_energy) m_MMC->SetBeamEnergy(m_beam_energy);
  if (!m_lfv_leplep_refit) m_MMC->SetLFVLeplepRefit(false);
  m_MMC->SaveLlhHisto(m_save_llh_histo);

  // could be made a property but maybe not with the enum
  // What about a string argument ?

  return StatusCode::SUCCESS;
}

StatusCode MissingMassTool::finalize()

{
  ATH_MSG_INFO("Finalize MissingMassTool");
  delete m_MMC;
  //delete Output;

  return StatusCode::SUCCESS;
}


// generic method
CP::CorrectionCode MissingMassTool::apply(const xAOD::EventInfo& ei,
    const xAOD::IParticle* part1,
    const xAOD::IParticle* part2,
    const xAOD::MissingET* met,
    const int & njets)
{
  // This is actually where the work is done
  m_MMC->SetEventNumber(ei.eventNumber());
  m_MMC->RunMissingMassCalculator(part1, part2, met, njets);

  return CP::CorrectionCode::Ok;
}
