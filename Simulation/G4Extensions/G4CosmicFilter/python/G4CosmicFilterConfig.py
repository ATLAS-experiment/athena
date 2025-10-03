# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def configCosmicFilterVolumeNames(flags):
    """returns a list with volume names. Can be merged with
    athena/Simulation/ISF/ISF_Core/ISF_Tools/python/ISF_ToolsConfig.py#0103
    """

    volmap = {
        "Muon": ["MuonExitLayer"],
        "Calo": ["MuonEntryLayer"],
        "InnerDetector": ["CaloEntryLayer"],
        "TRT_Barrel": ["TRTBarrelEntryLayer"],
        "TRT_EC": ["TRTECAEntryLayer","TRTECBEntryLayer"],
        "SCT_Barrel": ["SCTBarrelEntryLayer"],
        "Pixel": ["PixelEntryLayer"],
        "NONE": [],
     }
    volumeNames = []
    for vol in flags.Sim.CosmicFilterVolumeNames:
         volumeNames += volmap[vol]

    return volumeNames

def CosmicFilterToolCfg(flags, name="G4UA::G4CosmicFilterTool", **kwargs):
    result = ComponentAccumulator()
    volumes=configCosmicFilterVolumeNames(flags)

    # use simple cosmic filter
    if len(volumes)==1:

        if flags.Sim.CosmicFilterID:
            kwargs.setdefault("PDGId", flags.Sim.CosmicFilterID)
        if flags.Sim.CosmicFilterPTmin:
            kwargs.setdefault("PtMin", flags.Sim.CosmicFilterPTmin)
        if flags.Sim.CosmicFilterPTmax:
            kwargs.setdefault("PtMax", flags.Sim.CosmicFilterPTmax)
        kwargs.setdefault("CollectionName",volumes[0])

        print ('G4CosmicFilter: Filter volume is %s' % volumes[0])

        result.setPrivateTools(CompFactory.G4UA.G4CosmicFilterTool(name, **kwargs))

    elif len(volumes)==2:
        # need a cosmic AND filter
        kwargs.setdefault("CollectionName",volumes[0])
        kwargs.setdefault("CollectionName2",volumes[1])
        result.setPrivateTools(CompFactory.G4UA.G4CosmicAndFilterTool(name, **kwargs))

    else:
        # need a cosmic OR filter
        kwargs.setdefault("CollectionName",volumes[0])
        kwargs.setdefault("CollectionName2",volumes[1])
        kwargs.setdefault("CollectionName3",volumes[2])
        result.setPrivateTools(CompFactory.G4UA.G4CosmicOrFilterTool(name, **kwargs))
    return result


def StoppedParticleFilterToolCfg(flags, name="G4UA::StoppedParticleFilterTool", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("CollectionName",'StoppingPositions')
    result.setPrivateTools(CompFactory.G4UA.G4CosmicFilterTool(name, **kwargs))
    return result
