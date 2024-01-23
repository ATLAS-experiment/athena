"""ComponentAccumulator core ISF_Service configuration
Provides local configuration to be imported without circular dependencies
Complements ISF_ServicesConfig

Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm
from SubDetectorEnvelopes.SubDetectorEnvelopesConfig import EnvelopeDefSvcCfg
from ISF_Tools.ISF_ToolsConfig import ParticleKillerToolCfg


def ISFEnvelopeDefSvcCfg(flags, name="ISF_ISFEnvelopeDefSvc", **kwargs):
    # ATLAS common envlope definitions
    result = ComponentAccumulator()
    kwargs.setdefault("ATLASEnvelopeDefSvc", result.getPrimaryAndMerge(EnvelopeDefSvcCfg(flags)).name)
    result.addService(CompFactory.ISF.ISFEnvelopeDefSvc(name, **kwargs), primary = True)
    return result


def GeoIDSvcCfg(flags, name="ISF_GeoIDSvc", **kwargs):
    result = ComponentAccumulator()
    # with ISF volume definitions
    kwargs.setdefault("EnvelopeDefSvc", result.getPrimaryAndMerge(ISFEnvelopeDefSvcCfg(flags)).name)
    result.addService(CompFactory.ISF.GeoIDSvc(name, **kwargs), primary = True)
    return result


def ATLFAST_EnvelopeDefSvcCfg(flags, name="ISF_ATLFAST_EnvelopeDefSvc", **kwargs):
    result = ComponentAccumulator()
    # ATLAS common envlope definitions
    kwargs.setdefault("ISFEnvelopeDefSvc", result.getPrimaryAndMerge(ISFEnvelopeDefSvcCfg(flags)).name)
    kwargs.setdefault("InDetMaxExtentZ", 3549.5*mm)
    result.addService(CompFactory.ISF.ATLFAST_EnvelopeDefSvc(name, **kwargs), primary = True)
    return result


def ATLFAST_GeoIDSvcCfg(flags, name="ISF_ATLFAST_GeoIDSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("EnvelopeDefSvc", result.getPrimaryAndMerge(ATLFAST_EnvelopeDefSvcCfg(flags)).name)
    result.addService(CompFactory.ISF.GeoIDSvc(name, **kwargs), primary = True)
    return result


# previously in LegacySimServicesConfig
def ParticleKillerSvcCfg(flags, name="ISF_ParticleKillerSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("Identifier", "ParticleKiller")
    tool = result.popToolsAndMerge(ParticleKillerToolCfg(flags))
    kwargs.setdefault("SimulatorTool", tool)
    result.addService(CompFactory.ISF.LegacySimSvc(name, **kwargs), primary = True)
    return result
