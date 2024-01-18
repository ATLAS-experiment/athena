# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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
    acf.addFlag('Exec.OutputLevel',INFO) #Global Output Level
    acf.addFlag('Exec.PrintAlgsSequence', False) # Allows AppMgr to print the algorithm sequence
    acf.addFlag('Exec.MaxEvents',-1)
    acf.addFlag('Exec.SkipEvents',0)
    acf.addFlag('Exec.DebugStage','')
    acf.addFlag('Exec.Interactive',"")
    acf.addFlag('Exec.FPE',0) #-2: No FPE check at all, -1: Abort with core-dump, 0: FPE Auditor w/o stack-tace (default) , >0: number of stack-trace printed by the job

    #Custom messaging for components, see Utils.setupLoggingLevels
    acf.addFlag('Exec.VerboseMessageComponents',[])
    acf.addFlag('Exec.DebugMessageComponents',[])
    acf.addFlag('Exec.InfoMessageComponents',[])
    acf.addFlag('Exec.WarningMessageComponents',[])
    acf.addFlag('Exec.ErrorMessageComponents',[])

    #Multi-threaded event service mode
    acf.addFlag('Exec.MTEventService',False)
    acf.addFlag('Exec.MTEventServiceChannel','EventService_EventRanges') # The name of YAMPL communication channel between AthenaMT and Pilot

    #Activate per-event log-output of StoreGate content
    acf.addFlag('Debug.DumpEvtStore',False)
    acf.addFlag('Debug.DumpDetStore',False)
    acf.addFlag('Debug.DumpCondStore',False)

    acf.addFlag('ExecutorSplitting.TotalSteps', 0)
    acf.addFlag('ExecutorSplitting.Step', -1)
    acf.addFlag('ExecutorSplitting.TotalEvents', -1)

    #Flags describing the input data
    acf.addFlag('Input.Files', ["_ATHENA_GENERIC_INPUTFILE_NAME_",]) # former global.InputFiles
    acf.addFlag("Input.FileNentries", -1) # will be filled if available in the runArgs
    acf.addFlag('Input.SecondaryFiles', []) # secondary input files for DoubleEventSelector
    acf.addFlag('Input.isMC', lambda prevFlags : "IS_SIMULATION" in GetFileMD(prevFlags.Input.Files).get("eventTypes", [])) # former global.isMC
    acf.addFlag('Input.OverrideRunNumber', False )
    acf.addFlag("Input.ConditionsRunNumber", -1) # Override the HITS file Run Number with one from a data run (TODO merge with Input.RunNumbers)
    acf.addFlag('Input.RunNumbers', lambda prevFlags : list(GetFileMD(prevFlags.Input.Files).get("runNumbers", []))) # former global.RunNumber
    acf.addFlag('Input.MCChannelNumber', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("mc_channel_number", 0))
    acf.addFlag('Input.LumiBlockNumbers', lambda prevFlags : list(GetFileMD(prevFlags.Input.Files).get("lumiBlockNumbers", []))) # former global.RunNumber
    acf.addFlag('Input.TimeStamps', lambda prevFlags : getInitialTimeStampsFromRunNumbers(prevFlags.Input.RunNumbers) if prevFlags.Input.OverrideRunNumber else [])
    # Configure EvtIdModifierSvc with a list of dictionaries of the form:
    # {'run': 152166, 'lb': 202, 'starttstamp': 1269948352889940910, 'evts': 1, 'mu': 0.005}
    acf.addFlag("Input.RunAndLumiOverrideList", [])
    # Job number
    acf.addFlag("Input.JobNumber", 1)
    acf.addFlag('Input.FailOnUnknownCollections', False)

    def _dataYearFromFlags(prevFlags):
        dataYear = GetFileMD(prevFlags.Input.Files).get("data_year", "")
        if dataYear:
            return int(dataYear)
        if prevFlags.Input.ProjectName.startswith("data"):
            return 2000 + int(prevFlags.Input.ProjectName[4:6])
        return 0

    acf.addFlag('Input.ProjectName', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("project_name", "")) # former global.ProjectName
    acf.addFlag('Input.DataYear', _dataYearFromFlags)
    acf.addFlag('Input.MCCampaign', lambda prevFlags : Campaign(GetFileMD(prevFlags.Input.Files).get("mc_campaign", "")), enum=Campaign)
    acf.addFlag('Input.TriggerStream', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("stream", "") if prevFlags.Input.Format == Format.BS
                                                          else GetFileMD(prevFlags.Input.Files).get("triggerStreamOfFile", "")) # former global.TriggerStream
    acf.addFlag('Input.Format', lambda prevFlags : Format.BS if GetFileMD(prevFlags.Input.Files).get("file_type", "BS") == "BS" else Format.POOL, enum=Format) # former global.InputFormat
    acf.addFlag('Input.ProcessingTags', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("processingTags", []) ) # list of names of streams written to this file
    from GeneratorConfig.Versioning import generatorsGetFromMetadata
    acf.addFlag('Input.GeneratorsInfo', lambda prevFlags : generatorsGetFromMetadata( GetFileMD(prevFlags.Input.Files).get("generators", "") ))
    acf.addFlag('Input.SpecialConfiguration', lambda prevFlags : getSpecialConfigurationMetadata(prevFlags.Input.Files, prevFlags.Input.SecondaryFiles))  # special Configuration options read from input file metadata

    def _inputCollections(inputFile):
        rawCollections = [type_key[1] for type_key in GetFileMD(inputFile).get("itemList", [])]
        collections = [col for col in rawCollections if not col.endswith('Aux.')]
        return collections

    def _typedInputCollections(inputFile):
        collections = ['%s#%s' % type_key for type_key in GetFileMD(inputFile).get("itemList", [])]
        return collections

    acf.addFlag('Input.Collections', lambda prevFlags : _inputCollections(prevFlags.Input.Files) )
    acf.addFlag('Input.SecondaryCollections', lambda prevFlags : _inputCollections(prevFlags.Input.SecondaryFiles) )
    acf.addFlag('Input.TypedCollections', lambda prevFlags : _typedInputCollections(prevFlags.Input.Files) )
    acf.addFlag('Input.SecondaryTypedCollections', lambda prevFlags : _typedInputCollections(prevFlags.Input.SecondaryFiles) )

    def _metadataItems(inputFile):
        return GetFileMD(inputFile).get("metadata_items", {})

    acf.addFlag('Input.MetadataItems', lambda prevFlags : _metadataItems(prevFlags.Input.Files) )
    acf.addFlag('Input.Release',  lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("AtlasRelease", ""))
    acf.addFlag('Input.AODFixesDone', lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("AODFixVersion", ""))

    acf.addFlag('Concurrency.NumProcs', 0)
    acf.addFlag('Concurrency.NumThreads', 0 )
    acf.addFlag('Concurrency.NumConcurrentEvents', lambda prevFlags : prevFlags.Concurrency.NumThreads)
    acf.addFlag('Concurrency.DebugWorkers', False )

    acf.addFlag('Scheduler.CheckDependencies', True)
    acf.addFlag('Scheduler.CheckOutputUsage', False)
    acf.addFlag('Scheduler.ShowDataDeps', False)
    acf.addFlag('Scheduler.ShowDataFlow', False)
    acf.addFlag('Scheduler.ShowControlFlow', False)
    acf.addFlag('Scheduler.EnableVerboseViews', True)
    acf.addFlag('Scheduler.AutoLoadUnmetDependencies', True)

    acf.addFlag('MP.WorkerTopDir', 'athenaMP_workers')
    acf.addFlag('MP.OutputReportFile', 'AthenaMPOutputs')
    acf.addFlag('MP.Strategy', 'SharedQueue')
    acf.addFlag('MP.CollectSubprocessLogs', False)
    acf.addFlag('MP.PollingInterval', 100)
    acf.addFlag('MP.EventsBeforeFork', 0)
    acf.addFlag('MP.EventRangeChannel', 'EventService_EventRanges')
    acf.addFlag('MP.EvtRangeScattererCaching', False)
    acf.addFlag('MP.MemSamplingInterval', 0)
    """ Size of event chunks in the shared queue
        if chunk_size==-1, chunk size is set to auto_flush for files compressed with LZMA
        if chunk_size==-2, chunk size is set to auto_flush for files compressed with LZMA or ZLIB
        if chunk_size==-3, chunk size is set to auto_flush for files compressed with LZMA, ZLIB, or LZ4
        if chunk_size<=-4, chunk size is set to auto_flush
    """
    acf.addFlag('MP.ChunkSize', -1)
    acf.addFlag('MP.ReadEventOrders', False)
    acf.addFlag('MP.EventOrdersFile', 'athenamp_eventorders.txt')
    acf.addFlag('MP.UseSharedReader', False)
    acf.addFlag('MP.UseSharedWriter', False)
    acf.addFlag('MP.UseParallelCompression', True)

    acf.addFlag('Common.MsgSuppression', True) # Enable suppression of printout in MessageSvc
    acf.addFlag('Common.MsgSourceLength',50) #Length of the source-field in the format str of MessageSvc
    acf.addFlag('Common.ShowMsgStats',False) #Print stats about WARNINGs, etc at the end of the job

    acf.addFlag('Common.isOnline', False ) #  Job runs in an online environment (access only to resources available at P1) # former global.isOnline
    acf.addFlag('Common.useOnlineLumi', lambda prevFlags : prevFlags.Common.isOnline ) #  Use online version of luminosity. ??? Should just use isOnline?
    acf.addFlag('Common.isOverlay', lambda prevFlags: (prevFlags.Common.ProductionStep == ProductionStep.Overlay or
                                                       (prevFlags.Common.ProductionStep == ProductionStep.FastChain and
                                                        prevFlags.Overlay.FastChain)))  # Enable Overlay
    acf.addFlag('Common.doExpressProcessing', False)
    acf.addFlag('Common.ProductionStep', ProductionStep.Default, enum=ProductionStep)
    acf.addFlag('Common.Project', Project.determine(), enum=Project)

    # replace global.Beam*
    acf.addFlag('Beam.BunchSpacing', 25) # former global.BunchSpacing
    acf.addFlag('Beam.Type', lambda prevFlags : BeamType(GetFileMD(prevFlags.Input.Files).get('beam_type', 'collisions')), enum=BeamType)# former global.BeamType
    acf.addFlag("Beam.NumberOfCollisions", lambda prevFlags : 2. if prevFlags.Beam.Type is BeamType.Collisions else 0.) # former global.NumberOfCollisions

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


    acf.addFlag('Beam.Energy', lambda prevFlags : _configureBeamEnergy(prevFlags)) # former global.BeamEnergy
    acf.addFlag('Beam.estimatedLuminosity', lambda prevFlags : ( 1E33*(prevFlags.Beam.NumberOfCollisions)/2.3 ) *\
        (25./prevFlags.Beam.BunchSpacing)) # former flobal.estimatedLuminosity
    acf.addFlag('Beam.BunchStructureSource', lambda prevFlags: BunchStructureSource.MC if prevFlags.Input.isMC else BunchStructureSource.TrigConf)

    # output
    acf.addFlag('Output.EVNTFileName', '')
    acf.addFlag('Output.EVNT_TRFileName', '')
    acf.addFlag('Output.HITSFileName', '')
    acf.addFlag('Output.RDOFileName',  '')
    acf.addFlag('Output.RDO_SGNLFileName', '')
    acf.addFlag('Output.ESDFileName',  '')
    acf.addFlag('Output.AODFileName',  '')
    acf.addFlag('Output.HISTFileName', '')

    acf.addFlag('Output.doWriteHITS', lambda prevFlags: bool(prevFlags.Output.HITSFileName)) # write out HITS file
    acf.addFlag('Output.doWriteRDO', lambda prevFlags: bool(prevFlags.Output.RDOFileName)) # write out RDO file
    acf.addFlag('Output.doWriteRDO_SGNL', lambda prevFlags: bool(prevFlags.Output.RDO_SGNLFileName)) # write out RDO_SGNL file
    acf.addFlag('Output.doWriteESD', lambda prevFlags: bool(prevFlags.Output.ESDFileName)) # write out ESD file
    acf.addFlag('Output.doWriteAOD', lambda prevFlags: bool(prevFlags.Output.AODFileName)) # write out AOD file
    acf.addFlag('Output.doWriteBS',  False) # write out RDO ByteStream file
    acf.addFlag('Output.doWriteDAOD',  False) # write out at least one DAOD file
    acf.addFlag('Output.doJiveXML',  False) # write out a JiveXML file

    acf.addFlag('Output.TreeAutoFlush', {})  # {} = automatic for all streams, otherwise {'STREAM': 123}
    acf.addFlag('Output.StorageTechnology.EventData', 'ROOTTREEINDEX')  # Set the underlying POOL storage technology for event data
    acf.addFlag('Output.StorageTechnology.MetaData', 'ROOTTREEINDEX')  # Set the underlying POOL storage technology for metadata

    # Might move this elsewhere in the future.
    # Some flags from https://gitlab.cern.ch/atlas/athena/blob/master/Tracking/TrkDetDescr/TrkDetDescrSvc/python/TrkDetDescrJobProperties.py
    # (many, e.g. those that set properties of one tool are not needed)
    acf.addFlag('TrackingGeometry.MagneticFileMode', 6)
    acf.addFlag('TrackingGeometry.MaterialSource', 'COOL') # Can be COOL, Input or None

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

