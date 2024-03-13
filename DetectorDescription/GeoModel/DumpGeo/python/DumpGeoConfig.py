# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#----------------------------------------------------------------
# Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
# Initial version: Feb 2024
#----------------------------------------------------------------

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

    # Trigger the build of the InDetServMat geometry 
    # if any ID subsystems have been enabled
    if flags.Detector.GeometryID:
        from InDetServMatGeoModel.InDetServMatGeoModelConfig import (
             InDetServiceMaterialCfg)
        cfg.merge(InDetServiceMaterialCfg(flags))




def getATLASVersion():
    import os

    if "AtlasVersion" in os.environ:
        return os.environ["AtlasVersion"]
    if "AtlasBaseVersion" in os.environ:
        return os.environ["AtlasBaseVersion"]
    return "Unknown"

def DumpGeoCfg(flags, name="DumpGeoCA", outFileName="", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    #print(dir("args: ", args)) # debug

    
    # set diitional Alg's properties
    # Note: at this point, the user-defined Geo TAG args.detDescr, 
    #       if set, has already replaced the default GeoModel.AtlasVersion in 'flags';
    #       so, we can use the latter, directy.
    _logger.verbose("Using ATLAS/Athena version: %s", getATLASVersion())
    _logger.verbose("Using GeoModel ATLAS version: %s", flags.GeoModel.AtlasVersion)
    kwargs.setdefault("AtlasRelease", getATLASVersion())
    kwargs.setdefault("AtlasVersion", flags.GeoModel.AtlasVersion)
    kwargs.setdefault("OutSQLiteFileName", outFileName)
    if args.filterDetManagers:
        kwargs.setdefault("UserFilterDetManager", args.filterDetManagers.split(","))


    the_alg = CompFactory.DumpGeo(name="DumpGeoAlg", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result


if __name__=="__main__":
    import os, sys
    # Run with e.g. python -m DumpGeo.DumpGeoConfig --detDescr=<ATLAS-geometry-tag> --filter=[<list of tree tops>]
    
    # from AthenaConfiguration.Enums import Format
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import VERBOSE

    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    # ++++ Firstly we setup flags ++++
    _logger = logging.getLogger('DumpGeo')
    _logger.setLevel(VERBOSE)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 0 
    # ^ We do not need any events to get the GeoModel tree from the GeoModelSvc.
    # So, we don't need to run on any events, 
    # and we don't need to trigger any execute() Athena methods either. 
    # So, we set 'EvtMax' to 0 and Athena will skip the 'execute' phase; 
    # only the 'finalize' step will be run after the 'init'.
    # -- Note: 
    # Also, if we run on events (even on 1 event) and we dump the Pixel 
    # as part of the  DetectorManager filter, then we get a crash because 
    # the PixelDetectorManager complains during the 'execute' phase, 
    # because we 'stole' a handle on its TreeTop, 
    # which contains a FullPhysVol and cannot be shared.
    flags.Concurrency.NumThreads = 0 
    # ^ VP1 will not work with the scheduler, since its condition/data dependencies are not known in advance
    # More in details: the scheduler needs to know BEFORE the event, what the dependencies of each Alg are. 
    # So for VP1, no dependencies are declared, which means the conditions data is not there. 
    # So when I load tracks, the geometry is missing and it crashes. 
    # Turning off the scheduler (with NumThreads=0) fixes this.

    parser = flags.getArgumentParser()
    parser.prog = 'dump-geo'
    # here we extend the parser with CLI options specific to DumpGeo
    parser.add_argument("--detDescr", default=defaultGeometryTags.RUN3,
                        help="The ATLAS geometry tag you want to dump (a convenience alias for the Athena flag 'GeoModel.AtlasVersion=TAG')", metavar="TAG")
    # parser.add_argument("--filterTreeTops", help="Only output the GeoModel Tree Tops specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("--filterDetManagers", help="Only output the GeoModel Detector Managers specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("-f", "--forceOverwrite",
                        help="Force to overwrite an existing SQLite output file with the same name, if any", action = 'store_true')

    args = flags.fillFromArgs(parser=parser)

    if 'help' in args:
        # No point doing more here, since we just want to print the help.
        sys.exit()

    _logger.verbose("+ About to set flags related to the input")

    # Empty input is not normal for Athena, so we will need to check 
    # this repeatedly below (the same as with VP1)
    dumpgeo_empty_input = False  
    # This covers the use case where we launch DumpGeo
    # without input files; e.g., to check the detector description
    if (flags.Input.Files == [] or 
        flags.Input.Files == ['_ATHENA_GENERIC_INPUTFILE_NAME_']):
        from Campaigns.Utils import Campaign
        from AthenaConfiguration.TestDefaults import defaultGeometryTags

        dumpgeo_empty_input = True
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
    _logger.verbose("+ ... Done")
    _logger.verbose("+ empty input: '%s'" % dumpgeo_empty_input)

    _logger.verbose("+ detDescr flag: '%s'" % args.detDescr)


    _logger.verbose("+ About to set the detector flags")
    # So we can now set up the geometry flags from the input
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=not dumpgeo_empty_input,
                       toggle_geometry=True, keep_beampipe=True)
    _logger.verbose("+ ... Done")

    if args.detDescr:
        _logger.verbose("+ About to set a custom user-defined detector description tag: '%s'" % args.detDescr)
        flags.GeoModel.AtlasVersion = args.detDescr
        _logger.verbose("+ ... Done")


    # finalize setting flags: lock them.
    flags.lock()



    # Handle the user's inputs and create a file name 
    # for the output SQLite, accordingly
    outFileName = "geometry"
    filterDetManagers = []
    # - Put Geometry TAG into the file name
    # NOTE: at this point, the user-defined Geo TAG args.detDescr, 
    #       if set, has already replaced the default TAG in 'flags';
    #       so, we can use the latter, directy.
    geoTAG = flags.GeoModel.AtlasVersion
    print("+ DumpGeo -- INFO -- This is the Detector Description geometry TAG you are dumping: '%s'" % geoTAG)
    outFileName = outFileName + "-" + geoTAG
    # - Put DetectorManagers' names into the file name, 
    #   if the filter has been used by the user
    if args.filterDetManagers:
        print("+ DumpGeo -- NOTE -- Your 'GeoModel Detector Manager' filter set: '%s'" % args.filterDetManagers)
        filterDetManagers = args.filterDetManagers.split(",")
        outFileName = outFileName + "-" + "-".join(filterDetManagers)
    # - Add the final extension to the name of the output SQLite file 
    outFileName = outFileName + ".db"

    # Overwrite the output SQLite file, if existing
    if os.path.exists(outFileName):
        if args.forceOverwrite is True:
            print("+ DumpGeo -- NOTE -- You chose to overwrite an existing geometry dump file with the same name, if present.")
            # os.environ["DUMPGEOFORCEOVERWRITE"] = "1" # save to an env var, for later use in GeoModelStandalone/GeoExporter
            # Check if the file exists before attempting to delete it   
            if os.path.exists(outFileName):
                os.remove(outFileName)
                print(f"The file {outFileName} has been deleted.")
            else:
                print(f"The file {outFileName} does not exist. So, it was not needed to 'force-delete' it. Continuing...")
        else:
            print(f"\nDumpGeo -- ERROR! The ouput file '{outFileName}' exists already!\nPlease move or remove it, or use the 'force' option: '-f' or '--forceOverWrite'.\n\n")
            sys.exit()
            #raise ValueError("The output file exists already!")


    # DEBUG -- inspect the flags
    # flags.dump()
    # flags._loadDynaFlags('GeoModel')
    # flags._loadDynaFlags('Detector')
    # flags.dump('Detector.(Geometry|Enable)', True)

    # ++++ Now we setup the actual configuration ++++
    _logger.verbose("+ Setup main services")
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    _logger.verbose("+ ...Done")

    _logger.verbose("+ About to setup geometry")
    configureGeometry(flags,cfg)
    _logger.verbose("+ ...Done")

    # configure DumpGeo
    cfg.merge(DumpGeoCfg(flags, args, outFileName=outFileName)) 
    cfg.run()

