# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Pythia8B production and decay-selection configurations."""

_B_PARTICLE_PDG_CODES = (511, 521, 531, 541, 5122, 5132, 5232, 5332)
_B_PDG_CODES = _B_PARTICLE_PDG_CODES + tuple(
    -pdg for pdg in _B_PARTICLE_PDG_CODES
)
_CLOSE_B_DECAYS = (
    "511:onMode = 3", "521:onMode = 3", "531:onMode = 3",
    "541:onMode = 3", "5122:onMode = 2", "5132:onMode = 2",
    "5232:onMode = 2", "5332:onMode = 2",
)
_CLOSE_ANTI_B_DECAYS = (
    "511:onMode = 2", "521:onMode = 2", "531:onMode = 2",
    "541:onMode = 2", "5122:onMode = 3", "5132:onMode = 3",
    "5232:onMode = 3", "5332:onMode = 3",
)
_CHARMONIUM_PDG_CODES = (443, 100443, 445, 10441, 10443, 20443)
_OPEN_B_JPSI_DECAYS = tuple(
    f"{pdg}:onIfAny = {' '.join(map(str, _CHARMONIUM_PDG_CODES))}"
    for pdg in _B_PARTICLE_PDG_CODES
)
_HARD_B_COMMANDS = (
    "HardQCD:all = on",
    "ParticleDecays:mixB = off",
    "HadronLevel:all = off",
)


def add_process_commands(flags, *, ShowerCfg, commands, **kwargs):
    """Collect nested process commands into one MATCHING layer."""
    outer_commands = tuple(kwargs.pop("_process_commands", ()))
    return ShowerCfg(
        flags, _process_commands=tuple(commands) + outer_commands, **kwargs,
    )


def _set_b_defaults(kwargs):
    kwargs.setdefault("SelectBQuarks", True)
    kwargs.setdefault("SelectCQuarks", False)
    kwargs.setdefault("VetoDoubleBEvents", True)
    kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))


def _set_quarkonium_defaults(kwargs):
    defaults = {
        "SuppressSmallPT": True, "pT0timesMPI": 1.0, "numberAlphaS": 3.0,
        "useSameAlphaSasMPI": False, "SelectBQuarks": False,
        "SelectCQuarks": False, "VetoDoubleBEvents": False,
        "VetoDoubleCEvents": False, "NHadronizationLoops": 1,
    }
    for key, value in defaults.items():
        kwargs.setdefault(key, value)


def _b_process_cfg(flags, *, ShowerCfg, decay_commands, **kwargs):
    _set_b_defaults(kwargs)
    return add_process_commands(
        flags, ShowerCfg=ShowerCfg,
        commands=_HARD_B_COMMANDS + decay_commands, **kwargs,
    )


def Pythia8B_exclusiveB_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    return _b_process_cfg(
        flags, ShowerCfg=ShowerCfg, decay_commands=_CLOSE_B_DECAYS, **kwargs,
    )


def Pythia8B_exclusiveAntiB_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    return _b_process_cfg(
        flags, ShowerCfg=ShowerCfg,
        decay_commands=_CLOSE_ANTI_B_DECAYS, **kwargs,
    )


def Pythia8B_inclusiveBJpsi_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    kwargs.setdefault("UserSelection", "BJPSIINCLUSIVE")
    return _b_process_cfg(
        flags, ShowerCfg=ShowerCfg,
        decay_commands=_CLOSE_B_DECAYS + _OPEN_B_JPSI_DECAYS, **kwargs,
    )


def Pythia8B_inclusiveAntiBJpsi_Common_Cfg(
        flags, *, ShowerCfg, **kwargs):
    kwargs.setdefault("UserSelection", "BJPSIINCLUSIVE")
    kwargs.setdefault("UserSelectionVariables", [-1])
    return _b_process_cfg(
        flags, ShowerCfg=ShowerCfg,
        decay_commands=_CLOSE_ANTI_B_DECAYS + _OPEN_B_JPSI_DECAYS, **kwargs,
    )


def _quarkonium_cfg(flags, *, ShowerCfg, process, **kwargs):
    _set_quarkonium_defaults(kwargs)
    if process == "Bottomonium":
        kwargs.setdefault("BPDGCodes", list(_B_PDG_CODES))
    commands = (
        "PhaseSpace:pTHatMin = 1.", "ParticleDecays:mixB = off",
        "HadronLevel:all = off", f"{process}:all = on",
        "PhaseSpace:pTHatMinDiverge = 0.5",
    )
    return add_process_commands(
        flags, ShowerCfg=ShowerCfg, commands=commands, **kwargs,
    )


def Pythia8B_Charmonium_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    return _quarkonium_cfg(
        flags, ShowerCfg=ShowerCfg, process="Charmonium", **kwargs,
    )


def Pythia8B_Bottomonium_Common_Cfg(flags, *, ShowerCfg, **kwargs):
    return _quarkonium_cfg(
        flags, ShowerCfg=ShowerCfg, process="Bottomonium", **kwargs,
    )


def Pythia8B_CloseBDecays_Cfg(flags, *, ShowerCfg, **kwargs):
    return add_process_commands(
        flags, ShowerCfg=ShowerCfg, commands=_CLOSE_B_DECAYS, **kwargs,
    )


def Pythia8B_CloseAntiBDecays_Cfg(flags, *, ShowerCfg, **kwargs):
    return add_process_commands(
        flags, ShowerCfg=ShowerCfg, commands=_CLOSE_ANTI_B_DECAYS, **kwargs,
    )


def Pythia8B_OpenBJpsiDecays_Cfg(flags, *, ShowerCfg, **kwargs):
    return add_process_commands(
        flags, ShowerCfg=ShowerCfg, commands=_OPEN_B_JPSI_DECAYS, **kwargs,
    )
