# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import GeV

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("SherpaConfig")


def write_pretty_fragment(text):
    """
    Keep multiline fragments readable in source code while preserving output indentation.
    Sherpa_i C++ currently strips the first line and final character of the incoming string,
    so we intentionally prepend a blank line and append a trailing newline.
    """
    from textwrap import dedent
    return "\n" + dedent(text).strip("\n") + "\n"


def validate_sherpa3_yaml_fragment(fragment_name, fragment_text):
    """Validate Sherpa 3 YAML input."""
    if fragment_text is None:
        return

    text = fragment_text if isinstance(fragment_text, str) else str(fragment_text)
    if not text.strip():
        return

    try:
        import yaml
    except Exception as exc:
        log.warning(f"YAML validation skipped for {fragment_name} (could not import yaml: {exc})")
        return

    try:
        yaml.compose(text)
    except Exception as exc:
        mark = getattr(exc, "problem_mark", None)
        problem = getattr(exc, "problem", None)
        location = ""
        if mark is not None:
            location = f" (line {mark.line + 1}, column {mark.column + 1})"

        detail = problem if problem else str(exc).splitlines()[0]
        raise RuntimeError(f"{fragment_name} is invalid YAML{location}: {detail}")


def _default_particle_data():
    """Default particle data used in Sherpa base fragments"""
    return {
        # mb consistent with McProductionCommonParametersMC15 and https://cds.cern.ch/record/2047636
        "5": {"mass": "4.95", "width": "0."},
        "6": {"mass": "1.725E+02", "width": "1.32E+00"},
        "15": {"mass": "1.777", "width": "2.26735e-12"},
        "23": {"mass": "91.1876", "width": "2.4952"},
        "24": {"mass": "80.399", "width": "2.085"},
    }


def build_sherpa3_base_fragment(flags):
    """Build the Sherpa 3 BaseFragment YAML string."""
    particle_data = _default_particle_data()

    base_fragment = write_pretty_fragment(
        f"""
        BEAMS: 2212
        BEAM_ENERGIES: {flags.Beam.Energy / GeV}

        MAX_PROPER_LIFETIME: 10.0
        HEPMC_TREE_LIKE: 1
        PRETTY_PRINT: Off
        EXTERNAL_RNG: Atlas_RNG

        OVERWEIGHT_THRESHOLD: 10
        MC@NLO:
          HPSMODE: 0

        SCALE_VARIATIONS:
        - 4.0*

        PARTICLE_DATA:
        """
    )

    # Add particle data
    for pdg_id in sorted(particle_data, key=int):
        values = particle_data[pdg_id]
        base_fragment += (
            f"  {pdg_id}:\n"
            f"    Mass: {values['mass']}\n"
            f"    Width: {values['width']}\n"
        )

    # Add hardcoded partial widths for H, W, Z decays
    # and OpenLoops parameters
    base_fragment += write_pretty_fragment(
        """
        EW_SCHEME: Gmu
        GF: 1.166397e-5

        HARD_DECAYS:
          Enabled: true
          Channels:
            "6 -> 24 5": { Width: 1.32 }
            "-6 -> -24 -5": { Width: 1.32 }
            "25 -> 5 -5": { Width: 2.35e-3 }
            "25 -> 15 -15": { Width: 2.57e-4 }
            "25 -> 13 -13": { Width: 8.91e-7 }
            "25 -> 4 -4": { Width: 1.18e-4 }
            "25 -> 3 -3": { Width: 1.00e-6 }
            "25 -> 21 21": { Width: 3.49e-4 }
            "25 -> 22 22": { Width: 9.28e-6 }
            "24 -> 2 -1": { Width: 0.7041 }
            "24 -> 4 -3": { Width: 0.7041 }
            "24 -> 12 -11": { Width: 0.2256 }
            "24 -> 14 -13": { Width: 0.2256 }
            "24 -> 16 -15": { Width: 0.2256 }
            "-24 -> -2 1": { Width: 0.7041 }
            "-24 -> -4 3": { Width: 0.7041 }
            "-24 -> -12 11": { Width: 0.2256 }
            "-24 -> -14 13": { Width: 0.2256 }
            "-24 -> -16 15": { Width: 0.2256 }
            "23 -> 1 -1": { Width: 0.3828 }
            "23 -> 2 -2": { Width: 0.2980 }
            "23 -> 3 -3": { Width: 0.3828 }
            "23 -> 4 -4": { Width: 0.2980 }
            "23 -> 5 -5": { Width: 0.3828 }
            "23 -> 11 -11": { Width: 0.0840 }
            "23 -> 12 -12": { Width: 0.1663 }
            "23 -> 13 -13": { Width: 0.0840 }
            "23 -> 14 -14": { Width: 0.1663 }
            "23 -> 15 -15": { Width: 0.0840 }
            "23 -> 16 -16": { Width: 0.1663 }
        OL_PARAMETERS:
          preset: 2
          write_parameters: 1
        """
    )

    # Add OpenLoops prefix if available in environment
    from os import environ
    openloops_path = environ.get("OPENLOOPSPATH")
    if openloops_path:
        base_fragment += f"OL_PREFIX: {openloops_path}\n"
    else:
        log.warning("OPENLOOPSPATH not set")

    return base_fragment
