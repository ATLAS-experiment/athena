# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
#from AthenaConfiguration.Enums import Format

import os

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

def DumpGeoCfg(flags, name="DumpGeoCA", **kwargs):
    # This is based on a few old-style configuation files:
    # JiveXML_RecEx_config.py
    # JiveXML_jobOptionBase.py
    result = ComponentAccumulator()

    #print(dir("args: ", args)) # debug

    # set Alg's properties
    kwargs.setdefault("AtlasRelease", getATLASVersion())

    # TODO: Fix this
    # This is a temporary hack to reflect how detDescr is handled in the old bash-driven DumpGeo
    # This should be replaced by proper python flags and Gaudy properties
    if args.detDescr:
        os.environ["DUMPGEODETDESCRTAG"] = args.detDescr # save to an env var, for later use in GeoModelStandalone/GeoExporter
        print("+ DumpGeo -- NOTE -- You chose to overwrite the default Detector Descriptoon Geometry TAG with this: '%s'" % args.detDescr)
    if args.forceOverwrite is True:
        print("+ DumpGeo -- NOTE -- You chose to overwrite an existing geometry dump file with the same name, if present.")
        os.environ["DUMPGEOFORCEOVERWRITE"] = "1" # save to an env var, for later use in GeoModelStandalone/GeoExporter
    if args.filterTreeTops:
        print("+ DumpGeo -- NOTE -- Your 'GeoModel TreeTop' filter set: '%s'" % args.filterTreeTops)
        os.environ["DUMPGEOFILTERTREETOPS"] = args.filterTreeTops # save to an env var, for later use in GeoModelStandalone/GeoExporter
    if args.filterDetManagers:
        print("+ DumpGeo -- NOTE -- Your 'GeoModel Detector Manager' filter set: '%s'" % args.filterDetManagers)
        os.environ["DUMPGEOFILTERDETMANAGERS"] = args.filterDetManagers # save to an env var, for later use in GeoModelStandalone/GeoExporter


    the_alg = CompFactory.DumpGeo(name="DumpGeoAlg", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result


if __name__=="__main__":
    # Run with e.g. python -m DumpGeo.DumpGeoConfig --detDescr=<ATLAS-geometry-tag> --filter=[<list of tree tops>]
    
    # from AthenaConfiguration.Enums import Format
    from AthenaCommon.Logging import logging
    from AthenaCommon.Constants import VERBOSE

    # ++++ Firstly we setup flags ++++
    _logger = logging.getLogger('DumpGeo')
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
    parser.prog = 'dump-geo'
    # here we extend the parser
    # Add DumpGeo-specific arguments here, but remember you can also directly pass flags in form <flagName>=<value>.
    # e.g.
    # parser.add_argument("-o", "--output", dest="output", default='Event.json',
    #                     help="write JSON to FILE", metavar="FILE")
    parser.add_argument("--detDescr", default='',
                        help="The ATLAS geometry tag you want to dump (a convenience alias for the Athena flag 'GeoModel.AtlasVersion=TAG')", metavar="TAG")
    parser.add_argument("--filterTreeTops", help="Only output the GeoModel Tree Tops specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("--filterDetManagers", help="Only output the GeoModel Detector Managers specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("-f", "--forceOverwrite",
                        help="Force to overwrite an existing SQLite output file with the same name, if any", action = 'store_true')

    args = flags.fillFromArgs(parser=parser)

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
    _logger.verbose("+ ... Done")
    _logger.verbose("+ empty input: '%s'" % vp1_empty_input)

    _logger.verbose("+ detDescr flag: '%s'" % args.detDescr)


    _logger.verbose("+ About to set the detector flags")
    # So we can now set up the geometry flags from the input
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=not vp1_empty_input,
                       toggle_geometry=True, keep_beampipe=True)
    _logger.verbose("+ ... Done")

    if args.detDescr:
        _logger.verbose("+ About to set a custom detector description tag")
        flags.GeoModel.AtlasVersion = args.detDescr
        _logger.verbose("+ ... Done")


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
  
    _logger.verbose("+ ...Done")

    _logger.verbose("+ About to setup geometry")
    configureGeometry(flags,cfg)
    _logger.verbose("+ ...Done")

    # configure VP1
    cfg.merge(DumpGeoCfg(flags, args)) 
    cfg.run()

