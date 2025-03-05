# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum, Format, LHCPeriod
from AthenaCommon.SystemOfUnits import GeV
from AthenaCommon.Logging import logging

log=logging.getLogger('TriggerConfigFlags')

class ROBPrefetching(FlagEnum):
    # Enable mapping step InputMaker outputs as ROBPrefetchingAlg inputs
    StepRoI = 'StepRoI'
    # Enable mapping chains' first step to pre-HLT prefetching rules based on initial RoIs
    InitialRoI = 'InitialRoI'
    # Enable using larger RoI in TauCore step to speculatively prefetch ROBs for the subsequent TauIso step (ATR-26419)
    TauCoreLargeRoI = 'TauCoreLargeRoI'


def trigGlobalTag(flags):
    """Return global conditions data to be used in the HLT. Return None to indicate that
    no trigger-specific tag is required. Used for IOVDb.GlobalTag in AllConfigFlags.py.
    """
    return None if flags.Input.isMC else 'CONDBR2-HLTP-2024-03'

def trigGeoTag(flags):
    """Return geometry tag to be used in the HLT. Returns None to indicate that
    no trigger-specific tag is required. Used for GeoModel.AtlasVersion in GeoModelConfigFlags.py.
    """
    return None if flags.Input.isMC else 'ATLAS-R3S-2021-03-02-00'


