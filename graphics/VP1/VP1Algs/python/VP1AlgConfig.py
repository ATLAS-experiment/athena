# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
#from AthenaConfiguration.Enums import Format

def configureGeometry(flags, cfg):
    if flags.Detector.GeometryBpipe:
        from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
        cfg.merge(BeamPipeGeometryCfg(flags))

    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        cfg.merge(PixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        cfg.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        cfg.merge(TRT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        cfg.merge(ITkPixelReadoutGeometryCfg(flags))

    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        cfg.merge(ITkStripReadoutGeometryCfg(flags))

    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        cfg.merge(LArGMCfg(flags))

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        cfg.merge(TileGMCfg(flags))

    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        cfg.merge(MuonGeoModelCfg(flags))

def getATLASVersion():
    import os

    if "AtlasVersion" in os.environ:
        return os.environ["AtlasVersion"]
    if "AtlasBaseVersion" in os.environ:
        return os.environ["AtlasBaseVersion"]
    return "Unknown"

def VP1AlgCfg(flags, name="VP1AlgCA", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    kwargs.setdefault("AtlasRelease", getATLASVersion())

    the_alg = CompFactory.VP1Alg(name="VP1EventDisplayAlg", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result

def SetupVP1():
    from AthenaConfiguration.Enums import Format
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import VERBOSE

    # ++++ Firstly we setup flags ++++
    _logger = logging.getLogger('VP1')
    _logger.setLevel(VERBOSE)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 0 
    # ^ VP1 will not work with the scheduler, since its condition/data dependencies are not known in advance
    # More in details: the scheduler needs to know BEFORE the event, what the dependencies of each Alg are. 
    # So for VP1, no dependencies are declared, which means the conditions data is not there. 
    # So when I load tracks, the geometry is missing and it crashes. 
    # Turning off the scheduler (with NumThreads=0) fixes this.

    parser = flags.getArgumentParser()
    parser.prog = 'vp1'
    parser.description = """
    VP1, or Virtual Point 1, is the interactive 3D event display for the ATLAS experiment at CERN.
    Detailed documentation can be found at the webpage: https://atlas-vp1.web.cern.ch/atlas-vp1, 
    but below are the flags that can be used to configure VP1 (most are standard Athena flags, but some are VP1-specific).
    """
    parser.description ="""
    So for example, to run VP1 on a file: vp1 [options] myESD.pool.root"""
    # Add VP1-specific arguments here, but remember you can also directly pass flags in form <flagName>=<value>.
    group = parser.add_argument_group('VP1 specific')
    group.add_argument('Filename', help="Input file to pass to VP1 (i.e. vp1 myESD.pool.root as an alternative to vp1 --filesInput=[])", metavar="File name")           
    group.add_argument('--verboseAthena', action='store_true', help="If false, tell Athena to suppress INFO messages and below.")  
    group.add_argument('--cruise', type=int, help="Start in cruise mode, changing events after N seconds.")           
    # Batch
    group.add_argument('--batch', action='store_true', help="Run VP1 in 'batch' mode with a given configuration file.")         
    group.add_argument('--batch-all-events', action='store_true', help="Process all events in the input data file in '-batch' mode. Use this together with '-batch'.")         
    group.add_argument('--batch-n-events', type=int, help="Process 'N' events in the input data file in '-batch' mode. Use this together with '-batch'.")           
    group.add_argument('--batch-output-folder', help="Specify an output folder to store the event displays produced with the '-batch' option.")           
    group.add_argument('--batch-random-config', action='store_true', help="Run VP1 in 'batch' mode; for each single event a configuration file will be randomly picked out of the configuration files provided by the user. Use this together with '-batch'.")         
    # Live / Livelocal
    group.add_argument('--live', action='store_true', help="Run on live events from point 1.")         
    group.add_argument('--livelocal', action='store_true', help="Run on live events from point 1 in local directory.")         
    group.add_argument('--eventsrc', help="Directory to take single event files from (do not specify input files in this case). To get files from a web server (i.e. live events), put instead the url to the file residing in the same directory (most users should just use the --live option instead).")           
    group.add_argument('--extraevtsrcdir', help="Directory to take single event files from (do not specify input files in this case). To get files from a web server (i.e. live events), put instead the url to the file residing in the same directory (most users should just use the -live option instead).")           
    group.add_argument('--eventcpy', help="Directory to keep local copies of processed event files. If --eventsrc is set, then -eventcpy will default to /tmp/emoyse/vp1events/6897 .")           
    #group.add_argument('--nocleanupeventcpy', action='store_true', help="Prevent removal of eventcpy directory after athena process finishes.")         
    # Commented, because I'm not sure how to implement this safely.

    # Support the positional version of passing file name e.g. vp1 myESD.pool.root
    args = flags.fillFromArgs(parser=parser)
    if args.Filename and (flags.Input.Files == [] or 
        flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']):
        flags.Input.Files = [args.Filename]

    if 'help' in args:
        # No point doing more here, since we just want to print the help.
        import sys
        sys.exit()

    _logger.verbose("+ About to set flags related to the input")

    # Empty input is not normal for Athena, so we will need to check 
    # this repeatedly below
    vp1_empty_input = False  
    # This covers the use case where we launch VP1
    # without input files; e.g., to check the detector description
    if (flags.Input.Files == [] or 
        flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']):
        from Campaigns.Utils import Campaign
        from AthenaConfiguration.TestDefaults import defaultGeometryTags

        vp1_empty_input = True
        # NB Must set e.g. ConfigFlags.Input.Runparse_args() Number and
        # ConfigFlags.Input.TimeStamp before calling the 
        # MainServicesCfg to avoid it attempting auto-configuration 
        # from an input file, which is empty in this use case.
        # If you don't have it, it (and/or other Cfg routines) complains and crashes. 
        # See also: 
        # https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetConditions/SCT_ConditionsAlgorithms/python/SCT_DCSConditionsTestAlgConfig.py#0023
        flags.Input.ProjectName = "mc20_13TeV"
        flags.Input.RunNumbers = [330000]  
        flags.Input.TimeStamps = [1]  
        flags.Input.TypedCollections = []

        # set default CondDB and Geometry version
        flags.IOVDb.GlobalTag = "OFLCOND-MC23-SDR-RUN3-02"
        flags.Input.isMC = True
        flags.Input.MCCampaign = Campaign.Unknown
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    else:
        # Now just check file exists, or fail gracefully
        from os import path
        for file in flags.Input.Files:
            if not path.exists(flags.Input.Files[0]):
                _logger.warning("Input file", file, "does not exist")
                import sys
                sys.exit(1)
    _logger.verbose("+ ... Input flags done")

    _logger.verbose("+ About to set the detector flags")
    # So we can now set up the geometry flags from the input
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=not vp1_empty_input,
                       toggle_geometry=True, keep_beampipe=True)
    _logger.verbose("+ ... Detector flags done")

    # finalize setting flags: lock them.
    flags.lock()

    # DEBUG -- inspect the flags
    # flags.dump()
    # flags._loadDynaFlags('GeoModel')
    # flags._loadDynaFlags('Detector')
    # flags.dump('Detector.(Geometry|Enable)', True)

    # ++++ Now we setup the actual configuration ++++

    # NB Must have set ConfigFlags.Input.RunNumber and
    # ConfigFlags.Input.TimeStamp before calling to avoid
    # attempted auto-configuration from an input file.
    _logger.verbose("+ Setup main services, and input file reading")

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    if not vp1_empty_input:
        # Only do this if we have input files, otherwise flags will try to read metadata
        # Check if we are reading from POOL and setupo convertors if so
        if flags.Input.Format is Format.POOL:
            from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
            cfg.merge(PoolReadCfg(flags))
            # Check if running on legacy inputs
            if "EventInfo" not in flags.Input.Collections:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import (
                    EventInfoCnvAlgCfg)
                cfg.merge(EventInfoCnvAlgCfg(flags))
            from TrkConfig.TrackCollectionReadConfig import TrackCollectionReadCfg
            cfg.merge(TrackCollectionReadCfg(flags, 'Tracks'))
        if flags.Input.isMC:
            # AOD2xAOD Truth conversion
            from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
            cfg.merge(GEN_AOD2xAODCfg(flags))
    _logger.verbose("+ ... Main services done")

    _logger.verbose("+ About to setup geometry")
    configureGeometry(flags,cfg)
    _logger.verbose("+ ... Geometry done")

    # Setup some VP1 specific stuff
    vp1config = {}
    if not args.verboseAthena:
        # Suppress the output from Athena
        print('Suppressing most messages from Athena.')
        print('To see more, set the --verboseAthena flag to true.')
        msgService = cfg.getService('MessageSvc')
        msgService.OutputLevel = 4

    if args.cruise:
        vp1config.setdefault('InitialCruiseMode', 'EVENT')
        vp1config.setdefault('vp1Alg.InitialCruiseModePeriod', args.cruise)

    # Batch mode
    if 'batch' in args:
       setup_batch_mode(args)
    
    # Event copying and live
    if 'eventsrc' in args:
        vp1config.setdefault('MultipleFilesON', True)

    if 'live' in args or 'livelocal' in args:
        setup_live_mode(args, vp1config)

    # configure VP1
    cfg.merge(VP1AlgCfg(flags, vp1config)) 
    cfg.run()


def setup_live_mode(args, vp1config):
    vp1config.setdefault('MultipleFilesON', True)
    if 'eventcpy' in args:
        vp1config.setdefault('vp1Alg.MFLocalCopyDir', args.eventcpy)
    if 'extraevtsrcdir' in args:
        vp1config.setdefault('MFAvailableLocalInputDirectories', args.extraevtsrcdir)
    if 'live' in args:
        vp1config.setdefault('MFSourceDir', "https://atlas-live.cern.ch/event_files/L1MinBias/vp1fileinfo.txt")
    elif 'livelocal' in args:
        vp1config.setdefault('MFSourceDir', "/VP1_events/")


def setup_batch_mode(args):
    # BATCH-MODE
    # If "--batch" is True, then set the corresponding env var. 
    # The GUI of VP1 will not be shown, but the config file will be taken
    # and in the end a render of the 3D window will be saved as PNG file.
    import os
    if args.batch:
        os.putenv("VP1_BATCHMODE","1")
    if args.batch_all_events:
        os.putenv("VP1_BATCHMODE_ALLEVENTS","1")
    if args.batch-n-events > 0:
        os.putenv("VP1_BATCHMODE_NEVENTS", str(args.batch-n-events) )
    if (args.batch-output-folder != ""):
        os.putenv("VP1_BATCHMODE_OUT_FOLDER", args.batch-output-folder)
    if args.batch_random_config:
        os.putenv("VP1_BATCHMODE_RANDOMCONFIG","1")

if __name__=="__main__":
    # Run with e.g. python -m VP1Algs.VP1AlgConfig --filesInput=myESD_ca.pool.root
    SetupVP1()    
    import sys
    sys.exit()
