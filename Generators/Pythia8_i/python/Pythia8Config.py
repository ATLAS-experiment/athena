# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from Pythia8_i.Pythia8Tunes import (
    a14_nnpdf23lo_tune_cmds,
    a2_mstw2008lo_tune_cmds,
)
from AthenaCommon.SystemOfUnits import GeV

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("Pythia8Config")


def Pythia8CommandsCfg(flags, source, commands, precedence, name="Pythia8_i"):
    """
    Return a CA fragment that adds one command layer to Pythia8_i.
    """
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(
        CompFactory.Pythia8_i(name, Commands=GeneratorSettingsLayer(
            source=source,
            values=tuple(commands or ()),
            precedence=precedence,
            report_context="Pythia8Cfg.Commands",
        ))
    )
    return ca


def Pythia8BaseCfg(flags, name="Pythia8_i", **kwargs):
    """
    The main Pythia8 configuration fragment that sets up the Pythia8_i algorithm
    and returns a CA instance
    """
    # Baseline P8 settings
    base_cmds = [
        "Main:timesAllowErrors = 500",
        "ParticleDecays:limitTau0 = on",
        "ParticleDecays:tau0Max = 10.0"
    ]

    # Collision energy
    if "CollisionEnergy" not in kwargs:
        kwargs["CollisionEnergy"] = flags.Beam.Energy * 2 / GeV

    # Random Seed and DSID
    kwargs.setdefault("RandomSeed", flags.Random.SeedOffset)
    kwargs.setdefault("Dsid", flags.Generator.DSID)

    # Load basic parameters
    base_cmds.extend([
        "6:m0 = 172.5",
        "23:m0 = 91.1876",
        "23:mWidth = 2.4952",
        "24:m0 = 80.399",
        "24:mWidth = 2.085",
        "StandardModel:sin2thetaW = 0.23113",
        "StandardModel:sin2thetaWbar = 0.23146",
    ])

    user_cmds = kwargs.pop("Commands", None)
    kwargs["Commands"] = GeneratorSettingsLayer(
        source="base_fragment_commands",
        values=tuple(base_cmds),
        precedence=GeneratorSettingsPrecedence.BASE,
        report_context="Pythia8Cfg.Commands",
    )

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.Pythia8_i(name, **kwargs)
    )

    # Add the user commands
    if user_cmds:
        ca.merge(Pythia8CommandsCfg(
            flags,
            source="user_commands",
            commands=user_cmds,
            precedence=GeneratorSettingsPrecedence.USER,
            name=name,
        ))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Pythia8"]), sequenceName=EvgenSequence.Generator.value)

    return ca


def Pythia8EvtGenBaseCfg(flags, **kwargs):
    """
    Fragment for setting up EvtGen on top of Pythia 8
    """
    # Custom settings
    auxfiles = ["inclusiveP8DsDPlus.pdt"]
    # FHerwig has problems with omega b* (5334), so not present in the base EvtGen fragment.
    whiteList = [-5334, 5334] 
    
    # Add custom EvtGenCfg
    from EvtGen_i.EvtGenConfig import EvtGenCfg
    ca = EvtGenCfg(
        flags,
        whiteList = whiteList,
        auxfiles = auxfiles
    )

    return ca


def Pythia8_A2_MSTW2008LO_Common_Cfg(flags, **kwargs):
    """
    Fragment for setting up A2 MSTW2008LO tune
    """

    # Remove any user command before calling base config
    user_cmds = list(kwargs.pop("Commands", []))

    # Get the base config
    ca = Pythia8BaseCfg(flags, **kwargs)
    
    # Get the tune commands and apply rapidity ordering
    tune_cmds = a2_mstw2008lo_tune_cmds()
    tune_cmds = ensureRapidityOrderMPI(tune_cmds)

    # Add the tune commands
    ca.merge(Pythia8CommandsCfg(
        flags,
        source="pythia_tune_A2_MSTW2008LO",
        commands=tune_cmds,
        precedence=GeneratorSettingsPrecedence.TUNE,
    ))

    # Now apply the user commands
    if user_cmds:
        ca.merge(Pythia8CommandsCfg(
            flags,
            source="job_options",
            commands=user_cmds,
            precedence=GeneratorSettingsPrecedence.USER,
        ))

    # Broadcast tune to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune="A2 MSTW2008LO"), sequenceName=EvgenSequence.Generator.value)

    # Call the base config
    return ca