def createTriggerFlags(doTriggerRecoFlags):
    flags = AthConfigFlags()

    flags.addFlag('Trigger.doLVL1', lambda prevFlags: prevFlags.Input.isMC,
                  help='enable L1 simulation')

    flags.addFlag('Trigger.doHLT', False,
                  help='run HLT selection algorithms')

    flags.addFlag("Trigger.forceEnableAllChains", False,
                  help='always enable all configured chains (for testing)')

    flags.addFlag("Trigger.disableL1ConsistencyChecker", False,
                  help='force disabling the L1 ConsistencyChecker')


    flags.addFlag('Trigger.enableL1MuonPhase1', lambda prevFlags:
                  prevFlags.Trigger.EDMVersion >= 3 or prevFlags.Detector.EnableMM or prevFlags.Detector.EnablesTGC,
                  help='enable Run-3 LVL1 muon decoding')

    flags.addFlag('Trigger.enableL1CaloPhase1', lambda prevFlags:
                  prevFlags.Trigger.EDMVersion >= 3 or prevFlags.GeoModel.Run >= LHCPeriod.Run3,
                  help='enable Phase-1 LVL1 calo simulation and/or decoding for Run-3+')

    flags.addFlag('Trigger.enableL1TopoDump', False,
                  help='enable L1Topo simulation to write inputs to txt file')

    flags.addFlag('Trigger.enableL1TopoBWSimulation', True,
                  help='enable bitwise L1Topo simulation')

    flags.addFlag('Trigger.enableL1CaloLegacy', lambda prevFlags:
                  not (not prevFlags.Input.isMC and prevFlags.Trigger.doHLT), #Disable when we're running the trigger on data, keep when doing offline simulation
                  help='enable Run-2 L1Calo simulation and/or decoding')

    # L1MuonSim category
    flags.addFlag('Trigger.L1MuonSim.EmulateNSW', False,
                  help='enable emulation tool for NSW-TGC coincidence')

    flags.addFlag('Trigger.L1MuonSim.doMMTrigger', True,
                  help='enable NSW MM trigger')

    flags.addFlag('Trigger.L1MuonSim.doPadTrigger', True,
                  help='enable NSW sTGC pad trigger')

    flags.addFlag('Trigger.L1MuonSim.doStripTrigger', False,
                  help='enable NSW sTGC strip trigger')

    flags.addFlag('Trigger.L1MuonSim.WriteNSWDebugNtuple', False,
                  help='enable Storing NSW debug Ntuple')

    flags.addFlag('Trigger.L1MuonSim.WriteMMBranches', False,
                  help='enable storing of Micromega branches in NSW debug Ntuple')

    flags.addFlag('Trigger.L1MuonSim.WritesTGCBranches', False,
                  help='enable storing of TGC branches in NSW debug Ntuple')

    flags.addFlag('Trigger.L1MuonSim.NSWVetoMode', True,
                  help='enable the veto mode of the NSW-TGC coincidence')

    flags.addFlag('Trigger.L1MuonSim.doBIS78', True,
                  help='enable TGC-RPC BIS78 coincidence')

    flags.addFlag('Trigger.L1MuonSim.CondDBOffline', 'OFLCOND-MC16-SDR-RUN2-04',
                  help='offline CondDB tag for RPC/TGC coincidence window in rerunLVL1 on data')

    flags.addFlag('Trigger.L1MuonSim.RPCNBX', lambda prevFlags:
                  8 if prevFlags.Input.isMC else 4,
                  help='Number of bunch crossings in RPC readout')

    flags.addFlag('Trigger.L1MuonSim.RPCNBCZ', lambda prevFlags:
                  3 if prevFlags.Input.isMC else 1,
                  help='Nominal BC for RPC readout')


    # Detector flags
    flags.addFlag('Trigger.doID', True,
                  help='enable Inner Detector')

    flags.addFlag('Trigger.doMuon', True,
                  help='enable muon systems')

    flags.addFlag('Trigger.doCalo', True,
                  help='enable calorimeters')

    flags.addFlag('Trigger.doZDC', False,
                  help='enable ZDC system')

    flags.addFlag('Trigger.ZdcLUT', 'TrigT1ZDC/zdcRun3T1LUT_v2_08_08_2023.json',
                  help='path to Run3 ZDC LUT')
    
    # Enable TRT fast-OR trigger
    flags.addFlag('Trigger.doTRT', False)

    # Set TTC multiplicity required for the TRT fast-OR trigger
    flags.addFlag('Trigger.TRT.TTCMultiplicity', 4)

    # Path to file with list of masked chips
    flags.addFlag('Trigger.TRT.maskedChipsFile', 'TrigT1TRT/fastORmaskedChips.json')


    flags.addFlag('Trigger.doValidationMonitoring', False,
                  help='enable additional validation histograms')

    flags.addFlag('Trigger.doRuntimeNaviVal', False,
                  help=('Check validity of each Decision objects in the entire decision tree (CPU expensive). '
                        'Also enable per-step decision printouts.'))

    flags.addFlag('Trigger.ROBPrefetchingOptions',
                  [ROBPrefetching.InitialRoI, ROBPrefetching.StepRoI, ROBPrefetching.TauCoreLargeRoI],
                  help='select ROB prefetching types, empty list disables prefetching')

    def EDMVersion(flags):
        """Determine Trigger EDM version based on the input file."""
        _log = logging.getLogger('TriggerConfigFlags.EDMVersion')

        default_version = -1     # intentionally invalid default value, ATR-22856

        if flags.Input.Format is Format.BS:
            _log.debug("Input format is ByteStream")

            if not any(flags.Input.Files) and flags.Common.isOnline:
                _log.info("Online reconstruction, no input file. Return default EDMVersion=%d", default_version)
                return default_version
            try:
                from TrigEDMConfig.Utils import getEDMVersionFromBS
            except ImportError:
                log.error("Failed to import TrigEDMConfig, analysing ByteStream files is not possible in this release!")
                raise


            version = getEDMVersionFromBS(flags.Input.Files[0])

            return version if version is not None else default_version

        else:
            # POOL files: decide based on HLT output type present in the file
            _log.debug("Input format is POOL -- determine from input file collections")
            collections = flags.Input.Collections
            if "HLTResult_EF" in collections:
                _log.info("Determined EDMVersion to be 1, because HLTResult_EF found in POOL file")
                return 1
            elif "TrigNavigation" in collections:
                _log.info("Determined EDMVersion to be 2, because TrigNavigation found in POOL file")
                return 2
            elif flags.Trigger.doHLT:
                if flags.GeoModel.Run >= LHCPeriod.Run4:
                    _log.info("Determined EDMVersion to be 4, because we're now running the trigger and GeoModel.Run >= 4")
                    return 4
                else:
                    _log.info("Determined EDMVersion to be 3, because we're now running the trigger")
                    return 3
            elif any("HLTNav_Summary" in s for s in collections):
                if flags.GeoModel.Run >= LHCPeriod.Run4:
                    _log.info("Determined EDMVersion to be 4, because HLTNav_Summary.* found in POOL file and GeoModel.Run >= 4")
                    return 4
                else:
                    _log.info("Determined EDMVersion to be 3, because HLTNav_Summary.* found in POOL file")
                    return 3
            elif not flags.Input.Collections:
                # Special case for empty input files (can happen in merge jobs on the grid)
                # The resulting version doesn't really matter as there's nothing to be done, but we want a valid configuration
                _log.warning("All input files seem to be empty, cannot determine EDM version. Guessing EDMVersion=3")
                return 3

        _log.info("Could not determine EDM version from the input file. Return default EDMVersion=%d",
                     default_version)
        return default_version

    flags.addFlag('Trigger.EDMVersion', lambda prevFlags: EDMVersion(prevFlags),
                  help='Trigger EDM version (determined by input file or set to the version to be produced)')

    flags.addFlag('Trigger.doEDMVersionConversion', False,
                  help='convert Run-1&2 EDM to Run-3 EDM')

    flags.addFlag('Trigger.doxAODConversion', True,
                  help=('convert Run-1 EDM to xAOD'))

    flags.addFlag('Trigger.doOnlineNavigationCompactification', True,
                  help='enable trigger Navigation compactification into a single collection')

    flags.addFlag('Trigger.doNavigationSlimming', True,
                  help='enable Navigation slimming for RAWtoXYZ or AODtoDAOD transforms')

    # CostMonitoring category
    flags.addFlag('Trigger.CostMonitoring.doCostMonitoring', True,
                  help='enable cost monitoring')

    flags.addFlag('Trigger.CostMonitoring.chain', 'HLT_noalg_CostMonDS_L1All',
                  help='Cost monitoring chain name')

    flags.addFlag('Trigger.CostMonitoring.outputCollection', 'HLT_TrigCostContainer',
                  help='Cost monitoring output collection name')

    flags.addFlag('Trigger.CostMonitoring.monitorAllEvents', False,
                  help='enable Cost monitoring for all events')

    flags.addFlag('Trigger.CostMonitoring.monitorROBs', True,
                  help='enable Cost monitoring of ROB accesses')

    # L1 category
    flags.addFlag('Trigger.L1.doMuon', True,
                  help='enable L1Muon ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.doMuonTopoInputs', True,
                  help='enable ByteStream conversion/simulation of MUCTPI Topo TOBs')

    flags.addFlag('Trigger.L1.doCalo', True,
                  help='enable L1Calo ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.doCaloInputs', lambda prevFlags:
                  prevFlags.Trigger.L1.doCalo and prevFlags.Trigger.enableL1CaloPhase1 and not prevFlags.Trigger.doHLT,
                  help='enable L1Calo Input ([ejg]Towers) ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.doeFex', lambda prevFlags:
                  prevFlags.Trigger.L1.doCalo and prevFlags.Trigger.enableL1CaloPhase1,
                  help='enable eFEX ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.Menu.doeFexBDTTau', True,
                  help='set BDT tau algorithm as the active one for eFEX when constructing L1 menus')

    flags.addFlag('Trigger.L1.dojFex', lambda prevFlags:
                  prevFlags.Trigger.L1.doCalo and prevFlags.Trigger.enableL1CaloPhase1,
                  help='enable jFEX ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.dogFex', lambda prevFlags:
                  prevFlags.Trigger.L1.doCalo and prevFlags.Trigger.enableL1CaloPhase1,
                  help='enable gFEX ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.L1CaloSuperCellContainerName', lambda prevFlags:
                  "EmulatedSCell" if prevFlags.GeoModel.Run is LHCPeriod.Run2 else "SCell",
                  help='name of SuperCell container')

    flags.addFlag('Trigger.L1.doTopo', True,
                  help='enable L1Topo ByteStream conversion/simulation (steering both legacy and phase-1 Topo)')

    flags.addFlag('Trigger.L1.doTopoPhase1', lambda prevFlags:
                  prevFlags.Trigger.L1.doTopo and prevFlags.Trigger.enableL1CaloPhase1,
                  help='control Phase-I L1Topo simulation even if L1.doTopo is True')

    flags.addFlag('Trigger.L1.doCTP', True,
                  help='enable CTP ByteStream conversion/simulation')

    flags.addFlag('Trigger.L1.Menu.doHeavyIonTobThresholds', lambda prevFlags:
                  'HI' in prevFlags.Trigger.triggerMenuSetup,
                  help='modify min-pt-to-Topo threshold for TOBs to HI values')

    flags.addFlag('Trigger.L1.errorOnMissingTOB', True,
                  help='Set to true to enable strict-mode which will generate an ERROR on missing (non-overflow) TOB events in HLT-seeding from L1')

    # Online category
    flags.addFlag('Trigger.Online.partitionName', os.getenv('TDAQ_PARTITION') or '',
                  help='partition name used to determine online vs offline BS result writing')

    flags.addFlag('Trigger.Online.isPartition', lambda prevFlags: len(prevFlags.Trigger.Online.partitionName)>0,
                  help='check if job is running in a partition (i.e. partition name is not empty)')

    flags.addFlag('Trigger.Online.useOnlineTHistSvc', False,
                  help='use online THistSvc')

    flags.addFlag('Trigger.Online.useOnlineWebdaqHistSvc', False,
                      help='use online Webdaq HistSvc')

    flags.addFlag('Trigger.Online.BFieldAutoConfig', True,
                  help='auto-configure magnetic field from currents in IS')

    flags.addFlag('Trigger.writeBS', False,
                  help='enable bytestream writing of trigger information')

    flags.addFlag('Trigger.doTransientByteStream', lambda prevFlags:
                  True if prevFlags.Input.Format is Format.POOL and prevFlags.Trigger.doCalo else False,
                  help='create transient BS (for running on MC RDO with clients that require BS inputs)')

    flags.addFlag('Trigger.AODEDMSet', lambda flags: 'AODSLIM' if flags.Input.isMC else 'AODFULL',
                  help='list of EDM objects to be written to AOD')

    flags.addFlag('Trigger.ESDEDMSet', 'ESD',
                  help='list of EDM objects to be written to ESD')

    flags.addFlag('Trigger.ExtraEDMList', [],
                  help='list of extra EDM objects to be stored (for testing). Supported features: Add new items. Add extra decorations to existing Aux. Add additional output targets.')

    def __availableRecoMetadata(flags):
        systems = ['L1','HLT']
        # Online reco without input files
        if not any(flags.Input.Files) and flags.Common.isOnline:
            return systems
        # Makes no sense when running HLT
        elif flags.Trigger.doHLT:
            raise RuntimeError('Trigger.availableRecoMetadata is ill-defined if Trigger.doHLT==True')
        # RAW: check if keys are in COOL
        elif flags.Input.Format is Format.BS:
            from TrigConfigSvc.TriggerConfigAccess import getKeysFromCool
            keys = getKeysFromCool(flags.Input.RunNumbers[0], lbNr = 1)  # currently only checking first file
            return ( (['L1'] if 'L1PSK' in keys else []) +
                     (['HLT'] if 'HLTPSK' in keys else []) )
        # POOL: metadata (do not distinguish L1/HLT yet, see discussions on GitLab commit f83ae2bc)
        else:
            return systems if flags.Trigger.triggerConfig == 'INFILE' else []

    flags.addFlag('Trigger.availableRecoMetadata', lambda flags: __availableRecoMetadata(flags),
                  help="list of enabled trigger sub-systems in reconstruction: ['L1,'HLT']")

    flags.addFlag("Trigger.decodeHLT", True,
                  help='enable decoding of HLT trigger decision/result in reconstruction')

    flags.addFlag("Trigger.DecisionMakerValidation.Execute", True,
                  help='run trigger decision validation algorithm in reconstruction')

    flags.addFlag("Trigger.DecisionMakerValidation.ErrorMode", True,
                  help='emit an ERROR (or WARNING) in case of trigger decision validation failure')

    # Auto configure most probable choice for trigger configuration source based on job setup
    def __triggerConfig(flags):
        _log = logging.getLogger('TriggerConfigFlags.triggerConfig')
        if flags.Common.isOnline and not flags.Trigger.doHLT:
            # When running reconstruction at P1 (e.g. global monitoring, event display, etc.)
            _log.debug("Autoconfigured default value for running reconstruction inside Point 1: 'DB'")
            return 'DB'
        elif flags.Input.Format is Format.BS:
            from glob import glob
            hasLocal = True if (glob("HLTMenu*.json") and glob("L1Menu*.json") and glob("HLTPrescales*.json") and glob("L1Prescales*.json") and glob("HLTMonitoring*.json") and glob("BunchGroupSet*.json")) else False
            if flags.Trigger.doHLT:
                # When running the Run 3 trigger on data, data the default config source is from the JSON created by compiling the menu in the job config phase
                _log.debug("Autoconfigured default value for running the trigger on data: 'FILE'")
                return 'FILE'
            elif hasLocal:
                # When running reco (doHLT == False) from RAW in a directory which already has a full suite of JSONs, assume that the user has just run the trigger manually
                # and now wants to reconstruct the output using the menu files created when the trigger was executed, rather than reading the DB configuration for the run.
                # A number of ART tests chain trigger then reco like this.
                _log.debug("Autoconfigured default value for running reconstruction with a pre-supplied set of trigger configuration JSON files: 'FILE'")
                return 'FILE'
            elif flags.GeoModel.Run >= LHCPeriod.Run3:
                # When reconstructing Run 3 data the default config source is the database
                _log.debug("Autoconfigured default value for reconstruction of Run 3 data: 'DB'")
                return 'DB'
            else:
                # When reconstructing Run 2 or Run 1 data, a stand-alone database converter will be called from python. We then need to load the converted JSON from disk
                _log.debug("Autoconfigured default value for reconstruction of Run 1 or Run 2 data: 'FILE'")
                return 'FILE'
        else: # Format.POOL
            from AthenaConfiguration.AutoConfigFlags import GetFileMD
            md = GetFileMD(flags.Input.Files)
            # Note: the following comprehension will detect both Run 2 and Run 3 in-file metadata formats.
            # As of 2023, the Run 2 metadata format is still in production use for Run 2 MC AODs, DAODs produced with the Release 21 Run 2 trigger.
            hasTrigMeta = ("metadata_items" in md and any(('TriggerMenu' in key) for key in md["metadata_items"].keys()))
            if hasTrigMeta:
                # When running over a file which already has metadata content (RDO_TRIG, ESD, AOD, DAOD), then read this from within the file's meta store
                _log.debug("Autoconfigured default value to read trigger configuration data from the input file: 'INFILE'")
                return 'INFILE'
            else:
                # MC production, read the menu JSON generated during the trigger job configuration
                _log.debug("Autoconfigured default value to read trigger configuration data from disk for MC production: 'FILE'")
                return 'FILE'

    flags.addFlag('Trigger.triggerConfig', lambda flags: __triggerConfig(flags),
                  help='Trigger configuration source (https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerConfigFlag)')

    flags.addFlag('Trigger.triggerMenuSetup', lambda flags: 'MC_pp_run3_v1_BulkMCProd_prescale' if flags.GeoModel.Run is LHCPeriod.Run3 else 'MC_pp_run4_v1_BulkMCProd_prescale',
                  help='name of the trigger menu')

    flags.addFlag('Trigger.generateMenuDiagnostics', False,
                  help='print debug output from control flow generation')

    flags.addFlag('Trigger.fastMenuGeneration', True,
                  help='avoid re-merging CAs that were already seen once')

    flags.addFlag('Trigger.disableCPS', False,
                  help='disable coherent prescale sets (for testing with small menu)')

    flags.addFlag('Trigger.enableEndOfEventProcessing', True,
                  help='enable execution of extra algorithms for accepted events')

    flags.addFlag('Trigger.doCFEmulationTest', False,
                  help='enable run Control Flow Emulation test')

    # trigger reconstruction
    # Protection against import of packages not in the analysis release
    # Signature and other trigger reco flags should be handled here
    if doTriggerRecoFlags:
        flags.join( createTriggerRecoFlags() )

        # Disable the CPS system if the restricted menu flags are active
        flags.Trigger.disableCPS = lambda prevFlags: prevFlags.Trigger.selectChains or len(prevFlags.Trigger.enabledSignatures)==1

    return flags


