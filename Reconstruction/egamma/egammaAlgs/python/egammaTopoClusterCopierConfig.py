# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = "Instantiate egammaTopoClusterCopier with default configuration"

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaTopoClusterCopierCfg(flags, name='', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("InputTopoCollection",
                      flags.Egamma.Keys.Input.TopoClusters)
    kwargs.setdefault("OutputTopoCollection",
                      flags.Egamma.Keys.Internal.EgammaTopoClusters)

    if flags.Egamma.doForward:
        kwargs.setdefault("OutputFwdTopoCollection",
                          flags.Egamma.Keys.Internal.ForwardTopoClusters)

    kwargs.setdefault("ECut", 700 if not flags.Egamma.doLowMu else 300)

    kwargs.setdefault('hasITk', flags.Detector.GeometryITk)

    if name == '':
        name = kwargs["OutputTopoCollection"]+'Copier'

    egcopierAlg = CompFactory.egammaTopoClusterCopier(name, **kwargs)

    acc.addEventAlgo(egcopierAlg)

    return acc


def indetTopoClusterCopierCfg(flags, name='', **kwargs):
    """Create a copier to be used in tracking. 
       If 'OutputTopoCollection' is the same as used in 
        'egammaTopoClusterCopierCfg', these two functions will produce
        the same tool that will be de-duplicated later, preventing 
        duplication of the output containers. This will happen in
        a standard pp reconstruction.
       If 'OutputTopoCollection' is not the same, two tools will be
        created, each with a different output container. This will
        happen in a HI reconstruction."""

    kwargs.setdefault(
        "InputTopoCollection",
        flags.Tracking.TopoClusters)
    kwargs.setdefault(
        "OutputTopoCollection",
        flags.Tracking.EgammaTopoClusters)

    if name == '':
        name = kwargs["OutputTopoCollection"]+'Copier'

    return egammaTopoClusterCopierCfg(flags, name, **kwargs)


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()
    acc = MainServicesCfg(flags)
    mlog = logging.getLogger("egammaTopoClusterCopierConfigTest")
    mlog.info("Configuring  egammaTopoClusterCopier: ")
    acc.merge(egammaTopoClusterCopierCfg(flags))
    printProperties(mlog,
                    acc.getEventAlgo("egammaTopoClustersCopier"),
                    nestLevel=1,
                    printDefaults=True)
    with open("egammatopoclustercopier.pkl", "wb") as f:
        acc.store(f)
