# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""ComponentAccumulator configuration for the Photos++ afterburner."""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


def PhotosppCfg(flags, name="Photospp", **kwargs):
    """Append Photos++ to the generator sequence with legacy fragment defaults."""
    kwargs.setdefault("InfraRedCutOff", 1e-7)
    kwargs.setdefault("RandomSeed", flags.Random.SeedOffset)
    kwargs.setdefault("Dsid", flags.Generator.DSID)
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Photospp_i(name, **kwargs))
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Photospp"]),
             sequenceName=EvgenSequence.Generator.value)
    return ca
