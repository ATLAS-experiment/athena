# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from xAODEgamma.xAODEgammaParameters import xAOD


def egammaMVAToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.egammaMVACalibTool(**kwargs))
    return acc


def egammaMVASvcCfg(flags, name="egammaMVASvc", **kwargs):

    acc = ComponentAccumulator()

    if "folder" not in kwargs:
        folder = flags.Egamma.Calib.MVAVersion
    else:
        # we pop. As folder is not a property of
        # egammaMVASvc but of the tools
        folder = kwargs.pop("folder")

    if "ElectronTool" not in kwargs:
        kwargs["ElectronTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="electronMVATool",
                ParticleType=xAOD.EgammaParameters.electron,
                folder=folder)
        )

    if "UnconvertedPhotonTool" not in kwargs:
        kwargs["UnconvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="unconvertedPhotonMVATool",
                ParticleType=xAOD.EgammaParameters.unconvertedPhoton,
                folder=folder)
        )

    if "ConvertedPhotonTool" not in kwargs:
        kwargs["ConvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="convertedPhotonMVATool",
                ParticleType=xAOD.EgammaParameters.convertedPhoton,
                folder=folder)
        )

    acc.addService(
        CompFactory.egammaMVASvc(
            name=name,
            **kwargs), primary=True)
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.fillFromArgs()
    flags.lock()

    cfg = ComponentAccumulator()
    mlog = logging.getLogger("egammaMVASvcConfigTest")
    mlog.info("Configuring egammaMVASvc :")
    printProperties(mlog, cfg.getPrimaryAndMerge(
        egammaMVASvcCfg(flags,
                        folder=flags.Egamma.Calib.MVAVersion)),
                    nestLevel=1,
                    printDefaults=True)
    cfg.printConfig()

    f = open("egmvatools.pkl", "wb")
    cfg.store(f)
    f.close()
