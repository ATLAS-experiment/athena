# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from EvgenJobTransforms.EvgenHelpers import BeamType
from AthenaCommon.SystemOfUnits import GeV

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("HijingConfig")


def HijingInitializeCfg(flags, source, values, precedence, name="Hijing"):
    """
    Return a CA fragment that adds one initialize layer to Hijing.
    """
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(
        CompFactory.Hijing(name, Initialize=GeneratorSettingsLayer(
            source=source,
            values=tuple(values or ()),
            precedence=precedence,
            report_context="Hijing.Initialize"
        ))
    )
    return ca


def HijingBaseCfg(flags, name="Hijing", **kwargs):
    """
    The main Hijing configuration fragment that sets up the Hijing algorithm
    and returns a CA instance.
    """

    # By default LEAD beams are assumed.
    projecBeam = kwargs.pop("projecBeam", BeamType.LEAD)
    targetBeam = kwargs.pop("targetBeam", BeamType.LEAD)

    # Basic initialize parameters
    base_init = [f"efrm {2*flags.Beam.Energy/GeV}",
        "frame CMS",
        f"proj {'P' if projecBeam is BeamType.PROTON else 'A'}",
        f"targ {'P' if targetBeam is BeamType.PROTON else 'A'}",
        f"izp {projecBeam.Z}",   f"izt {targetBeam.Z}",
        f"iap {projecBeam.A}",   f"iat {targetBeam.A}"
    ]

    # Get user initialize parameters
    user_init = kwargs.pop("Initialize", None)
    kwargs["Initialize"] = GeneratorSettingsLayer(
        source="base_initialize",
        values=tuple(base_init),
        precedence=GeneratorSettingsPrecedence.BASE,
        report_context="Hijing.Initialize"
    )

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.Hijing(name, **kwargs)
    )

    # Add user initialize parameters
    if user_init:
        ca.merge(HijingInitializeCfg(
            flags,
            source="user_initialize",
            values=user_init,
            precedence=GeneratorSettingsPrecedence.USER,
            name=name
        ))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Hijing"]), sequenceName=EvgenSequence.Generator.value)

    return ca



