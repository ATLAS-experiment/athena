# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""ComponentAccumulator configuration fragments for Pythia8B."""

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsKeep,
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


_B_PARTICLE_PDG_CODES = (511, 521, 531, 541, 5122, 5132, 5232, 5332)
_B_PDG_CODES = _B_PARTICLE_PDG_CODES + tuple(-pdg for pdg in _B_PARTICLE_PDG_CODES)

_CLOSE_B_DECAY_COMMANDS = (
    "511:onMode = 3",
    "521:onMode = 3",
    "531:onMode = 3",
    "541:onMode = 3",
    "5122:onMode = 2",
    "5132:onMode = 2",
    "5232:onMode = 2",
    "5332:onMode = 2",
)

_CLOSE_ANTI_B_DECAY_COMMANDS = (
    "511:onMode = 2",
    "521:onMode = 2",
    "531:onMode = 2",
    "541:onMode = 2",
    "5122:onMode = 3",
    "5132:onMode = 3",
    "5232:onMode = 3",
    "5332:onMode = 3",
)

_CHARMONIUM_PDG_CODES = (443, 100443, 445, 10441, 10443, 20443)

_OPEN_B_JPSI_DECAY_COMMANDS = tuple(
    f"{b_hadron}:onIfAny = {' '.join(map(str, _CHARMONIUM_PDG_CODES))}"
    for b_hadron in _B_PARTICLE_PDG_CODES
)


def _command_tuple(commands):
    """Copy a command sequence without interpreting Pythia operations."""
    if isinstance(commands, str):
        raise TypeError("Commands must be a sequence of strings, not a string")
    commands = tuple(commands or ())
    if any(not isinstance(command, str) for command in commands):
        raise TypeError("Commands must contain only strings")
    return commands


def _command_layer(source, commands, precedence):
    return GeneratorSettingsLayer(
        source=source,
        values=_command_tuple(commands),
        precedence=precedence,
        keep=GeneratorSettingsKeep.ALL,
        report_context="Pythia8BCfg.Commands",
    )


def Pythia8BCommandsCfg(flags, source, commands, precedence, name="Pythia8B"):
    """Add an ordered command layer; each precedence may occur only once."""
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(
        CompFactory.Pythia8B_i(
            name, Commands=_command_layer(source, commands, precedence)
        )
    )
    return ca


def Pythia8BBaseCfg(flags, name="Pythia8B", **kwargs):
    """Configure the Pythia8B algorithm with the ATLAS base settings."""
    base_commands = (
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

    kwargs.setdefault("CollisionEnergy", flags.Beam.Energy * 2 / GeV)
    kwargs.setdefault("RandomSeed", flags.Random.SeedOffset)
    kwargs.setdefault("Dsid", flags.Generator.DSID)

    user_commands = _command_tuple(kwargs.pop("Commands", ()))
    process_commands = _command_tuple(kwargs.pop("_process_commands", ()))
    kwargs["Commands"] = _command_layer(
        "base_fragment_commands", base_commands, GeneratorSettingsPrecedence.BASE
    )

    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Pythia8B_i(name, **kwargs))

    if process_commands:
        ca.merge(Pythia8BCommandsCfg(
            flags,
            source="process_commands",
            commands=process_commands,
            precedence=GeneratorSettingsPrecedence.PROCESS,
            name=name,
        ))

    if user_commands:
        ca.merge(
            Pythia8BCommandsCfg(
                flags,
                source="user_commands",
                commands=user_commands,
                precedence=GeneratorSettingsPrecedence.USER,
                name=name,
            )
        )

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg

    ca.merge(
        GeneratorInfoSvcCfg(flags, Generators=["Pythia8B"]),
        sequenceName=EvgenSequence.Generator.value,
    )
    return ca


def _Pythia8BTuneCfg(flags, commands, source, tune, name="Pythia8B", **kwargs):
    """Configure a tune between the base and user command layers."""
    ca = Pythia8BBaseCfg(flags, name=name, **kwargs)
    ca.merge(
        Pythia8BCommandsCfg(
            flags,
            source=source,
            commands=commands,
            precedence=GeneratorSettingsPrecedence.TUNE,
            name=name,
        )
    )

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg

    ca.merge(
        GeneratorInfoSvcCfg(flags, Tune=tune),
        sequenceName=EvgenSequence.Generator.value,
    )
    return ca


