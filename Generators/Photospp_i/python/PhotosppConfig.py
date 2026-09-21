# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("PhotosppConfig")


def PhotosppCfg(flags, name="Photospp_i", **kwargs):
    """
    The main Photos configuration fragment that sets up the
    algorithm and returns a CA instance.
    """

    # Set default Photos++ QED config
    kwargs.setdefault("InfraRedCutOff", 1E-7)

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.Photospp_i(name, **kwargs)
    )

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Photospp"]), sequenceName=EvgenSequence.Generator.value)

    return ca
