# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import GeV, TeV
from AthenaConfiguration.AthConfigFlags import AthConfigFlags, isGaudiEnv
from AthenaConfiguration.AutoConfigFlags import GetFileMD, getInitialTimeStampsFromRunNumbers, getRunToTimestampDict, getSpecialConfigurationMetadata
from AthenaConfiguration.Enums import BeamType, Format, ProductionStep, BunchStructureSource, Project
from Campaigns.Utils import Campaign
from PyUtils.moduleExists import moduleExists


def _addFlagsCategory (acf, name, generator, modName = None):
    """Add flags category and return True/False on success/failure"""
    if moduleExists (modName):
        acf.addFlagsCategory (name, generator)
        return True
    return False


def initConfigFlags():

    acf=AthConfigFlags()

    #Flags steering the job execution:
    from AthenaCommon.Constants import INFO
    acf.addFlag('Exec.OutputLevel', INFO, help='Global OutputLevel')
    acf.addFlag('Exec.PrintAlgsSequence', False, help='print algorithm sequence in ApplicationMgr')
    acf.addFlag('Exec.MaxEvents', -1, help='number of events to proceess (-1 for all)')
    acf.addFlag('Exec.SkipEvents', 0, help='number of events to skip')
    acf.addFlag('Exec.DebugStage', '', help='attach debugger at stage: conf, init, exec, fini')
    acf.addFlag('Exec.Interactive', "", help='interactive prompt at stage: init, exec')
    acf.addFlag('Exec.FPE', 0, help='FPE check mode: -2 (no FPE check), -1 (abort with core-dump), 0 (FPE Auditor w/o stack-tace) , >0 (number of stack-traces printed by the job)')

    #Custom messaging for components, see Utils.setupLoggingLevels
    acf.addFlag('Exec.VerboseMessageComponents', [], help='verbose output for listed components (wildcards)')
    acf.addFlag('Exec.DebugMessageComponents', [], help='debug output for listed components (wildcards)')
    acf.addFlag('Exec.InfoMessageComponents', [], help='info output for listed components (wildcards)')
    acf.addFlag('Exec.WarningMessageComponents', [], help='warning output for listed components (wildcards)')
    acf.addFlag('Exec.ErrorMessageComponents', [], help='error output for listed components (wildcards)')

    #Multi-threaded event service mode
    acf.addFlag('Exec.MTEventService', False, help='use multi-threaded event service')
    acf.addFlag('Exec.MTEventServiceChannel', 'EventService_EventRanges', help='name of YAMPL communication channel between AthenaMT and pilot')

    #Activate per-event log-output of StoreGate content
    acf.addFlag('Debug.DumpEvtStore', False, help='dump event store on each event')
    acf.addFlag('Debug.DumpDetStore', False, help='dump detector store on each event')
    acf.addFlag('Debug.DumpCondStore', False, help='dump conditions store on each event')

    acf.addFlag('ExecutorSplitting.TotalSteps', 0, help='number of steps for pileup overlay')
    acf.addFlag('ExecutorSplitting.Step', -1, help='step number of current pileup overlay job')
    acf.addFlag('ExecutorSplitting.TotalEvents', -1, help='events per pileup overlay step')

    #Flags describing the input data
    acf.addFlag('Input.Files', ["_ATHENA_GENERIC_INPUTFILE_NAME_",], help='input files')
    acf.addFlag("Input.FileNentries", -1, help='actual number of events in file (filled from runArgs')
    acf.addFlag('Input.SecondaryFiles', [], help='secondary input files for DoubleEventSelector')
    acf.addFlag('Input.isMC', lambda prevFlags : "IS_SIMULATION" in GetFileMD(prevFlags.Input.Files).get("eventTypes", []), help='Monte Carlo input')
    acf.addFlag('Input.OverrideRunNumber', False, help='override run number' )
    acf.addFlag("Input.ConditionsRunNumber", -1, help='override HITS file run number with one from a data') # TODO merge with Input.RunNumbers
    acf.addFlag('Input.RunNumbers', lambda prevFlags : list(GetFileMD(prevFlags.Input.Files).get("runNumbers", [])), type=list, help='run numbers of input files')
    acf.addFlag('Input.MCChannelNumber', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("mc_channel_number", 0), help='Monte Carlo channel number')
    acf.addFlag('Input.LumiBlockNumbers', lambda prevFlags : list(GetFileMD(prevFlags.Input.Files).get("lumiBlockNumbers", [])), type=list, help='lumi block numbers of input files')
    acf.addFlag('Input.TimeStamps', lambda prevFlags : getInitialTimeStampsFromRunNumbers(prevFlags.Input.RunNumbers) if prevFlags.Input.OverrideRunNumber else [], type=list, help='timestamps of inputs files')
    # Configure EvtIdModifierSvc with a list of dictionaries of the form:
    # {'run': 152166, 'lb': 202, 'starttstamp': 1269948352889940910, 'evts': 1, 'mu': 0.005}
    acf.addFlag("Input.RunAndLumiOverrideList", [], help='list of dictionaries to configure EvtIdModifierSvc')
    # Job number
    acf.addFlag("Input.JobNumber", 1, help='job number for pileup overlay')
    acf.addFlag('Input.FailOnUnknownCollections', False, help='fail on unknown collections in input load')

    def _dataYearFromFlags(prevFlags):
        dataYear = GetFileMD(prevFlags.Input.Files).get("data_year", "")
        if dataYear:
            return int(dataYear)
        if prevFlags.Input.ProjectName.startswith("data"):
            return 2000 + int(prevFlags.Input.ProjectName[4:6])
        return 0

    acf.addFlag('Input.ProjectName', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("project_name", ""), help='project name')
    acf.addFlag('Input.DataYear', _dataYearFromFlags, help='year of input data')
    acf.addFlag('Input.MCCampaign', lambda prevFlags : Campaign(GetFileMD(prevFlags.Input.Files).get("mc_campaign", "")), type=Campaign, help='Monte Carlo campaign')
    acf.addFlag('Input.TriggerStream', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("stream", "") if prevFlags.Input.Format == Format.BS
                                                          else GetFileMD(prevFlags.Input.Files).get("triggerStreamOfFile", ""), help='trigger stream name')
    acf.addFlag('Input.Format', lambda prevFlags : Format.BS if GetFileMD(prevFlags.Input.Files).get("file_type", "BS") == "BS" else Format.POOL, type=Format, help='input format type')
    acf.addFlag('Input.ProcessingTags', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("processingTags", []), help='list of stream names in this file')
    from GeneratorConfig.Versioning import generatorsGetFromMetadata
    acf.addFlag('Input.GeneratorsInfo', lambda prevFlags : generatorsGetFromMetadata( GetFileMD(prevFlags.Input.Files).get("generators", "") ), help='generator version')
    acf.addFlag('Input.SpecialConfiguration', lambda prevFlags : getSpecialConfigurationMetadata(prevFlags.Input.Files, prevFlags.Input.SecondaryFiles), help='special configuration options read from input file metadata')

    def _inputCollections(inputFile):
        rawCollections = [type_key[1] for type_key in GetFileMD(inputFile).get("itemList", [])]
        collections = [col for col in rawCollections if not col.endswith('Aux.')]
        return collections

    def _typedInputCollections(inputFile):
        collections = ['%s#%s' % type_key for type_key in GetFileMD(inputFile).get("itemList", [])]
        return collections

    acf.addFlag('Input.Collections', lambda prevFlags : _inputCollections(prevFlags.Input.Files), help='input collections')
    acf.addFlag('Input.SecondaryCollections', lambda prevFlags : _inputCollections(prevFlags.Input.SecondaryFiles), help='secondary input collections for overlay')
    acf.addFlag('Input.TypedCollections', lambda prevFlags : _typedInputCollections(prevFlags.Input.Files), help='input collections key#type')
    acf.addFlag('Input.SecondaryTypedCollections', lambda prevFlags : _typedInputCollections(prevFlags.Input.SecondaryFiles), help='secondary input collections key#type')

    def _metadataItems(inputFile):
        return GetFileMD(inputFile).get("metadata_items", {})

    acf.addFlag('Input.MetadataItems', lambda prevFlags : _metadataItems(prevFlags.Input.Files), help='metadata items in input' )
    acf.addFlag('Input.Release',  lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("AtlasRelease", ""), help='release of input file')
    acf.addFlag('Input.AODFixesDone', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("AODFixVersion", ""), help='set of already applied AODFixes')

    acf.addFlag('Concurrency.NumProcs', 0, help='number of concurrent processes')
    acf.addFlag('Concurrency.NumThreads', 0, help='number of threads' )
    acf.addFlag('Concurrency.NumConcurrentEvents', lambda prevFlags : prevFlags.Concurrency.NumThreads, help='number of concurrent events')
    acf.addFlag('Concurrency.DebugWorkers', False, help='stops the worker in bootstrap until SIGUSR1 is received')

    acf.addFlag('Scheduler.CheckDependencies', True, help='runtime check of algorithm input dependencies')
    acf.addFlag('Scheduler.CheckOutputUsage', False, help='runtime check of algorithm output usage')
    acf.addFlag('Scheduler.ShowDataDeps', False, help='show data dependencies')
    acf.addFlag('Scheduler.ShowDataFlow', False, help='show data flow')
    acf.addFlag('Scheduler.ShowControlFlow', False, help='show data flow')
    acf.addFlag('Scheduler.EnableVerboseViews', True, help='enable verbose view output')
    acf.addFlag('Scheduler.AutoLoadUnmetDependencies', True, help='auto-load unmet data dependencies')

    acf.addFlag('MP.WorkerTopDir', 'athenaMP_workers', help='work directory for MP workers')
    acf.addFlag('MP.OutputReportFile', 'AthenaMPOutputs', help='name of MP report file')
    acf.addFlag('MP.Strategy', 'SharedQueue', help='event assignment strategy')
    acf.addFlag('MP.CollectSubprocessLogs', False, help='collects log of sub-processes')
    acf.addFlag('MP.PollingInterval', 100, help='time interval in milliseconds between subsequent polling of subproceses')
    acf.addFlag('MP.EventsBeforeFork', 0, help='number of events to process before forking')
    acf.addFlag('MP.EventRangeChannel', 'EventService_EventRanges', help='channel name for communicating event ranges with the pilot')
    acf.addFlag('MP.EvtRangeScattererCaching', False, help='activate extra event caching by the EvtRangeScatterer')
    acf.addFlag('MP.MemSamplingInterval', 0, help='time interval in seconds between taking memory samples')
    acf.addFlag('MP.ChunkSize', -1, help='size of event chunks in shared queue (-1: auto_flush for LZMA-compressed files, -2: auto_flush for LZMA or ZLIB, -3: auto_flush for LZMA, ZLIB or LZ4, -4: auto_flush)')
    acf.addFlag('MP.ReadEventOrders', False, help='read event order from ASCII file for reproducibility')
    acf.addFlag('MP.EventOrdersFile', 'athenamp_eventorders.txt', help='file name for event order')
    acf.addFlag('MP.UseSharedReader', False, help='use shared reader')
    acf.addFlag('MP.UseSharedWriter', False, help='use shared writer')
    acf.addFlag('MP.UseParallelCompression', True, help='enable event compression in workers')

    acf.addFlag('Common.MsgSuppression', True, help='enable log message suppression')
    acf.addFlag('Common.MsgSourceLength', 50, help='length of the source-field in the log message format')
    acf.addFlag('Common.ShowMsgStats', False ,help='print message statistics at the end of the job')

    acf.addFlag('Common.isOnline', False, help='job runs in an online environment')
    acf.addFlag('Common.useOnlineLumi', lambda prevFlags : prevFlags.Common.isOnline, help='use online version of luminosity')
    acf.addFlag('Common.isOverlay', lambda prevFlags: (prevFlags.Common.ProductionStep == ProductionStep.Overlay or
                                                       (prevFlags.Common.ProductionStep == ProductionStep.FastChain and
                                                        prevFlags.Overlay.FastChain)),
                help='enable overlay')
    acf.addFlag('Common.doExpressProcessing', False, help='do express stream processing')
    acf.addFlag('Common.ProductionStep', ProductionStep.Default, type=ProductionStep, help='production step')
    acf.addFlag('Common.Project', Project.determine(), type=Project, help='current athena software project')

    # replace global.Beam*
    acf.addFlag('Beam.BunchSpacing', 25, help='bunch spacing in nanoseconds')
    acf.addFlag('Beam.Type', lambda prevFlags : BeamType(GetFileMD(prevFlags.Input.Files).get('beam_type', 'collisions')), type=BeamType, help='beam type')
    acf.addFlag("Beam.NumberOfCollisions", lambda prevFlags : 2. if prevFlags.Beam.Type is BeamType.Collisions else 0., help='number of pileup collisions')

    def _configureBeamEnergy(prevFlags):
        metadata = GetFileMD(prevFlags.Input.Files)
        default = 6.8 * TeV
        # pool files
        if prevFlags.Input.Format == Format.POOL:
            return float(metadata.get("beam_energy", default))
        # BS files
        elif prevFlags.Input.Format == Format.BS:
            if metadata.get("eventTypes", [""])[0] == "IS_DATA":
                # special option for online running
                if prevFlags.Common.isOnline:
                    from PyUtils.OnlineISConfig import GetRunType

                    return float(GetRunType()[1] or default)

                # configure Beam energy depending on beam type:
                if prevFlags.Beam.Type.value == "cosmics":
                    return 0.0
                elif prevFlags.Beam.Type.value == "singlebeam":
                    return 450.0 * GeV
                elif prevFlags.Beam.Type.value == "collisions":
                    projectName = prevFlags.Input.ProjectName
                    beamEnergy = None

                    if "GeV" in projectName:
                        beamEnergy = (
                            float(
                                (str(projectName).split("_")[1]).replace("GeV", "", 1)
                            )
                            / 2
                            * GeV
                        )
                    elif "TeV" in projectName:
                        if "hip5TeV" in projectName:
                            # Approximate 'beam energy' here as sqrt(sNN)/2.
                            beamEnergy = 1.577 * TeV
                        elif "hip8TeV" in projectName:
                            # Approximate 'beam energy' here as sqrt(sNN)/2.
                            beamEnergy = 2.51 * TeV
                        else:
                            beamEnergy = (
                                float(
                                    (str(projectName).split("_")[1])
                                    .replace("TeV", "", 1)
                                    .replace("p", ".")
                                )
                                / 2
                                * TeV
                            )
                            if "5TeV" in projectName:
                                # these are actually sqrt(s) = 5.02 TeV
                                beamEnergy = 2.51 * TeV
                    elif projectName.endswith("_hi") or projectName.endswith("_hip"):
                        if projectName in ("data10_hi", "data11_hi"):
                            beamEnergy = 1.38 * TeV  # 1.38 TeV (=3.5 TeV * (Z=82/A=208))
                        elif projectName == "data12_hi":
                            beamEnergy = 1.577 * TeV  # 1.577 TeV (=4 TeV * (Z=82/A=208))
                        elif projectName in ("data12_hip", "data13_hip"):
                            # Pb (p) Beam energy in p-Pb collisions in 2012/3 was 1.577 (4) TeV.
                            # Approximate 'beam energy' here as sqrt(sNN)/2.
                            beamEnergy = 2.51 * TeV
                        elif projectName in ("data15_hi", "data18_hi"):
                            beamEnergy = 2.51 * TeV  # 2.51 TeV (=6.37 TeV * (Z=82/A=208)) - lowered to 6.37 to match s_NN = 5.02 in Pb-p runs.
                        elif projectName == "data17_hi":
                            beamEnergy = 2.721 * TeV  # 2.72 TeV for Xe-Xe (=6.5 TeV * (Z=54/A=129))
                    return beamEnergy or default
            elif metadata.get("eventTypes", [""])[0] == "IS_SIMULATION":
                return float(metadata.get("beam_energy", default))
        return default


    acf.addFlag('Beam.Energy', lambda prevFlags : _configureBeamEnergy(prevFlags), help='beam energy in MeV')
    acf.addFlag('Beam.estimatedLuminosity', lambda prevFlags : ( 1E33*(prevFlags.Beam.NumberOfCollisions)/2.3 ) *\
                (25./prevFlags.Beam.BunchSpacing), help='luminosity estimated from pileup')
    acf.addFlag('Beam.BunchStructureSource', lambda prevFlags: BunchStructureSource.MC if prevFlags.Input.isMC else BunchStructureSource.TrigConf, help='source of bunch structure')

    # output
    acf.addFlag('Output.EVNTFileName', '', help='EVNT output file name')
    acf.addFlag('Output.EVNT_TRFileName', '', help='EVNT_TR output file name')
    acf.addFlag('Output.HITSFileName', '', help='HITS output file name')
    acf.addFlag('Output.RDOFileName',  '', help='RDO output file name')
    acf.addFlag('Output.RDO_SGNLFileName', '', help='RDO_SGNL output file name')
    acf.addFlag('Output.ESDFileName',  '', help='ESD output file name')
    acf.addFlag('Output.AODFileName',  '', help='AOD output file name')
    acf.addFlag('Output.HISTFileName', '', help='HIST output file name')
    
    acf.addFlag('Output.doWriteEVNT', lambda prevFlags: bool(prevFlags.Output.EVNTFileName), help='write EVNT file')
    acf.addFlag('Output.doWriteHITS', lambda prevFlags: bool(prevFlags.Output.HITSFileName), help='write HITS file')
    acf.addFlag('Output.doWriteRDO', lambda prevFlags: bool(prevFlags.Output.RDOFileName), help='write RDO file')
    acf.addFlag('Output.doWriteRDO_SGNL', lambda prevFlags: bool(prevFlags.Output.RDO_SGNLFileName), help='write RDO_SGNL file')
    acf.addFlag('Output.doWriteESD', lambda prevFlags: bool(prevFlags.Output.ESDFileName), help='write ESD file')
    acf.addFlag('Output.doWriteAOD', lambda prevFlags: bool(prevFlags.Output.AODFileName), help='write AOD file')
    acf.addFlag('Output.doWriteBS', False,  help='write bytestream file')
    acf.addFlag('Output.doWriteDAOD', False, help='write at least one DAOD file')
    acf.addFlag('Output.doJiveXML', False, help='write JiveXML file')

    acf.addFlag('Output.TreeAutoFlush', {}, help="dict with auto-flush settings for stream e.g. {'STREAM': 123}")
    acf.addFlag('Output.StorageTechnology.EventData', 'ROOTTREEINDEX', help='set the underlying POOL storage technology for event data')
    acf.addFlag('Output.StorageTechnology.MetaData', 'ROOTTREE', help='set the underlying POOL storage technology for metadata')

    # Might move this elsewhere in the future.
    # Some flags from https://gitlab.cern.ch/atlas/athena/blob/master/Tracking/TrkDetDescr/TrkDetDescrSvc/python/TrkDetDescrJobProperties.py
    # (many, e.g. those that set properties of one tool are not needed)
    acf.addFlag('TrackingGeometry.MagneticFileMode', 6) # TODO: unused?
    acf.addFlag('TrackingGeometry.MaterialSource', 'COOL', help='material source (COOL, Input or None)')

