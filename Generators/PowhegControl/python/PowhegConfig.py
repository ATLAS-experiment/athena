# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


class _RunArgs:
    """Legacy runArgs compatibility object."""
    pass


class _RunOpts:
    """Legacy run-options compatibility object."""
    nprocs = 0


def _make_run_args(flags):
    """Create the legacy runArgs interface from CA flags."""
    run_args = _RunArgs()
    run_args.ecmEnergy = flags.Beam.Energy * 2 / GeV
    run_args.maxEvents = (
        flags.Exec.MaxEvents
        if flags.Exec.MaxEvents > 0
        else flags.Generator.nEventsPerJob
    )
    run_args.randomSeed = flags.Random.SeedOffset

    if flags.Output.EVNTFileName:
        run_args.outputEVNTFile = flags.Output.EVNTFileName
    if flags.Generator.outputYODAFile:
        run_args.outputYODAFile = flags.Generator.outputYODAFile
    if flags.Output.TXTFileName:
        run_args.outputTXTFile = flags.Output.TXTFileName

    return run_args


def PowhegCfg(flags, process, settings=None, **generate_kwargs):
    """
    Configure and run a legacy PowhegControl process.
    'settings' contains assignments to public PowhegControl parameters.
    Additional keyword arguments are forwarded to 'generate()'.
    """
    from PowhegControl.powheg_control import PowhegControl

    run_args = _make_run_args(flags)

    control = PowhegControl(
        process_name=process,
        run_args=run_args,
        run_opts=_RunOpts(),
    )

    for setting, value in (settings or {}).items():
        setattr(control, setting, value)

    control.generate(**generate_kwargs)

    lhe_file = run_args.inputGeneratorFile

    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg

    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Powheg"]), sequenceName=EvgenSequence.Generator.value)

    return ca, lhe_file
