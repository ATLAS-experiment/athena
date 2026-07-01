# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""
This module contains all the CA fragments for Herwig7.

It is analogous to MadGraphConfig.py but more CA friendly since it avoids side effects.
It creates the Athena Herwig7 algorithm, hands the flags to the
control object H7C, and adds the configured algorithm to the evgen
sequence. The actual execution happens later when Athena runs the sequence.
"""

from textwrap import dedent

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

from Herwig7_i.H7C import H7C


def _builtin_me_local_pre_commands():
    """Default local pre-commands for built-in matrix-element setups"""
    return dedent(
        """
        ## =================================================
        ## Local Pre-Commands from Herwig7ConfigBuiltinME.py
        ## =================================================

        # > no local pre-commands at the moment
        """
    )


def _builtin_me_local_post_commands(run_name):
    """Default local post-commands for built-in matrix-element setups"""
    return dedent(
        f"""
        ## ==================================================
        ## Local Post-Commands from Herwig7ConfigBuiltinME.py
        ## ==================================================

        saverun {run_name} /Herwig/Generators/EventGenerator
        """
    )


def _finalize_h7c_commands(h7c, shower_var=True):
    """
    Add the necessary settings fir beams, energy, seed,
    etc. It uses the h7c object which inherits from Hw7Config,
    so several of the methods called below are defined in the parent class.
    """
    h7c.default_commands += h7c.beam_commands()
    h7c.default_commands += h7c.energy_commands()
    h7c.default_commands += h7c.random_seed_commands()

    if not h7c.set_printout_commands:
        h7c.default_commands += h7c.printout_commands()
    if not h7c.set_physics_parameter_commands:
        h7c.default_commands += h7c.physics_parameter_commands()
    if not h7c.set_technical_parameter_commands:
        h7c.default_commands += h7c.technical_parameter_commands()

    h7c.enable_angularShowerScaleVariations(shower_var)


def Herwig7BaseCfg(flags,
                   name="Herwig7",
                   run_name="Herwig",
                   **kwargs):
    """This is the CA equivalent of Herwig7Config.py"""

    commands = kwargs.pop("commands", None)
    shower_var = kwargs.pop("shower_var", True)
    cleanup_herwig_scratch = kwargs.pop("cleanup_herwig_scratch", False)
    local_pre_commands = kwargs.pop("local_pre_commands", None)
    local_post_commands = kwargs.pop("local_post_commands", None)
    me_pdf_order = kwargs.pop("me_pdf_order", None)
    me_pdf_name = kwargs.pop("me_pdf_name", None)
    mpi_pdf_name = kwargs.pop("mpi_pdf_name", None)
    shower_pdf_order = kwargs.pop("shower_pdf_order", None)
    shower_pdf_name = kwargs.pop("shower_pdf_name", None)

    # Pop the tune - we don't want the user to set it in the jO
    # and we also don't want to forward it to the C++ instance 
    # via the herwig7 object below
    kwargs.pop("tune", None)

    # Instantiate H7 algorithm with base settings
    # TODO: add deduplication using GeneratorSettingsLayer as done in Pythia
    herwig7 = CompFactory.Herwig7(name, **kwargs)
    h7c = H7C(
        flags,
        run_name=run_name,
        local_pre_commands=local_pre_commands,
        local_post_commands=local_post_commands
    )
    h7c.add_flags(flags)

    if me_pdf_name is not None:
        h7c.me_pdf_commands(order=me_pdf_order or "NLO", name=me_pdf_name)

    if mpi_pdf_name is not None:
        h7c.mpi_pdf_commands(name=mpi_pdf_name)

    if shower_pdf_name is not None:
        h7c.shower_pdf_commands(order=shower_pdf_order or "LO", name=shower_pdf_name)

    if commands:
        h7c.add_commands(dedent(commands))

    _finalize_h7c_commands(h7c, shower_var=shower_var)

    from Herwig7_i import Herwig7Control as hw7Control

    run_settings = hw7Control.render_infile(h7c)
    hw7Control.configure_algorithm(
        herwig7,
        hw7Control.get_runfile_name(h7c.run_name),
        h7c.random_seed,
        me_pdf_name=h7c.me_pdf_name,
        mpi_pdf_name=h7c.mpi_pdf_name,
        cleanup_herwig_scratch=cleanup_herwig_scratch,
        run_settings=run_settings,
    )

    # The algorithm is scheduled here. Athena executes it later with the evgen sequence
    # so that there are no side effects.
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(herwig7)

    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(
        GeneratorInfoSvcCfg(flags, 
                            Generators=["Herwig7"], 
                            Tune=hw7Control.default_tune_name()
                            )
    )

    return ca


def Herwig7BuiltinMECfg(flags,
                        name="Herwig7",
                        run_name="Herwig",
                        **kwargs):
    """This is the CA equivalent of Herwig7ConfigBuiltinME.py"""

    kwargs.setdefault("local_pre_commands", _builtin_me_local_pre_commands)
    kwargs.setdefault("local_post_commands", lambda: _builtin_me_local_post_commands(run_name))

    # Get CA from base fragment
    return Herwig7BaseCfg(
        flags,
        name=name,
        run_name=run_name,
        **kwargs
    )
