/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Asg wrapper around the MissingMassCalculator
// author Quentin Buat <quentin.buat@no.spam.cern.ch>
#ifndef DITAUMASSTOOLS_MISSINGMASSTOOL_H
#define DITAUMASSTOOLS_MISSINGMASSTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

//Local include(s):
#include "DiTauMassTools/IMissingMassTool.h"
#include "DiTauMassTools/MissingMassCalculator.h"
#include "DiTauMassTools/HelperFunctions.h"

#include <string>

namespace DiTauMassTools{
  using ROOT::Math::PtEtaPhiMVector;
  using ROOT::Math::VectorUtil::Phi_mpi_pi;

class MissingMassTool : virtual public IMissingMassTool, virtual public asg::AsgTool
{

  /// Proper constructor for Athena
  ASG_TOOL_CLASS(MissingMassTool, IMissingMassTool)

 public:
  
  /// Standard constructor for standalone usage
  MissingMassTool(const std::string& name);
  /// Copy constructor for reflex in Athena
  MissingMassTool(const MissingMassTool& other);

  /// virtual destructor
  virtual ~MissingMassTool() { };

  /// Initialize the tool
  virtual StatusCode initialize();

  /// Initialize the tool
  virtual StatusCode finalize();


  // generic method
  virtual CP::CorrectionCode apply (const xAOD::EventInfo& ei,
				    const xAOD::IParticle* part1,
				    const xAOD::IParticle* part2,
				    const xAOD::MissingET* met,
				    const int & njets);

  virtual void calculate(const xAOD::EventInfo & ei, 
			 const PtEtaPhiMVector & vis_tau1,
			 const PtEtaPhiMVector & vis_tau2,
			 const int & tau1_decay_type,
			 const int & tau2_decay_type,
			 const xAOD::MissingET & met,
			 const int & njets){
	  ignore(ei); ignore(vis_tau1); ignore(vis_tau2);
	  ignore(tau1_decay_type); ignore(tau2_decay_type);
	  ignore(met); ignore(njets);}

  virtual MissingMassCalculator* get() {return m_MMC;}
  virtual double GetFitStatus(int method) {(void) method; return m_MMC->OutputInfo.GetFitStatus();}
  virtual double GetFittedMass(int method) {return m_MMC->OutputInfo.GetFittedMass(method);}
  virtual double GetFittedMassErrorUp(int method) {return m_MMC->OutputInfo.GetFittedMassErrorUp(method);}
  virtual double GetFittedMassErrorLow(int method) {return m_MMC->OutputInfo.GetFittedMassErrorLow(method);}
  virtual PtEtaPhiMVector GetResonanceVec(int method) {return m_MMC->OutputInfo.GetResonanceVec(method);}
  virtual XYVector GetFittedMetVec(int method) {return m_MMC->OutputInfo.GetFittedMetVec(method);}
  virtual PtEtaPhiMVector GetNeutrino4vec(int method, int index) {return m_MMC->OutputInfo.GetNeutrino4vec(method, index);}
  virtual PtEtaPhiMVector GetTau4vec(int method, int index) {return m_MMC->OutputInfo.GetTau4vec(method, index);}
  virtual int GetNNoSol() {return m_MMC->GetNNoSol();}
  virtual int GetNMetroReject() {return m_MMC->GetNMetroReject();}
  virtual int GetNSol() {return m_MMC->GetNSol();}

 private:

  MissingMassCalculator* m_MMC{};

  Gaudi::Property<bool> m_decorate{this, "Decorate", false};
  Gaudi::Property<bool> m_float_stop{this, "FloatStoppingCrit", true, "Applying Floating Stopping Criterion to speed up MMC"};
  Gaudi::Property<int>  m_float_stop_miniter{this, "FloatStoppingCritMinIter", 10000, "Minimum number of iteration to apply Floating Stopping Criterion"};
  Gaudi::Property<int>  m_float_stop_checkfreq{this, "FloatStoppingCritCheckFreq", 1000, "Number of events frequency for Floating Stopping Criterion to be applied after minimum number of iteration"}; 
  Gaudi::Property<double> m_float_stop_comp{this, "FloatStoppingCritCheckComp", 0.05, "Percentage to assess the sigma compatibilities in the Floating Stopping Criterion"};  
  Gaudi::Property<std::string> m_calib_set{this, "CalibSet", "2024"}; // Change to "2019" if the old MMC version is to be used.
  // default negative. Only set parameter if positive
  // so that the default are in MissingMassCalculator code
  Gaudi::Property<double> m_n_sigma_met{this, "NsigmaMET", -1};
  Gaudi::Property<int> m_tail_cleanup{this, "UseTailCleanup", -1};
  Gaudi::Property<int> m_use_verbose{this, "UseVerbose", -1};
  Gaudi::Property<int> m_niter_fit_2{this, "NiterFit2", -1};
  Gaudi::Property<int> m_niter_fit_3{this, "NiterFit3", -1}; 
  Gaudi::Property<int> m_use_tau_probability{this, "UseTauProbability", -1};
  Gaudi::Property<bool> m_use_mnu_probability{this, "UseMnuProbability", false}; 
  Gaudi::Property<int> m_use_defaults{this, "UseDefaults", -1};
  Gaudi::Property<int> m_use_efficiency_recovery{this, "UseEfficiencyRecovery", -1};
  Gaudi::Property<bool> m_use_met_param_dphiLL{this, "UseMETDphiLL", false};
  Gaudi::Property<std::string> m_param_file_path{this, "ParamFilePath", "MMC_params_v051224_angle_noLikelihoodFit.root"}; // // Available parameterization files: MMC_params_v051224_angle_noLikelihoodFit.root and MMC_params_v051224_angle_likelihoodFit.root. More details on the differences between these two options can be found in the slides https://indico.cern.ch/event/1487242/contributions/6269201/attachments/2989313/5265428/HbbHtautau_MMCstudies_statusReport_181224_v2.pdf . Use MMC_params_v1_fixed.root for 2019 CalibSet
  Gaudi::Property<double> m_beam_energy{this, "BeamEnergy", 6500.0};
  Gaudi::Property<bool> m_lfv_leplep_refit{this, "LFVLeplepRefit", true};
  Gaudi::Property<bool> m_save_llh_histo{this, "SaveLlhHisto", false}; 

};
} // namespace DiTauMassTools  

#endif
