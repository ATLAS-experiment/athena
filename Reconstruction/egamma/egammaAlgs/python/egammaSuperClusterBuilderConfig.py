# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """Instantiate the two supercluster
builders with default configuration"""

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

from egammaTools.EMTrackMatchBuilderConfig import EMTrackMatchBuilderCfg
from egammaTools.EMConversionBuilderConfig import EMConversionBuilderCfg
from egammaTools.egammaSwToolConfig import egammaSwToolCfg
from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg

def electronSuperClusterBuilderCfg(flags,
                                   name='electronSuperClusterBuilder',
                                   sequenceName=None,
                                   **kwargs):

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    seqkw = {'sequence': sequenceName} if sequenceName else {}
    acc = ComponentAccumulator(**seqkw)

    kwargs.setdefault("doTrackMatching", flags.Egamma.doTracking)
    kwargs.setdefault("UseExtendedTG3", flags.GeoModel.Run is LHCPeriod.Run3)
    kwargs.setdefault("EtThresholdCut",
                      1000 if not flags.Egamma.doLowMu else 300)
    kwargs.setdefault(
        "ClusterCorrectionTool", 
        acc.popToolsAndMerge(egammaSwToolCfg(flags)))
    kwargs.setdefault(
        "MVACalibSvc", 
        acc.getPrimaryAndMerge(egammaMVASvcCfg(flags)))
    kwargs.setdefault(
        "egammaCheckEnergyDepositTool",
        CompFactory.egammaCheckEnergyDepositTool())
    kwargs.setdefault(
        "TrackMatchBuilderTool",
        acc.popToolsAndMerge(EMTrackMatchBuilderCfg(flags)))
    kwargs.setdefault(
        "InputEgammaRecContainerName",
        flags.Egamma.Keys.Internal.EgammaRecs)
    kwargs.setdefault(
        "OutputEgammaRecContainerKey",
        flags.Egamma.Keys.Internal.ElectronSuperRecs)
    kwargs.setdefault(
        "SuperClusterCollectionName",
        flags.Egamma.Keys.Internal.ElectronSuperClusters)

    acc.addEventAlgo(CompFactory.electronSuperClusterBuilder(name, **kwargs))
    return acc


def photonSuperClusterBuilderCfg(
        flags,
        name='photonSuperClusterBuilder',
        sequenceName=None,
        **kwargs):

    seqkw = {'sequence': sequenceName} if sequenceName else {}
    acc = ComponentAccumulator(**seqkw)

    kwargs.setdefault("doConversions", flags.Egamma.doConversionBuilding)
    kwargs.setdefault("UseExtendedTG3", flags.GeoModel.Run is LHCPeriod.Run3)
    kwargs.setdefault(
        "EtThresholdCut",
        1500 if not flags.Egamma.doLowMu else 300)
    kwargs.setdefault(
        "ClusterCorrectionTool", 
        acc.popToolsAndMerge(egammaSwToolCfg(flags)))
    kwargs.setdefault(
        "MVACalibSvc", 
        acc.getPrimaryAndMerge(egammaMVASvcCfg(flags)))
    kwargs.setdefault(
        "egammaCheckEnergyDepositTool",
        CompFactory.egammaCheckEnergyDepositTool())
    kwargs.setdefault(
        "ConversionBuilderTool",
        acc.popToolsAndMerge(EMConversionBuilderCfg(flags)))
    kwargs.setdefault(
        "InputEgammaRecContainerName",
        flags.Egamma.Keys.Internal.EgammaRecs)
    kwargs.setdefault(
        "OutputEgammaRecContainerKey",
        flags.Egamma.Keys.Internal.PhotonSuperRecs)
    kwargs.setdefault(
        "SuperClusterCollectionName",
        flags.Egamma.Keys.Internal.PhotonSuperClusters)

    acc.addEventAlgo(CompFactory.photonSuperClusterBuilder(name, **kwargs))
  
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(electronSuperClusterBuilderCfg(flags))
    mlog = logging.getLogger("egammaSuperClusterBuilderConfigTest")
    mlog.info("Configuring  electronSuperClusterBuilder: ")
    printProperties(mlog,
                    acc.getEventAlgo("electronSuperClusterBuilder"),
                    nestLevel=1,
                    printDefaults=True)
    acc.merge(photonSuperClusterBuilderCfg(flags))
    mlog.info("Configuring  photonSuperClusterBuilder: ")
    printProperties(mlog,
                    acc.getEventAlgo("photonSuperClusterBuilder"),
                    nestLevel=1,
                    printDefaults=True)

    with open("egammasuperclusterbuilder.pkl", "wb") as f:
        acc.store(f)
