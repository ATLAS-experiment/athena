# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#----------------------------------------------------------------
# Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
# Initial version: Feb 2024
#
# Main updates:
# - 2025, Feb -- Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
#                Added dedicated DumpGeo flags as GeoModel.DumpGeo; 
#                also,  support the use of DumpGeo transforms and other
#                Athena jobs -- that is, not standalone. This is useful 
#                when we want to dump the geometry that comes out as the 
#                output of an Athena job.
#----------------------------------------------------------------
import os, sys

# Set the CA environment
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Set the Athena Logger
from AthenaCommon.Logging import logging
_logger = logging.getLogger('DumpGeo')

def configureGeometry(flags, cfg):

    # Beam pipe
    if flags.Detector.GeometryBpipe:
        from BeamPipeGeoModel.BeamPipeGMConfig import BeamPipeGeometryCfg
        cfg.merge(BeamPipeGeometryCfg(flags))

    # Inner Detectors
    if flags.Detector.GeometryPixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        cfg.merge(PixelReadoutGeometryCfg(flags))
    # TODO: do we need to set this separately?
    # if flags.Detector.GeometryBCM:

    if flags.Detector.GeometrySCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        cfg.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.GeometryTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        cfg.merge(TRT_ReadoutGeometryCfg(flags))

    # InDetServMat
    # Trigger the build of the InDetServMat geometry 
    # if any ID subsystems have been enabled
    if flags.Detector.GeometryID:
        from InDetServMatGeoModel.InDetServMatGeoModelConfig import (
             InDetServiceMaterialCfg)
        cfg.merge(InDetServiceMaterialCfg(flags))

    # Calorimeters
    if flags.Detector.GeometryLAr:
        from LArGeoAlgsNV.LArGMConfig import LArGMCfg
        cfg.merge(LArGMCfg(flags))

    if flags.Detector.GeometryTile:
        from TileGeoModel.TileGMConfig import TileGMCfg
        #flags.Tile.forceFullGeometry = True
        cfg.merge(TileGMCfg(flags))
        # We must set the "FULL" geometry explicitely, otherwise the "RECO" version will be used by default,
        # which is almost 'empty' (just the first level of child volumes is created for the "RECO" geo).
        cfg.getService("GeoModelSvc").DetectorTools["TileDetectorTool"].GeometryConfig="FULL"
    # TODO: do we need to set this separately?
    # if flags.Detector.GeometryMBTS:

    # Muon spectrometer
    if flags.Detector.GeometryMuon:
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
        cfg.merge(MuonGeoModelCfg(flags))

    # HGTD (defined only for Run4 geometry tags)
    if flags.Detector.GeometryHGTD:
        #set up geometry
        if flags.HGTD.Geometry.useGeoModelXml:
            from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg
        else:
            from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_SimulationGeometryCfg
        cfg.merge(HGTD_SimulationGeometryCfg(flags))
        
    # ITk (defined only for Run4 geometry tags)
    if flags.Detector.GeometryITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        cfg.merge(ITkPixelReadoutGeometryCfg(flags))
    if flags.Detector.GeometryITkStrip:
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        cfg.merge(ITkStripReadoutGeometryCfg(flags))
    # TODO: do we need to set those separately?
    # if flags.Detector.GeometryBCMPrime:
    # if flags.Detector.GeometryPLR:

    # Cavern (disabled by default)
    if flags.Detector.GeometryCavern:
        from AtlasGeoModel.CavernGMConfig import CavernGeometryCfg
        cfg.merge(CavernGeometryCfg(flags))
    
    # Forward detectors (disabled by default)
    if flags.Detector.GeometryLucid or flags.Detector.GeometryALFA or flags.Detector.GeometryAFP or flags.Detector.GeometryFwdRegion :
        from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
        cfg.merge(ForDetGeometryCfg(flags))
    if flags.Detector.GeometryZDC:
        from ZDC_GeoM.ZdcGeoModelConfig import ZDC_DetToolCfg
        cfg.merge(ZDC_DetToolCfg(flags))

    # Temporary 'hack': 
    # Replace EllipticTube with Box, 
    # to bypass a crash due to lack of support 
    # for EllipticTube in GeoModelIO 
    # See: https://its.cern.ch/jira/browse/ATLASSIM-7263
    if "ForwardRegionGeoModelTool" in cfg.getService("GeoModelSvc").DetectorTools:
        cfg.getService("GeoModelSvc").DetectorTools["ForwardRegionGeoModelTool"].vp1Compatibility=True