#Detector Flags:
    def __detector():
        from AthenaConfiguration.DetectorConfigFlags import createDetectorConfigFlags
        return createDetectorConfigFlags()
    acf.addFlagsCategory( "Detector", __detector )

#Simulation Flags:
    def __simulation():
        from SimulationConfig.SimConfigFlags import createSimConfigFlags
        return createSimConfigFlags()
    _addFlagsCategory (acf, "Sim", __simulation, 'SimulationConfig' )

#Test Beam Simulation Flags:
    def __testbeam():
        from SimulationConfig.TestBeamConfigFlags import createTestBeamConfigFlags
        return createTestBeamConfigFlags()
    _addFlagsCategory (acf, "TestBeam", __testbeam, 'SimulationConfig' )

#Digitization Flags:
    def __digitization():
        from Digitization.DigitizationConfigFlags import createDigitizationCfgFlags
        return createDigitizationCfgFlags()
    _addFlagsCategory(acf, "Digitization", __digitization, 'Digitization' )

#Overlay Flags:
    def __overlay():
        from OverlayConfiguration.OverlayConfigFlags import createOverlayConfigFlags
        return createOverlayConfigFlags()
    _addFlagsCategory(acf, "Overlay", __overlay, 'OverlayConfiguration' )

#Geo Model Flags:
    def __geomodel():
        from AthenaConfiguration.GeoModelConfigFlags import createGeoModelConfigFlags
        return createGeoModelConfigFlags(not isGaudiEnv() or acf.Common.Project is Project.AthAnalysis)
    acf.addFlagsCategory( "GeoModel", __geomodel )

