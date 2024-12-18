# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# TRUTH0.py - direct and complete translation of HepMC in EVNT to xAOD truth 
# No additional information is added

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import MetadataCategory

def TRUTH0Cfg(flags):
    """Main config for TRUTH0"""
    acc = ComponentAccumulator()
    
    # Ensure EventInfoCnvAlg is scheduled
    if "EventInfo#McEventInfo" in flags.Input.TypedCollections and "xAOD::EventInfo#EventInfo" not in flags.Input.TypedCollections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        acc.merge(EventInfoCnvAlgCfg(flags, inputKey="McEventInfo", outputKey="EventInfo", disableBeamSpot=True))
 
    # Decide what kind of input HepMC container we are dealing with
    # and schedule the xAOD converter appropriately
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    if "McEventCollection#GEN_EVENT" in flags.Input.TypedCollections:
        acc.merge(GEN_EVNT2xAODCfg(flags,name="GEN_EVNT2xAOD",AODContainerName="GEN_EVENT"))
    elif "McEventCollection#TruthEvent" in flags.Input.TypedCollections:
        acc.merge(GEN_EVNT2xAODCfg(name="GEN_EVNT2xAOD",AODContainerName="TruthEvent"))
    else:
        raise RuntimeError("No recognised HepMC truth information found in the input")
 
    # Contents
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TRUTH0SlimmingHelper = SlimmingHelper("TRUTH0SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TRUTH0SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD::EventAuxInfo',
                                               'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                               'TruthVertices':'xAOD::TruthVertexContainer','TruthVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'TruthParticles':'xAOD::TruthParticleContainer','TruthParticlesAux':'xAOD::TruthParticleAuxContainer'} 

    TRUTH0SlimmingHelper.AllVariables = [ 'EventInfo',
                                          'TruthEvents', 
                                          'TruthVertices',
                                          'TruthParticles']

    # Create output stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    TRUTH0ItemList = TRUTH0SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_TRUTH0", ItemList=TRUTH0ItemList))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TRUTH0", createMetadata=[MetadataCategory.CutFlowMetaData, MetadataCategory.TruthMetaData]))

    return acc
