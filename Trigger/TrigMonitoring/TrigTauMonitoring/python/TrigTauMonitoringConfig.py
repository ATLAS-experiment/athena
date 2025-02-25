# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoring.DQConfigFlags import DQDataType

import functools

class TrigTauMonAlgBuilder:
  # Configuration flags
  # Can be accessed in the --preExec with e.g.:
  # --preExec 'from TrigTauMonitoring.TrigTauMonitoringConfig import TrigTauMonAlgBuilder; TrigTauMonAlgBuilder.do_total_efficiency=True'
  # But be careful! Changing settings in this way will affect all instances of the TrigTauMonitoring (although you should normally have only 1)

  #=============================================
  # Monitoring modules
  #=============================================
  do_single_tau = True
  do_L1 = True
  do_ditau = True
  do_tag_and_probe = True
  do_truth = True # Truth monitoring will only be used when running on MC data

  #=============================================
  # Configuration
  #=============================================
  do_total_efficiency = False # Enable total efficiency plots (HLT vs Offline, without the L1 matching as in the normal HLT Efficiency plots)

  require_offline_taus = True # Require at least 1 offline good-quality tau (regardless of p_T) on ALL events (except in the Truth monitoring).
  # This will bias the background events variable distributions of online objects, but will better represent the signal events (the events included in the efficiency numerators)

  do_duplicate_var_plots_without_offline_taus = True # Duplicate variable distribution plots without the requirement of at least 1 offline good-quality tau (regardless of p_T) on ALL events (except in the Truth monitoring).

  #=============================================
  # TauID monitoring
  #=============================================
  hlt_tauid_scores = {
    'tracktwoMVA': {
      'GNTau': ('GNTau_Score', 'GNTau_ScoreSigTrans'),
      'DeepSet': ('RNNJetScore', 'RNNJetScoreSigTrans'),
    },
    'tracktwoLLP': {
      'RNNLLP': ('RNNJetScore', 'RNNJetScoreSigTrans'),
    },
    'trackLRT': {
      'RNNLLP': ('RNNJetScore', 'RNNJetScoreSigTrans'),
    },
    # Archive:
    'tracktwoMVABDT': { 'RNN': ('RNNJetScore', 'RNNJetScoreSigTrans') }, # Deprecated
  }

  offline_tauid_scores = {
    'RNN': ('RNNJetScore', 'RNNJetScoreSigTrans'),
  }

  #=============================================
  # Setup for L1Calo monitoring
  #=============================================
  do_alternative_eTAU_monitoring = False # Run the L1 monitoring again, for the Alt (heuristic) eTAU simulation

  def __init__(self, helper):
    from AthenaCommon.Logging import logging
    self.logger = logging.getLogger('TrigTauMonAlgBuilder')

    self.base_path = 'HLT/TauMon'
    self.helper = helper

    # Threshold information for all Phase 1 items
    self.L1_Phase1_thresholds = {} # E_T cuts
    self.L1_Phase1_threshold_mappings = {} # thresholdMappings bit masks

    # Monitoring algorithms, and lists of items to monitor (will be filled on configure())
    self.activate_single_tau = self.do_single_tau
    self.mon_alg_single = None
    self.HLT_single_items = []

    self.activate_ditau = self.do_ditau
    self.mon_alg_ditau = None
    self.HLT_ditau_items = []

    self.activate_tag_and_probe = self.do_tag_and_probe
    self.mon_alg_tag_and_probe = None
    self.HLT_tag_and_probe_items = []

    self.activate_truth = self.do_truth
    self.mon_alg_truth = None
    self.HLT_truth_items = []

    self.activate_L1 = self.do_L1
    self.mon_alg_L1 = None
    self.L1_items = []

    self.configureMode()


  def configureMode(self):
    self.is_mc = False

    self.data_type = self.helper.flags.DQ.DataType
    self.logger.debug('Configuring for %s', self.data_type)

    if self.data_type is DQDataType.MC:
      self.is_mc = True
      self.logger.debug('Enabling Truth monitoring')
    else:
      self.activate_truth = False
      self.logger.debug('Using default monitoring configuration for collisions')
      
    if self.helper.flags.DQ.Environment == "tier0":
      self.do_alternative_eTAU_monitoring = True
    # We don't have any configuration specific for Cosmics or HI (the HLT Tau Monitoring is disabled for this one)
    # If we did, we could specify it here


  def configure(self):
    # First load and classify the list of triggers
    self.configureTriggers()

    # Now create, configure, and book the histograms for all the individual algorithms
    self.logger.info('Creating the Tau monitoring algorithms...')

    if self.activate_single_tau:
      self.configureAlgorithmSingle()

    if self.activate_ditau:
      self.configureAlgorithmDiTau()

    if self.activate_tag_and_probe:
      self.configureAlgorithmTagAndProbe()

    if self.activate_truth:
      self.configureAlgorithmTruth()

    if self.activate_L1:
      self.configureAlgorithmL1()

  @functools.cached_property
  def _L1_Phase1_thresholds_stdmap(self):
    import ROOT
    m = ROOT.std.map[ROOT.std.string, ROOT.float]()
    for item, thr in self.L1_Phase1_thresholds.items():
      m[item] = thr
    return m

  @functools.cached_property
  def _L1_Phase1_threshold_mappings_stdmap(self):
    import ROOT
    m = ROOT.std.map[ROOT.std.string, ROOT.uint64_t]()
    for item, thr in self.L1_Phase1_threshold_mappings.items():
      m[item] = thr
    return m

  @functools.lru_cache(maxsize=1000)
  def getTriggerInfo(self, trigger: str, use_thresholds=True):
    from TrigTauMonitoring.TrigTauInfo import TrigTauInfo
    if use_thresholds:
      return TrigTauInfo(trigger, self._L1_Phase1_thresholds_stdmap, self._L1_Phase1_threshold_mappings_stdmap)
    else:
      return TrigTauInfo(trigger)


  def configureTriggers(self):
    self.logger.info('Configuring triggers')

    from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess, getHLTMenuAccess, getHLTMonitoringAccess
    # The L1 and HLT menus should always be available
    L1_menu = getL1MenuAccess(self.helper.flags)
    HLT_menu = getHLTMenuAccess(self.helper.flags)

    # Try to load the monitoring groups
    HLT_monitoring = getHLTMonitoringAccess(self.helper.flags)
    all_items = HLT_monitoring.monitoredChains(signatures='tauMon', monLevels=['shifter', 't0', 'val'])
    # If the mon groups are not available, fallback to the hard-coded trigger monitoring list
    if not all_items:
      from TrigTauMonitoring.ManualChains import monitored_chains
      self.logger.info('Could not find any monitored tau chains in the HLTMonitoring information. Will use the available items from the fallback trigger list')
      all_items = monitored_chains

    # Classify HLT trigger chains:
    for trigger in all_items:
      # Skip items not included in the menu. This is needed if e.g. using the fallback list on new files without Legacy triggers, or old files
      # without PhI triggers. Also some old SMKs have broken HLTMonitoring DB links, with chains that are not in the Trigger Menu
      if trigger not in HLT_menu: continue

      info = self.getTriggerInfo(trigger, use_thresholds=False)

      if self.activate_single_tau and info.isHLTSingleTau():
        self.HLT_single_items.append(trigger)
      elif self.activate_ditau and info.isHLTDiTau():
        self.HLT_ditau_items.append(trigger)
      elif self.activate_tag_and_probe and info.isHLTTandP():
        self.HLT_tag_and_probe_items.append(trigger)

      if len(info.getL1TauItems()):
        for l1_tau_item in map(str, info.getL1TauItems()): # The objects are of type std::string by default, and 'in' doesn't work properly on them
          is_phase_1 = 'eTAU' in l1_tau_item or 'jTAU' in l1_tau_item or 'cTAU' in l1_tau_item
          if is_phase_1 and l1_tau_item not in self.L1_Phase1_thresholds:
            # We have only one threshold entry, because we don't use eta-dependent thresholds for Phase 1 TAU items:
            self.L1_Phase1_thresholds[l1_tau_item] = float(L1_menu.thresholds()[l1_tau_item]['thrValues'][0]['value'])

            self.L1_Phase1_threshold_mappings[l1_tau_item] = 1 << int(L1_menu.thresholds()[l1_tau_item]['mapping']) # thresholdPatterns property mask

          if self.activate_L1 and f'L1{l1_tau_item}' not in self.L1_items:
            self.L1_items.append(f'L1{l1_tau_item}')

    if self.activate_single_tau:
      self.HLT_single_items.sort()
      self.logger.info(f'Configuring HLT single-tau monitored chains: {self.HLT_single_items}')
      if not self.HLT_single_items:
        self.logger.warning('Empty trigger list, disabling the single-tau monitoring')
        self.activate_single_tau = False

    if self.activate_ditau:
      self.HLT_ditau_items.sort()
      self.logger.info(f'Configuring HLT di-tau monitored chains: {self.HLT_ditau_items}')
      if not self.HLT_ditau_items:
        self.logger.warning('Empty trigger list, disabling the di-tau monitoring')
        self.activate_ditau = False

    if self.activate_tag_and_probe:
      self.HLT_tag_and_probe_items.sort()
      self.logger.info(f'Configuring HLT Tag and Probe tau monitored chains: {self.HLT_tag_and_probe_items}')
      if not self.HLT_tag_and_probe_items:
        self.logger.warning('Empty trigger list, disabling the tag and probe monitoring')
        self.activate_tag_and_probe = False

    if self.activate_truth:
      # We add all chains to the Truth monitoring
      self.HLT_truth_items = self.HLT_single_items 
      self.logger.info(f'Configuring HLT truth tau monitored chains: {self.HLT_truth_items}')
      if not self.HLT_truth_items:
        self.logger.warning('Empty trigger list, disabling the truth tau monitoring')
        self.activate_truth = False

    if self.activate_L1:
      self.L1_items.sort()
      self.logger.info(f'Configuring L1 tau monitored items: {self.L1_items}')
      if not self.L1_items:
        self.logger.warning('Empty trigger list, disabling the L1 tau monitoring')
        self.activate_L1 = False


  def _configureAlgorithm(self, algorithm_factory, name):
    self.logger.info(f'Creating the monitoring algorithm: {name}')
    mon_alg = self.helper.addAlgorithm(algorithm_factory, name)
    mon_alg.L1Phase1Thresholds = self.L1_Phase1_thresholds
    mon_alg.L1Phase1ThresholdPatterns = self.L1_Phase1_threshold_mappings
    return mon_alg


  def configureAlgorithmSingle(self):
    self.mon_alg_single = self._configureAlgorithm(CompFactory.TrigTauMonitorSingleAlgorithm, 'TrigTauMonAlgSingle')
    self.mon_alg_single.TriggerList = self.HLT_single_items
    self.mon_alg_single.DoTotalEfficiency = self.do_total_efficiency
    self.mon_alg_single.RequireOfflineTaus = self.require_offline_taus
    self.mon_alg_single.HLTTauIDScores = self.hlt_tauid_scores
    self.mon_alg_single.OfflineTauIDScores = self.offline_tauid_scores

    self.logger.info('  |- Booking all histograms')
    for trigger in self.HLT_single_items:
      # Efficiencies
      for p in ('1P', '3P'):
          self.bookHLTEffHistograms(self.mon_alg_single, self.base_path, trigger, n_prong=p)

      # Online distributions
      for p in ('0P', '1P', 'MP'):
        self.bookBasicVars(self.mon_alg_single, self.base_path, trigger, n_prong=p, online=True)
        self.bookIDScores(self.mon_alg_single, self.base_path, trigger, n_prong=p, online=True)
        self.bookIDInputScalar(self.mon_alg_single, self.base_path, trigger, n_prong=p, online=True)
      self.bookIDInputTrack(self.mon_alg_single, self.base_path, trigger, online=True)
      self.bookIDInputCluster(self.mon_alg_single, self.base_path, trigger, online=True)

      # Offline distributions
      for p in ('1P', '3P'):
        self.bookBasicVars(self.mon_alg_single, self.base_path, trigger, p, online=False)
        self.bookIDScores(self.mon_alg_single, self.base_path, trigger, p, online=False)
        self.bookIDInputScalar(self.mon_alg_single, self.base_path, trigger, n_prong=p, online=False)
      self.bookIDInputTrack(self.mon_alg_single, self.base_path, trigger, online=False)
      self.bookIDInputCluster(self.mon_alg_single, self.base_path, trigger, online=False)

    if self.do_duplicate_var_plots_without_offline_taus:
      self.mon_alg_single_no_offline = self._configureAlgorithm(CompFactory.TrigTauMonitorSingleAlgorithm, 'TrigTauMonAlgSingleNoOffline')
      self.mon_alg_single_no_offline.TriggerList = self.HLT_single_items
      self.mon_alg_single_no_offline.RequireOfflineTaus = False
      self.mon_alg_single_no_offline.DoOfflineTausDistributions = False
      self.mon_alg_single_no_offline.DoEfficiencyPlots = False
      self.mon_alg_single_no_offline.HLTTauIDScores = self.hlt_tauid_scores

      self.logger.info('  |- Booking all histograms')
      path = f'{self.base_path}/OnlineOnlyVars'
      for trigger in self.HLT_single_items:
        for p in ('0P', '1P', 'MP'):
          self.bookBasicVars(self.mon_alg_single_no_offline, path, trigger, n_prong=p, online=True)
          self.bookIDScores(self.mon_alg_single_no_offline, path, trigger, n_prong=p, online=True)
          self.bookIDInputScalar(self.mon_alg_single_no_offline, path, trigger, n_prong=p, online=True)
        self.bookIDInputTrack(self.mon_alg_single_no_offline, path, trigger, online=True)
        self.bookIDInputCluster(self.mon_alg_single_no_offline, path, trigger, online=True)


  def configureAlgorithmDiTau(self):
    self.mon_alg_ditau = self._configureAlgorithm(CompFactory.TrigTauMonitorDiTauAlgorithm, 'TrigTauMonAlgDiTau')
    self.mon_alg_ditau.TriggerList = self.HLT_ditau_items
    self.mon_alg_ditau.DoTotalEfficiency = self.do_total_efficiency
    self.mon_alg_ditau.RequireOfflineTaus = self.require_offline_taus

    self.logger.info('  |- Booking all histograms')
    for trigger in self.HLT_ditau_items:
      self.bookDiTauHLTEffHistograms(self.mon_alg_ditau, self.base_path, trigger)
      self.bookDiTauVars(self.mon_alg_ditau, self.base_path, trigger)


  def configureAlgorithmTagAndProbe(self):
    self.mon_alg_tag_and_probe = self._configureAlgorithm(CompFactory.TrigTauMonitorTandPAlgorithm, 'TrigTauMonAlgTandP')
    self.mon_alg_tag_and_probe.TriggerList = self.HLT_tag_and_probe_items
    self.mon_alg_tag_and_probe.RequireOfflineTaus = self.require_offline_taus

    self.logger.info('  |- Booking all histograms')
    for trigger in self.HLT_tag_and_probe_items:
      self.bookTAndPHLTEffHistograms(self.mon_alg_tag_and_probe, self.base_path, trigger)
      self.bookTAndPVars(self.mon_alg_tag_and_probe, self.base_path, trigger)


  def configureAlgorithmTruth(self):
    self.mon_alg_truth = self._configureAlgorithm(CompFactory.TrigTauMonitorTruthAlgorithm, 'TrigTauMonAlgTruth')
    self.mon_alg_truth.TriggerList = self.HLT_truth_items

    self.logger.info('  |- Booking all histograms')
    for trigger in self.HLT_truth_items:
      for p in ('1P', '3P'):
        self.bookTruthEfficiency(self.mon_alg_truth, self.base_path, trigger, n_prong=p)
        self.bookTruthVars(self.mon_alg_truth, self.base_path, trigger, n_prong=p)


  def configureAlgorithmL1(self):
    has_xtob_etau_rois = 'L1_eTauxRoI' in self.helper.flags.Input.Collections or self.helper.flags.DQ.Environment == "tier0"

    self.mon_alg_L1 = self._configureAlgorithm(CompFactory.TrigTauMonitorL1Algorithm, 'TrigTauMonAlgL1')
    self.mon_alg_L1.TriggerList = self.L1_items
    self.mon_alg_L1.RequireOfflineTaus = self.require_offline_taus
    if not has_xtob_etau_rois:
      self.logger.info('  |- No L1_eTauxRoI container is available: e/cTAU BDT scores will be set to 0')
      self.mon_alg_L1.Phase1L1eTauxRoIKey = ''

    self.logger.info('  |- Booking all histograms')
    for trigger in self.L1_items:
      for p in ('1P', '3P'):
        self.bookL1EffHistograms(self.mon_alg_L1, self.base_path, trigger, n_prong=p)
      self.bookL1Vars(self.mon_alg_L1, self.base_path, trigger)

    if self.do_duplicate_var_plots_without_offline_taus:
      self.mon_alg_L1_no_offline = self._configureAlgorithm(CompFactory.TrigTauMonitorL1Algorithm, 'TrigTauMonAlgL1NoOffline')
      self.mon_alg_L1_no_offline.TriggerList = self.L1_items
      self.mon_alg_L1_no_offline.RequireOfflineTaus = False
      self.mon_alg_L1_no_offline.DoEfficiencyPlots = False
      if not has_xtob_etau_rois:
        self.logger.info('  |- No L1_eTauxRoI container is available: e/cTAU BDT scores will be set to 0')
        self.mon_alg_L1_no_offline.Phase1L1eTauxRoIKey = ''

      self.logger.info('  |- Booking all histograms')
      path = f'{self.base_path}/OnlineOnlyVars'
      for trigger in self.L1_items:
        self.bookL1Vars(self.mon_alg_L1_no_offline, path, trigger)

    if self.do_alternative_eTAU_monitoring:
      self.mon_alg_L1_alt = self._configureAlgorithm(CompFactory.TrigTauMonitorL1Algorithm, 'TrigTauMonAlgL1eTAUAlt')
      self.mon_alg_L1_alt.Phase1L1eTauRoIKey = 'L1_eTauRoIAltSim' # Use alternative RoIs (with heuristic eTAU algorithm simulation)
      self.mon_alg_L1_alt.SelectL1ByETOnly = True # We don't have threshold patterns for the Alt RoIs, so we match by ET only
      self.mon_alg_L1_alt.RequireOfflineTaus = False
      self.mon_alg_L1_alt.Phase1L1eTauxRoIKey = ''

      l1_items = [item for item in self.L1_items if 'eTAU' in item and not self.getTriggerInfo(item).isL1TauIsolated()] # Only non-isolated eTAU items
      self.mon_alg_L1_alt.TriggerList = l1_items

      self.logger.info('  |- Booking all histograms')
      path = f'{self.base_path}/L1eTAUAlt'
      for trigger in l1_items:
        for p in ('1P', '3P'):
          self.bookL1EffHistograms(self.mon_alg_L1_alt, path, trigger, n_prong=p)
        self.bookL1Vars(self.mon_alg_L1_alt, path, trigger)


  def bookHLTEffHistograms(self, mon_alg, base_path, trigger, n_prong):
    mon_group_name = f'{trigger}_HLT_Efficiency_{n_prong}'
    mon_group_path = f'{base_path}/HLT_Efficiency/{trigger}/HLT_Efficiency_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax, eff='HLT', high_pt=False, coarse=False):
      pass_flag = f'{eff}_pass'
      sfx = ''

      if high_pt:
        pass_flag += '_highPt'
        sfx += '_highPt'
        xlabel += ' (p_{T} > p_{T}^{thr} + 20 GeV)'
      elif coarse: sfx = '_coarse'

      mon_group.defineHistogram(f'{pass_flag},{xvariable};Eff{eff}_{xvariable}{sfx}_wrt_Offline',
                                title=f'{eff} Efficiency {trigger} {n_prong}; {xlabel}; Efficiency',
                                type='TEfficiency', xbins=xbins, xmin=xmin, xmax=xmax, opt='kAlwaysCreate')

    coarse_binning = self.getCustomPtBinning(trigger)

    eff_list = ['HLT'] + (['Total'] if self.do_total_efficiency else [])
    for eff in eff_list:
      defineEachStepHistograms('tauPt', 'p_{T} [GeV]', 60, 0.0, 300., eff)
      defineEachStepHistograms('tauPt', 'p_{T} [GeV]', coarse_binning, coarse_binning[0], coarse_binning[-1], eff, coarse=True)
      defineEachStepHistograms('tauEta', '#eta', 13, -2.6, 2.6, eff)
      defineEachStepHistograms('tauPhi', '#phi', 16, -3.2, 3.2, eff)
      defineEachStepHistograms('tauEta', '#eta', 13, -2.6, 2.6, eff, high_pt=True)
      defineEachStepHistograms('tauPhi', '#phi', 16, -3.2, 3.2, eff, high_pt=True)
      defineEachStepHistograms('averageMu', '#LT#mu#GT', 10, 0, 80, eff)

    # Save quantities in TTree for offline analysis
    mon_group.defineTree('tauPt,tauEta,tauPhi,averageMu,HLT_pass;HLTEffTree', 
                        treedef='tauPt/F:tauEta/F:tauPhi/F:averageMu/F:HLT_pass/I')


  def bookIDInputScalar(self, mon_alg, base_path, trigger, n_prong, online):
    type_str = 'HLT' if online else 'Offline'
    mon_group_name = f'{trigger}_ID_{type_str}_InputScalar_{n_prong}'
    mon_group_path = f'{base_path}/TauIDVars/InputScalar_{n_prong}/{trigger}/{type_str}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('centFrac', title=f'Centrality Fraction ({n_prong}); centFrac; Events', xbins=50, xmin=-0.05, xmax=1.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('etOverPtLeadTrk', title=f'etOverPtLeadTrk log ({n_prong}); etOverPtLeadTrk_log; Events', xbins=60, xmin=-3, xmax=3, opt='kAlwaysCreate')
    mon_group.defineHistogram('dRmax', title=f'max dR of associated tracks ({n_prong}); dRmax; Events', xbins=50, xmin=-0.1, xmax=0.3, opt='kAlwaysCreate')
    mon_group.defineHistogram('absipSigLeadTrk', title=f'AbsIpSigLeadTrk ({n_prong}); absipSigLeadTrk; Events', xbins=25, xmin=0.0, xmax=20.0, opt='kAlwaysCreate')
    mon_group.defineHistogram('sumPtTrkFrac', title=f'SumPtTrkFrac ({n_prong}); SumPtTrkFrac; Events', xbins=50, xmin=-0.5, xmax=1.1, opt='kAlwaysCreate')
    mon_group.defineHistogram('emPOverTrkSysP', title=f'EMPOverTrkSysP log ({n_prong}); EMPOverTrkSysP_log; Events', xbins=50, xmin=-5, xmax=3, opt='kAlwaysCreate')
    mon_group.defineHistogram('ptRatioEflowApprox', title=f'ptRatioEflowApprox ({n_prong}); ptRatioEflowApprox; Events', xbins=50, xmin=0.0, xmax=2.0, opt='kAlwaysCreate')
    mon_group.defineHistogram('mEflowApprox', title=f'mEflowApprox log ({n_prong}); mEflowApprox_log; Events', xbins=50, xmin=0, xmax=5, opt='kAlwaysCreate')
    mon_group.defineHistogram('ptDetectorAxis', title=f'ptDetectorAxis log ({n_prong}); ptDetectorAxis_log; Events', xbins=50, xmin=0, xmax=5, opt='kAlwaysCreate')
    if n_prong == 'MP' or n_prong == '3P':  
      mon_group.defineHistogram('massTrkSys', title=f'massTrkSys log ({n_prong}); massTrkSys_log; Events', xbins=50, xmin=0, xmax=3, opt='kAlwaysCreate')
      mon_group.defineHistogram('trFlightPathSig', title=f'trFlightPathSig ({n_prong}); trFlightPathSig; Events', xbins=100, xmin=-20, xmax=40, opt='kAlwaysCreate')


  def bookIDInputTrack(self, mon_alg, base_path, trigger, online):
    type_str = 'HLT' if online else 'Offline'
    mon_group_name = f'{trigger}_ID_{type_str}_InputTrack'
    mon_group_path = f'{base_path}/TauIDVars/InputTrack/{trigger}/{type_str}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('n_track', title='Number of tracks; N_{track}; Events', xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_pt_log', title='track_pt_log; track_pt_log; Events', xbins=20, xmin=2, xmax=7, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_pt_jetseed_log', title='track_pt_jetseed_log; track_pt_jetseed_log; Events', xbins=50, xmin=2, xmax=7, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_eta', title='Track #eta; #eta; Events', xbins=26, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_phi', title='Track #phi; #phi; Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_dEta', title='Track #Delta#eta; #Delta#eta; Events', xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_dPhi', title='Track #Delta#phi; #Delta#phi; Events', xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_d0_abs_log', title='track_d0_abs_log; track_d0_abs_log; Events', xbins=50, xmin=-7, xmax=2, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_z0sinthetaTJVA_abs_log', title='track_z0sinthetaTJVA_abs_log; track_z0sinthetaTJVA_abs_log; Events', xbins=50, xmin=-10, xmax=4, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_nIBLHitsAndExp', title='track_nIBLHitsAndExp; track_nIBLHitsAndExp; Events', xbins=3, xmin=0, xmax=3, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_nPixelHitsPlusDeadSensors', title='track_nPixelHitsPlusDeadSensors; track_nPixelHitsPlusDeadSensors; Events', xbins=11, xmin=0, xmax=11, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_nSCTHitsPlusDeadSensors', title='track_nSCTHitsPlusDeadSensors; track_nSCTHitsPlusDeadSensors; Events', xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_eta,track_phi', type='TH2F', title='Track #eta vs #phi; #eta; #phi', xbins=26, xmin=-2.6, xmax=2.6, ybins=16, ymin=-3.2, ymax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('track_dEta,track_dPhi', type='TH2F', title='Track #Delta#eta vs #Delta#phi; #Delta#eta; #Delta#phi', xbins=100, xmin=-0.5, xmax=0.5, ybins=100, ymin=-0.5, ymax=0.5, opt='kAlwaysCreate')


  def bookIDInputCluster(self, mon_alg, base_path, trigger, online):
    type_str = 'HLT' if online else 'Offline'
    mon_group_name = f'{trigger}_ID_{type_str}_InputCluster'
    mon_group_path = f'{base_path}/TauIDVars/InputCluster/{trigger}/{type_str}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('n_cluster', title='Number of clusters; N_{cluster}; Events', xbins=30, xmin=0, xmax=30, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_et_log', title='cluster_et_log; cluster_et_log; Events', xbins=30, xmin=0, xmax=6, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_pt_jetseed_log', title='cluster_pt_jetseed_log; cluster_pt_jetseed_log; Events', xbins=50, xmin=2, xmax=7, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_eta', title='Cluster #eta; #eta; Events', xbins=26, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_phi', title='Cluster #phi; #phi; Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_dEta', title='Cluster #Delta#eta; #Delta#eta; Events', xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_dPhi', title='Cluster #Delta#phi; #Delta#phi; Events', xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_SECOND_R_log10', title='cluster_SECOND_R_log10; cluster_SECOND_R_log10; Events', xbins=50, xmin=-3, xmax=7, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_SECOND_LAMBDA_log10', title='cluster_SECOND_LAMBDA_log10; cluster_SECOND_LAMBDA_log10; Events', xbins=50, xmin=-3, xmax=7, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_CENTER_LAMBDA_log10', title='cluster_CENTER_LAMBDA_log10; cluster_CENTER_LAMBDA_log10; Events', xbins=50, xmin=-2, xmax=5, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_eta,cluster_phi', type='TH2F', title='Cluster #eta vs #phi; #eta; #phi', xbins=26, xmin=-2.6, xmax=2.6, ybins=16, ymin=-3.2, ymax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('cluster_dEta,cluster_dPhi', type='TH2F', title='Cluster #Delta#eta vs #Delta#phi; #Delta#eta; #Delta#phi', xbins=100, xmin=-0.5, xmax=0.5, ybins=100, ymin=-0.5, ymax=0.5, opt='kAlwaysCreate')


  def bookIDScores(self, mon_alg, base_path, trigger, n_prong, online):
    info = self.getTriggerInfo(trigger)
    store_all = info.getHLTTauID() in ['idperf', 'perf']
    
    if online:
      if info.getHLTTauType() not in self.hlt_tauid_scores: return
      variables = {
        tau_id: p
        for tau_id, p in self.hlt_tauid_scores[info.getHLTTauType()].items()
        if tau_id == info.getHLTTauID() or store_all
      }
    else:
      variables = self.offline_tauid_scores

    type_str = 'HLT' if online else 'Offline'
    mon_group_name = f'{trigger}_{type_str}_IDScores_{n_prong}'
    mon_group_path = f'{base_path}/basicVars/{trigger}/{type_str}_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)
 
    for tau_id, (score, score_sig_trans) in variables.items():
      if tau_id in ['RNN', 'DeepSet', 'RNNLLP', 'GNTau']: xbins, xmax = 20, 1
      else: xbins, xmax = 100, 5

      mon_group.defineHistogram(f'{tau_id}_TauIDScore', title=f'{type_str} {tau_id} TauID score; TauID score; Events', xbins=xbins, xmin=0, xmax=xmax, opt='kAlwaysCreate')
      mon_group.defineHistogram(f'{tau_id}_TauIDScoreSigTrans', title=f'{type_str} {tau_id} TauID score sig. transformed; TauID score sig. transformed; Events', xbins=xbins, xmin=0, xmax=xmax, opt='kAlwaysCreate')


  def bookBasicVars(self, mon_alg, base_path, trigger, n_prong, online):
    type_str = 'HLT' if online else 'Offline'
    mon_group_name = f'{trigger}_{type_str}_basicVars_{n_prong}'
    mon_group_path = f'{base_path}/basicVars/{trigger}/{type_str}_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)
 
    binning = self.getCustomPtBinning(trigger, fine=True)

    mon_group.defineHistogram('Pt', title=f'{type_str} p_{{T}}; p_{{T}} [GeV]; Events', xbins=binning, opt='kAlwaysCreate')
    mon_group.defineHistogram('Eta', title=f'{type_str} #eta; #eta; Events', xbins=26, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('Phi', title=f'{type_str} #phi; #phi; Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('nTrack', title=f'{type_str} Number of tracks; N_{{track}}; Events', xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    mon_group.defineHistogram('Eta,Phi', type='TH2F', title=f'{type_str} #eta vs #phi; #eta; #phi', xbins=26, xmin=-2.6, xmax=2.6, ybins=16, ymin=-3.2, ymax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('Pt,Phi', type='TH2F',  title=f'{type_str} p_{{T}} vs #phi; p_{{T}} [GeV]; #phi', xbins=binning, ybins=16, ymin=-3.2, ymax=3.2, opt='kAlwaysCreate') 
    mon_group.defineHistogram('Pt,Eta', type='TH2F',  title=f'{type_str} p_{{T}} vs #eta; p_{{T}} [GeV]; #eta', xbins=binning, ybins=26, ymin=-2.6, ymax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('nIsoTrack', title=f'{type_str} Number of isolation tracks; N_{{track}}^{{iso}}; Events', xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    mon_group.defineHistogram('averageMu', title=f'{type_str} Average #mu; #LT#mu$GT; Events', xbins=20, xmin=0, xmax=80, opt='kAlwaysCreate')
    mon_group.defineHistogram('TauVertexX', title=f'{type_str} Tau Vertex X; x [mm]; Events', xbins=100, xmin=-1, xmax=1, opt='kAlwaysCreate')
    mon_group.defineHistogram('TauVertexY', title=f'{type_str} Tau Vertex Y; y [mm]; Events', xbins=100, xmin=-2, xmax=0, opt='kAlwaysCreate')
    mon_group.defineHistogram('TauVertexZ', title=f'{type_str} Tau Vertex Z; z [mm]; Events', xbins=120, xmin=-120, xmax=120, opt='kAlwaysCreate')


  def bookDiTauHLTEffHistograms(self, mon_alg, base_path, trigger):
    mon_group_name = f'{trigger}_DiTauHLT_Efficiency'
    mon_group_path = f'{base_path}/DiTauHLT_Efficiency/{trigger}/DiTauHLT_Efficiency'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax, eff='HLT', high_pt=False):
      pass_flag = f'{eff}_pass'
      sfx = ''

      if high_pt: 
        pass_flag += '_highPt'
        sfx += '_highPt'
        xlabel += ' (p_{T}^{1,2} > p_{T}^{thr 1,2} + 20 GeV)'

      mon_group.defineHistogram(f'{pass_flag},{xvariable};EffDiTau{eff}_{xvariable}{sfx}_wrt_Offline',
                                title=f'DiTau {eff} Efficiency {trigger};{xlabel};Efficiency',
                                type='TEfficiency', xbins=xbins, xmin=xmin, xmax=xmax, opt='kAlwaysCreate')

    eff_list = ['HLT'] + (['Total'] if self.do_total_efficiency else [])
    for eff in eff_list:
      defineEachStepHistograms('dR', '#Delta R(#tau,#tau)', 20, 0, 4, eff)
      defineEachStepHistograms('dEta', '#Delta#eta(#tau,#tau)', 20, 0, 4, eff)
      defineEachStepHistograms('dPhi', '#Delta#phi(#tau,#tau)', 8, -3.2, 3.2, eff)

      defineEachStepHistograms('dR', '#Delta R(#tau,#tau)', 20, 0, 4, eff, high_pt=True)
      defineEachStepHistograms('dEta', '#Delta#eta(#tau,#tau)', 20, 0, 4, eff, high_pt=True)
      defineEachStepHistograms('dPhi', '#Delta#phi(#tau,#tau)', 8, -3.2, 3.2, eff, high_pt=True)
      defineEachStepHistograms('averageMu', '#LT#mu#GT', 10, 0, 80, eff)

    # Save quantities in TTree for offline analysis
    mon_group.defineTree('dR,dEta,dPhi,averageMu,HLT_pass;DiTauHLTEffTree', 
                        treedef='dR/F:dEta/F:dPhi/F:averageMu/F:HLT_pass/I')
  

  def bookDiTauVars(self, mon_alg, base_path, trigger):
    mon_group_name = f'{trigger}_DiTauVars'
    mon_group_path = f'{base_path}/DiTauVars/{trigger}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('leadHLTEt,subleadHLTEt', type='TH2F', title='p_{T}^{lead} vs p_{T}^{sublead}; p_{T}^{lead} [GeV]; p_{T}^{sublead} [GeV]',
                              xbins=50, xmin=0, xmax=250, ybins=50, ymin=0, ymax=250, opt='kAlwaysCreate')
    mon_group.defineHistogram('leadHLTEta,subleadHLTEta', type='TH2F', title='#eta_{lead} vs #eta_{sublead}; #eta_{lead}; #eta_{sublead}',
                              xbins=26, xmin=-2.6, xmax=2.6, ybins=26, ymin=-2.6, ymax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('leadHLTPhi,subleadHLTPhi', type='TH2F', title='#phi_{lead} vs #phi_{sublead}; #phi_{lead}; #phi{sublead}',
                              xbins=16, xmin=-3.2, xmax=3.2, ybins=16, ymin=-3.2, ymax=3.2, opt='kAlwaysCreate') 
    mon_group.defineHistogram('dR', title='#Delta R(#tau,#tau); #Delta R(#tau,#tau); Events', xbins=40, xmin=0, xmax=4, opt='kAlwaysCreate')
    mon_group.defineHistogram('dEta', title='#Delta#eta(#tau,#tau); #Delta#eta(#tau,#tau); Events', xbins=40, xmin=0, xmax=4, opt='kAlwaysCreate')
    mon_group.defineHistogram('dPhi', title='#Delta#phi(#tau,#tau); #Delta#phi(#tau,#tau); Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')

    mon_group.defineHistogram('Pt', title='p_{T}(#tau,#tau); p_{T} [GeV]; Events', xbins=50, xmin=0, xmax=250, opt='kAlwaysCreate')
    mon_group.defineHistogram('Eta', title='#eta(#tau,#tau); #eta(#tau,#tau); Events', xbins=26, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('Phi', title='#phi(#tau,#tau); #phi(#tau,#tau); Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('M', title='m(#tau,#tau); m_{#tau,#tau}; Events', xbins=50, xmin=0, xmax=250, opt='kAlwaysCreate')
    mon_group.defineHistogram('dPt', title='#Delta p_{T}(#tau, #tau); p_{T} [GeV]; Events', xbins=20, xmin=0, xmax=200, opt='kAlwaysCreate')  

    mon_group.defineTree('leadHLTEt,subleadHLTEt,leadHLTEta,subleadHLTEta,leadHLTPhi,subleadHLTPhi,dR,dEta,dPhi,Pt,Eta,Phi,M,dPt;DiTauVarsTree',
                        treedef='leadHLTEt/F:subleadHLTEt/F:leadHLTEta/F:subleadHLTEta/F:leadHLTPhi/F:subleadHLTPhi/F:dR/F:dEta/F:dPhi/F:Pt/F:Eta/F:Phi/F:M/F:dPt/F')


  def bookTAndPHLTEffHistograms(self, mon_alg, base_path, trigger):
    mon_group_name = f'{trigger}_TAndPHLT_Efficiency'
    mon_group_path = f'{base_path}/TAndPHLT_Efficiency/{trigger}/TAndPHLT_Efficiency'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax, high_pt=False, coarse=False):
      pass_flag = 'HLT_pass'
      sfx = ''
      
      if high_pt:
        pass_flag += '_highPt'
        sfx += '_highPt'
        xlabel += ' (p_{T}^{#tau} > p_{T}^{#tau thr} + 20 GeV)'
      elif coarse:
        sfx += '_coarse'

      mon_group.defineHistogram(f'{pass_flag},{xvariable};EffTAndPHLT_{xvariable}{sfx}_wrt_Offline',
                                title=f'TAndP HLT Efficiency {trigger}; {xlabel}; Efficiency',
                                type='TEfficiency', xbins=xbins, xmin=xmin, xmax=xmax, opt='kAlwaysCreate')

    coarse_binning = self.getCustomPtBinning(trigger)

    defineEachStepHistograms('tauPt', 'p_{T}^{#tau} [GeV]', 60, 0.0, 300)
    defineEachStepHistograms('tauPt', 'p_{T}^{#tau} [GeV]', coarse_binning, coarse_binning[0], coarse_binning[-1], coarse=True)
    defineEachStepHistograms('tauEta', '#eta_{#tau}', 13, -2.6, 2.6)
    defineEachStepHistograms('tauPhi', '#phi_{#tau}', 16, -3.2, 3.2)
    defineEachStepHistograms('tauEta', '#eta_{#tau}', 13, -2.6, 2.6, high_pt=True)
    defineEachStepHistograms('tauPhi', '#phi_{#tau}', 16, -3.2, 3.2, high_pt=True)
    defineEachStepHistograms('dR', '#Delta R(#tau,lep)', 20, 0, 4)
    defineEachStepHistograms('dEta', '#Delta#eta(#tau,lep)', 20, 0,4)
    defineEachStepHistograms('dPhi', '#Delta#phi(#tau,lep)', 8, -3.2, 3.2)
    defineEachStepHistograms('averageMu', '#LT#mu#GT', 10, 0, 80)

    # Save quantities in TTree for offline analysis
    mon_group.defineTree('tauPt,tauEta,tauPhi,dR,dEta,dPhi,averageMu,HLT_pass;TAndPHLTEffTree',
                         treedef='tauPt/F:tauEta/F:tauPhi/F:dR/F:dEta/F:dPhi/F:averageMu/F:HLT_pass/I')


  def bookTAndPVars(self, mon_alg, base_path, trigger):
    mon_group_name = f'{trigger}_TAndPVars'
    mon_group_path = f'{base_path}/TAndPVars/{trigger}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('dR', title='#Delta R(#tau,lep); #Delta R(#tau,lep); Events', xbins=40, xmin=0, xmax=4, opt='kAlwaysCreate')
    mon_group.defineHistogram('dEta', title='#Delta#eta(#tau,lep); #Delta#eta(#tau,lep); Events', xbins=40, xmin=0, xmax=4, opt='kAlwaysCreate')
    mon_group.defineHistogram('dPhi', title='#Delta#phi(#tau,lep); #Delta#phi(#tau,lep); Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')

    mon_group.defineHistogram('Pt', title='p_{T}(#tau,lep); p_{T} [GeV]; Events', xbins=50, xmin=0, xmax=250, opt='kAlwaysCreate')
    mon_group.defineHistogram('Eta', title='#eta(#tau,lep); #eta; Events', xbins=26, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('Phi', title='#phi(#tau,lep); #phi; Events', xbins=16, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('M', title='m(#tau,lep); m_{#tau,lep}; Events', xbins=50, xmin=0, xmax=250, opt='kAlwaysCreate')
    mon_group.defineHistogram('dPt', title='#Delta p_{T}(#tau,lep); p_{T} [GeV]; Events', xbins=20, xmin=0, xmax=200, opt='kAlwaysCreate')


  def bookTruthEfficiency(self, mon_alg, base_path, trigger, n_prong):
    mon_group_name = f'{trigger}_Truth_Efficiency_{n_prong}'
    mon_group_path = f'{base_path}/Truth_Efficiency/{trigger}/Truth_Efficiency_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    info = self.getTriggerInfo(trigger)
  
    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax, high_pt=False, coarse=False):
      pass_flag = 'HLT_pass'
      sfx = ''
      
      if high_pt:
        pass_flag += '_highPt'
        sfx += '_highPt'
        if info.isHLTDiTau():
          xlabel += ' (p_{T}^{#tau} > p_{T}^{#tau min thr} + 20 GeV)'
        else:
          xlabel += ' (p_{T}^{#tau} > p_{T}^{#tau thr} + 20 GeV)'
      elif coarse:
        sfx += '_coarse'

      mon_group.defineHistogram(f'{pass_flag},{xvariable};EffHLT_{xvariable}{sfx}_wrt_Truth',
                                title=f'HLT Efficiency {trigger} {n_prong}; {xlabel}; Efficiency',
                                type='TEfficiency', xbins=xbins, xmin=xmin, xmax=xmax)

    coarse_binning = self.getCustomPtBinning(trigger)

    defineEachStepHistograms('pt_vis', 'p_{T, vis} [GeV]', 60, 0.0, 300)
    if info.isHLTSingleTau() or info.isHLTTandP(): defineEachStepHistograms('pt_vis', 'p_{T, vis} [GeV]', coarse_binning, coarse_binning[0], coarse_binning[-1], coarse=True)
    defineEachStepHistograms('eta_vis', '#eta_{vis}', 13, -2.6, 2.6)
    defineEachStepHistograms('phi_vis', '#phi_{vis}', 16, -3.2, 3.2)
    defineEachStepHistograms('eta_vis', '#eta_{vis}', 13, -2.6, 2.6, high_pt=True)
    defineEachStepHistograms('phi_vis', '#phi_{vis}', 16, -3.2, 3.2, high_pt=True)


  def bookTruthVars(self, mon_alg, base_path, trigger, n_prong):
    mon_group_name = f'{trigger}_TruthVars_{n_prong}'
    mon_group_path = f'{base_path}/TruthVars/{trigger}/TruthVars_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('pt_vis,PtRatio', title='p_{T} ratio vs p_{T, vis}; p_{T, vis} [GeV]; (p_{T}^{reco} - p_{T, vis}^{truth})/p_{T, vis}^{truth}', type='TProfile', xbins=21, xmin=20, xmax=250)
    mon_group.defineHistogram('eta_vis,PtRatio', title='p_{T} ratio vs #eta_{vis}; #eta_{vis}; (p_{T}^{reco} - p_{T, vis}^{truth})/p_{T, vis}^{truth}', type='TProfile', xbins=21, xmin=-3, xmax=3)
    mon_group.defineHistogram('phi_vis,PtRatio', title='p_{T} ratio vs #phi_{vis}; #phi_{vis}; (p_{T}^{reco} - p_{T, vis}^{truth})/p_{T, vis}^{truth}', type='TProfile', xbins=21, xmin=-3, xmax=3)

    mon_group.defineHistogram('pt_vis', title='p_{T, vis}; p_{T, vis}; Events', xbins=50, xmin=0, xmax=250)
    mon_group.defineHistogram('eta_vis', title='#eta_{vis}; #eta_{vis}; Events', xbins=26, xmin=-2.6, xmax=2.6)
    mon_group.defineHistogram('phi_vis', title='#phi_{vis}; #phi_{vis}; Events', xbins=16, xmin=-3.2, xmax=3.2)

  
  def bookL1EffHistograms(self, mon_alg, base_path, trigger, n_prong):
    mon_group_name = f'{trigger}_L1_Efficiency_{n_prong}'
    mon_group_path = f'{base_path}/L1_Efficiency/{trigger}/L1_Efficiency_{n_prong}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax, high_pt=False, coarse=False):
      pass_flag = 'L1_pass'
      sfx = ''
      
      if high_pt:
        pass_flag += '_highPt'
        sfx += '_highPt'
        xlabel += ' (p_{T}^{#tau} > p_{T}^{#tau thr} + 20 GeV)'
      elif coarse:
        sfx += '_coarse'

      mon_group.defineHistogram(f'{pass_flag},{xvariable};EffL1_{xvariable}{sfx}_wrt_Offline',
                                title=f'L1 Efficiency {trigger} {n_prong}; {xlabel}; Efficiency',
                                type='TEfficiency', xbins=xbins, xmin=xmin, xmax=xmax, opt='kAlwaysCreate')

    coarse_binning = self.getCustomPtBinning(trigger)

    defineEachStepHistograms('tauPt', 'p_{T} [GeV]', 60, 0, 300)
    defineEachStepHistograms('tauPt', 'p_{T} [GeV]', coarse_binning, coarse_binning[0], coarse_binning[-1], coarse=True)
    defineEachStepHistograms('tauEta', '#eta', 13, -2.6, 2.6)
    defineEachStepHistograms('tauPhi', '#phi', 16, -3.2, 3.2)
    defineEachStepHistograms('tauEta', '#eta', 13, -2.6, 2.6, high_pt=True)
    defineEachStepHistograms('tauPhi', '#phi', 16, -3.2, 3.2, high_pt=True)
    defineEachStepHistograms('averageMu', '#LT#mu#GT', 10, 0, 80)


  def bookL1Vars(self, mon_alg, base_path, trigger):
    mon_group_name = f'{trigger}_L1Vars'
    mon_group_path = f'{base_path}/L1Vars/{trigger}'
    mon_group = self.helper.addGroup(mon_alg, mon_group_name, mon_group_path)

    mon_group.defineHistogram('L1RoIEt,L1RoIEta', type='TH2F', title='L1 RoI E_{T} vs #eta; E_{T} [GeV]; #eta',
                              xbins=60, xmin=0, xmax=300, 
                              ybins=60, ymin=-2.6, ymax=2.6,  opt='kAlwaysCreate')
    mon_group.defineHistogram('L1RoIEt,L1RoIPhi', type='TH2F', title='L1 RoI E_{T} vs #phi; E_{T} [GeV]; #phi',
                              xbins=60, xmin=0, xmax=300, 
                              ybins=60, ymin=-3.2, ymax=3.2,  opt='kAlwaysCreate')
    mon_group.defineHistogram('L1RoIEta,L1RoIPhi', type='TH2F', title='L1 RoI #eta vs #phi; #eta; #phi',
                              xbins=60, xmin=-2.6, xmax=2.6, 
                              ybins=60, ymin=-3.2, ymax=3.2,  opt='kAlwaysCreate')
    mon_group.defineHistogram('L1RoIEta', title='L1 RoI #eta; #eta; RoIs', xbins=60, xmin=-2.6, xmax=2.6, opt='kAlwaysCreate')
    mon_group.defineHistogram('L1RoIPhi', title='L1 RoI #phi; #phi; RoIs', xbins=60, xmin=-3.2, xmax=3.2, opt='kAlwaysCreate')
    mon_group.defineHistogram('L1RoIEt', title='L1 RoI E_{T}; E_{T} [GeV]; RoIs', xbins=60, xmin=0, xmax=300, opt='kAlwaysCreate')

    if 'eTAU' in trigger:
        mon_group.defineHistogram('L1eFexRoIRCore', title='L1 eTAU RoI rCore Isolation; rCore Isolation; RoIs', xbins=250, xmin=0, xmax=1, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1eFexRoIRHad' , title='L1 eTAU RoI rHad Isolation; rHad Isolation; RoIs', xbins=250, xmin=0, xmax=1, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1eFexRoIBDTScore' , title='L1 eTAU RoI BDT score; BDT Score; RoIs', xbins=128, xmin=512, xmax=1024, opt='kAlwaysCreate')

    elif 'cTAU' in trigger:
        mon_group.defineHistogram('L1eFexRoIRCore', title='L1 eTAU RoI rCore Isolation; eTAU rCore Isolation; RoIs', xbins=250, xmin=0, xmax=1, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1eFexRoIRHad', title='L1 eTAU RoI rHad Isolation; eTAU rHad Isolation; RoIs', xbins=250, xmin=0, xmax=1, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1cTauRoITopoMatch', title='L1Topo match between eTAU and jTAU RoI; Match; RoIs', xbins=2, xmin=0, xmax=2, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1jFexRoIIso', title='L1 jTAU RoI Isolation; E_{T}^{jTAU Iso} [GeV]; RoIs', xbins=25, xmin=0, xmax=50, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1cTauMatchedRoIIso', title='L1 cTAU Isolation score; E_{T}^{jTAU Iso}/E_{T}^{eTAU}; RoIs', xbins=50, xmin=0, xmax=5, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1RoIcTauMatchedEtRatio', title='Et ratio between matched eTAU and jTAU RoIs; E_{T}^{jTAU}/E_{T}^{eTAU}; RoIs', xbins=40, xmin=0, xmax=4, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1eFexRoIBDTScore' , title='L1 eTAU RoI BDT score; BDT Score; RoIs', xbins=128, xmin=512, xmax=1024, opt='kAlwaysCreate')

    elif 'jTAU' in trigger:
        mon_group.defineHistogram('L1jFexRoIIso', title='L1 jTAU RoI Isolation; jTAU Isolation [GeV]; N RoI', xbins=25, xmin=0, xmax=50, opt='kAlwaysCreate')

    else: # Legacy
        mon_group.defineHistogram('L1RoIEMIsol', title='L1 Legacy RoI EM Isol; E_{T}^{EM Iso} [GeV]; RoIs', xbins=16, xmin=-2, xmax=30, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1RoIHadCore', title='L1 Legacy RoI Had Core; E_{T}^{Had} [GeV]; RoIs', xbins=16, xmin=-2, xmax=30, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1RoIHadIsol', title='L1 Legacy RoI Had Isol; E_{T}^{Had Iso} [GeV]; RoIs', xbins=16, xmin=-2, xmax=30, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1RoITauClus', title='L1 Legacy RoI E_{T}; E_{T} [GeV]; RoIs', xbins=260, xmin=0, xmax=130, opt='kAlwaysCreate')
        mon_group.defineHistogram('L1RoITauClus,L1RoIEMIsol', type='TH2F', title='L1 RoI E_{T} vs EM Isol; E_{T} [GeV]; E_{T}^{EM Iso} [GeV]',
                                  xbins=140, xmin=10, xmax=80, ybins=42, ymin=-1, ymax=20, opt='kAlwaysCreate')


  def getCustomPtBinning(self, trigger, fine=False):
    info = self.getTriggerInfo(trigger)

    def getList(ranges, others=[250]):
      ret = set(others + [500]) # The upper end of the x-axis will always be 500
      for jump, interval in ranges.items():
        ret.update(range(interval[0], interval[1], jump), interval)
      return sorted(list(ret))

    if info.isL1TauOnly():
      thr = info.getL1TauThreshold()
  
      if thr <= 8: return getList({5:(0, 30), 50:(50, 150)})
      elif thr <= 12: return getList({5:(0, 30), 50:(50, 150)})
      elif thr <= 20: return getList({5:(5, 40), 10:(40, 70), 50:(100, 150)})
      elif thr <= 30: return getList({5:(15, 50), 10:(50, 70), 50:(100, 150)})
      elif thr <= 35: return getList({5:(20, 55), 10:(60, 80), 50:(100, 150)})
      elif thr <= 40: return getList({5:(25, 60), 10:(60, 80), 50:(100, 150)})
      elif thr <= 60: return getList({5:(45, 80), 10:(80, 100), 50:(100, 150)})
      elif thr <= 100: return getList({5:(85, 120), 10:(120, 140), 20:(140, 180), 50:(200, 250)})
      else: return getList({50:(0, 200)})
    
    else: # HLT triggers
      thr = info.getHLTTauThreshold()

      if fine:
        if thr == 0: return getList({5:(0, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 20: return getList({5:(15, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 25: return getList({5:(20, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 30: return getList({5:(25, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 35: return getList({5:(30, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 60: return getList({5:(55, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 80: return getList({5:(75, 80), 10:(80, 120), 20:(120, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 160: return getList({5:(155, 160), 40:(160, 240), 60:(240, 420)}, [])
        elif thr <= 180: return getList({5:(175, 180), 40:(180, 260), 60:(260, 380)}, [])
        else: return getList({5:(195, 200), 40:(200, 240), 60:(240, 420)}, [])

      else:
        if thr == 0:
          if info.getL1TauItems(): return self.getCustomPtBinning(f'L1{info.getL1TauItem()}')
          else: return getList({5:(0, 30), 50:(50, 150)})
        elif thr <= 20: return getList({5:(10, 40), 10:(40, 60), 20:(60, 80)}, [150, 250])
        elif thr <= 25: return getList({5:(15, 40), 10:(40, 60), 20:(60, 80)}, [150, 250])
        elif thr <= 30: return getList({5:(20, 50), 10:(50, 60), 20:(60, 80)}, [150, 250])
        elif thr <= 35: return getList({5:(25, 50), 10:(50, 60), 20:(60, 80)}, [150, 250])
        elif thr <= 60: return getList({5:(50, 70), 10:(70, 80)}, [110, 150, 250])
        elif thr <= 80: return getList({5:(70, 90)}, [110, 150, 250])
        elif thr <= 160: return getList({5:(150, 170), 10:(170, 180), 20:(180, 200)}, [240, 300])
        elif thr <= 180: return getList({5:(170, 180), 10:(180, 200)}, [240, 300])
        else: return getList({5:(190, 200), 10:(200, 210)}, [240, 300])
