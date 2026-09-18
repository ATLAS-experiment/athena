# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
from AthenaConfiguration.ComponentAccumulator import (
    ComponentAccumulator,
    ConfigurationError,
)
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
        # We must set the "FULL" geometry explicitly, otherwise the "RECO" version will be used by default,
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


def dumpGeoOutputFileName(flags):
    """Return the configured DumpGeo SQLite output file name."""
    if flags.GeoModel.DumpGeo.OutputFileName:
        return flags.GeoModel.DumpGeo.OutputFileName

    output_file = f"geometry-{flags.GeoModel.AtlasVersion}"
    filter_det_managers = flags.GeoModel.DumpGeo.FilterDetManagers
    if filter_det_managers:
        output_file += "-" + "-".join(filter_det_managers)

    return output_file + ".db"


def resolveDumpGeoGeometryTag(det_descr, configured_tag, fallback_tag):
    """Resolve the geometry tag using the DumpGeo command-line precedence."""
    if det_descr:
        return det_descr
    if configured_tag:
        return configured_tag
    return fallback_tag


def dumpGeoHasInputFiles(input_files):
    """Return whether standalone DumpGeo was given real input files."""
    return bool(input_files) and input_files != [
        "_ATHENA_GENERIC_INPUTFILE_NAME_"
    ]


def configureDumpGeoInputFlags(flags):
    """Configure deterministic defaults for an input-less standalone job."""
    if dumpGeoHasInputFiles(flags.Input.Files):
        return False

    from Campaigns.Utils import Campaign
    from AthenaConfiguration.TestDefaults import defaultConditionsTags

    flags.Input.Files = []

    # MainServicesCfg and detector configuration require these normally
    # metadata-derived flags to be initialized for an input-less job.
    flags.Input.ProjectName = "mc23_13p6TeV"
    flags.Input.RunNumbers = [330000]
    flags.Input.TimeStamps = [1]
    flags.Input.TypedCollections = []
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
    flags.Input.isMC = True
    flags.Input.MCCampaign = Campaign.Unknown
    return True


def validateDumpGeoOutputFile(output_file, force_overwrite):
    """Reject an existing output file unless overwrite is enabled."""
    if os.path.exists(output_file) and not force_overwrite:
        raise ConfigurationError(
            f"DumpGeo output file '{output_file}' already exists. "
            "Move or remove it, or enable "
            "'GeoModel.DumpGeo.ForceOverwrite' "
            "(or use the '-f' option from the command line)."
        )


def logZDCFailureReminder(zdc_enabled, run_succeeded):
    """Remind standalone users about ZDC diagnostics after a failed run."""
    if zdc_enabled and not run_succeeded:
        _logger.error(
            "DumpGeo failed while ZDC geometry was enabled. Check the "
            "preceding ZDC_DetTool messages: the selected geometry tag "
            "may not contain ZDC geometry information."
        )


