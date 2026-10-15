# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CaloClusterTritonToolCfg(flags, name="CaloClusterTritonTool", **kwargs):
    """Configure CaloClusterTritonTool with its AthInfer::TritonTool backend."""
    acc = ComponentAccumulator()

    from AthTritonComps.TritonToolConfig import TritonToolCfg
    kwargs.setdefault("TritonInferenceTool",
                      acc.popToolsAndMerge(TritonToolCfg(flags,
                                                         model_name=flags.Calo.Triton.name,
                                                         url=flags.Calo.Triton.url,
                                                         port=flags.Calo.Triton.port)))
    kwargs.setdefault("CellEnergyThreshold", 0.005)  # GeV

    acc.setPrivateTools(CompFactory.CaloClusterTritonTool(name, **kwargs))
    return acc


def CaloClusterTritonAlgCfg(flags,
                            clusterContainer="CaloCalTopoClusters",
                            decorName="GNNCalibratedEnergy",
                            name="CaloClusterTritonAlg",
                            **kwargs):
    """Schedule CaloClusterTritonAlg, which decorates clusters with the GNN energy [MeV]."""
    acc = ComponentAccumulator()

    kwargs.setdefault("CaloClusterTritonTool",
                      acc.popToolsAndMerge(CaloClusterTritonToolCfg(flags)))
    kwargs.setdefault("ClusterContainer", clusterContainer)
    kwargs.setdefault("ClusterGNNCalibratedEnergyKeyName", f"{clusterContainer}.{decorName}")
    kwargs.setdefault("RapidityRange", [-2.5, 2.5])
    kwargs.setdefault("MinClusterEnergy", 300.0)  # MeV

    acc.addEventAlgo(CompFactory.CaloClusterTritonAlg(name, **kwargs))
    return acc



if __name__ == "__main__":
    import argparse
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    parser = argparse.ArgumentParser()
    parser.add_argument("--input", nargs="+", required=True, help="Input ESD/AOD file(s) with cell links")
    parser.add_argument("--model", required=True, help="Triton model name")
    parser.add_argument("--url", default="localhost", help="Triton server URL")
    parser.add_argument("--port", type=int, default=8001, help="Triton gRPC port")
    parser.add_argument("--maxEvents", type=int, default=10)
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = args.input
    flags.Exec.MaxEvents = args.maxEvents
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # Cell links require the calorimeter geometry and cell container to be readable
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(LArGMCfg(flags))
    cfg.merge(TileGMCfg(flags))

    cfg.merge(CaloClusterTritonAlgCfg(flags,
                                      model_name=args.model,
                                      url=args.url,
                                      port=args.port))

    import sys
    sys.exit(cfg.run().isFailure())