#Reco Flags:
    def __reco():
        from RecJobTransforms.RecoConfigFlags import createRecoConfigFlags
        return createRecoConfigFlags()
    _addFlagsCategory(acf, "Reco", __reco, 'RecJobTransforms')

#Generator Flags:
    def __generators():
        from GeneratorConfig.GeneratorConfigFlags import createGeneratorConfigFlags
        return createGeneratorConfigFlags()
    _addFlagsCategory(acf, "Generator", __generators, 'GeneratorConfig')

#IOVDbSvc Flags:
    if isGaudiEnv():
        from IOVDbSvc.IOVDbAutoCfgFlags import getLastGlobalTag, getDatabaseInstanceDefault

        def __getTrigTag(flags):
            from TriggerJobOpts.TriggerConfigFlags import trigGlobalTag
            return trigGlobalTag(flags)

        acf.addFlag("IOVDb.GlobalTag", lambda flags :
                    (__getTrigTag(flags) if flags.Trigger.doLVL1 or flags.Trigger.doHLT else None) or
                    getLastGlobalTag(flags), help='global conditions tag')

        acf.addFlag("IOVDb.DatabaseInstance", getDatabaseInstanceDefault, help='conditions DB instance')

        # Run dependent simulation
        acf.addFlag("IOVDb.RunToTimestampDict", lambda prevFlags: getRunToTimestampDict(), help='runNumber to timestamp map')
        acf.addFlag("IOVDb.DBConnection", lambda prevFlags : "sqlite://;schema=mycool.db;dbname=" + prevFlags.IOVDb.DatabaseInstance, help='default DB connection string')
        #For HLT-jobs, the ring-size should be 0 (eg no cleaning at all since there are no IOV-updates during the job)
        acf.addFlag("IOVDb.CleanerRingSize",lambda prevFlags : 0 if prevFlags.Trigger.doHLT else 2*max(1, prevFlags.Concurrency.NumConcurrentEvents), help='size of ring-buffer for conditions cleaner')
        acf.addFlag("IOVDb.SqliteInput","",help="Folders found in this file will be used instead of the production db")
        acf.addFlag("IOVDb.SqliteFolders",(),help="Folders listed here will be taken from the IOVDb.SqliteInput file instead of the production db. If empty, all folders found in the file are used.")
