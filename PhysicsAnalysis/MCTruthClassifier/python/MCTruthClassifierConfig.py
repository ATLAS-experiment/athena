# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Tool configuration to instantiate MCTruthClassifier
          with default configurations."""


def MCTruthClassifierCfg(flags, **kwargs):
    """
    This is the default configuration allowing all options.
    By default, it does not do calo truth matching.
    """
    kwargs.setdefault("ParticleCaloExtensionTool", "")
    kwargs.setdefault("CaloDetDescrManager", "")
    return MCTruthClassifierCaloTruthMatchCfg(flags, **kwargs)


def MCTruthClassifierCaloTruthMatchCfg(flags, **kwargs):
    """
    This is the default configuration allowing all options.
    By default, it does calo truth matching using a
    dedicated instance of the ParticleCaloExtensionTool
    """
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import LHCPeriod

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

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.MCTruthClassifier(**kwargs))
    return acc


##########################################################
# The function below are for the old style and should be
# condsidered deprecated
##########################################################
def firstSimCreatedBarcode():
    """DEPRECATED!!! Return the simulation barcode offset for G4
    particles from metadata
    In the new configuration scheme use the Sim.SimBarcodeOffset flag instead
    """

    from AthenaCommon.Logging import logging
    mlog = logging.getLogger("firstSimCreatedBarcode")
    mlog.info("This function should not be used in the new configuration"
              "scheme. Use Sim.SimBarcodeOffset flag insted")

    offset = 200e3

    # Don't try to run the input peeker for a generator job; it will fail
    # without a proper input file.
    # Is there a better way of doing this test?
    from AthenaCommon.AppMgr import theApp
    if getattr(theApp, 'EvtSel', None) == 'McEventSelector/EventSelector':
        mlog.info('Generator job: leaving SimBarcodeOffset at 200k')

    else:
        from PyUtils.MetaReaderPeeker import metadata
        try:
            offset = int(metadata['SimBarcodeOffset'])
        except Exception:
            mlog.info(
                'Could not retrieve SimBarcodeOffset from /Simulation/Parameters, leaving at 200k')
    return int(offset + 1)


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import logging

    from AthenaConfiguration.ComponentAccumulator import (
        ComponentAccumulator, printProperties)

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
