# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Configure egammaLargeClusterMaker which chooses cells to store in the AOD"""

__author__ = "Jovan Mitrevski"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaLargeClusterMakerCfg(flags, name="egammaLCMakerTool",  **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("CellsName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("InputClusterCollection",
                      flags.Egamma.Keys.Output.CaloClusters)
    acc.setPrivateTools(CompFactory.egammaLargeClusterMaker(name, **kwargs))

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.fillFromArgs()
    flags.lock()
    flags.dump()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("egammaLargeClusterMakerConfigTest")
    mlog.info("Configuring  egammaLargeClusterMaker: ")
    printProperties(mlog, cfg.popToolsAndMerge(
        egammaLargeClusterMakerCfg(flags)),
        nestLevel=1,
        printDefaults=True)

    f = open("egammalargeclustermaker.pkl", "wb")
    cfg.store(f)
    f.close()