def DumpGeoCfg(flags, name="DumpGeoAlg", **kwargs):
    if _logger.isEnabledFor(logging.DEBUG):
        _logger.debug(
            "Dumping the 'GeoModel.DumpGeo' configuration flags:"
        )
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

    # Configure DetectorManager filtering independently of the output file
    # name. In particular, a user-defined file name must not disable the
    # filter passed to the C++ algorithm.
    filterDetManagers = flags.GeoModel.DumpGeo.FilterDetManagers
    if filterDetManagers:
        _logger.info(
            "+++ Filtering on these GeoModel 'Detector Managers': '%s'",
            filterDetManagers,
        )
        kwargs.setdefault("UserFilterDetManager", filterDetManagers)

    # Set the name of the output '.db' file.
    configuredOutFileName = dumpGeoOutputFileName(flags)
    if not flags.GeoModel.DumpGeo.OutputFileName:
        _logger.info(
            "+++ Dumping this Detector Description geometry TAG: '%s'",
            flags.GeoModel.AtlasVersion,
        )

    # Set the output file name variable in the C++ code
    kwargs.setdefault("OutSQLiteFileName", configuredOutFileName)

    # The final algorithm property is authoritative. A caller can override the
    # flag-derived filename through kwargs, so validation and deletion must use
    # the same path that the C++ algorithm will write.
    outFileName = kwargs["OutSQLiteFileName"]
    kwargs.setdefault(
        "ForceOverwrite",
        flags.GeoModel.DumpGeo.ForceOverwrite,
    )
    forceOverwrite = kwargs["ForceOverwrite"]

    # Reject an existing file unless overwrite is enabled. Destructive removal
    # is deliberately deferred to DumpGeo::initialize(), immediately before
    # the C++ algorithm opens the output database.
    validateDumpGeoOutputFile(
        outFileName,
        forceOverwrite,
    )

    # Schedule the DumpGeo Athena Algorithm
    result = ComponentAccumulator()
    the_alg = CompFactory.DumpGeo(name=name, **kwargs)
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
    parser.add_argument(
        "--detDescr",
        default=None,
        help=(
            "Override the ATLAS geometry tag. This is a convenience alias "
            "for 'GeoModel.AtlasVersion=TAG'. If omitted, the generic flag "
            "or input-file metadata is used."
        ),
        metavar="TAG",
    )
    parser.add_argument("--outFilename", default="",
                        help="Here you can set a custom name for the output '.db' file. It will replace the name that is built with the geometry tag and the list of filtered Detector Managers, if any.", metavar="FILENAME")
    # parser.add_argument("--filterTreeTops", help="Only output the GeoModel Tree Tops specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("--filterDetManagers", help="Only output the GeoModel Detector Managers specified in the FILTER list; input is a comma-separated list")
    parser.add_argument("-f", "--forceOverwrite",
                        help="Force to overwrite an existing SQLite output file with the same name, if any", action = 'store_true')
    parser.add_argument("--showTreetopContent",
                        help="Show the content of the Treetops --- (by default, only the list of Treetops is shown)", action = 'store_true')
    parser.add_argument("--debugCA", help="Debug the CA configuration: print flags, tools, ... --- mainly, for DumpGeo developers. '1' prints a subset of the CA flags, '2' prints all of them.")

    args = flags.fillFromArgs(parser=parser)

    # +++ Get CLI parameters and set the corresponding configuration flags
    # Get the user's custom file name, if set;
    # this will replace the filename computed
    # from the geometry tag and the filtered volumes
    if args.outFilename:
        flags.GeoModel.DumpGeo.OutputFileName = args.outFilename
    if args.filterDetManagers:
        flags.GeoModel.DumpGeo.FilterDetManagers = [
            manager.strip()
            for manager in args.filterDetManagers.split(",")
            if manager.strip()
        ]
    if args.showTreetopContent:
        flags.GeoModel.DumpGeo.ShowTreetopContent = True
    if args.forceOverwrite:
        flags.GeoModel.DumpGeo.ForceOverwrite = True

    # Athena uses a placeholder as the default Input.Files value. Treat that
    # placeholder, as well as an explicitly empty list, as a genuine
    # input-less job. In particular, do not replace it with a test EVNT file:
    # standalone geometry dumping must not depend on CVMFS or on unrelated
    # event-file metadata.
    dumpgeo_empty_input = configureDumpGeoInputFlags(flags)

    # A custom filename is already final and can be checked without resolving
    # the geometry tag, which may otherwise require a metadata lookup.
    if flags.GeoModel.DumpGeo.OutputFileName:
        validateDumpGeoOutputFile(
            flags.GeoModel.DumpGeo.OutputFileName,
            flags.GeoModel.DumpGeo.ForceOverwrite,
        )

    # Resolve the geometry tag with this precedence: an explicit --detDescr,
    # the generic GeoModel.AtlasVersion flag (including its metadata-derived
    # value), then the standalone Run-3 fallback. Avoid evaluating the generic
    # flag when --detDescr was supplied, since doing so may trigger an
    # unnecessary metadata lookup.
    configured_geometry_tag = None
    if not args.detDescr:
        configured_geometry_tag = flags.GeoModel.AtlasVersion
    geometry_tag = resolveDumpGeoGeometryTag(
        args.detDescr,
        configured_geometry_tag,
        defaultGeometryTags.RUN3,
    )
    if args.detDescr:
        _logger.verbose(
            "+ About to set this detector description tag: '%s'",
            args.detDescr,
        )
    flags.GeoModel.AtlasVersion = geometry_tag
    _logger.verbose("+ Using detector description tag: '%s'", geometry_tag)
    ### Setup the most recent conditions tag
    from MuonConfig.MuonConfigUtils import configureCondTag
    configureCondTag(flags)

    # Fail as soon as the final geometry tag is known and before detector or
    # geometry configuration. Resolving a metadata-derived tag necessarily
    # performs the metadata lookup first. This check deliberately does not
    # delete an existing file when force overwrite is enabled; deletion is
    # deferred to the C++ algorithm's initialize() method.
    validateDumpGeoOutputFile(
        dumpGeoOutputFileName(flags),
        flags.GeoModel.DumpGeo.ForceOverwrite,
    )

    # +++ Set the empty input
    _logger.verbose("+ About to set flags related to the input")

    # Empty input is not normal for Athena, so we will need to check 
    # this repeatedly below (the same as with VP1)
    dumpgeo_empty_input = False

    # This covers the use case where we launch DumpGeo
    # without input files; e.g., to check the detector description
    from AthenaConfiguration.AutoConfigFlags import GetFileMD

    input_metadata = GetFileMD(flags.Input.Files)
    input_geometry_tag = input_metadata.get("GeoAtlas", None)

    # Treat both None and an empty string as a missing geometry tag.
    dumpgeo_empty_input = (
        len(flags.Input.Files) == 0 or not input_geometry_tag
    )

    if dumpgeo_empty_input:
        from Campaigns.Utils import Campaign
        from AthenaConfiguration.TestDefaults import (
            defaultConditionsTags,
            defaultGeometryTags
        )

        # NB Must set e.g. ConfigFlags.Input.Runparse_args() Number and
        # ConfigFlags.Input.TimeStamp before calling the 
        # MainServicesCfg to avoid it attempting auto-configuration 
        # from an input file, which is empty in this use case.
        # If you don't have it, it (and/or other Cfg routines) complains and crashes. 
        # See also: 
        # https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/InnerDetector/InDetConditions/SCT_ConditionsAlgorithms/python/SCT_DCSConditionsTestAlgConfig.py#0023
        flags.Input.ProjectName = "mc23_13p6TeV"
        flags.Input.RunNumbers = [330000]  
        flags.Input.TimeStamps = [1]  
        flags.Input.TypedCollections = []

        # set default CondDB and Geometry version
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        flags.Input.isMC = True
        flags.Input.MCCampaign = Campaign.Unknown

    _logger.verbose("+ ... Done")
    _logger.verbose("+ empty input: '%s'", dumpgeo_empty_input)


    # +++ Set the detector geometry
    _logger.verbose("+ About to set the detector flags")
    # So we can now set up the geometry flags from the input
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(
        flags,
        None,
        use_metadata=not dumpgeo_empty_input,
        toggle_geometry=True,
        keep_beampipe=True
    )
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
    status = cfg.run()

    logZDCFailureReminder(
        flags.Detector.GeometryZDC,
        status.isSuccess(),
    )

    sys.exit(not status.isSuccess())
