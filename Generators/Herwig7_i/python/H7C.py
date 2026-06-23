# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""
This module provides a stateful object for preparing a H7 run.
It mirrors what is done in MGC.py but in a more CA-friendly way
(i.e. avoiding side effects).
"""

from Herwig7_i import Herwig7Utils as hw7Utils
from Herwig7_i.Herwig7Config import Hw7Config
from AthenaCommon.SystemOfUnits import GeV

from textwrap import dedent


class H7C(Hw7Config):
    """
    Stateful control object for preparing a Herwig7 CA run.
    Note that this inherits from Hw7Config, which provides 
    all the baseline settings, so that we don't have to rewrite 
    the full config.
    """
    def __init__(
        self,
        flags,
        run_name="Herwig",
        local_pre_commands=None,
        local_post_commands=None,
    ):
        self.flags = flags
        self.run_name = run_name
        self._local_pre_commands = local_pre_commands or (lambda: "")
        self._local_post_commands = local_post_commands or (lambda: "")

        self.me_pdf_name = "MMHT2014lo68cl"
        self.mpi_pdf_name = "MMHT2014lo68cl"
        self.ecmEnergy = 0
        self.random_seed = 0

        self.set_printout_commands = False
        self.set_physics_parameter_commands = False
        self.set_technical_parameter_commands = False

        self.default_commands = hw7Utils.ConfigurationCommands()
        self.commands = hw7Utils.ConfigurationCommands()

    def local_pre_commands(self):
        return self._local_pre_commands()

    def local_post_commands(self):
        return self._local_post_commands()

    def get_flags_info(self, flags):
        """Populate beam energy and random seed from CA flags"""
        if flags is None:
            raise RuntimeError("flags must be provided!")

        try:
            self.ecmEnergy = float(flags.Beam.Energy * 2) / GeV
        except AttributeError as exc:
            raise RuntimeError("No beam energy found in flags (expected flags.Beam.Energy).") from exc

        try:
            self.random_seed = int(flags.Random.SeedOffset)
        except AttributeError as exc:
            raise RuntimeError("No random seed found in flags (expected flags.Random.SeedOffset).") from exc

    def add_flags(self, flags=None):
        if flags is not None:
            self.get_flags_info(flags)

    def beam_commands(self):
        return dedent("""
            ## Commands for proton-proton collisions
            read snippets/PPCollider.in
        """)

    def random_seed_commands(self):
        return dedent(f"""
            ## Random number generator seed
            set /Herwig/Random:Seed {self.random_seed}
        """)

    def energy_commands(self):
        return dedent(f"""
            ## Center-of-mass energy
            set /Herwig/Generators/EventGenerator:EventHandler:LuminosityFunction:Energy {self.ecmEnergy}
        """)
