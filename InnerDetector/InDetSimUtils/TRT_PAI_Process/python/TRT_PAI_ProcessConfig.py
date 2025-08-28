"""Define methods to construct configured TRT_PAI_Process tools

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TRT_PAI_Process_BaseCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.TRT_PAI_Process(name, **kwargs))
    return acc


def TRT_PAI_Process_XeToolCfg(flags, name="TRT_PAI_Process_Xe", **kwargs):
    """Return a Xenon-configured TRT_PAI_Process"""
    kwargs.setdefault("GasType", "Xenon")
    return TRT_PAI_Process_BaseCfg(flags, name, **kwargs)


def TRT_PAI_Process_ArToolCfg(flags, name="TRT_PAI_Process_Ar", **kwargs):
    """Return an Argon-configured TRT_PAI_Process"""
    kwargs.setdefault("GasType", "Argon")
    return TRT_PAI_Process_BaseCfg(flags, name, **kwargs)


def TRT_PAI_Process_KrToolCfg(flags, name="TRT_PAI_Process_Kr", **kwargs):
    """Return a Krypton-configured TRT_PAI_Process"""
    kwargs.setdefault("GasType", "Krypton")
    return TRT_PAI_Process_BaseCfg(flags, name, **kwargs)
