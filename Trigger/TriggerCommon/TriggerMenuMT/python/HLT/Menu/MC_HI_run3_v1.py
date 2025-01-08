# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_HI_run3_v1.py menu for the obsoleted Phase-0 L1Calo based chains
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.PhysicsP1_HI_run3_v1 as physics_menu
from .Physics_pp_run3_v1 import (
        SingleElectronGroup,
        SinglePhotonGroup,
        SingleJetGroup,
        SingleBjetGroup,
        SingleMuonGroup,
        PrimaryL1MuGroup,
        MultiMuonGroup,
        MultiElectronGroup,
        MinBiasGroup,
        SupportLegGroup,
        PrimaryLegGroup,
        PrimaryPhIGroup,
        SupportPhIGroup,
        SupportGroup,
)

HardProbesStream="HardProbes"
MinBiasStream="MinBias"
UPCStream="UPC"
MinBiasOverlayStream="MinBiasOverlay"
PCStream="PC"
CCStream="CC"

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

def getMCSignatures():
    chains = ChainStore()


    chains['Muon'] = []

    chains['Egamma'] = []

    chains['Jet'] = []

    chains['Combined'] = []

    chains['MinBias'] = []

    chains['Streaming'] = []


    return chains

def setupMenu():

    chains = physics_menu.getPhysicsHISignatures()

    log.info('[setupMenu] going to add the MC menu chains now')

    for sig,chainsInSig in getMCSignatures().items():
        chains[sig] += chainsInSig

    return chains


