# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""ComponentAccumulator configuration fragments for Pythia8B."""

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


_B_PDG_CODES = (
    511, 521, 531, 541, 5122, 5132, 5232, 5332,
    -511, -521, -531, -541, -5122, -5132, -5232, -5332,
)

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
    f"{b_hadron}:onIfAny = {charmonium}"
    for b_hadron in _B_PDG_CODES[:8]
    for charmonium in _CHARMONIUM_PDG_CODES
)


def Pythia8BCommandsCfg(flags, source, commands, precedence, name="Pythia8B"):
    """Return a CA fragment containing one Pythia8B command layer."""
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(
        CompFactory.Pythia8B_i(
            name,
            Commands=GeneratorSettingsLayer(
                source=source,
                values=tuple(commands or ()),
                precedence=precedence,
                report_context="Pythia8BCfg.Commands",
            ),
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

    user_commands = tuple(kwargs.pop("Commands", ()) or ())
    kwargs["Commands"] = GeneratorSettingsLayer(
        source="base_fragment_commands",
        values=base_commands,
        precedence=GeneratorSettingsPrecedence.BASE,
        report_context="Pythia8BCfg.Commands",
    )

    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(CompFactory.Pythia8B_i(name, **kwargs))

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
    user_commands = tuple(kwargs.pop("Commands", ()) or ())

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

    if user_commands:
        ca.merge(
            Pythia8BCommandsCfg(
                flags,
                source="job_options",
                commands=user_commands,
                precedence=GeneratorSettingsPrecedence.USER,
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


def Pythia8BEvtGenCfg(flags):
    """Configure the Pythia8-compatible EvtGen afterburner."""
    from Pythia8_i.Pythia8Config import Pythia8EvtGenBaseCfg

    return Pythia8EvtGenBaseCfg(flags)


def Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg(
    flags, name="Pythia8B", **kwargs
):
    """Configure Pythia8B with the A14 CTEQ6L1 tune and EvtGen."""
    ca = Pythia8B_A14_CTEQ6L1_Common_Cfg(flags, name=name, **kwargs)
    ca.merge(Pythia8BEvtGenCfg(flags))
    return ca


def Pythia8B_A14_NNPDF23LO_EvtGen_Common_Cfg(
    flags, name="Pythia8B", **kwargs
):
    """Configure Pythia8B with the A14 NNPDF23LO tune and EvtGen."""
    ca = Pythia8B_A14_NNPDF23LO_Common_Cfg(flags, name=name, **kwargs)
    ca.merge(Pythia8BEvtGenCfg(flags))
    return ca


def _Pythia8BProcessCfg(
    flags,
    *,
    ShowerCfg,
    source,
    commands,
    name="Pythia8B",
    **kwargs,
):
    """Apply process settings after the tune and before job-option commands."""
    user_commands = tuple(kwargs.pop("Commands", ()) or ())

    ca = ShowerCfg(flags, name=name, **kwargs)
    ca.merge(
        Pythia8BCommandsCfg(
            flags,
            source=source,
            commands=commands,
            precedence=GeneratorSettingsPrecedence.PROCESS,
            name=name,
        )
    )

    if user_commands:
        ca.merge(
            Pythia8BCommandsCfg(
                flags,
                source="job_options",
                commands=user_commands,
                precedence=GeneratorSettingsPrecedence.USER,
                name=name,
            )
        )
    return ca


def _set_b_selection_defaults(kwargs):
    """Set properties shared by exclusive and inclusive B configurations."""
    kwargs.setdefault("SelectBQuarks", True)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", True)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))


def Pythia8B_Bottomonium_Common_Cfg(
    flags, *, ShowerCfg, name="Pythia8B", **kwargs
):
    """Configure direct bottomonium production."""
    kwargs.setdefault("SuppressSmallPT", True)
    kwargs.setdefault("pT0timesMPI", 1.0)
    kwargs.setdefault("numberAlphaS", 3.0)
    kwargs.setdefault("useSameAlphaSasMPI", False)
    kwargs.setdefault("SelectBQuarks", False)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", False)
    kwargs.setdefault("VetoDoubleCEvents", False)
    kwargs.setdefault("NHadronizationLoops", 1)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))

    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        source="pythia8b_bottomonium",
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
    kwargs.setdefault("SuppressSmallPT", True)
    kwargs.setdefault("pT0timesMPI", 1.0)
    kwargs.setdefault("numberAlphaS", 3.0)
    kwargs.setdefault("useSameAlphaSasMPI", False)
    kwargs.setdefault("SelectBQuarks", False)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", False)
    kwargs.setdefault("VetoDoubleCEvents", False)
    kwargs.setdefault("NHadronizationLoops", 1)

    return _Pythia8BProcessCfg(
        flags,
        ShowerCfg=ShowerCfg,
        source="pythia8b_charmonium",
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
        source="pythia8b_close_b_decays",
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
        source="pythia8b_close_anti_b_decays",
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
        source="pythia8b_open_b_jpsi_decays",
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
        source="pythia8b_exclusive_b",
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
        source="pythia8b_exclusive_anti_b",
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
        source="pythia8b_inclusive_b_jpsi",
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
        source="pythia8b_inclusive_anti_b_jpsi",
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
