# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""ComponentAccumulator configuration for PowhegControl."""

import json
import os
import sys

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from PowhegControl.PowhegRunConfig import (
    PowhegGenerationOptions,
    PowhegRunConfig,
    PowhegWeight,
    PowhegWeightGroup,
)


def setupPowhegFlags(flags):
    """Transfer AthenaMP worker count to Powheg before flags are locked.

    PowhegControl uses multiple local Powheg processes itself. Athena must stay
    serial, otherwise every Athena worker would independently generate an LHE
    file.
    """
    requested_cores = max(1, int(flags.Concurrency.NumProcs))
    flag_name = "Generator.Powheg.NCores"
    if not flags.hasFlag(flag_name):
        flags.addFlag(flag_name, requested_cores, type=int)
    elif requested_cores > 1:
        flags.Generator.Powheg.NCores = requested_cores
    flags.Concurrency.NumProcs = 0


def PowhegOutputFile(flags):
    """Return the LHE filename produced for the configured transform output."""
    output_tarball = flags.Output.TXTFileName
    if output_tarball:
        for suffix in (".tar.gz", ".tgz"):
            if output_tarball.endswith(suffix):
                return output_tarball[:-len(suffix)] + ".events"
        raise ValueError(
            "Powheg TXT output must end in '.tar.gz' or '.tgz', got "
            f"'{output_tarball}'"
        )
    return "PowhegOTF._1.events"


def _normalise_weight_groups(weight_groups):
    groups = []
    for entry in weight_groups or ():
        if isinstance(entry, PowhegWeightGroup):
            groups.append(entry)
            continue

        weights = tuple(
            weight
            if isinstance(weight, PowhegWeight)
            else PowhegWeight(
                name=weight["name"],
                values=tuple(weight.get("values", ())),
            )
            for weight in entry.get("weights", ())
        )
        groups.append(
            PowhegWeightGroup(
                name=entry["name"],
                parameters=tuple(entry.get("parameters", ())),
                combination_method=entry.get("combination_method", "none"),
                weights=weights,
            )
        )
    return tuple(groups)


def _powheg_cores(flags, n_cores):
    if n_cores is not None:
        cores = int(n_cores)
    elif flags.hasFlag("Generator.Powheg.NCores"):
        cores = int(flags.Generator.Powheg.NCores)
    elif flags.Concurrency.NumProcs > 0:
        raise RuntimeError(
            "Powheg generation cannot run inside AthenaMP workers. Call "
            "setupPowhegFlags(flags) from Sample.setupFlags() before flags "
            "are locked, or pass n_cores explicitly and keep Athena serial."
        )
    else:
        cores = 1

    if cores < 1:
        raise ValueError(f"n_cores must be positive, got {cores}")
    return cores


def buildPowhegRunConfig(
    flags,
    process,
    *,
    settings=None,
    output_lhe=None,
    n_cores=None,
    weight_groups=None,
    parameter_stages=None,
    create_run_card_only=False,
    save_integration_grids=True,
    use_external_run_card=False,
    remove_old_style_rwt_comments=False,
    is_bb4l_semilep=False,
):
    """Build the immutable run plan without constructing PowhegControl."""
    if not process or not isinstance(process, str):
        raise TypeError("process must be a non-empty string")

    requested_events = (
        flags.Exec.MaxEvents
        if flags.Exec.MaxEvents > 0
        else flags.Generator.nEventsPerJob
    )
    if requested_events <= 0:
        raise ValueError(
            f"Powheg requires a positive event count, got {requested_events}"
        )

    plan = PowhegRunConfig(
        process=process,
        beam_energy=float(flags.Beam.Energy * 2 / GeV),
        max_events=int(requested_events),
        random_seed=int(flags.Random.SeedOffset),
        n_cores=_powheg_cores(flags, n_cores),
        shower=bool(
            flags.Output.EVNTFileName or flags.Generator.outputYODAFile
        ),
        output_lhe=output_lhe or PowhegOutputFile(flags),
        output_tarball=flags.Output.TXTFileName or "",
        settings=dict(settings or {}),
        weight_groups=_normalise_weight_groups(weight_groups),
        parameter_stages=dict(parameter_stages or {}),
        generation_options=PowhegGenerationOptions(
            create_run_card_only=bool(create_run_card_only),
            save_integration_grids=bool(save_integration_grids),
            use_external_run_card=bool(use_external_run_card),
            remove_old_style_rwt_comments=bool(
                remove_old_style_rwt_comments
            ),
            is_bb4l_semilep=bool(is_bb4l_semilep),
        ),
    )

    # Fail during configuration if the supplied values cannot be serialized.
    json.loads(plan.to_json())
    return plan


def PowhegCfg(
    flags,
    process,
    *,
    name="PowhegGenerationSvc",
    working_directory=".",
    python_executable=None,
    **kwargs,
):
    """Configure side-effect-free, one-shot Powheg LHE generation."""
    plan = buildPowhegRunConfig(flags, process, **kwargs)

    ca = ComponentAccumulator()
    ca.addService(
        CompFactory.PowhegGenerationSvc(
            name,
            RunnerModule="PowhegControl.PowhegRunner",
            Configuration=plan.to_json(),
            OutputLHE=plan.output_lhe,
            WorkingDirectory=os.fspath(working_directory),
            PythonExecutable=python_executable or sys.executable,
            ValidateOutput=not plan.generation_options.create_run_card_only,
        ),
        create=True,
    )

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Powheg"]))
    return ca
