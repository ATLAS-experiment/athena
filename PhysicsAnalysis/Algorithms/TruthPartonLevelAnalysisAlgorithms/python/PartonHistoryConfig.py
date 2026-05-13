# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

KINEMATIC_VARS = ("pt", "eta", "phi", "m")
INT_SUFFIXES = frozenset(("pdgId", "IsOnShell", "origin"))
VECTOR_PREFIXES = ("MC_b_", "MC_bbar_", "MC_c_", "MC_cbar_")
VECTOR_HISTORIES = frozenset(("Ttbarbbbar", "Ttbarccbar"))

def _get_aux_type(branch: str, prefix: str) -> str:
    # Handle specific cases where vectors are returned for each particle type
    # In those cases, only external particles can be vectors, there is still
    # a single particle allowed in named positions within decay chains
    is_vector = prefix in VECTOR_HISTORIES and branch.startswith(VECTOR_PREFIXES) and "_from_" not in branch

    if is_vector:
        return "vector_int" if any(branch.endswith(s) for s in INT_SUFFIXES) else "vector_float"
    else:
        return "int" if any(branch.endswith(s) for s in INT_SUFFIXES) else "float"

def _make_particle_branches(
    prefix: str,
    *,
    include_pdgid: bool = True,
    stages: tuple = None,
    extra: tuple = (),
) -> list:
    """Generate standard kinematic branch names for a particle.

    Parameters
    ----------
    prefix:
        Particle label as it appears in the branch name, e.g. ``"t"`` ->
        ``MC_t_beforeFSR_pt``. When the prefix already encodes the FSR stage
        (e.g. ``"W_beforeFSR_from_t"``), pass ``stages=()``.
    include_pdgid:
        Append ``pdgId`` after the kinematic variables for each stage.
    stages:
        FSR stage tokens to iterate over. Defaults to
        ``("beforeFSR", "afterFSR")``. Pass ``()`` when the FSR stage is
        already embedded in prefix — branches become ``MC_{prefix}_{var}``.
    extra:
        Additional branch suffixes appended verbatim, e.g. ``("IsOnShell",)``.
    """
    if stages is None:
        stages = ("beforeFSR", "afterFSR")
    result = []
    if stages:
        for stage in stages:
            for var in KINEMATIC_VARS:
                result.append(f"MC_{prefix}_{stage}_{var}")
            if include_pdgid:
                result.append(f"MC_{prefix}_{stage}_pdgId")
    else:
        for var in KINEMATIC_VARS:
            result.append(f"MC_{prefix}_{var}")
        if include_pdgid:
            result.append(f"MC_{prefix}_pdgId")
    for suffix in extra:
        result.append(f"MC_{prefix}_{suffix}")
    return result


def _replace_in_list(string_list: list[str], old: str, new: str) -> list[str]:
    return [item.replace(old, new) for item in string_list]


# ---------------------------------------------------------------------------
# Per-particle branch lists
# ---------------------------------------------------------------------------


def _t_branches(suffix: str = "t") -> list:
    """Branches for a top quark and its full decay chain.

    Sub-particles carry the FSR stage in their prefix so we pass ``stages=()``
    to avoid appending a spurious stage token to the variable name.
    For tbar, the b-quark is named 'bbar'.
    """
    b_name = "bbar" if suffix == "tbar" else "b"
    sub_particles = ("W", b_name, "Wdecay1", "Wdecay2")
    result = _make_particle_branches(suffix)
    for particle in sub_particles:
        for stage in ("beforeFSR", "afterFSR"):
            result += _make_particle_branches(
                f"{particle}_{stage}_from_{suffix}", stages=()
            )
    return result


