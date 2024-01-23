# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""
ISF_SimulationSelectors configurations for ISF
Elmar Ritsch, 04/02/2013
"""

from AthenaCommon import CfgMgr
from ISF_SimulationSelectors import SimulationFlavor
### DefaultSimSelector configurations

def usesSimKernelMT():
    from ISF_Config.ISF_jobProperties import ISF_Flags
    return (ISF_Flags.Simulator.get_Value() in ['FullG4MT', 'FullG4MT_QS', 'PassBackG4MT', 'ATLFASTIIMT', 'ATLFAST3MT', 'ATLFAST3MT_QS', 'FullG4MT_LongLived', 'ATLFASTIIF_ACTS', 'ATLFAST3F_ACTSMT'])


def getDefaultSimSelector(name="ISF_DefaultSimSelector", **kwargs):
    return CfgMgr.ISF__DefaultSimSelector(name, **kwargs )


def getDefaultParticleKillerSelector(name="ISF_DefaultParticleKillerSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_ParticleKillerSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.ParticleKiller)
    return getDefaultSimSelector(name, **kwargs )


def getDefaultGeant4Selector(name="ISF_DefaultGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_Geant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )


def getDefaultATLFAST_Geant4Selector(name="ISF_DefaultATLFAST_Geant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_ATLFAST_Geant4SimSvc')
    return getDefaultGeant4Selector(name, **kwargs )


def getDefaultLongLivedGeant4Selector(name="ISF_DefaultLongLivedGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FullGeant4SimSvc')
    return getDefaultGeant4Selector(name, **kwargs )


def getFullGeant4Selector(name="ISF_FullGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FullGeant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )


def getPassBackGeant4Selector(name="ISF_PassBackGeant4Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_PassBackGeant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return getDefaultSimSelector(name, **kwargs )


def getDefaultFastCaloSimV2Selector(name="ISF_DefaultFastCaloSimV2Selector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FastCaloSimSvcV2')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.FastCaloSimV2)
    return getDefaultSimSelector(name, **kwargs )


def getDefaultDNNCaloSimSelector(name="ISF_DefaultDNNCaloSimSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_DNNCaloSimSvc')
    return getDefaultSimSelector(name, **kwargs )


def getDefaultFatrasSelector(name="ISF_DefaultFatrasSelector", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault("Simulator"   , 'ISF_FatrasSimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return getDefaultSimSelector(name, **kwargs )


def getDefaultActsSelector(name="ISF_DefaultActsSelector", **kwargs):
    if not usesSimKernelMT():
        raise RuntimeError("SimulationSelector '%s' does not support running with SimKernel." % name)
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return getDefaultSimSelector(name, **kwargs)


### KinematicSimSelector Configurations

# BASE METHODS
def getKinematicGeant4Selector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_Geant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)


def getKinematicATLFAST_Geant4Selector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_ATLFAST_Geant4SimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Geant4)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)


def getKinematicFatrasSelector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_FatrasSimSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.Fatras)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)


def getKinematicParticleKillerSimSelector(name="DONOTUSEDIRECTLY", **kwargs):
    if usesSimKernelMT():
        kwargs.setdefault('Simulator', '')
    kwargs.setdefault('Simulator'       , 'ISF_ParticleKillerSvc')
    kwargs.setdefault('SimulationFlavor', SimulationFlavor.ParticleKiller)
    return CfgMgr.ISF__KinematicSimSelector(name, **kwargs)


# Electrons
def getElectronGeant4Selector(name="ISF_ElectronGeant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 11)
    return getKinematicGeant4Selector(name, **kwargs)


# Protons
def getProtonG4FastCalo_Geant4Selector(name="ISF_ProtonG4FastCalo_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 2212)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


def getProtonATLFAST_Geant4Selector(name="ISF_ProtonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2212)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


# Pions
def getPionATLFAST_Geant4Selector(name="ISF_PionATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'          , 200)
    kwargs.setdefault('ParticlePDG'     , 211)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


# Neutrons
def getNeutronATLFAST_Geant4Selector(name="ISF_NeutronATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 2112)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


# Charged Kaons
def getChargedKaonG4FastCalo_Geant4Selector(name="ISF_ChargedKaonG4FastCalo_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 321)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


def getChargedKaonATLFAST_Geant4Selector(name="ISF_ChargedKaonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 321)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


# KLongs
def getKLongG4FastCalo_Geant4Selector(name="ISF_KLongG4FastCalo_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxMom'          , 750)
    kwargs.setdefault('ParticlePDG'     , 130)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


def getKLongATLFAST_Geant4Selector(name="ISF_KLongATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('MaxEkin'         , 400)
    kwargs.setdefault('ParticlePDG'     , 130)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


# Muons
def getMuonGeant4Selector(name="ISF_MuonGeant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicGeant4Selector(name, **kwargs)


def getMuonATLFAST_Geant4Selector(name="ISF_MuonATLFAST_Geant4Selector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicATLFAST_Geant4Selector(name, **kwargs)


def getMuonFatrasSelector(name="ISF_MuonFatrasSelector", **kwargs):
    kwargs.setdefault('ParticlePDG'     , 13)
    return getKinematicFatrasSelector(name, **kwargs)


# General Eta-based selectors
def getEtaGreater5ParticleKillerSimSelector(name="ISF_EtaGreater5ParticleKillerSimSelector", **kwargs):
    kwargs.setdefault('MinPosEta'       , -5.0 )
    kwargs.setdefault('MaxPosEta'       ,  5.0 )
    kwargs.setdefault('InvertCuts'      , True )
    return getKinematicParticleKillerSimSelector(name, **kwargs)
