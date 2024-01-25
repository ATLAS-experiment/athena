# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID
from egammaTools.EMTrackMatchBuilderConfig import EMTrackMatchBuilderCfg
from ElectronPhotonSelectorTools.AsgForwardElectronIsEMSelectorsConfig import (
    AsgForwardElectronIsEMSelectorCfg)
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from egammaTools.egammaOQFlagsBuilderConfig import egammaOQFlagsBuilderCfg


def egammaForwardBuilderCfg(flags, name='egammaForwardElectron', **kwargs):

    mlog = logging.getLogger(name)
    mlog.info('Starting configuration')

    acc = ComponentAccumulator()

    if flags.Detector.GeometryITk:
        kwargs["doTrackMatching"] = True
        kwargs.setdefault("TrackMatchBuilderTool", acc.popToolsAndMerge(EMTrackMatchBuilderCfg(flags)))
    if "forwardelectronIsEMselectors" not in kwargs:
        LooseFwdElectronSelector = AsgForwardElectronIsEMSelectorCfg(
            flags,
            "LooseForwardElectronSelector",
            egammaPID.ForwardElectronIDLoose)
        MediumFwdElectronSelector = AsgForwardElectronIsEMSelectorCfg(
            flags,
            "MediumForwardElectronSelector",
            egammaPID.ForwardElectronIDMedium)
        TightFwdElectronSelector = AsgForwardElectronIsEMSelectorCfg(
            flags,
            "TightForwardElectronSelector",
            egammaPID.ForwardElectronIDTight)

        kwargs.setdefault("forwardelectronIsEMselectors",
                          [LooseFwdElectronSelector.popPrivateTools(),
                           MediumFwdElectronSelector.popPrivateTools(),
                           TightFwdElectronSelector.popPrivateTools()])
        kwargs.setdefault(
            "forwardelectronIsEMselectorResultNames",
            ["Loose", "Medium", "Tight"])

        acc.merge(LooseFwdElectronSelector)
        acc.merge(MediumFwdElectronSelector)
        acc.merge(TightFwdElectronSelector)

    if "ObjectQualityTool" not in kwargs and not flags.Common.isOnline:
        egOQ = egammaOQFlagsBuilderCfg(flags)
        kwargs["ObjectQualityTool"] = egOQ.popPrivateTools()
        acc.merge(egOQ)

    kwargs.setdefault("ElectronOutputName",
                      flags.Egamma.Keys.Output.ForwardElectrons)
    kwargs.setdefault("TopoClusterName",
                      flags.Egamma.Keys.Internal.ForwardTopoClusters)
    kwargs.setdefault("ClusterContainerName",
                      flags.Egamma.Keys.Output.ForwardClusters)
    kwargs.setdefault("FourMomBuilderTool", CompFactory.EMFourMomBuilder())

    fwdAlg = CompFactory.egammaForwardBuilder(name, **kwargs)

    acc.addEventAlgo(fwdAlg)
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
    mlog = logging.getLogger("egammaForwardBuilderConfigTest")
    mlog.info("Configuring  egammaForwardBuilder: ")
    acc.merge(egammaForwardBuilderCfg(flags))
    printProperties(mlog,
                    acc.getEventAlgo("egammaForwardElectron"),
                    nestLevel=1,
                    printDefaults=True)
    with open("egammaforwardbuilder.pkl", "wb") as f:
        acc.store(f)
