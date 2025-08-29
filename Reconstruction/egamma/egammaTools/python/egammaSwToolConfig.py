# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Configure cluster correction """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
import GaudiKernel.GaudiHandles as GaudiHandles
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def _configureClusterCorrections(flags, swTool):
    "Add attributes ClusterCorrectionToolsXX to egammaSwTool object"

    acc = ComponentAccumulator()
    from CaloClusterCorrection.CaloSwCorrections import (
        make_CaloSwCorrectionsCfg)

    clusterTypes = dict(
        Ele35='ele35', Ele55='ele55', Ele37='ele37',
        Gam35='gam35_unconv', Gam55='gam55_unconv', Gam37='gam37_unconv',
        Econv35='gam35_conv', Econv55='gam55_conv', Econv37='gam37_conv'
    )

    version = flags.Egamma.Calib.SuperClusterCorrectionVersion
    suffix = 'EGSuperCluster'
    attrPref = 'ClusterCorrectionToolsSuperCluster'

    for attrName, clName in clusterTypes.items():
        attrName = attrPref + attrName
        if not hasattr(swTool, attrName):
            continue
        toolsAcc = make_CaloSwCorrectionsCfg(
            flags, clName, suffix=suffix,
            version=version,
            cells_name=flags.Egamma.Keys.Input.CaloCells)
        tools = acc.popToolsAndMerge(toolsAcc)
        setattr(swTool, attrName, GaudiHandles.PrivateToolHandleArray(tools))

    return acc


def egammaSwToolCfg(flags, name='egammaSwTool', **kwargs):

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    acc = ComponentAccumulator()

    egswtool = CompFactory.egammaSwTool(name, **kwargs)

    # For now, the correction is called, but this might change.
    utilsAcc = _configureClusterCorrections(flags, egswtool)
    acc.merge(utilsAcc)

    acc.setPrivateTools(egswtool)
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
    mlog = logging.getLogger("egammaSwToolConfigTest")
    mlog.info("Configuring  egammaSwTool: ")
    printProperties(mlog, cfg.popToolsAndMerge(
        egammaSwToolCfg(flags)),
        nestLevel=1,
        printDefaults=True)

    f = open("egammaswtool.pkl", "wb")
    cfg.store(f)
    f.close()
