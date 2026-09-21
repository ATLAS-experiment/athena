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
    "Tune:ee = 7", "Tune:pp = 14", "PDF:pSet = LHAPDF6:cteq6l1",
    "SpaceShower:rapidityOrder = on",
    "SigmaProcess:alphaSvalue = 0.144", "SpaceShower:pT0Ref = 1.30",
    "SpaceShower:pTmaxFudge = 0.95", "SpaceShower:pTdampFudge = 1.21",
    "SpaceShower:alphaSvalue = 0.125", "TimeShower:alphaSvalue = 0.126",
    "BeamRemnants:primordialKThard = 1.72",
    "MultipartonInteractions:pT0Ref = 1.98",
    "MultipartonInteractions:alphaSvalue = 0.118",
    "ColourReconnection:range = 2.08",
)


def _commands_cfg(flags, source, commands, precedence, name="Pythia8B"):
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


def _tune_cfg(flags, commands, source, tune, name="Pythia8B", **kwargs):
    user_commands = kwargs.pop("Commands", ())
    ca = Pythia8BBaseCfg(flags, name=name, **kwargs)
    ca.merge(_commands_cfg(
        flags, source, commands, GeneratorSettingsPrecedence.TUNE, name,
    ))
    if user_commands:
        ca.merge(_commands_cfg(
            flags, "user_commands", user_commands,
            GeneratorSettingsPrecedence.USER, name,
        ))

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune=tune),
             sequenceName=EvgenSequence.Generator.value)
    return ca


def Pythia8B_A14_CTEQ6L1_Common_Cfg(flags, name="Pythia8B", **kwargs):
    from Pythia8_i.Pythia8Config import ensureRapidityOrderMPI
    return _tune_cfg(
        flags, ensureRapidityOrderMPI(list(_A14_CTEQ6L1_COMMANDS)),
        "pythia8b_tune_A14_CTEQ6L1", "A14 CTEQ6L1", name, **kwargs,
    )


def Pythia8B_A14_NNPDF23LO_Common_Cfg(flags, name="Pythia8B", **kwargs):
    from Pythia8_i.Pythia8Config import ensureRapidityOrderMPI
    from Pythia8_i.Pythia8Tunes import a14_nnpdf23lo_tune_cmds
    return _tune_cfg(
        flags, ensureRapidityOrderMPI(a14_nnpdf23lo_tune_cmds()),
        "pythia8b_tune_A14_NNPDF23LO", "A14 NNPDF23LO", name, **kwargs,
    )


def Pythia8BEvtGenCfg(flags, **kwargs):
    """Configure EvtGen with the Pythia8B particle data table."""
    pdt_file = kwargs.setdefault("pdtFile", "inclusiveP8DsDPlus.pdt")
    auxfiles = list(kwargs.pop("auxfiles", ()))
    if pdt_file not in auxfiles:
        auxfiles.append(pdt_file)
    user_decay_file = kwargs.get("userDecayFile")
    if user_decay_file and user_decay_file not in auxfiles:
        auxfiles.append(user_decay_file)
    white_list = list(kwargs.pop("whiteList", ())) + [-5334, 5334]

    from EvtGen_i.EvtGenConfig import EvtGenCfg
    return EvtGenCfg(
        flags, whiteList=white_list, auxfiles=auxfiles, **kwargs,
    )


def _evtgen_tune_cfg(tune_cfg, flags, evtgen_options, **kwargs):
    ca = tune_cfg(flags, **kwargs)
    ca.merge(Pythia8BEvtGenCfg(flags, **(evtgen_options or {})))
    return ca


def Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg(
        flags, *, EvtGenOptions=None, **kwargs):
    return _evtgen_tune_cfg(
        Pythia8B_A14_CTEQ6L1_Common_Cfg, flags, EvtGenOptions, **kwargs,
    )


def Pythia8B_A14_NNPDF23LO_EvtGen_Common_Cfg(
        flags, *, EvtGenOptions=None, **kwargs):
    return _evtgen_tune_cfg(
        Pythia8B_A14_NNPDF23LO_Common_Cfg, flags, EvtGenOptions, **kwargs,
    )


def Pythia8B_Photospp_Cfg(
        flags, *, ShowerCfg, PhotosppOptions=None, **kwargs):
    """Disable native lepton QED showering and append Photos++."""
    from Pythia8B_i.Pythia8BProcesses import add_process_commands
    ca = add_process_commands(
        flags, ShowerCfg=ShowerCfg,
        commands=("TimeShower:QEDshowerByL = off",), **kwargs,
    )

    from Photospp_i.PhotosppConfig import PhotosppCfg
    ca.merge(PhotosppCfg(flags, **(PhotosppOptions or {})))
    return ca