def createTriggerRecoFlags():
    flags = AthConfigFlags()

    # Additional flags to filter chains
    def _allSignatures (prevFlags):
        from TriggerMenuMT.HLT.Config.GenerateMenuMT import allSignatures
        l = list(allSignatures())
        l.sort()
        return l
    flags.addFlag("Trigger.enabledSignatures", _allSignatures, help='list of enabled trigger signatures')
    flags.addFlag("Trigger.disabledSignatures", [], help='list of disabled trigger signatures')
    flags.addFlag("Trigger.selectChains", [], help='list of enabled chains')
    flags.addFlag("Trigger.disableChains", [], help='list of disabled chains')

    def __egamma():
        from TriggerMenuMT.HLT.Egamma.TrigEgammaConfigFlags import createTrigEgammaConfigFlags
        return createTrigEgammaConfigFlags()
    flags.addFlagsCategory('Trigger.egamma', __egamma )

    # muon offline reco flags varaint for trigger
    def __muonSA():
        from MuonConfig.MuonConfigFlags import createMuonConfigFlags
        muonflags = createMuonConfigFlags()
        muonflags.Muon.useTGCPriorNextBC=True
        muonflags.Muon.MuonTrigger=True
        muonflags.Muon.SAMuonTrigger=True
        muonflags.Muon.runCommissioningChain=False
        muonflags.Muon.enableErrorTuning=False
        return muonflags
    flags.addFlagsCategory('Trigger.Offline.SA', __muonSA, prefix=True)

    def __muon():
        from MuonConfig.MuonConfigFlags import createMuonConfigFlags
        muonflags = createMuonConfigFlags()
        muonflags.Muon.useTGCPriorNextBC=True
        muonflags.Muon.MuonTrigger=True
        muonflags.Muon.enableErrorTuning=False
        return muonflags
    flags.addFlagsCategory('Trigger.Offline', __muon, prefix=True)

    def __muonCombined():
        from MuonCombinedConfig.MuonCombinedConfigFlags import createMuonCombinedConfigFlags
        muonflags = createMuonCombinedConfigFlags()
        muonflags.MuonCombined.doCaloTrkMuId = False
        muonflags.MuonCombined.doSiAssocForwardMuons = False
        muonflags.MuonCombined.doStatisticalCombination = False
        muonflags.MuonCombined.doMuGirl = False
        muonflags.MuonCombined.doCombinedFit = True
        return muonflags
    flags.addFlagsCategory('Trigger.Offline.Combined', __muonCombined, prefix=True)

    def __tau():
        from TrigTauRec.TrigTauConfigFlags import createTrigTauConfigFlags
        return createTrigTauConfigFlags()
    flags.addFlagsCategory('Trigger.Offline.Tau', __tau )

    def __idTrk():
        from TrigInDetConfig.TrigTrackingPassFlags import createTrigTrackingPassFlags
        return createTrigTrackingPassFlags()
    flags.addFlagsCategory( 'Trigger.InDetTracking', __idTrk )

    def __idITk():
        from TrigInDetConfig.TrigTrackingPassFlags import createTrigTrackingPassFlags
        return createTrigTrackingPassFlags(mode='ITk')
    flags.addFlagsCategory( 'Trigger.ITkTracking', __idITk )

    def __trigCalo():
        from TrigCaloRec.TrigCaloConfigFlags import createTrigCaloConfigFlags
        return createTrigCaloConfigFlags()
    flags.addFlagsCategory( 'Trigger.Calo', __trigCalo )

    def __muctpiFlags():
        from TrigT1MuctpiPhase1.TrigMUCTPIConfigFlags import createTrigMUCTPIConfigFlags
        return createTrigMUCTPIConfigFlags()
    flags.addFlagsCategory('Trigger.MUCTPI', __muctpiFlags )

    def __fpgatracksimFlags():
        """Additional function delays import"""
        from FPGATrackSimConfTools.FPGATrackSimConfigFlags import createFPGATrackSimConfigFlags
        return createFPGATrackSimConfigFlags()
    flags.addFlagsCategory("Trigger.FPGATrackSim", __fpgatracksimFlags, prefix=True )


    # NB: Longer term it may be worth moving these into a PF set of config flags, but right now the only ones that exist do not seem to be used in the HLT.
    # When we use component accumulators for this in the HLT maybe we should revisit this
    # PFO-muon removal option for the full-scan hadronic signatures.
    # Options are:
    #   "None": Do no PFO-muon removal
    #   "Calo": Use the calo-tagging tools from the muon slice
    #   "Iso" : Use the mainly isolation-based selections based on the MET associator package
    flags.addFlag("Trigger.FSHad.PFOMuonRemoval", "Calo",
                  help='PFO-muon removal option: None, Calo, Iso)')

    flags.addFlag("Trigger.FSHad.PFOMuonRemovalMinPt", 10 * GeV,
                  help='minimum pT threshold to use for the muon removal')

    flags.addFlag('Trigger.Jet.doJetSuperPrecisionTracking', False,
                  help='enable precision tracking in jet super-ROI before fast b-tagging (EMTopo jets)')

    flags.addFlag("Trigger.Jet.fastbtagPFlow", True,
                  help='enable fast b-tagging for all fully calibrated HLT PFlow jets')

    flags.addFlag("Trigger.Jet.fastbtagVertex", True,
                  help='enable the addition of the super ROI PV to the b-tagging')

    flags.addFlag("Trigger.Jet.doVRJets", False,
                  help='enable the addition of the VR track jet reconstruction sequence')

    flags.addFlag("Trigger.FSTrk.doJetRestrictedVertexSort", False,
                  help='use tracks in jets for computing sumpt2 for vertex sorting')

    # chooses calibration config file for HLT small-R jets
    # mapping in: Reconstruction/Jet/JetCalibTools/python/JetCalibToolsConfig.py
    # All calib keys for HLT jets have to start with "Trig" otherwise the JetCalibTool config fails!
    flags.addFlag("Trigger.Jet.pflowCalibKey", lambda prevFlags: "TrigHIUPC" if 'HI' in prevFlags.Trigger.triggerMenuSetup else "TrigR22Prerec",
                  help='calibration config file for HLT small-R jets')

    flags.addFlag("Trigger.Jet.emtopoCalibKey", "TrigLS2",
                  help='calibration config file for HLT small-R jets')

    flags.addFlag("Trigger.Jet.pflowLJCalibKey", "TrigSoftDrop",
                  help='calibration config file for HLT large-R PFlow jets')                  

    flags.addFlag("Trigger.Jet.PFlowTolerance", 1e-2,
                  help='tolerance in STEP Propagator')
    
    flags.addFlag("Trigger.Jet.TrackVtxAssocWP", "Custom", # offline default is "Nonprompt_All_MaxWeight"
                  help='working point for the TVA algorithm')

    flags.addFlag("Trigger.Jet.LowPtFilter", lambda prevFlags: 'HI' in prevFlags.Trigger.triggerMenuSetup,
                  help='apply low pT filter on antiKt4 jets (used for HI UPC jet reco)')

    return flags


if __name__ == "__main__":
    import unittest
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import Project
    flags = initConfigFlags()

    class Tests(unittest.TestCase):

        @unittest.skipIf(flags.Common.Project is Project.AthAnalysis, "project is AthAnalysis")
        def test_recoFlags(self):
            """Check if offline reco flags can be added to trigger"""
            flags = initConfigFlags()
            flags.Trigger.Offline.Tau.doTauRec=False
            flags.Tau.doTauRec=True
            self.assertEqual(flags.Trigger.Offline.Tau.doTauRec, False, "dependent flag setting does not work")
            self.assertEqual(flags.Tau.doTauRec, True, "dependent flag setting does not work")

            newflags = flags.cloneAndReplace('Tau', 'Trigger.Offline.Tau')

            self.assertEqual(flags.Tau.doTauRec, True, "dependent flag setting does not work")
            self.assertEqual(newflags.Tau.doTauRec, False, "dependent flag setting does not work")

        def test_allFlags(self):
            """Force load all dynamic flags"""
            flags = initConfigFlags()
            flags.loadAllDynamicFlags()

    unittest.main()