def Pythia8_A14_NNPDF23LO_Common_Cfg(flags, **kwargs):
    """
    Fragment for setting up A14 tune with NNPDF23LO PDF
    """

    # Remove any user command before calling base config
    user_cmds = list(kwargs.pop("Commands", []))

    # Get the base config
    ca = Pythia8BaseCfg(flags, **kwargs)

    # Get the tune commands and apply rapidity ordering
    tune_cmds = a14_nnpdf23lo_tune_cmds()
    tune_cmds = ensureRapidityOrderMPI(tune_cmds)

    # Add the tune commands
    ca.merge(Pythia8CommandsCfg(
        flags,
        source="pythia_tune_A14_NNPDF23LO",
        commands=tune_cmds,
        precedence=GeneratorSettingsPrecedence.TUNE,
    ))

    # Now apply the user commands
    if user_cmds:
        ca.merge(Pythia8CommandsCfg(
            flags,
            source="job_options",
            commands=user_cmds,
            precedence=GeneratorSettingsPrecedence.USER,
        ))

    # Broadcast tune to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune="A14 NNPDF23LO"), sequenceName=EvgenSequence.Generator.value)

    # Call the base config
    return ca


def Pythia8_A2_MSTW2008LO_EvtGen_Common_Cfg(flags, **kwargs):
    """
    Config for Py8 tune A2 with MSTW2008LO tune
    The default version of this includes EvtGen for standardised b fragmentation
    This tune is generally only used for pile up samples at the start of run 2 
    for high pT physics at the start of run 2 the A14 tune is more appropriate.  
    There are also more recent soft QCD tunes, such as Monash, 
    but A2 was a conservative choice for initial 13 TeV pile up
    """

    # Add Pythia 8 to CA with correct tune settings
    ca = Pythia8_A2_MSTW2008LO_Common_Cfg(flags, **kwargs)

    # Add EvtGen
    ca.merge(Pythia8EvtGenBaseCfg(flags, **kwargs))

    return ca


def Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg(flags, **kwargs):
    """
    Config for setting up Py8 with A14 tune 
    with EvtGen
    """

    # Add Pythia 8 to CA with correct tune settings
    ca = Pythia8_A14_NNPDF23LO_Common_Cfg(flags, **kwargs)

    # Add EvtGen
    ca.merge(Pythia8EvtGenBaseCfg(flags, **kwargs))

    return ca


def ensureRapidityOrderMPI(cmds):
    """
    A function that ensures rapidity ordering is set
    """
    # if MPI‐ordering already explicitly set, do nothing
    if any("SpaceShower:rapidityOrderMPI" in c for c in cmds):
        return cmds

    # find the first rapidityOrder value
    for c in cmds:
        if "SpaceShower:rapidityOrder" in c and "MPI" not in c:
            val = c.split("=", 1)[-1].strip()
            cmds.append(f"SpaceShower:rapidityOrderMPI = {val}")
            break
    return cmds


def Pythia8_MadGraph_Cfg(flags, ShowerCfg=Pythia8BaseCfg, **kwargs):
    """
    Modular fragment for MadGraph LHE input in Pythia8.
    The Pythia8_i algorithm is configured through ShowerCfg (defaults to
    Pythia8BaseCfg) so tune/EvtGen fragments can be injected without
    instantiating Pythia8_i twice.
    """
    # Match Pythia8's input name to the file prepared by EvgenHelpers.
    # This can still be overridden by setting in the config LHEFile="myfile.lhe[.gz]".
    lhe_file = (
        "events.lhe.gz"
        if flags.Generator.avoidExtracting
        else "events.lhe"
    )
    kwargs.setdefault("LHEFile", lhe_file)

    # Configure Pythia8 through the selected shower fragment.
    ca = ShowerCfg(flags, **kwargs)

    # Announce MadGraph to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["MadGraph"]), sequenceName=EvgenSequence.Generator.value)

    return ca
