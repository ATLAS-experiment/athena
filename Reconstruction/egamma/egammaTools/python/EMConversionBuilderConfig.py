# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Configure Conversion building """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from egammaTrackTools.egammaTrackToolsConfig import EMExtrapolationToolsCfg


def EMConversionBuilderCfg(flags, name='EMConversionBuilder', **kwargs):

    mlog = logging.getLogger(name)
    mlog.debug('Start configuration')

    acc = ComponentAccumulator()
    EMConversionBuilder = CompFactory.EMConversionBuilder

    if "ExtrapolationTool" not in kwargs:
        kwargs["ExtrapolationTool"] = acc.popToolsAndMerge(
            EMExtrapolationToolsCfg(flags)
        )

    kwargs.setdefault("ConversionContainerName",
                      flags.Egamma.Keys.Output.ConversionVertices)

    emconv = EMConversionBuilder(name, **kwargs)

    acc.setPrivateTools(emconv)
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
    mlog = logging.getLogger("EMConversionBuilderConfigTest")
    mlog.info("Configuring  EMConversionBuilder: ")
    printProperties(mlog, cfg.popToolsAndMerge(
        EMConversionBuilderCfg(flags)),
        nestLevel=1,
        printDefaults=True)

    f = open("emshowerbuilder.pkl", "wb")
    cfg.store(f)
    f.close()
