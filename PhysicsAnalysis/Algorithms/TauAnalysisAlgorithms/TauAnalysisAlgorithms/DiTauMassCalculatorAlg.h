/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef DI_TAU_MASS_CALCULATOR_ALG_H
#define DI_TAU_MASS_CALCULATOR_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODJet/JetContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <DiTauMassTools/MissingMassTool.h>
#include "Math/Vector4D.h"

namespace CP {
  using ROOT::Math::PtEtaPhiMVector;

  class DiTauMassCalculatorAlg final : public EL::AnaAlgorithm
  {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    // configurable properties
    Gaudi::Property<bool> m_doMAXW        {this, "doMAXW", false, "save information about the reconstruction with the maximum-weight estimator"};
    Gaudi::Property<bool> m_doMLNU3P      {this, "doMLNU3P", false, "save information about the reconstruction with the best-fit neutrino kinematics"};

    // the MMC tool
    ToolHandle<DiTauMassTools::MissingMassTool> m_mmc {this, "mmcTool", "DiTauMassTools::MissingMassTool", "the Missing Mass Calculator tool"};

    // systematics
    SysListHandle m_systematicsList{this};

    // inputs needed for reconstruction
    SysReadHandle<xAOD::ElectronContainer>  m_electronsHandle   {this, "electrons", "", "the electron container to use"};
    SysReadSelectionHandle                  m_electronSelection {this, "electronSelection", "", "the selection on the input electrons"};

    SysReadHandle<xAOD::MuonContainer>      m_muonsHandle       {this, "muons", "", "the muon container to use"};
    SysReadSelectionHandle                  m_muonSelection     {this, "muonSelection", "", "the selection on the input muons"};

    SysReadHandle<xAOD::JetContainer>       m_jetsHandle        {this, "jets", "", "the jet container to use"};
    SysReadSelectionHandle                  m_jetSelection      {this, "jetSelection", "", "the selection on the input jets"};

    SysReadHandle<xAOD::TauJetContainer>    m_tausHandle        {this, "taus", "", "the tau jet container to use"};
    SysReadSelectionHandle                  m_tauSelection      {this, "tauSelection", "", "the selection on the input tau jets"};

    SysReadHandle<xAOD::MissingETContainer> m_metHandle         {this, "met", "", "the MET container to use"};

    SysReadHandle<xAOD::EventInfo>          m_eventInfoHandle   {this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"};

    SysReadSelectionHandle                  m_preselection      {this, "eventSelection", "", "Name of the selection on which this MMC instance is allowed to run"};

    // output decorations
    SysWriteDecorHandle<int>            m_fitStatus_decor          {this, "fitStatus", "mmc_fit_status_%SYS%", "Status of the MMC fit"};
    SysWriteDecorHandle<double>         m_maxw_mass_decor          {this, "maxw_mass", "mmc_maxw_mass_%SYS%", "Mass of the resonance estimated by the MMC MaxW method"};
    SysWriteDecorHandle<double>         m_mlm_mass_decor           {this, "mlm_mass", "mmc_mlm_mass_%SYS%", "Mass of the resonance estimated by the MMC MLM method"};
    SysWriteDecorHandle<double>         m_mlnu3p_mass_decor        {this, "mlnu3p_mass", "mmc_mlnu3p_mass_%SYS%", "Mass of the resonance estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_mlnu3p_res_4vect_decor  {this, "mlnu3p_res_4vect", "mmc_mlnu3p_res_4vect_%SYS%", "Four-momentum of the resonance estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_mlnu3p_nu1_4vect_decor  {this, "mlnu3p_nu1_4vect", "mmc_mlnu3p_nu1_4vect_%SYS%", "Four-momentum of the leading pt neutrino estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_mlnu3p_nu2_4vect_decor  {this, "mlnu3p_nu2_4vect", "mmc_mlnu3p_nu2_4vect_%SYS%", "Four-momentum of the subleading pt neutrino estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_mlnu3p_tau1_4vect_decor {this, "mlnu3p_tau1_4vect", "mmc_mlnu3p_tau1_4vect_%SYS%", "Four-momentum of the leading lepton estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_mlnu3p_tau2_4vect_decor {this, "mlnu3p_tau2_4vect", "mmc_mlnu3p_tau2_4vect_%SYS%", "Four-momentum of the subleading lepton estimated by the MMC MLNU3P method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_maxw_res_4vect_decor    {this, "maxw_res_4vect", "mmc_maxw_res_4vect_%SYS%", "Mass of the resonance estimated by the MMC MaxW method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_maxw_nu1_4vect_decor    {this, "maxw_nu1_4vect", "mmc_maxw_nu1_4vect_%SYS%", "Four-momentum of the leading pt neutrino estimated by the MMC MaxW method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_maxw_nu2_4vect_decor    {this, "maxw_nu2_4vect", "mmc_maxw_nu2_4vect_%SYS%", "Four-momentum of the subleading pt neutrino estimated by the MMC MaxW method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_maxw_tau1_4vect_decor   {this, "maxw_tau1_4vect", "mmc_maxw_tau1_4vect_%SYS%", "Four-momentum of the leading lepton estimated by the MMC MaxW method"};
    SysWriteDecorHandle<PtEtaPhiMVector> m_maxw_tau2_4vect_decor   {this, "maxw_tau2_4vect", "mmc_maxw_tau2_4vect_%SYS%", "Four-momentum of the subleading lepton estimated by the MMC MaxW method"};

  };

} // namespace

#endif
