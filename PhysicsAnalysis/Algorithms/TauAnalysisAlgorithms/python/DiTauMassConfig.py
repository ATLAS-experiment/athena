# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaConfiguration.Enums import LHCPeriod


class DiTauMassBlock(ConfigBlock):
  """ConfigBlock for the di-tau Missing Mass Calculator algorithm"""
  """Further notes on the tool are available at [DiTauMassTools](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/TauID/DiTauMassTools)."""
  r"""Usage in the ATLAS Run 2 $H\to\tau\tau$ analysis is documented in Section 10 of [ATL-COM-PHYS-2020-721](https://cds.cern.ch/record/2741326)."""
  """A detailed description of the Missing Mass Calculator (MMC) method and its alternatives is given in Chapter 4 of [Michael Hübner's PhD thesis](https://bonndoc.ulb.uni-bonn.de/xmlui/bitstream/handle/20.500.11811/9734/6567.pdf)."""
  r"""
    The MMC method can be applied to had-had, had-lep and lep-lep di-tau decays. Based on the input collections given to the algorithm, the following priority ordering is made internally:

    1. $\tau$-had + $\tau$-had
    1. $\tau$-had + $\mu$
    1. $\tau$-had + e
    1. e + $\mu$
    1. $\mu$ + $\mu$
    1. e + e

    This means that if your event has 2 hadronic tau-jets and 1 electron, the MMC fit will be run under the assumption of a had-had event. To force the MMC fit to consider the 1 electron in a had-lep topology, you'd need to edit the C++ code. Alternatively, if you have determined that some objects should not be used as inputs (e.g. hadronic tau-jet already assigned to top reconstruction, pair of leptons assigned to a Z boson), you should decorate these objects with a flag and use the relevant `container.selection` options of the algorithm. In that way, the MMC fit will only be run on the "left-over" objects.
  """
  """ WARNING """
  r""" The MMC method assumes that the MET in a given event originates mostly from the neutrinos associated to the decay of the di-tau system. If your topology has additional sources of MET (e.g. $t\bar{t}H(\to\tau\tau)$, $W(\to\ell\nu)H(\to\tau\tau)$), the MMC method is not recommended and will give nonsensical answers. See e.g. the ATLAS Run 2 search for BSM $VH(\to\tau\tau)$ in [ATL-COM-PHYS-2022-022](https://cds.cern.ch/record/2799543) where the MMC method is combined with alternatives. Additional neutrinos from the decay of B-hadrons typically do not lead to significant enough MET to be a problem, i.e. $t\bar{t}(\to\text{jets})H(\to\tau\tau)$ should be safe."""

  def __init__(self):
    super(DiTauMassBlock, self).__init__()
    self.addOption('algName', '', type=str,
                   info='optional name to distinguish between multiple instances of the algorithm. The default is `''` (empty string).')
    self.addOption('electrons', '', type=str,
                   info='the input electron container, with a possible selection, in the format `container` or `container.selection`. The default is `''` (empty string).')
    self.addOption('muons', '', type=str,
                   info='the input muon container, with a possible selection, in the format `container` or `container.selection`. The default is `''` (empty string).')
    self.addOption('jets', '', type=str,
                   info='the input jet container, with a possible selection, in the format `container` or `container.selection`. The default is `''` (empty string).')
    self.addOption('taus', '', type=str,
                   info='the input tau-jet container, with a possible selection, in the format `container` or `container.selection`. The default is `''` (empty string).')
    self.addOption('met', '', type=str,
                   info='the input MET container. The default is `''` (empty string).')
    self.addOption('eventSelection', '', type=str,
                   info='optional event filter to run on. The default is `''` (empty string), i.e. all events.')
    self.addOption('saveExtraVariables', False, type=bool,
                   info='whether to save additional output information from the MMC. The default is `False`.')
    self.addOption('floatStopCriterion', True, type=bool,
                   info='whether to activate the floating stopping criterion. The default is `True`.')
    self.addOption('floatStopCriterionMinIter', 10000, type=int,
                   info='minimum number of iteration to activate the floating stopping criterion. The default is 10000.')
    self.addOption('floatStopCriterionCheckFreq', 1000, type=int,
                   info='event frequency for floating stopping criterion to be checked after minimum number of iteration. The default is 1000.')
    self.addOption('floatStopCriterionComp', 0.05, type=float,
                   info='percentage used to assess compatibility for floating stopping criterion to be applied. The default is 5%.') 
    self.addOption('calibration', '2024', type=str,
                   info='the calibration set (string) to use. The default is `2024` (recommended).')
    self.addOption('nSigmaMet', -1, type=int,
                   info='the number (int) of sigmas for the MET resolution scan. The default is `-1` (no scan).')
    self.addOption('useTailCleanup', -1, type=int,
                   info='whether to activate the tail cleanup feature. The default is -1.')
    self.addOption('niterFit2', -1, type=int,
                   info='the number of iterations for each MET scan loop. The default is -1.')
    self.addOption('niterFit3', -1, type=int,
                   info='the number of iterations for each Mnu loop. The default is -1.')
    self.addOption('useTauProbability', 1, type=int,
                   info='whether to apply tau probability (additional PDF term corresponding to the ratio of the neutrino momentum to the reconstructed tau momentum). The default is 1.')
    self.addOption('useMnuProbability', False, type=bool,
                   info='whether to apply $m_\nu$ probability (additional PDF term corresponding to the mass of the neutrino system per tau decay, only applied to leptonic tau decays). The default is `False`.')
    self.addOption('useDefaultSettings', -1, type=int,
                   info='whether to take all default options from the tool itself. The default is -1.')
    self.addOption('useEfficiencyRecovery', -1, type=int,
                   info='whether to enable refitting for failed events, to improve efficiency. The default is -1.')
    self.addOption('useMETdphiLL', False, type=bool,
                   info='whether to parameterise the MET resolution using sumET and dphiLL (only for the lep-lep case). The default is `False`.')
    self.addOption('paramFilePath', 'MMC_params_v051224_angle_noLikelihoodFit.root', type=str,
                   info='path (string) to the ROOT file used with `calibSet` ≥ 2024, containing the PDFs for the likelihood. The default is `MMC_params_v051224_angle_noLikelihoodFit.root` (recommended).')
    self.addOption('doMLNU3P', False, type=bool,
                   info='save information about the reconstruction with the best-fit neutrino kinematics.')
    self.addOption('doMAXW', False, type=bool,
                   info='save information about the reconstruction with the maximum-weight estimator.')
    self.addOption('saveLlhHisto', False, type=bool,
                   info='save likelihood histograms for debugging purpose. If enabled, it can slow down MMC running time.')

    def instanceName (self) :
        """Return the instance name for this block"""
        if self.algName:
            return self.taus.replace('.', '_') + self.algName
        else:
            return self.taus.replace('.', '_')

  def makeAlgs(self, config):

    alg = config.createAlgorithm('CP::DiTauMassCalculatorAlg', 'DiTauMMCAlg')

    alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
    alg.muons,     alg.muonSelection     = config.readNameAndSelection(self.muons)
    alg.jets,      alg.jetSelection      = config.readNameAndSelection(self.jets)
    alg.taus,      alg.tauSelection      = config.readNameAndSelection(self.taus)
    alg.met                              = config.readName(self.met)

    config.addPrivateTool( 'mmcTool', 'DiTauMassTools::MissingMassTool' )
    alg.mmcTool.Decorate                   = False # this sets decorations on EventInfo that are not compatible with systematics
    alg.mmcTool.FloatStoppingCrit          = self.floatStopCriterion
    alg.mmcTool.FloatStoppingCritMinIter   = self.floatStopCriterionMinIter
    alg.mmcTool.FloatStoppingCritCheckFreq = self.floatStopCriterionCheckFreq
    alg.mmcTool.FloatStoppingCritCheckComp = self.floatStopCriterionComp 
    alg.mmcTool.CalibSet                   = self.calibration
    alg.mmcTool.NsigmaMET                  = self.nSigmaMet
    alg.mmcTool.UseTailCleanup             = self.useTailCleanup
    alg.mmcTool.NiterFit2                  = self.niterFit2
    alg.mmcTool.NiterFit3                  = self.niterFit3
    alg.mmcTool.UseTauProbability          = self.useTauProbability
    alg.mmcTool.UseMnuProbability          = self.useMnuProbability
    alg.mmcTool.UseDefaults                = self.useDefaultSettings
    alg.mmcTool.UseEfficiencyRecovery      = self.useEfficiencyRecovery
    alg.mmcTool.UseMETDphiLL               = self.useMETdphiLL
    alg.mmcTool.ParamFilePath              = self.paramFilePath
    alg.mmcTool.SaveLlhHisto               = self.saveLlhHisto

    if config.geometry() is LHCPeriod.Run2:
      alg.mmcTool.BeamEnergy = 6500.0
    else:
      alg.mmcTool.BeamEnergy = 6800.0

    alg.eventSelection = self.eventSelection
    alg.doMAXW         = self.doMAXW
    alg.doMLNU3P       = self.doMLNU3P

    config.addOutputVar('EventInfo', 'mmc_fit_status_%SYS%',  self.algName + 'mmc_fit_status')
    config.addOutputVar('EventInfo', 'mmc_mlm_mass_%SYS%',    self.algName + 'mmc_mlm_mass')
    if self.doMAXW:
      config.addOutputVar('EventInfo', 'mmc_maxw_mass_%SYS%',   self.algName + 'mmc_maxw_mass')
    if self.doMLNU3P:
      config.addOutputVar('EventInfo', 'mmc_mlnu3p_mass_%SYS%', self.algName + 'mmc_mlnu3p_mass')

    if self.saveExtraVariables:
      if self.doMLNU3P:
        config.addOutputVar('EventInfo', 'mmc_mlnu3p_res_4vect_%SYS%',  self.algName + 'mmc_mlnu3p_res_4vect')
        config.addOutputVar('EventInfo', 'mmc_mlnu3p_nu1_4vect_%SYS%',  self.algName + 'mmc_mlnu3p_nu1_4vect')
        config.addOutputVar('EventInfo', 'mmc_mlnu3p_nu2_4vect_%SYS%',  self.algName + 'mmc_mlnu3p_nu2_4vect')
        config.addOutputVar('EventInfo', 'mmc_mlnu3p_tau1_4vect_%SYS%', self.algName + 'mmc_mlnu3p_tau1_4vect')
        config.addOutputVar('EventInfo', 'mmc_mlnu3p_tau2_4vect_%SYS%', self.algName + 'mmc_mlnu3p_tau2_4vect')
      if self.doMAXW:
        config.addOutputVar('EventInfo', 'mmc_maxw_res_4vect_%SYS%',    self.algName + 'mmc_maxw_res_4vect')
        config.addOutputVar('EventInfo', 'mmc_maxw_nu1_4vect_%SYS%',    self.algName + 'mmc_maxw_nu1_4vect')
        config.addOutputVar('EventInfo', 'mmc_maxw_nu2_4vect_%SYS%',    self.algName + 'mmc_maxw_nu2_4vect')
        config.addOutputVar('EventInfo', 'mmc_maxw_tau1_4vect_%SYS%',   self.algName + 'mmc_maxw_tau1_4vect')
        config.addOutputVar('EventInfo', 'mmc_maxw_tau2_4vect_%SYS%',   self.algName + 'mmc_maxw_tau2_4vect')
