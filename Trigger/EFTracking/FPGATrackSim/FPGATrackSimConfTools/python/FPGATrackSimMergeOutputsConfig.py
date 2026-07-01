# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger

log = AthenaLogger(__name__)
import glob

def FPGATrackSimMergeOutputsAlgCfg(flags,**kwargs):
    acc=ComponentAccumulator()
    flags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=False)
    files = glob.glob(flags.Trigger.FPGATrackSim.FPGATrackSimTestFiles)
    print("FPGATrackSimMergeOutputsConfig looked for files =",flags.Trigger.FPGATrackSim.FPGATrackSimTestFiles," and found", files)
    flags.lock()
    kwargs.setdefault('InFileNames', files) 

    from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimOverlapRemovalToolCfg
    MergeOutputsAlg = CompFactory.FPGATrackSimMergeOutputsAlg(name = 'FPGAMergeOutputsAlg', **kwargs,
                                                            OverlapRemoval = acc.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags)))
    MergeOutputsAlg.OverlapRemoval.MinChi2 = 1e15 ## disable here
    MergeOutputsAlg.SkipEvents = flags.Exec.SkipEvents
    MergeOutputsAlg.SortTracks = flags.Trigger.FPGATrackSim.SortTracks
    acc.addEventAlgo(MergeOutputsAlg)

    return acc