def Pythia8B_A14_CTEQ6L1_Common_Cfg(flags, name="Pythia8B", **kwargs):
    """Configure the Pythia8B A14 CTEQ6L1 tune."""
    commands = [
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
    ]

    from Pythia8_i.Pythia8Config import ensureRapidityOrderMPI

    return _Pythia8BTuneCfg(
        flags,
        commands=ensureRapidityOrderMPI(commands),
        source="pythia8b_tune_A14_CTEQ6L1",
        tune="A14 CTEQ6L1",
        name=name,
        **kwargs,
    )


def Pythia8B_A14_NNPDF23LO_Common_Cfg(flags, name="Pythia8B", **kwargs):
    """Configure the Pythia8B A14 NNPDF23LO tune."""
    commands = [
        "Tune:ee = 7",
        "Tune:pp = 14",
        "PDF:pSet = LHAPDF6:NNPDF23_lo_as_0130_qed",
        "SpaceShower:rapidityOrder = on",
        "SigmaProcess:alphaSvalue = 0.140",
        "SpaceShower:pT0Ref = 1.56",
        "SpaceShower:pTmaxFudge = 0.91",
        "SpaceShower:pTdampFudge = 1.05",
        "SpaceShower:alphaSvalue = 0.127",
        "TimeShower:alphaSvalue = 0.127",
        "BeamRemnants:primordialKThard = 1.88",
        "MultipartonInteractions:pT0Ref = 2.09",
        "MultipartonInteractions:alphaSvalue = 0.126",
        "ColourReconnection:range = 1.71",
    ]

    from Pythia8_i.Pythia8Config import ensureRapidityOrderMPI

    return _Pythia8BTuneCfg(
        flags,
        commands=ensureRapidityOrderMPI(commands),
        source="pythia8b_tune_A14_NNPDF23LO",
        tune="A14 NNPDF23LO",
        name=name,
        **kwargs,
    )


def Pythia8BEvtGenCfg(flags, name="EvtInclusiveDecay", **kwargs):
    """Configure EvtGen; kwargs are EvtGen properties, including userDecayFile."""
    from EvtGen_i.EvtGenConfig import EvtGenCfg

    kwargs.setdefault("pdtFile", "inclusiveP8DsDPlus.pdt")
    kwargs.setdefault("whiteList", [-5334, 5334])
    return EvtGenCfg(flags, name=name, **kwargs)


def Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg(
    flags, name="Pythia8B", *, EvtGenOptions=None, **kwargs
):
    """Configure Pythia8B with the A14 CTEQ6L1 tune and EvtGen."""
    ca = Pythia8B_A14_CTEQ6L1_Common_Cfg(flags, name=name, **kwargs)
    ca.merge(Pythia8BEvtGenCfg(flags, **(EvtGenOptions or {})))
    return ca


def Pythia8B_A14_NNPDF23LO_EvtGen_Common_Cfg(
    flags, name="Pythia8B", *, EvtGenOptions=None, **kwargs
):
    """Configure Pythia8B with the A14 NNPDF23LO tune and EvtGen."""
    ca = Pythia8B_A14_NNPDF23LO_Common_Cfg(flags, name=name, **kwargs)
    ca.merge(Pythia8BEvtGenCfg(flags, **(EvtGenOptions or {})))
    return ca


def _Pythia8BProcessCfg(flags, *, ShowerCfg, commands, name="Pythia8B", **kwargs):
    """Collect nested fragments into one layer, inner commands before outer ones.

    The selected ShowerCfg must ultimately call Pythia8BBaseCfg. Its private
    _process_commands argument is consumed there, never passed to the algorithm.
    """
    outer_commands = _command_tuple(kwargs.pop("_process_commands", ()))
    return ShowerCfg(
        flags,
        name=name,
        _process_commands=_command_tuple(commands) + outer_commands,
        **kwargs,
    )


