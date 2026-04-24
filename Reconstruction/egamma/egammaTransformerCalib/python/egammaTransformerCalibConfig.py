# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from xAODEgamma.xAODEgammaParameters import xAOD


def egammaTransformerToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    isMC = flags.Input.isMC
    kwargs["isMC"] = isMC
    acc.setPrivateTools(CompFactory.egammaTransformerCalibTool(**kwargs))
    return acc


def egammaTransformerSvcCfg(flags, name="egammaTransformerSvc", **kwargs):

    acc = ComponentAccumulator()
    
    kwargs.setdefault("folder", flags.Egamma.Calib.TransformerVersion)
    if "ElectronTool" not in kwargs:
        kwargs["ElectronTool"] = acc.popToolsAndMerge(
            egammaTransformerToolCfg(
                flags,
                name="electronTransformerTool",
                ParticleType=xAOD.EgammaParameters.electron,
                folder=kwargs['folder'],
                isMC = flags.Input.isMC),
        )

    if flags.Egamma.doForwardCalib and "FwdElectronTool" not in kwargs:
        kwargs["FwdElectronTool"] = acc.popToolsAndMerge(
            egammaTransformerToolCfg(
                flags,
                name="fwdelectronTransformerTool",
                ParticleType=xAOD.EgammaParameters.forwardelectron,
                folder=kwargs['folder'],
                isMC = flags.Input.isMC),
        )

    if "UnconvertedPhotonTool" not in kwargs:
        kwargs["UnconvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaTransformerToolCfg(
                flags,
                name="unconvertedPhotonTransformerTool",
                ParticleType=xAOD.EgammaParameters.unconvertedPhoton,
                folder=kwargs['folder'],
                isMC = flags.Input.isMC),
        )

    if "ConvertedPhotonTool" not in kwargs:
        kwargs["ConvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaTransformerToolCfg(
                flags,
                name="convertedPhotonTransformerTool",
                ParticleType=xAOD.EgammaParameters.convertedPhoton,
                folder=kwargs['folder'],
                isMC = flags.Input.isMC),
        )

    acc.addService(
        CompFactory.egammaTransformerSvc(
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
    mlog = logging.getLogger("egammaTransformerSvcConfigTest")
    mlog.info("Configuring egammaTransformerSvc :")
    printProperties(mlog, cfg.getPrimaryAndMerge(
        egammaTransformerSvcCfg(flags,
                                folder=flags.Egamma.Calib.TransformerVersion)),
                    nestLevel=1,
                    printDefaults=True)
    cfg.printConfig()

    f = open("egtransformertools.pkl", "wb")
    cfg.store(f)
    f.close()
