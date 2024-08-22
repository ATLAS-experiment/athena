#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GeneratorPhysValMonitoringToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("binning_N_TruthParticle", [600,0.,6000])
    kwargs.setdefault("binning_N_GeneratorLevelParticle", [50,0.,1500])
    kwargs.setdefault("binning_N_SimulationLevelParticle", [200,0,200])

    acc.setPrivateTools(CompFactory.GeneratorPhysVal.GeneratorPhysValMonitoringTool(**kwargs))
    return acc


