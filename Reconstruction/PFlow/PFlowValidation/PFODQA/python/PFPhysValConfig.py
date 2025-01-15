#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file PFlowPhysVal.py
@author N. Pettersson
@date 2022-07-11
@brief Main CA-based python configuration for PFODQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValPFOToolCfg(name, container='', **kwargs):
    acc = ComponentAccumulator()

    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("FlowElementContainerName", container)
    kwargs.setdefault("useNeutralFE", "Neutral" in container)
    tool = CompFactory.PhysValFE(name, **kwargs)
    acc.setPrivateTools(tool)
    return acc

def PhysValPFOCfg(flags, useGlobalContainers=False):
    acc = ComponentAccumulator()

    if useGlobalContainers:
        chargedName="GlobalChargedParticleFlowObjects"
        neutralName="GlobalNeutralParticleFlowObjects"
    else:
        chargedName="JetETMissChargedParticleFlowObjects"
        neutralName="JetETMissNeutralParticleFlowObjects"

    tools = [ acc.popToolsAndMerge(PhysValPFOToolCfg("PhysValFE_charged", chargedName)) ]
    tools += [ acc.popToolsAndMerge(PhysValPFOToolCfg("PhysValFE_neutral", neutralName)) ]
    acc.setPrivateTools(tools)
    return acc