def _set_b_selection_defaults(kwargs):
    """Set properties shared by exclusive and inclusive B configurations."""
    kwargs.setdefault("SelectBQuarks", True)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", True)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))


def _set_quarkonium_defaults(kwargs):
    """Set the shared direct-quarkonium hook and selection defaults."""
    kwargs.setdefault("SuppressSmallPT", True)
    kwargs.setdefault("pT0timesMPI", 1.0)
    kwargs.setdefault("numberAlphaS", 3.0)
    kwargs.setdefault("useSameAlphaSasMPI", False)
    kwargs.setdefault("SelectBQuarks", False)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", False)
    kwargs.setdefault("VetoDoubleCEvents", False)
    kwargs.setdefault("NHadronizationLoops", 1)


def Pythia8B_Bottomonium_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure direct bottomonium production."""
    _set_quarkonium_defaults(kwargs)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))

    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "PhaseSpace:pTHatMin = 1.",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            "Bottomonium:all = on",
            "PhaseSpace:pTHatMinDiverge = 0.5",
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_Charmonium_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure direct charmonium production."""
    _set_quarkonium_defaults(kwargs)

    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "PhaseSpace:pTHatMin = 1.",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            "Charmonium:all = on",
            "PhaseSpace:pTHatMinDiverge = 0.5",
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_CloseBDecays_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Disable anti-B decays while retaining B-hadron decays."""
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=_CLOSE_B_DECAY_COMMANDS,
        name=name,
        **kwargs,
    )


def Pythia8B_CloseAntiBDecays_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Disable B-hadron decays while retaining anti-B decays."""
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=_CLOSE_ANTI_B_DECAY_COMMANDS,
        name=name,
        **kwargs,
    )


def Pythia8B_OpenBJpsiDecays_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Enable B-hadron decays containing a charmonium state."""
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=_OPEN_B_JPSI_DECAY_COMMANDS,
        name=name,
        **kwargs,
    )


def Pythia8B_exclusiveB_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure exclusive B-hadron production."""
    _set_b_selection_defaults(kwargs)
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "HardQCD:all = on",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            *_CLOSE_B_DECAY_COMMANDS,
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_exclusiveAntiB_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure exclusive anti-B-hadron production."""
    _set_b_selection_defaults(kwargs)
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "HardQCD:all = on",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            *_CLOSE_ANTI_B_DECAY_COMMANDS,
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_inclusiveBJpsi_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure inclusive B-hadron to charmonium production."""
    _set_b_selection_defaults(kwargs)
    kwargs.setdefault("UserSelection", "BJPSIINCLUSIVE")
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "HardQCD:all = on",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            *_CLOSE_B_DECAY_COMMANDS,
            *_OPEN_B_JPSI_DECAY_COMMANDS,
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_inclusiveAntiBJpsi_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure inclusive anti-B-hadron to charmonium production."""
    _set_b_selection_defaults(kwargs)
    kwargs.setdefault("UserSelection", "BJPSIINCLUSIVE")
    kwargs.setdefault("UserSelectionVariables", [-1])
    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=(
            "HardQCD:all = on",
            "ParticleDecays:mixB = off",
            "HadronLevel:all = off",
            *_CLOSE_ANTI_B_DECAY_COMMANDS,
            *_OPEN_B_JPSI_DECAY_COMMANDS,
        ),
        name=name,
        **kwargs,
    )


def Pythia8B_Photospp_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", PhotosppOptions=None, **kwargs
):
    """Append Photos++ after the shower (and EvtGen, when configured).

    Disable native lepton QED showering as in the legacy Pythia8B fragment.
    Explicit user Commands still execute last and can override this default.
    """
    from Photospp_i.PhotosppConfig import PhotosppCfg

    ca = _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        commands=("TimeShower:QEDshowerByL = off",),
        name=name,
        **kwargs,
    )
    ca.merge(PhotosppCfg(flags, **(PhotosppOptions or {})))
    return ca
