"""ComponentAccumulator BarcodeServices configurations

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BarcodeSvcCfg(flags, **kwargs):
    """Return the MCxBarcodeSvcCfg config flagged by Sim.TruthStrategy"""
    from SimulationConfig.SimEnums import TruthStrategy
    stratmap = {
        TruthStrategy.MC12: MC12BarcodeSvcCfg,
        TruthStrategy.MC12LLP: MC12LLPBarcodeSvcCfg,
        TruthStrategy.MC12Plus: MC12PlusBarcodeSvcCfg,
        TruthStrategy.MC15: MC15aBarcodeSvcCfg,
        TruthStrategy.MC15a: MC15aBarcodeSvcCfg,
        TruthStrategy.MC15aPlus: MC15aPlusBarcodeSvcCfg,
        TruthStrategy.MC15aPlusLLP: MC15aPlusLLPBarcodeSvcCfg,
        TruthStrategy.Validation: ValidationBarcodeSvcCfg,
        TruthStrategy.MSLLPValidation: MSLLPValidationBarcodeSvcCfg,
        # TruthStrategy.Cosmic: CosmicBarcodeSvcCfg,
    }
    MCxCfg = stratmap[flags.Sim.TruthStrategy]
    return MCxCfg(flags, name="BarcodeSvc", **kwargs)


def MC12BarcodeSvcCfg(flags, name="Barcode_MC12BarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    svc = CompFactory.Barcode.LegacyBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result


def MC12LLPBarcodeSvcCfg(flags, name="Barcode_MC12LLPBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC12PlusBarcodeSvcCfg(flags, name="Barcode_MC12PlusBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aPlusBarcodeSvcCfg(flags, name="Barcode_MC15aPlusBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aPlusLLPBarcodeSvcCfg(flags, name="Barcode_MC15aPlusLLPBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aBarcodeSvcCfg(flags, name="Barcode_MC15aBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MSLLPValidationBarcodeSvcCfg(flags, name="Barcode_MSLLPValidationBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def ValidationBarcodeSvcCfg(flags, name="Barcode_ValidationBarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    svc = CompFactory.Barcode.ValidationBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result
