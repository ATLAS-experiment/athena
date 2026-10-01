#!/usr/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ObjTrackVisualizationAlgCfg(flags, name="ObjTrackVisualizationAlg", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", result.popToolsAndMerge(ActsExtrapolationToolCfg(flags)))
    the_alg = CompFactory.ActsTrk.ObjTrackVisualizationAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result    

def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_14TeV/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8481_s4383_r15934/AOD.41490164._005514.pool.root.1"], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--outDir", help="Output directory", default ="objDebug/")
    parser.add_argument("--nEvents", help="Number of events to run", type = int ,default = 1)
    parser.add_argument("--skipEvents", help="Number of events to skip", type = int, default = 0)
    return parser


if __name__ == "__main__":
    args = setupArgParser().parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Exec.MaxEvents = args.nEvents
    flags.Exec.SkipEvents = args.skipEvents

    ### Assemble all files in a directory or all files not having the suffix txt conf. 
    ### The latter are interpreted as file lists
    from MuonConfig.MuonConfigUtils import prepareInput
    prepareInput(flags, args.inputFile)

    from MuonGeoModelTestR4.testGeoModel import setupServicesCfg

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True
    flags.Acts.TrackingGeometry.UseBlueprint = True

    from MuonConfig.MuonConfigUtils import configureDefaultTags
    configureDefaultTags(flags)


    flags.lock()
    flags.dump(evaluate = True)


    cfg = setupServicesCfg(flags)
    cfg.merge(ObjTrackVisualizationAlgCfg(flags, outPath = args.outDir))

    from MuonConfig.MuonConfigUtils import executeTest
    executeTest(cfg)