def getATLASVersion():
    if "AtlasVersion" in os.environ:
        return os.environ["AtlasVersion"]
    if "AtlasBaseVersion" in os.environ:
        return os.environ["AtlasBaseVersion"]
    return "Unknown"

def DumpGeoCfg(flags, name="DumpGeoCA", **kwargs):
    result = ComponentAccumulator()

    _logger.info("We're using these 'GeoModel.DumpGeo' configuration flags:")
    flags.dump("GeoModel.DumpGeo")

    # Debug messages
    _logger.debug("kwargs: %s", kwargs)
    
    # set additional DumpGeo Alg's properties
    _logger.verbose("Using ATLAS/Athena version: %s", getATLASVersion())
    _logger.verbose("Using GeoModel ATLAS version: %s", flags.GeoModel.AtlasVersion)
    kwargs.setdefault("AtlasRelease", getATLASVersion())
    kwargs.setdefault("AtlasVersion", flags.GeoModel.AtlasVersion)
    
    # Set the user's choice to see the content of the Treetops
    if flags.GeoModel.DumpGeo.ShowTreetopContent:
        kwargs.setdefault("ShowTreetopContent", True)

    # Set the name of the output '.db' file.
    # Pick the custom name if the user set it;
    # otherwise, build it from the geometry tag
    # and the filtered Detector Managers (if any).
    outFileName = ""
    if flags.GeoModel.DumpGeo.OutputFileName:
        outFileName = flags.GeoModel.DumpGeo.OutputFileName
    else:
        # Handle the user's inputs and create a file name 
        # for the output SQLite, accordingly
        outFileName = "geometry"
        filterDetManagers = []
        # - Put Geometry TAG into the file name
        # NOTE: at this point, the user-defined Geo TAG args.detDescr, 
        #       if set, has already replaced the default TAG in 'flags';
        #       so, we can use the latter, directy.
        geoTAG = flags.GeoModel.AtlasVersion
        _logger.info("+++ Dumping this Detector Description geometry TAG: '%s'", geoTAG)
        outFileName = outFileName + "-" + geoTAG
        
        if flags.GeoModel.DumpGeo.FilterDetManagers:
            
            _logger.info("+++ Filtering on these GeoModel 'Detector Managers': '%s'", flags.GeoModel.DumpGeo.FilterDetManagers)

            filterDetManagers = flags.GeoModel.DumpGeo.FilterDetManagers
            
            # Set the filter variable that is used in the C++ code
            kwargs.setdefault("UserFilterDetManager", filterDetManagers)
            
            # - Put the filtered Detector Managers' names into the file name, 
            #   if the user asked to filter on them
            outFileName = outFileName + "-" + "-".join(filterDetManagers)

        # - Add the final extension to the name of the output SQLite file 
        outFileName = outFileName + ".db"

    # Set the output file name variable in the C++ code
    kwargs.setdefault("OutSQLiteFileName", outFileName)

    # Check if the output SQLite file exists already, 
    # and overwrite it if the user asked to do so; 
    # otherwise, throw an error.
    if os.path.exists(outFileName):
        if flags.GeoModel.DumpGeo.ForceOverwrite:
            print("+ DumpGeo -- NOTE -- You chose to overwrite an existing geometry dump file with the same name, if present.")
            # os.environ["DUMPGEOFORCEOVERWRITE"] = "1" # save to an env var, for later use in GeoModelStandalone/GeoExporter
            # Check if the file exists before attempting to delete it   
            if os.path.exists(outFileName):
                os.remove(outFileName)
                _logger.verbose(f"The file {outFileName} has been deleted.")
            else:
                _logger.verbose(f"The file {outFileName} does not exist. So, it was not needed to 'force-delete' it. Continuing...")
        else:
            _logger.error(f"+++ DumpGeo -- ERROR!! The ouput file '{outFileName}' exists already!\nPlease move or remove it, or use the 'force' option: '-f' or '--forceOverWrite'.\n\n")
            sys.exit()

    # Schedule the DumpGeo Athena Algorithm
    the_alg = CompFactory.DumpGeo(name="DumpGeoAlg", **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result


if __name__=="__main__":
    # Run with e.g. python -m DumpGeo.DumpGeoConfig --detDescr=<ATLAS-geometry-tag> --filterDetManagers=[<list of tree tops>]
    
    from AthenaConfiguration.TestDefaults import defaultGeometryTags

    # ++++ Firstly we setup flags ++++
    # +++ Set the Athena Flags
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
    # ^ DumpGeo will not work with the scheduler, since its condition/data dependencies are not known in advance
    # More in details: the scheduler needs to know BEFORE the event, what the dependencies of each Alg are. 
    # So for DumpGeo, no dependencies are declared, which means the conditions data is not there. 
    # So when I load tracks, the geometry is missing and it crashes. 
    # Turning off the scheduler (with NumThreads=0) fixes this.

    # +++ Set custom CLI parameters for DumpGeo when ran as a standalone program
    # (that is, from the command line, 
    # and not as part of an Athena Transform or job)
    parser = flags.getArgumentParser(description="Dump the detector geometry to a GeoModel-based SQLite '.db' file.")
    parser.prog = 'dump-geo'
    # here we extend the parser with CLI options specific to DumpGeo
    parser.add_argument("--detDescr", default=defaultGeometryTags.RUN3,
                        help="The ATLAS geometry tag you want to dump (a convenience alias for the Athena flag 'GeoModel.AtlasVersion=TAG')", metavar="TAG")
    parser.add_argument("--outFilename", default="",
                        help="Here you can set a custom name for the output '.db' file. It will replace the name that is built with the geometry tag and the list of fileterd Detector Managers, if any.", metavar="FILENAME")
    # parser.add_argument("--filterTreeTops", help="Only output the GeoModel Tree Tops specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("--filterDetManagers", help="Only output the GeoModel Detector Managers specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("-f", "--forceOverwrite",
                        help="Force to overwrite an existing SQLite output file with the same name, if any", action = 'store_true')
    parser.add_argument("--showTreetopContent",
                        help="Show the content of the Treetops --- (by default, only the list of Treetops is shown)", action = 'store_true')
    parser.add_argument("--debugCA", help="Debug the CA configuration: print flags, tools, ... --- mainly, for DumpGeo developers. '1' prints a subset of the CA flags, '2' prints all of them.")

    args = flags.fillFromArgs(parser=parser)

    if args.help:
        # No point doing more here, since we just want to print the help.
        sys.exit()

    # +++ Get CLI parameters and set the corresponding configuration flags
    # Get the user's custom file name, if set;
    # this will replace the filename computed
    # from the geometry tag and the filtered volumes
    if args.outFilename:
        flags.GeoModel.DumpGeo.OutputFileName = args.outFilename
    if args.showTreetopContent:
        flags.GeoModel.DumpGeo.ShowTreetopContent = True
    if args.forceOverwrite:
        flags.GeoModel.DumpGeo.ForceOverwrite = True


    # +++ Set the empty input
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


    # +++ Set the detector geometry
    _logger.verbose("+ About to set the detector flags")
    # So we can now set up the geometry flags from the input
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, None, use_metadata=not dumpgeo_empty_input,
                       toggle_geometry=True, keep_beampipe=True)
    _logger.verbose("+ ... Done")

    if args.detDescr:
        _logger.verbose("+ About to set this detector description tag: '%s'" % args.detDescr)
        flags.GeoModel.AtlasVersion = args.detDescr
        _logger.verbose("+ ... Done")

    # finalize setting flags: lock them.
    flags.lock()

    # ++++ Now we setup the actual configuration ++++
    _logger.verbose("+ Setup main services")
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    _logger.verbose("+ ...Done")

    _logger.verbose("+ About to setup geometry")
    configureGeometry(flags,cfg)
    _logger.verbose("+ ...Done")

    # debug messages
    if args.debugCA:
        debugCAlevel = int(args.debugCA)
        if debugCAlevel >= 1:
            _logger.verbose("Debug --- printing flags...")
            print("\nflags:", flags)
            for fl in flags:
                print("fl:", fl)
            print("\nflags.Tile:", flags.Tile)
            for fl in flags.Tile:
                print("fl.Tile:", fl)
            print(dir(cfg))
            print("cfg._privateTools: ", cfg._privateTools)
            print("cfg._publicTools: ", cfg._publicTools)
        if debugCAlevel >= 2:
            flags.dump()
            flags._loadDynaFlags('GeoModel')
            flags._loadDynaFlags('Detector')
            flags.dump('Detector.(Geometry|Enable)', True)
        if debugCAlevel >= 1:
            _logger.verbose("We're in a debugCA session, flags have been printed out, now exiting...")
            sys.exit()
    
    # +++ Configure DumpGeo and run
    cfg.merge(DumpGeoCfg(flags))
    cfg.run()

