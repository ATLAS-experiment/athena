# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Tool configuration to instantiate MCTruthClassifier
          with default configurations."""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod


def MCTruthClassifierCfg(flags, name="MCTruthClassifier", **kwargs):
    """
    This is the default configuration allowing all options.
    By default, it does not do calo truth matching.
    """
    kwargs.setdefault("ParticleCaloExtensionTool", "")
    kwargs.setdefault("CaloDetDescrManager", "")
    return MCTruthClassifierCaloTruthMatchCfg(flags, name, **kwargs)


def MCTruthClassifierCaloTruthMatchCfg(flags, name="MCTruthClassifier", **kwargs):
    """
    This is the default configuration allowing all options.
    By default, it does calo truth matching using a
    dedicated instance of the ParticleCaloExtensionTool
    """
    acc = ComponentAccumulator()

    if "ParticleCaloExtensionTool" not in kwargs:

        from TrkConfig.AtlasExtrapolatorConfig import (
            MCTruthClassifierExtrapolatorCfg)
        extrapolator = acc.popToolsAndMerge(
            MCTruthClassifierExtrapolatorCfg(flags))

        from TrackToCalo.TrackToCaloConfig import (
            EMParticleCaloExtensionToolCfg)
        extension = EMParticleCaloExtensionToolCfg(
            flags, Extrapolator=extrapolator)
        kwargs["ParticleCaloExtensionTool"] = acc.popToolsAndMerge(extension)

    kwargs.setdefault("CaloDetDescrManager", "CaloDetDescrManager")

    if flags.GeoModel.Run >= LHCPeriod.Run4:
        kwargs.setdefault("FwdElectronUseG4Sel", False)

    acc.setPrivateTools(CompFactory.MCTruthClassifier(**kwargs))
    return acc


def DFCommonMCTruthClassifierCfg(flags):
    """Configure the MCTruthClassifier tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(acc.popToolsAndMerge(MCTruthClassifierCfg(flags, name = "DFCommonTruthClassifier")),
                      primary = True)
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import logging

    from AthenaConfiguration.ComponentAccumulator import printProperties

    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    mlog = logging.getLogger("MCTruthClassifierConfigTest")

    cfg = ComponentAccumulator()

    mlog.info("Configuring standard MCTruthClassifier")
    printProperties(mlog,
                    cfg.getPrimaryAndMerge(
                        MCTruthClassifierCfg(flags)),
                    nestLevel=1,
                    printDefaults=True)

    mlog.info("Configuring MCTruthClassifier with calo truth matching")
    printProperties(mlog,
                    cfg.getPrimaryAndMerge(
                        MCTruthClassifierCaloTruthMatchCfg(flags)),
                    nestLevel=1,
                    printDefaults=True)

    f = open("mctruthclassifer.pkl", "wb")
    cfg.store(f)
    f.close()
