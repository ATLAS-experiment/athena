"""
ISF_SimulationSelectors for ComponentAccumulator configuration

Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ISF_SimulationSelectors import SimulationFlavor
from ISF_Services.ISF_ServicesCoreConfig import ParticleKillerSvcCfg
from ISF_Geant4Services.ISF_Geant4ServicesConfig import (
    Geant4SimCfg, ATLFAST_Geant4SimCfg,
    FullGeant4SimCfg, PassBackGeant4SimCfg,
)


def DefaultParticleKillerSelectorCfg(flags, name="ISF_DefaultParticleKillerSelector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(ParticleKillerSvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.ParticleKiller)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultGeant4SelectorCfg(flags, name="ISF_DefaultGeant4Selector", **kwargs):
    acc = ComponentAccumulator()
    if "Simulator" not in kwargs:
        if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
            kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(Geant4SimCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Geant4)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultATLFAST_Geant4SelectorCfg(flags, name="ISF_DefaultATLFAST_Geant4Selector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        acc.merge(ATLFAST_Geant4SimCfg(flags))
        kwargs.setdefault("Simulator", acc.getService("ISF_ATLFAST_Geant4SimSvc"))
    tool = acc.popToolsAndMerge(DefaultGeant4SelectorCfg(flags, name, **kwargs))
    acc.setPrivateTools(tool)
    return acc


def FullGeant4SelectorCfg(flags, name="ISF_FullGeant4Selector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(FullGeant4SimCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Geant4)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def PassBackGeant4SelectorCfg(flags, name="ISF_PassBackGeant4Selector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(PassBackGeant4SimCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Geant4)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultFastCaloSimSelectorCfg(flags, name="ISF_DefaultFastCaloSimSelector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimSvcCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(FastCaloSimSvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.FastCaloSim)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultFastCaloSimV2SelectorCfg(flags, name="ISF_DefaultFastCaloSimV2Selector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimV2SvcCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(FastCaloSimV2SvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.FastCaloSimV2)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultDNNCaloSimSelectorCfg(flags, name="ISF_DefaultDNNCaloSimSelector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import DNNCaloSimSvcCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(DNNCaloSimSvcCfg(flags)))
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def FastHitConvAlgFastCaloSimSelectorCfg(flags, name="ISF_FastHitConvAlgFastCaloSimSelector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastHitConvAlgFastCaloSimSvcCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(FastHitConvAlgFastCaloSimSvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.FastCaloSim)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultFatrasSelectorCfg(flags, name="ISF_DefaultFatrasSelector", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FatrasServices.ISF_FatrasConfig import fatrasSimServiceIDCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(fatrasSimServiceIDCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Fatras)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


def DefaultActsSelectorCfg(flags, name="ISF_DefaultActsSelector", **kwargs):
    acc = ComponentAccumulator()
    if not flags.Sim.ISF.Simulator.isMT():
        raise RuntimeError("SimulationSelector '%s' does not support running with SimKernel." % name)
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    acc.setPrivateTools(CompFactory.ISF.DefaultSimSelector(name, **kwargs))
    return acc


### KinematicSimSelector Configurations

# BASE METHODS
def BaseKinematicGeant4SelectorCfg(flags, name="DONOTUSEDIRECTLY", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        acc.merge(Geant4SimCfg(flags))
        kwargs.setdefault("Simulator", acc.getService("ISFG4SimSvc"))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Geant4)
    acc.setPrivateTools(CompFactory.ISF.KinematicSimSelector(name, **kwargs))
    return acc


def BaseKinematicATLFAST_Geant4SelectorCfg(flags, name="DONOTUSEDIRECTLY", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(ATLFAST_Geant4SimCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Geant4)
    acc.setPrivateTools(CompFactory.ISF.KinematicSimSelector(name, **kwargs))
    return acc


def BaseKinematicFatrasSelectorCfg(flags, name="DONOTUSEDIRECTLY", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FatrasServices.ISF_FatrasConfig import fatrasSimServiceIDCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(fatrasSimServiceIDCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.Fatras)
    acc.setPrivateTools(CompFactory.ISF.KinematicSimSelector(name, **kwargs))
    return acc


def BaseKinematicFastCaloSimSelectorCfg(flags, name="DONOTUSEDIRECTLY", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        from ISF_FastCaloSimServices.ISF_FastCaloSimServicesConfig import FastCaloSimSvcCfg
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(FastCaloSimSvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.FastCaloSim)
    acc.setPrivateTools(CompFactory.ISF.KinematicSimSelector(name, **kwargs))
    return acc


def BaseKinematicParticleKillerSimSelectorCfg(flags, name="DONOTUSEDIRECTLY", **kwargs):
    acc = ComponentAccumulator()
    if flags.Concurrency.NumThreads == 0 and not flags.Sim.ISF.Simulator.isMT():
        kwargs.setdefault("Simulator", acc.getPrimaryAndMerge(ParticleKillerSvcCfg(flags)))
    kwargs.setdefault("SimulationFlavor", SimulationFlavor.ParticleKiller)
    acc.setPrivateTools(CompFactory.ISF.KinematicSimSelector(name, **kwargs))
    return acc


#Protons
def ProtonATLFAST_Geant4SelectorCfg(flags, name="ISF_ProtonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("MaxEkin", 400)
    kwargs.setdefault("ParticlePDG", 2212)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


#Pions
def PionATLFAST_Geant4SelectorCfg(flags, name="ISF_PionATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("MaxEkin", 200)
    kwargs.setdefault("ParticlePDG", 211)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


# Neutrons
def NeutronATLFAST_Geant4SelectorCfg(flags, name="ISF_NeutronATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("MaxEkin", 400)
    kwargs.setdefault("ParticlePDG", 2112)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


# Charged Kaons
def ChargedKaonATLFAST_Geant4SelectorCfg(flags, name="ISF_ChargedKaonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("MaxEkin", 400)
    kwargs.setdefault("ParticlePDG", 321)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


# KLongs
def KLongATLFAST_Geant4SelectorCfg(flags, name="ISF_KLongATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("MaxEkin", 400)
    kwargs.setdefault("ParticlePDG", 130)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


#Muons
def MuonATLFAST_Geant4SelectorCfg(flags, name="ISF_MuonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault("ParticlePDG", 13)
    return BaseKinematicATLFAST_Geant4SelectorCfg(flags, name, **kwargs)


# General Eta-based selectors
def WithinEta5FastCaloSimSelectorCfg(flags, name="ISF_WithinEta5FastCaloSimSelector", **kwargs):
    kwargs.setdefault('MinPosEta'       , -5.0 )
    kwargs.setdefault('MaxPosEta'       ,  5.0 )
    return BaseKinematicFastCaloSimSelectorCfg(flags, name, **kwargs)


def EtaGreater5ParticleKillerSimSelectorCfg(flags, name="ISF_EtaGreater5ParticleKillerSimSelector", **kwargs):
    kwargs.setdefault("MinPosEta", -5.0)
    kwargs.setdefault("MaxPosEta",  5.0)
    kwargs.setdefault("InvertCuts", True)
    return BaseKinematicParticleKillerSimSelectorCfg(flags, name, **kwargs)