#PoolSvc Flags:
    acf.addFlag("PoolSvc.MaxFilesOpen", lambda prevFlags : 2 if prevFlags.MP.UseSharedReader else 0, help='maximum number of open files')


    def __bfield():
        from MagFieldConfig.BFieldConfigFlags import createBFieldConfigFlags
        return createBFieldConfigFlags()
    _addFlagsCategory(acf, "BField", __bfield, 'MagFieldConfig')

    def __lar():
        from LArConfiguration.LArConfigFlags import createLArConfigFlags
        return createLArConfigFlags()
    _addFlagsCategory(acf, "LAr", __lar, 'LArConfiguration' )

    def __tile():
        from TileConfiguration.TileConfigFlags import createTileConfigFlags
        return createTileConfigFlags()
    _addFlagsCategory(acf, 'Tile', __tile, 'TileConfiguration' )


    def __calo():
        from CaloRec.CaloConfigFlags import createCaloConfigFlags
        return createCaloConfigFlags()
    _addFlagsCategory(acf, 'Calo', __calo, 'CaloRec' )

#Random engine Flags:
    acf.addFlag("Random.Engine", "dSFMT", help='random number service ("dSFMT", "Ranlux64", "Ranecu")')
    acf.addFlag("Random.SeedOffset", 0, help='seed offset') # TODO replace usage of Digitization.RandomSeedOffset with this flag

    def __trigger():
        from TriggerJobOpts.TriggerConfigFlags import createTriggerFlags
        return createTriggerFlags(acf.Common.Project is not Project.AthAnalysis)

    added = _addFlagsCategory(acf, "Trigger", __trigger, 'TriggerJobOpts' )
    if not added:
        # If TriggerJobOpts is not available, we add at least these basic flags
        # to indicate Trigger is not available:
        acf.addFlag('Trigger.doLVL1', False, help='enable L1 simulation')
        acf.addFlag('Trigger.doHLT', False, help='run HLT selection algorithms')

    def __indet():
        from InDetConfig.InDetConfigFlags import createInDetConfigFlags
        return createInDetConfigFlags()
    _addFlagsCategory(acf, "InDet", __indet, 'InDetConfig' )

    def __itk():
        from InDetConfig.ITkConfigFlags import createITkConfigFlags
        return createITkConfigFlags()
    _addFlagsCategory(acf, "ITk", __itk, 'InDetConfig' )

    def __tracking():
        from TrkConfig.TrkConfigFlags import createTrackingConfigFlags
        return createTrackingConfigFlags()
    _addFlagsCategory(acf, "Tracking", __tracking, 'TrkConfig')

    def __trackoverlay():
        from TrackOverlayConfig.TrackOverlayConfigFlags import createTrackOverlayConfigFlags
        return createTrackOverlayConfigFlags()
    _addFlagsCategory(acf, "TrackOverlay", __trackoverlay, 'TrackOverlayConfig')

    def __acts():
        from ActsConfig.ActsConfigFlags import createActsConfigFlags
        return createActsConfigFlags()
    _addFlagsCategory(acf, "Acts", __acts, 'ActsConfig')

    def __hgtd():
        from HGTD_Config.HGTD_ConfigFlags import createHGTD_ConfigFlags
        return createHGTD_ConfigFlags()
    _addFlagsCategory(acf, "HGTD", __hgtd, 'HGTD_Config' )

    def __muon():
        from MuonConfig.MuonConfigFlags import createMuonConfigFlags
        return createMuonConfigFlags()
    _addFlagsCategory(acf, "Muon", __muon, 'MuonConfig' )

    def __muoncombined():
        from MuonCombinedConfig.MuonCombinedConfigFlags import createMuonCombinedConfigFlags
        return createMuonCombinedConfigFlags()
    _addFlagsCategory(acf, "MuonCombined", __muoncombined, 'MuonCombinedConfig' )

    def __egamma():
        from egammaConfig.egammaConfigFlags import createEgammaConfigFlags
        return createEgammaConfigFlags()
    _addFlagsCategory(acf, "Egamma", __egamma, 'egammaConfig' )

    def __met():
        from METReconstruction.METConfigFlags import createMETConfigFlags
        return createMETConfigFlags()
    _addFlagsCategory(acf,"MET",__met, 'METReconstruction')

    def __jet():
        from JetRecConfig.JetConfigFlags import createJetConfigFlags
        return createJetConfigFlags()
    _addFlagsCategory(acf,"Jet",__jet, 'JetRecConfig')
    
    def __ufo():
        from TrackCaloClusterRecTools.UFOConfigFlags import createUFOConfigFlags
        return createUFOConfigFlags()
    _addFlagsCategory(acf,"UFO",__ufo, 'TrackCaloClusterRecTools') 

    def __tau():
        from tauRec.TauConfigFlags import createTauConfigFlags
        return createTauConfigFlags()
    _addFlagsCategory(acf, "Tau",__tau, 'tauRec')

    def __ditau():
        from DiTauRec.DiTauConfigFlags import createDiTauConfigFlags
        return createDiTauConfigFlags()
    _addFlagsCategory(acf, "DiTau",__ditau, 'DiTauRec')
 
    def __pflow():
        from eflowRec.PFConfigFlags import createPFConfigFlags
        return createPFConfigFlags()
    _addFlagsCategory(acf,"PF",__pflow, 'eflowRec')

    def __btagging():
        from JetTagConfig.BTaggingConfigFlags import createBTaggingConfigFlags
        return createBTaggingConfigFlags()
    _addFlagsCategory(acf,"BTagging",__btagging, 'JetTagConfig')

    def __hi():
        from HIRecConfig.HIRecConfigFlags import createHIRecConfigFlags
        return createHIRecConfigFlags()
    _addFlagsCategory(acf, "HeavyIon", __hi, "HIRecConfig")

    def __dq():
        from AthenaMonitoring.DQConfigFlags import createDQConfigFlags
        dqf = createDQConfigFlags()
        return dqf
    _addFlagsCategory(acf, "DQ", __dq, 'AthenaMonitoring' )

    def __perfmon():
        from PerfMonComps.PerfMonConfigFlags import createPerfMonConfigFlags
        return createPerfMonConfigFlags()
    _addFlagsCategory(acf, "PerfMon", __perfmon, 'PerfMonComps')

    def __physVal():
        from PhysValMonitoring.PhysValFlags import createPhysValConfigFlags
        return createPhysValConfigFlags()
    _addFlagsCategory(acf, "PhysVal", __physVal , "PhysValMonitoring")

    def __caloRinger():
        from CaloRingerAlgs.CaloRingerFlags import createCaloRingerConfigFlags
        return createCaloRingerConfigFlags()
    _addFlagsCategory(acf, "CaloRinger", __caloRinger, 'CaloRingerAlgs' )

