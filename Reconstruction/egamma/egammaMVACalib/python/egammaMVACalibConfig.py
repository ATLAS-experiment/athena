# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from xAODEgamma.xAODEgammaParameters import xAOD


def egammaMVAToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.egammaMVACalibTool(**kwargs))
    return acc

def egammaTransformerToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    from egammaAlgs.egammaAODFixesConfig import runAODFix
    _, fixes = runAODFix(flags)
    # cluster AOD fix done, do not try to recover. If HI, no timing cut, so no fix here
    if ('egClusterL2_3Fix' in fixes) or flags.Reco.EnableHI:
        kwargs['egammaCellRecoveryTool'] = None
        kwargs['useFixForMissingCells'] = False
    acc.setPrivateTools(CompFactory.egammaTransformerCalibTool(**kwargs))
    return acc

def egammaMVASvcCfg(flags, name="egammaMVASvc", **kwargs):

    acc = ComponentAccumulator()

    kwargs.setdefault("folder", flags.Egamma.Calib.MVAVersion)

    if "ElectronTool" not in kwargs:
        kwargs["ElectronTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="electronMVATool",
                ParticleType=xAOD.EgammaParameters.electron,
                folder=kwargs['folder'])
        )

    if flags.Egamma.doForwardCalib and "FwdElectronTool" not in kwargs:
        kwargs["FwdElectronTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="fwdelectronMVATool",
                ParticleType=xAOD.EgammaParameters.forwardelectron,
                ShiftType=0,
                folder=kwargs['folder'])
        )

    if "UnconvertedPhotonTool" not in kwargs:
        kwargs["UnconvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="unconvertedPhotonMVATool",
                ParticleType=xAOD.EgammaParameters.unconvertedPhoton,
                folder=kwargs['folder'])
        )

    if "ConvertedPhotonTool" not in kwargs:
        kwargs["ConvertedPhotonTool"] = acc.popToolsAndMerge(
            egammaMVAToolCfg(
                flags,
                name="convertedPhotonMVATool",
                ParticleType=xAOD.EgammaParameters.convertedPhoton,
                folder=kwargs['folder'])
        )

    acc.addService(
        CompFactory.egammaMVASvc(
            name=name,
            **kwargs), primary=True)
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

    kwargs['RemoveTRTConvBarrel'] = 1
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
        egammaMVASvcCfg(flags)),
        nestLevel=1,
        printDefaults=True)
    printProperties(mlog, cfg.getPrimaryAndMerge(
        egammaTransformerSvcCfg(flags)),
        nestLevel=1,
        printDefaults=True)
    cfg.printConfig()

    f = open("egmvatools.pkl", "wb")
    cfg.store(f)
    f.close()