BRANCHES: dict[str, list[str]] = {
    "t": _t_branches("t"),
    "tbar": _t_branches("tbar"),
    "ttbar": (
        _make_particle_branches("ttbar", include_pdgid=False)
        + _make_particle_branches("ttbar_fromDecay", include_pdgid=False)
    ),
    "b": _make_particle_branches("b"),
    "bbar": _make_particle_branches("bbar"),
    "c": _make_particle_branches("c"),
    "cbar": _make_particle_branches("cbar"),
    "Z": (
        _make_particle_branches("Z", extra=("IsOnShell",))
        + _make_particle_branches("Zdecay1")
        + _make_particle_branches("Zdecay2")
    ),
    "Z_extended": (
        _make_particle_branches("Z", extra=("IsOnShell",))
        + _make_particle_branches("Zdecay1")
        + _make_particle_branches("Zdecay1_decay1")
        + _make_particle_branches("Zdecay1_decay2")
        + _make_particle_branches("Zdecay1_decay3")
        + _make_particle_branches("Zdecay2")
        + _make_particle_branches("Zdecay2_decay1")
        + _make_particle_branches("Zdecay2_decay2")
        + _make_particle_branches("Zdecay2_decay3")
    ),
    "W": (
        _make_particle_branches("W", extra=("IsOnShell",))
        + _make_particle_branches("Wdecay1")
        + _make_particle_branches("Wdecay2")
    ),
    "Photon": [
        "MC_gamma_m",
        "MC_gamma_pt",
        "MC_gamma_eta",
        "MC_gamma_phi",
        "MC_gamma_origin",
        "MC_gamma_pdgId",
    ],
    "Higgs": (
        _make_particle_branches("H")
        + _make_particle_branches("Hdecay1")
        + _make_particle_branches("Hdecay2")
        + _make_particle_branches("Hdecay1_decay1")
        + _make_particle_branches("Hdecay1_decay2")
        + _make_particle_branches("Hdecay2_decay1")
        + _make_particle_branches("Hdecay2_decay2")
    ),
}

_TTBAR = BRANCHES["t"] + BRANCHES["tbar"] + BRANCHES["ttbar"]

TRUTH_BRANCHES: dict[str, list[str]] = {
    "Ttbar": _TTBAR,
    "Ttbarbbbar": _TTBAR + BRANCHES["b"] + BRANCHES["bbar"],
    "Ttbarccbar": _TTBAR + BRANCHES["c"] + BRANCHES["cbar"],
    "Ttz": _TTBAR + BRANCHES["Z"],
    "Ttw": _TTBAR + BRANCHES["W"],
    "Tth": _TTBAR + BRANCHES["Higgs"],
    "Ttgamma": _TTBAR + BRANCHES["Photon"],
    "Tzq": BRANCHES["t"] + BRANCHES["Z"] + BRANCHES["b"],
    "Thq": BRANCHES["t"] + BRANCHES["Higgs"] + BRANCHES["b"] + BRANCHES["W"],
    "Tqgamma": BRANCHES["t"] + BRANCHES["Photon"] + BRANCHES["b"],
    "Wtb": BRANCHES["t"] + BRANCHES["W"] + BRANCHES["b"],
    "FourTop": (
        _replace_in_list(BRANCHES["t"], "t_", "t1_")
        + _replace_in_list(BRANCHES["t"], "t_", "t2_")
        + _replace_in_list(BRANCHES["tbar"], "tbar_", "tbar1_")
        + _replace_in_list(BRANCHES["tbar"], "tbar_", "tbar2_")
    ),
    "HWW": BRANCHES["Higgs"],
    "WW_nonresonant": (
        _replace_in_list(BRANCHES["W"], "W", "W1")
        + _replace_in_list(BRANCHES["W"], "W", "W2")
    ),
    "HWW_nonresonant": BRANCHES["Higgs"],
    "HZZ": BRANCHES["Higgs"],
    "Zb": BRANCHES["Z"] + BRANCHES["b"] + BRANCHES["bbar"],
    "Ztautau": BRANCHES["Z_extended"],
}

# ---------------------------------------------------------------------------
# Config block
# ---------------------------------------------------------------------------


class PartonHistoryBlock(ConfigBlock):
    """ConfigBlock for truth/parton-level resonant histories."""

    def __init__(self):
        super().__init__()
        self.addOption(
            "history",
            None,
            type=str,
            required=True,
            info="parton-level interpretation of the MC truth record. Possible values:"
            + ", ".join(sorted(TRUTH_BRANCHES))
            + ".",
        )
        # Always skip on data
        self.setOptionValue("skipOnData", True)

    def makeAlgs(self, config):
        if self.history not in TRUTH_BRANCHES:
            valid = ", ".join(sorted(TRUTH_BRANCHES))
            raise ValueError(
                f"Unknown parton history '{self.history}'. Valid options: {valid}"
            )

        alg = config.createAlgorithm(
            "CP::RunPartonHistoryAlg", f"PartonHistory{self.history}"
        )
        alg.partonScheme = self.history

        for branch in TRUTH_BRANCHES[self.history]:
            config.addOutputVar(
                "EventInfo",
                f"{self.history}_{branch}",
                f"{self.history}_{branch}",
                noSys=True,
                auxType=_get_aux_type(branch, self.history),
            )