#egamma derivation Flags:
    def __egammaDerivation():
        from DerivationFrameworkEGamma.EGammaDFConfigFlags import createEGammaDFConfigFlags
        return createEGammaDFConfigFlags()
    _addFlagsCategory(acf, "Derivation.Egamma", __egammaDerivation, 'DerivationFrameworkEGamma' )

#llp derivation Flags:
    def __llpDerivation():
        from DerivationFrameworkLLP.LLPDFConfigFlags import createLLPDFConfigFlags
        return createLLPDFConfigFlags()
    _addFlagsCategory(acf, "Derivation.LLP", __llpDerivation, 'DerivationFrameworkLLP' )

    # onnxruntime flags
    def __onnxruntime():
        from AthOnnxComps.OnnxRuntimeFlags import createOnnxRuntimeFlags
        return createOnnxRuntimeFlags()
    _addFlagsCategory(acf, "AthOnnx", __onnxruntime, 'AthOnnxComps')

    # For AnalysisBase, pick up things grabbed in Athena by the functions above
    if not isGaudiEnv():
        def EDMVersion(flags):
            # POOL files: decide based on HLT output type present in the file
            default_version = 3
            collections = flags.Input.Collections
            if "HLTResult_EF" in collections:
                return 1
            elif "TrigNavigation" in collections:
                return 2
            elif any("HLTNav_Summary" in s for s in collections):
                return 3
            elif not flags.Input.Collections:
                # Special case for empty input files (can happen in merge jobs on the grid)
                # The resulting version doesn't really matter as there's nothing to be done, but we want a valid configuration
                return 3

            return default_version
        acf.addFlag('Trigger.EDMVersion', lambda prevFlags: EDMVersion(prevFlags),
                    help='Trigger EDM version (determined by input file or set to the version to be produced)')

    return acf


ConfigFlags=initConfigFlags()


if __name__=="__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    import sys
    if len(sys.argv) > 1:
        flags.Input.Files = sys.argv[1:]
    else:
        flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA

    flags.loadAllDynamicFlags()
    flags.initAll()
    flags.dump()
