# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def WriteHiveDataObjCfg(flags):
    """
    Configure and return a ComponentAccumulator with all Hive algorithms.
    """
    from AthExHive.AthExHiveConfig import (
        HiveAlgAConf,
        HiveAlgBConf,
        HiveAlgCConf,
        HiveAlgDConf,
        HiveAlgEConf,
        HiveAlgFConf,
        HiveAlgGConf,
        HiveAlgVConf,
    )

    # Merge algs into CA
    alg_configs = [
        HiveAlgAConf,
        HiveAlgBConf,
        HiveAlgCConf,
        HiveAlgDConf,
        HiveAlgEConf,
        HiveAlgFConf,
        HiveAlgGConf,
        HiveAlgVConf,
    ]

    cfg = ComponentAccumulator()
    for alg_conf in alg_configs:
        cfg.merge(alg_conf(flags))

    return cfg


if __name__ == "__main__":
    # Setup configuration flags
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.RunNumbers = [284500]
    flags.Input.TimeStamps = [1]  # dummy value
    # workaround for building xAOD::EventInfo without input files
    flags.Input.TypedCollections = []
    flags.Exec.MaxEvents = 20
    flags.fillFromArgs()
    flags.lock()

    # The example runs with no input file. We configure it with the McEventSelector
    cfg = MainEvgenServicesCfg(flags, withSequences=True)

    cfg.merge(WriteHiveDataObjCfg(flags))

    # Output stream
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

    cfg.merge(OutputStreamCfg(flags, "ExampleStream", ItemList=["HiveDataObj#*"]))

    # Execute
    import sys

    sys.exit(cfg.run().isFailure())
