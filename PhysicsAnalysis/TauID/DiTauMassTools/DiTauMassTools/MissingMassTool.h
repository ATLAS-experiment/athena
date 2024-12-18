/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Asg wrapper around the MissingMassCalculator
// author Quentin Buat <quentin.buat@no.spam.cern.ch>
#ifndef DITAUMASSTOOLS_MISSINGMASSTOOL_H
#define DITAUMASSTOOLS_MISSINGMASSTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"

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
  double m_n_sigma_met{};
  int m_tail_cleanup{};
  int m_use_verbose{};
  int m_use_tau_probability{};
  int m_niter_fit_2{};
  int m_niter_fit_3{};
  int m_use_defaults{};
  int m_use_efficiency_recovery{};
  std::string m_calib_set;
  std::string m_lfv_mode;
  bool m_decorate{};
  bool m_float_stop{};
  bool m_use_mnu_probability{};
  bool m_use_met_param_dphiLL{};
  std::string m_param_file_path;
  double m_beam_energy{};
  bool m_lfv_leplep_refit{};
  bool m_save_llh_histo{};

};
} // namespace DiTauMassTools  

#endif
