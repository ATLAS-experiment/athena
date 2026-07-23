# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Runtime entry point for isolated CA-based Powheg generation."""

import argparse
import os
from pathlib import Path
import shutil

from PowhegControl.PowhegRunConfig import PowhegRunConfig


class _RunArgs:
    pass


class _RunOpts:
    nprocs = 0


def _legacy_output_file(plan):
    if plan.output_tarball:
        for suffix in (".tar.gz", ".tgz"):
            if plan.output_tarball.endswith(suffix):
                return plan.output_tarball[:-len(suffix)] + ".events"
    return "PowhegOTF._1.events"


def _make_run_args(plan):
    run_args = _RunArgs()
    run_args.ecmEnergy = plan.beam_energy
    run_args.maxEvents = plan.max_events
    run_args.randomSeed = plan.random_seed
    if plan.shower:
        run_args.outputEVNTFile = "CA-managed-output"
    if plan.output_tarball:
        run_args.outputTXTFile = plan.output_tarball
    return run_args


def run_powheg(plan):
    """Execute one serialized plan using the legacy implementation."""
    os.environ.setdefault("PYTHONPATH", "")
    os.environ["PWD"] = os.getcwd()
    os.environ["ATHENA_CORE_NUMBER"] = str(plan.n_cores)

    # Import only in the child process. Importing and constructing this object
    # is intentionally kept away from ComponentAccumulator construction.
    from PowhegControl.powheg_control import PowhegControl

    control = PowhegControl(
        process_name=plan.process,
        run_args=_make_run_args(plan),
        run_opts=_RunOpts(),
    )

    for setting, value in plan.settings.items():
        setattr(control, setting, value)

    if plan.parameter_stages:
        control.set_parameter_stage(plan.parameter_stages)

    for group in plan.weight_groups:
        control.define_event_weight_group(
            group.name,
            list(group.parameters),
            combination_method=group.combination_method,
        )
        for weight in group.weights:
            control.add_weight_to_group(
                group.name,
                weight.name,
                list(weight.values),
            )

    options = plan.generation_options
    control.generate(
        create_run_card_only=options.create_run_card_only,
        save_integration_grids=options.save_integration_grids,
        use_external_run_card=options.use_external_run_card,
        remove_oldStyle_rwt_comments=options.remove_old_style_rwt_comments,
        is_bb4l_semilep=options.is_bb4l_semilep,
    )

    if options.create_run_card_only:
        return

    legacy_output = Path(_legacy_output_file(plan))
    requested_output = Path(plan.output_lhe)
    if legacy_output != requested_output:
        if not legacy_output.is_file():
            raise RuntimeError(
                f"Powheg did not produce expected LHE file '{legacy_output}'"
            )
        requested_output.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(os.fspath(legacy_output), os.fspath(requested_output))

    if not requested_output.is_file() or requested_output.stat().st_size == 0:
        raise RuntimeError(
            f"Powheg did not produce non-empty LHE output "
            f"'{requested_output}'"
        )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", required=True)
    args = parser.parse_args()

    payload = Path(args.config).read_text(encoding="utf-8")
    run_powheg(PowhegRunConfig.from_json(payload))


if __name__ == "__main__":
    main()
