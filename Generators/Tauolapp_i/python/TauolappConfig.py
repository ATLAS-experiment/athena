# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("TauolappConfig")


def TauolappCfg(flags, name="TauolaPP_i", **kwargs):
    """
    The main Tauola configuration fragment that sets up the
    algorithm and returns a CA instance.
    """

    # Set default config
    kwargs.setdefault("decay_mode_same", 0)
    kwargs.setdefault("decay_mode_opposite", 0)
    kwargs.setdefault("decay_particle", 15)
    kwargs.setdefault("tau_mass", 1.77684)
    kwargs.setdefault("spin_correlation", True)
    kwargs.setdefault("setRadiation", True)
    kwargs.setdefault("setRadiationCutOff", 0.01)

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.TauolaPP(name, **kwargs)
    )

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["TauolaPP"]), sequenceName=EvgenSequence.Generator.value)

    return ca
