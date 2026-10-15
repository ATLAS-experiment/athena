# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("FlowAfterburnerConfig")


def AddFlowByShiftingCfg(flags, name="AddFlowByShifting", **kwargs):
    """
    Configuration fragment that sets up the AddFlowByShifting
    algorithm and returns a CA instance.
    """

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.AddFlowByShifting(name, **kwargs)
    )

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["AddFlowByShifting"]), sequenceName=EvgenSequence.Generator.value)

    return ca
