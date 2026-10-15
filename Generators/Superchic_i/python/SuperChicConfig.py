# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from Pythia8_i.Pythia8Config import (
    Pythia8BaseCfg,
    Pythia8_A14_NNPDF23LO_Common_Cfg
)
from AthenaCommon.SystemOfUnits import GeV
from EvgenJobTransforms.EvgenHelpers import _get_nevents

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("SuperChicConfig")


def SuperChicBaseCfg(flags, Config={}, safety=1.1, **kwargs):
    """
    The main SuperChic configuration fragment. It creates a SuperChicControl
    instance that stores the generator configuration parameters and is used
    to call SuperChicUtils functions responsible for event generation.

    All the configurable parameters and their default values can be found
    in SuperChicUtils.
    """

    Config.setdefault("rts", 2*flags.Beam.Energy/GeV)
    Config.setdefault("nev", _get_nevents(flags, safety))
    Config.setdefault("iseed", flags.Random.SeedOffset)

    # Create SuperChicControl instance and generate events
    from Superchic_i.SuperChicControl import SuperChicControl
    control = SuperChicControl(Config=Config)
    control.initialize()
    control.generate()

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["SuperChic"]), sequenceName=EvgenSequence.Generator.value)

    return ca


def SuperChicPythia8BaseCfg(flags, Commands=[],
        LHEFile=None, Config={}, **kwargs):
    """
    Modular fragment for setting up base Pythia8 on top of SuperChic.

    LHEFile is the path to LHE output file produced by SuperChic,
    that is taken as input for Pythia8. It can be set explicitly,
    or read from the Config dictionary. Otherwise it is set to
    the default "evrecs/evrecout.dat".

    TestHepMC selection is additionally loosened to avoid crashes.
    """

    if LHEFile is None:
        # if no path for LHE file was specify try to read it from Config
        if 'outtg' in Config:
            LHEFile = f"evrecs/evrec{Config['outtg']}.dat"
        # if there was none set in Config use the default
        else:
            LHEFile = "evrecs/evrecout.dat"

    # Add Pythia8
    ca = Pythia8BaseCfg(
        flags,
        Commands=Commands,
        LHEFile=LHEFile,
        **kwargs
    )

    # Add TestHepMC configuration
    from EvgenProdTools.EvgenProdToolsConfig import TestHepMCCfg
    ca.merge(TestHepMCCfg(
        flags,
        MaxTransVtxDisp=1000000,
        MaxVtxDisp=1000000000
    ))

    return ca


def SuperChic_Pythia8_A14_NNPDF23LO_Common_Cfg(flags, Commands=[],
        LHEFile=None, Config={}, **kwargs):
    """
    Modular fragment for setting up Pythia8 with A14_NNPDF23LO
    tunes on top of SuperChic.

    LHEFile is the path to LHE output file produced by SuperChic,
    that is taken as input for Pythia8. It can be set explicitly,
    or read from the Config dictionary. Otherwise it is set to
    the default "evrecs/evrecout.dat".

    TestHepMC selection is additionally loosened to avoid crashes.
    """

    if LHEFile is None:
        # if no path for LHE file was specify try to read it from Config
        if 'outtg' in Config:
            LHEFile = f"evrecs/evrec{Config['outtg']}.dat"
        # if there was none set in Config use the default
        else:
            LHEFile = "evrecs/evrecout.dat"

    # Add Pythia8
    ca = Pythia8_A14_NNPDF23LO_Common_Cfg(
        flags,
        Commands=Commands,
        LHEFile=LHEFile,
        **kwargs
    )

    # Add TestHepMC configuration
    from EvgenProdTools.EvgenProdToolsConfig import TestHepMCCfg
    ca.merge(TestHepMCCfg(
        flags,
        MaxTransVtxDisp=1000000,
        MaxVtxDisp=1000000000
    ))

    return ca


def SuperChicPythia8LheConverterUpcCfg(flags,
        inputLHEFile="evrecs/evrecout.dat", **kwargs):
    """
    Modular fragment for setting the LheConverterUpc.
    """
    from MCJobOptionUtils.LheConverterUpc import LheConverterUpc
    lheConverter = LheConverterUpc(generator='SuperChic',
                                    mode='Pythia8',
                                    energy=2*flags.Beam.Energy/GeV)
    lheConverter.fileName = inputLHEFile

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(lheConverter)

    return ca


def SuperChicLheEvntFillerCfg(flags, name="LheEVNTFiller", **kwargs):
    """
    Modular fragment for setting the LheEVNTFiller.
    """

    from Superchic_i.EventFiller import LheEVNTFiller

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(LheEVNTFiller(
        flags,
        ecmEnergy=2*flags.Beam.Energy/GeV,
        name=name,
        **kwargs
    ))

    return ca