#IOVDbSvc Flags:
    if isGaudiEnv():
        from IOVDbSvc.IOVDbAutoCfgFlags import getLastGlobalTag, getDatabaseInstanceDefault

        def __getTrigTag(flags):
            from TriggerJobOpts.TriggerConfigFlags import trigGlobalTag
            return trigGlobalTag(flags)

        acf.addFlag("IOVDb.GlobalTag", lambda flags :
                    (__getTrigTag(flags) if flags.Trigger.doLVL1 or flags.Trigger.doHLT else None) or
                    getLastGlobalTag(flags))

        acf.addFlag("IOVDb.DatabaseInstance",getDatabaseInstanceDefault)

        # Run dependent simulation
        # map from runNumber to timestamp; migrated from RunDMCFlags.py
        acf.addFlag("IOVDb.RunToTimestampDict", lambda prevFlags: getRunToTimestampDict())
        acf.addFlag("IOVDb.DBConnection", lambda prevFlags : "sqlite://;schema=mycool.db;dbname=" + prevFlags.IOVDb.DatabaseInstance)
        #For HLT-jobs, the ring-size should be 0 (eg no cleaning at all since there are no IOV-updates during the job)
        acf.addFlag("IOVDb.CleanerRingSize",lambda prevFlags : 0 if prevFlags.Trigger.doHLT else 2*max(1, prevFlags.Concurrency.NumConcurrentEvents))
        acf.addFlag("IOVDb.SqliteInput","",help="Folders found in this file will be used instead of the production db")
        acf.addFlag("IOVDb.SqliteFolders",(),help="Folders listed here will be taken from the IOVDb.SqliteInput file instead of the production db. If empty, all folders found in the file are used.")
#PoolSvc Flags:
    acf.addFlag("PoolSvc.MaxFilesOpen", lambda prevFlags : 2 if prevFlags.MP.UseSharedReader else 0)


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
    acf.addFlag("Random.Engine", "dSFMT") # Random service used in {"dSFMT", "Ranlux64", "Ranecu"}
    acf.addFlag("Random.SeedOffset", 0) # TODO replace usage of Digitization.RandomSeedOffset with this flag

    def __trigger():
        from TriggerJobOpts.TriggerConfigFlags import createTriggerFlags
        return createTriggerFlags(acf.Common.Project is not Project.AthAnalysis)

    added = _addFlagsCategory(acf, "Trigger", __trigger, 'TriggerJobOpts' )
    if not added:
        # If TriggerJobOpts is not available, we add at least these basic flags
        # to indicate Trigger is not available:
        acf.addFlag('Trigger.doLVL1', False)
        acf.addFlag('Trigger.doHLT', False)

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
        acf.addFlag('Trigger.EDMVersion', lambda prevFlags: EDMVersion(prevFlags))

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
