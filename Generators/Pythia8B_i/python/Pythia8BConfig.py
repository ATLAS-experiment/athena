# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""ComponentAccumulator configuration for Pythia8B."""

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


_BASE_COMMANDS = (
    "Main:timesAllowErrors = 500",
    "6:m0 = 172.5",
    "23:m0 = 91.1876",
    "23:mWidth = 2.4952",
    "24:m0 = 80.399",
    "24:mWidth = 2.085",
    "StandardModel:sin2thetaW = 0.23113",
    "StandardModel:sin2thetaWbar = 0.23146",
    "ParticleDecays:limitTau0 = on",
    "ParticleDecays:tau0Max = 10.0",
)

_A14_CTEQ6L1_COMMANDS = (
    "Tune:ee = 7",
    "Tune:pp = 14",
    "PDF:pSet = LHAPDF6:cteq6l1",
    "SpaceShower:rapidityOrder = on",
    "SigmaProcess:alphaSvalue = 0.144",
    "SpaceShower:pT0Ref = 1.30",
    "SpaceShower:pTmaxFudge = 0.95",
    "SpaceShower:pTdampFudge = 1.21",
    "SpaceShower:alphaSvalue = 0.125",
    "TimeShower:alphaSvalue = 0.126",
    "BeamRemnants:primordialKThard = 1.72",
    "MultipartonInteractions:pT0Ref = 1.98",
    "MultipartonInteractions:alphaSvalue = 0.118",
    "ColourReconnection:range = 2.08",
)

_EXCLUSIVE_B_COMMANDS = (
    "HardQCD:all = on",
    "ParticleDecays:mixB = off",
    "HadronLevel:all = off",
    "511:onMode = 3",
    "521:onMode = 3",
    "531:onMode = 3",
    "541:onMode = 3",
    "5122:onMode = 2",
    "5132:onMode = 2",
    "5232:onMode = 2",
    "5332:onMode = 2",
)

_B_PARTICLE_PDG_CODES = (511, 521, 531, 541, 5122, 5132, 5232, 5332)
_B_PDG_CODES = _B_PARTICLE_PDG_CODES + tuple(
    -pdg for pdg in _B_PARTICLE_PDG_CODES
)


def _commands_cfg(flags, source, commands, precedence, name):
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Pythia8B_i(
        name,
        Commands=GeneratorSettingsLayer(
            source=source,
            values=tuple(commands),
            precedence=precedence,
            report_context="Pythia8BCfg.Commands",
        ),
    ))
    return ca


def _with_process_commands(flags, *, ShowerCfg, commands, **kwargs):
    """Collect nested fragments into one layer, with inner commands first.

    The resolver accepts only one distinct layer at each precedence. Pass the
    private _process_commands argument down to the base fragment so production
    and Photos settings share a layer while Commands remains the user layer.
    """
    pending_commands = tuple(kwargs.pop("_process_commands", ()))
    return ShowerCfg(
        flags,
        _process_commands=tuple(commands) + pending_commands,
        **kwargs,
    )


def Pythia8BBaseCfg(flags, name="Pythia8B", **kwargs):
    """Configure Pythia8B with the standard generator settings."""
    user_commands = kwargs.pop("Commands", ())
    process_commands = kwargs.pop("_process_commands", ())
    kwargs.setdefault("CollisionEnergy", flags.Beam.Energy * 2 / GeV)
    kwargs.setdefault("RandomSeed", flags.Random.SeedOffset)
    kwargs.setdefault("Dsid", flags.Generator.DSID)
    kwargs["Commands"] = GeneratorSettingsLayer(
        source="base_fragment_commands",
        values=_BASE_COMMANDS,
        precedence=GeneratorSettingsPrecedence.BASE,
        report_context="Pythia8BCfg.Commands",
    )

    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Pythia8B_i(name, **kwargs))
    if process_commands:
        # MATCHING places process/decay settings above the tune and below user
        # overrides; the shared resolver has no separate PROCESS precedence.
        ca.merge(_commands_cfg(
            flags, "process_commands", process_commands,
            GeneratorSettingsPrecedence.MATCHING, name,
        ))
    if user_commands:
        ca.merge(_commands_cfg(
            flags, "user_commands", user_commands,
            GeneratorSettingsPrecedence.USER, name,
        ))

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Pythia8B"]),
             sequenceName=EvgenSequence.Generator.value)
    return ca


def Pythia8B_A14_CTEQ6L1_Common_Cfg(flags, name="Pythia8B", **kwargs):
    """Configure Pythia8B with the A14 CTEQ6L1 tune."""
    ca = Pythia8BBaseCfg(flags, name=name, **kwargs)

    from Pythia8_i.Pythia8Config import ensureRapidityOrderMPI
    tune_commands = ensureRapidityOrderMPI(list(_A14_CTEQ6L1_COMMANDS))
    ca.merge(_commands_cfg(
        flags, "pythia8b_tune_A14_CTEQ6L1", tune_commands,
        GeneratorSettingsPrecedence.TUNE, name,
    ))

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune="A14 CTEQ6L1"),
             sequenceName=EvgenSequence.Generator.value)
    return ca


def Pythia8B_exclusiveB_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    """Configure exclusive B-hadron production."""
    kwargs.setdefault("SelectBQuarks", True)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", True)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))
    return _with_process_commands(
        flags, ShowerCfg=ShowerCfg,
        commands=_EXCLUSIVE_B_COMMANDS, **kwargs,
    )


def Pythia8B_Photospp_Cfg(flags, *, ShowerCfg, **kwargs):
    """Disable native lepton QED showering and append Photos++."""
    ca = _with_process_commands(
        flags, ShowerCfg=ShowerCfg,
        commands=("TimeShower:QEDshowerByL = off",), **kwargs,
    )

    from Photospp_i.PhotosppConfig import PhotosppCfg
    ca.merge(PhotosppCfg(flags))
    return ca
