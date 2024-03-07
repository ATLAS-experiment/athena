# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

############## Input: Reading Particles from TrackRecord Input File ###############
def Input_TrackRecordGeneratorCfg(ConfigFlags,name="TrackRecordGenerator", **kwargs):
    """Configuring the Athena application for a 'track record' job"""
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    # import G4AtlasApps.AtlasCosmicTrackRecordJob #todo make this new style

    from AthenaConfiguration.ComponentFactory import CompFactory
    TrackRecordGenerator=CompFactory.TrackRecordGenerator

    kwargs = {}
    from SimulationConfig.SimEnums import CavernBackground
    if ConfigFlags.Sim.CavernBackground is CavernBackground.Read:
        kwargs.setdefault('TRSmearing', -1) #in millimeters, e.g. 10
        kwargs.setdefault('TRPSmearing', -1) #in radians, e.g. 0.01
        kwargs.setdefault('TRCollection', "NeutronBG")
    else:
        kwargs.setdefault('TRCollection','CosmicRecord') # Collection name
        kwargs.setdefault("TRSmearing", -1 ) #in millimeters, e.g. 10
        kwargs.setdefault("TRPSmearing", -1 ) #in radians, e.g. 0.01
    kwargs.setdefault('StopParticles', False) # Stop the particles or not
    kwargs.setdefault('stopped_tminus', -25.) # in ns, lower bound
    kwargs.setdefault('stopped_tplus', 25.) # in ns, upper bound
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault('RndmSvc', acc.getPrimaryAndMerge(AthRNGSvcCfg(ConfigFlags)).name)
    #kwargs.setdefault('OutputLevel', DEBUG) # for turning up output during testing
    acc.addEventAlgo(TrackRecordGenerator(name,**kwargs))
    return acc

# def getTrackRecordCosmicGenerator(name="TrackRecordCosmicGenerator", **kwargs):
#     return getInput_TrackRecordGenerator(name, **kwargs)
