#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
Run geantino processing for material track creation
"""


from AthenaCommon.Logging import log
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg


def setupArgParser():
    # Argument parsing
    from argparse import ArgumentParser
    parser = ArgumentParser("RunGeantinoMaterialTrackProduction.py")
    parser.add_argument("--detectors", nargs="+",
                        default=['ITkPixel', 'ITkStrip', 'Bpipe'],
                        help="Specify the list of detectors")
    parser.add_argument("--localgeo", default=False, action="store_true",
                        help="Use local geometry Xml files")
    parser.add_argument("--storeHITS", help="Store the G4 hits",
                        default = False, action = "store_true")
    parser.add_argument("--geoModelSqLiteFile", default = "", help="Read geometry from sqlite file")
    parser.add_argument("-S", "--verboseStoreGate", default=False,
                        action="store_true",
                        help="Dump the StoreGate(s) each event iteration")
    parser.add_argument("--maxEvents",default=-1, type = int,
                        help="The number of events to run. 0 skips execution")
    parser.add_argument("--skipEvents",default=0, type=int,
                        help="The number of events to skip")
    parser.add_argument("--threads", default=1, type=int, help="The number of threads to run")
    parser.add_argument("--geometrytag",default="ATLAS-P2-RUN4-03-00-00", type=str,
                        help="The geometry tag to use")
    parser.add_argument("--inputevntfile",
                        default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc15_14TeV.singlegeantino_E10GeV_etaFlatnp0_6.5M.evgen.EVNT.pool.root"], 
                        nargs="+",
                        help="The input EVNT file to use")
    parser.add_argument("--outputhitsfile",default="myHITS.pool.root", type=str,
                        help="The output HITS filename")
    parser.add_argument("--outputfile",default="material-tracks.root", type=str,
                        help="The output Geantino filename")
    return parser

if __name__ == "__main__":
    args = setupArgParser().parse_args()
    # Some info about the job
    print("----RunGeantinoMaterialTrackProduction for ITk geometry----")
    print()
    print("Using Geometry Tag: "+args.geometrytag)
    if args.localgeo:
        print("...overridden by local Geometry Xml files")
    if(args.geoModelSqLiteFile):
       print("... overridden by Geometry Sqlite file: "+args.geoModelSqLiteFile)
    print("Input EVNT Files ")
    for f in args.inputevntfile:
        print (f" --- {f}")
    
    print("Running with: {}".format(", ".join(args.detectors)))
    print()

    # Configure
    flags = initConfigFlags()
    if args.localgeo:
        flags.ITk.Geometry.AllLocal = True

    flags.Input.Files = args.inputevntfile
    flags.Output.HITSFileName = args.outputhitsfile
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Exec.MaxEvents = args.maxEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True
    from SimulationConfig.SimEnums import SimulationFlavour
    flags.Sim.ISF.Simulator = SimulationFlavour.AtlasG4


    flags.GeoModel.AtlasVersion = args.geometrytag
    flags.IOVDb.GlobalTag = "OFLCOND-SIM-00-00-00"
    flags.GeoModel.Align.Dynamic = False

    from AthenaConfiguration.DetectorConfigFlags import getEnabledDetectors, setupDetectorFlags
    from AthenaConfiguration.AutoConfigFlags import getDefaultDetectors


    if args.geoModelSqLiteFile:
         flags.GeoModel.SQLiteDB = True
         flags.GeoModel.SQLiteDBFullPath = args.geoModelSqLiteFile
         from MuonConfig.MuonConfigUtils import configureCondTag
         configureCondTag(flags)
         if "toroid" in args.detectors:
            flags.Detector.GeometryMDT = False
            flags.Detector.GeometryRPC = False
            flags.Detector.GeometryTGC = False
            flags.Detector.GeometrysTGC = False
            flags.Detector.GeometryMM = False
            flags.Detector.GeometryBpipe = False
            flags.Detector.SpecialGeometryToroid = True
    else:
        detectors = args.detectors
        detectors.append('Bpipe')  # always run with beam pipe
        setupDetectorFlags(flags, detectors, toggle_geometry=True)

    flags.Acts.TrackingGeometry.UseBlueprint = True


    log.debug('Lock config flags now.')
    flags.lock()
    print (" ***\n ".join(getEnabledDetectors(flags)))

    print(flags.dump(evaluate=True))

    # Construct our accumulator to run
    acc = MainServicesCfg(flags)

    if args.verboseStoreGate:
      acc.getService("StoreGateSvc").Dump = True

    log.debug('Dumping of ConfigFlags now.')
    flags.dump()

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # add BeamEffectsAlg
    from BeamEffects.BeamEffectsAlgConfig import BeamEffectsAlgCfg
    acc.merge(BeamEffectsAlgCfg(flags))

    beamcond = acc.getCondAlgo("BeamSpotCondAlg")

    beamcond.useDB=False
    beamcond.posX=0.0
    beamcond.posY=0.0
    beamcond.posZ=0.0
    beamcond.sigmaX=0.0
    beamcond.sigmaY=0.0
    beamcond.sigmaZ=0.0
    beamcond.tiltX=0.0
    beamcond.tiltY=0.0


    from ActsConfig.ActsMaterialConfig import MaterialTrackRecorderUserActionSvcCfg
    from G4AtlasAlg.G4AtlasAlgConfig import G4AtlasAlgCfg
    acc.merge(G4AtlasAlgCfg(flags, "ITkG4AtlasAlg",
                            ExtraOutputs=[("ActsTrk::RecordedMaterialTrackCollection","StoreGateSvc+OutputMaterialTracks")],
                            UserActionSvc = acc.getPrimaryAndMerge(MaterialTrackRecorderUserActionSvcCfg(flags))))
    if args.storeHITS:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        from SimuJobTransforms.SimOutputConfig import getStreamHITS_ItemList
        acc.merge(OutputStreamCfg(flags,"HITS", ItemList=getStreamHITS_ItemList(flags), 
                                  disableEventTag=True, AcceptAlgs=['ITkG4AtlasAlg']) )


    from ActsConfig.ActsMaterialConfig import MaterialTrackWriterCfg
    acc.merge(MaterialTrackWriterCfg(flags, useTrackingGeometry= False, FileName=args.outputfile))

    from MuonConfig.MuonConfigUtils import executeTest
    executeTest(acc)



