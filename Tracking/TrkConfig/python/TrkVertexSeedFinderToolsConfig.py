# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkVertexSeedFinderTools package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DummySeedFinderCfg(flags, name="DummySeedFinder", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.DummySeedFinder(name, **kwargs))
    return acc

def TrackDensitySeedFinderCfg(flags, name="TrackDensitySeedFinder", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.TrackDensitySeedFinder(name, **kwargs))
    return acc

def MCTrueSeedFinderCfg(flags, name="MCTrueSeedFinder", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.Trk.MCTrueSeedFinder(name, **kwargs))
    return acc

def ZScanSeedFinderCfg(flags, name="ZScanSeedFinder", **kwargs):
    acc = ComponentAccumulator()

    if "IPEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            TrackToVertexIPEstimatorCfg)
        kwargs.setdefault("IPEstimator", acc.popToolsAndMerge(
            TrackToVertexIPEstimatorCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.ZScanSeedFinder(name, **kwargs))
    return acc

def CrossDistancesSeedFinderCfg(flags, name="CrossDistancesSeedFinder", **kwargs):
    acc = ComponentAccumulator()

    if "TrkDistanceFinder" not in kwargs:
        from TrkConfig.TrkVertexSeedFinderUtilsConfig import (
            SeedNewtonTrkDistanceFinderCfg)
        kwargs.setdefault("TrkDistanceFinder", acc.popToolsAndMerge(
            SeedNewtonTrkDistanceFinderCfg(flags)))

    acc.setPrivateTools(CompFactory.Trk.CrossDistancesSeedFinder(name, **kwargs))
    return acc


def IndexedCrossDistancesSeedFinderCfg(
        flags, name='IndexedCrossDistancesSeedFinder', **kwargs):
  acc = ComponentAccumulator()

  if "Mode3dFinder" not in kwargs:
      from TrkConfig.TrkVertexSeedFinderUtilsConfig import (
          Mode3dFromFsmw1dFinderCfg)
      kwargs.setdefault("Mode3dFinder", acc.popToolsAndMerge(
          Mode3dFromFsmw1dFinderCfg(flags)))

  if "TrkDistanceFinder" not in kwargs:
      from TrkConfig.TrkVertexSeedFinderUtilsConfig import (
          SeedNewtonTrkDistanceFinderCfg)
      kwargs.setdefault("TrkDistanceFinder", acc.popToolsAndMerge(
          SeedNewtonTrkDistanceFinderCfg(flags)))

  kwargs.setdefault("trackdistcutoff", 0.01)
  kwargs.setdefault("maximumTracksNoCut", 30)
  kwargs.setdefault("maximumDistanceCut", 7.5)

  acc.setPrivateTools(
      CompFactory.Trk.IndexedCrossDistancesSeedFinder(name, **kwargs))
  return acc


if __name__ == "__main__":

    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("--finder")
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    alg = CompFactory.Trk.VertexSeedFinderTestAlg

    if args.finder=="DummySeedFinder":
        finder = cfg.popToolsAndMerge(DummySeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             Expected1 = [0, 0, 0],
                             Expected2 = [0, 0, 0],
                             Expected3 = [0, 0, 0]))

    elif args.finder=="ZScanSeedFinder":
        finder = cfg.popToolsAndMerge(ZScanSeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             Expected1 = [  0,   0, -8.14159],
                             Expected2 = [1.7, 1.3, -7.82529],
                             Expected3 = [0, 0, 11.6246]))

    elif args.finder=="CrossDistancesSeedFinder":
        finder = cfg.popToolsAndMerge(CrossDistancesSeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             Expected1 = [8.58978, 8.67507, -6.4581],
                             Expected2 = [3.44328, 3.25688, -7.88419],
                             Expected3 = [1.23398, -1.18171, 11.9419]))

    elif args.finder=="IndexedCrossDistancesSeedFinder":
        finder = cfg.popToolsAndMerge(IndexedCrossDistancesSeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             PriVert = [0.3, 0.1],
                             Expected1 = [8.50042, 8.55733, -6.48532],
                             Expected2 = [3.44958, 3.26644, -7.06138],
                             Expected3 = [-0.155833, -1.84918, 12.5667],
                             Expected1PhiModes = [0.525193, 0.785398, 0.967545],
                             Expected1RModes = [3.61433, 12.4693, 7.05322],
                             Expected1ZModes = [-7.88629, -6.5, -5.61236],
                             Expected1Weights = [0.139025, 7.26331, 0.345227],
                             Expected1Indices = [0, 1, 2],
                             Expected1CorrDist = [-9.9, -9.9]))

    elif args.finder=="TrackDensitySeedFinder":
        finder = cfg.popToolsAndMerge(TrackDensitySeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             Expected1 = [0, 0, -7.01305],
                             Expected2 = [1.7, 1.3, -13.013],
                             Expected3 = [0, 0, 11.9761]))

    elif args.finder=="MCTrueSeedFinder":
        finder = cfg.popToolsAndMerge(MCTrueSeedFinderCfg(flags))
        cfg.addEventAlgo(alg(name='testalg1',
                             VertexSeedFinderTool = finder,
                             McEventCollectionKey = 'G4Truth',
                             Expected3 = [1, 2, 12,
                                          0.3, -0.7, -3,
                                          0.6, 0.2, 7]))

    cfg.run(2)
