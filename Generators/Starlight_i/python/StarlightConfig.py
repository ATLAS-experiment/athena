# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
)
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from EvgenJobTransforms.EvgenHelpers import _get_nevents
from Pythia8_i.Pythia8Config import (
    Pythia8_A14_NNPDF23LO_Common_Cfg,
    Pythia8CommandsCfg
)
from AthenaCommon.SystemOfUnits import GeV

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("StarlightConfig")


def StarlightInitializeCfg(flags, source, values, precedence, name="Starlight_i"):
    """
    Return a CA fragment that adds one initialize layer to Starlight_i.
    """
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.addEventAlgo(
        CompFactory.Starlight_i(name, Initialize=GeneratorSettingsLayer(
            source=source,
            values=tuple(values or ()),
            precedence=precedence,
            report_context="Starlight.Initialize",
        ))
    )
    return ca


def StarlightBaseCfg(flags, name="Starlight_i", safety=1., **kwargs):
    """
    The main Starlight configuration fragment that sets up the Starlight_i algorithm
    and returns a CA instance.
    By default Starlight is set to Pb+Pb collisions.
    """

    # By default write output directly to HepMC
    kwargs.setdefault("lheOutput", False)

    # If lhe output is requested `maxevents` determines the number of events written 
    # to lhe file, else events are generated one by one and `maxevents` is not utilised
    kwargs.setdefault("maxevents", _get_nevents(flags, safety))

    # Gamma calculation for Pb208
    # Gamma = E(Pb208) / (m(Pb208) in a.u. * atomic mass unit in GeV)
    gamma = 208*flags.Beam.Energy/GeV / (207.9766359 * 0.931494)

    # Basic initialize parameters
    base_init = [
        "beam1Z 82", "beam1A 208", #Z,A of projectile 
        "beam2Z 82", "beam2A 208", #Z,A of target
        f"beam1Gamma {int(gamma)}",
        f"beam2Gamma {int(gamma)}",
        "maxW -1", #Max value of w
        "minW -1", #Min value of w
        "nmbWBins 400", #Bins n w
        "maxRapidity 3.", #max y
        "nmbRapidityBins 300", #Bins n_y
        "accCutPt 0", #Cut in pT? 0 = (no, 1 = yes)
        "accCutEta 0", #Cut in pseudorapidity? (0 = no, 1 = yes)
        # `nmbEventsTot` is not utilised by the algorithm in any way, kept here only for printing
        f"nmbEventsTot {kwargs["maxevents"] if kwargs["lheOutput"] else 1}",
        "interferenceEnabled 0", #Interference (0 = off, 1 = on)
        "interferenceStrength 1.", #% of intefernce (0.0 - 0.1)
        "coherentProduction 1", #Coherent=1,Incoherent=0
        "incoherentFactor 1.", #percentage of incoherence
        "maxPtInterference 0.24", #Maximum pt considered, when interference is turned on
        "nmbPtBinsInterference 120", #Number of pt bins when interference is turned on
        "xsecMethod 0", #Set to 0 to use old method for calculating gamma-gamma luminosity
        "nThreads 1", #Number of threads used for calculating luminosity (when using the new method)
        "pythFullRec 0" #Write full pythia information to output (vertex, parents, daughter etc)
    ]

    # Get user initialize parameters
    user_init = kwargs.pop("Initialize", None)
    kwargs["Initialize"] = GeneratorSettingsLayer(
        source="base_initialize",
        values=tuple(base_init),
        precedence=GeneratorSettingsPrecedence.BASE,
        report_context="Starlight.Initialize",
    )

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.Starlight_i(name, **kwargs)
    )

    # Add user initialize parameters
    if user_init:
        ca.merge(StarlightInitializeCfg(
            flags,
            source="user_initialize",
            values=user_init,
            precedence=GeneratorSettingsPrecedence.USER,
            name=name,
        ))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Starlight"]), sequenceName=EvgenSequence.Generator.value)

    return ca


def StarlightPythia8BaseCfg(flags, ShowerCfg=Pythia8_A14_NNPDF23LO_Common_Cfg, **kwargs):
    """
    Modular fragment for setting up Pythia 8 on top of Starlight.
    """

    # Create CA for basic Pythia8 shower configuration
    ca = ShowerCfg(
        flags,
        LHEFile="events.lhe",
        # IsAfterburner must be true to remove empty HepMC
        # events produced by Starlight GenModule
        IsAfterburner=True,
        **kwargs
    )

    # Add Pythia8 commands common for QED final state radiation
    # If needed can be overriden by user comands
    common_commands = ['SpaceShower:QEDshowerByL = 1']
    ca.merge(Pythia8CommandsCfg(
        flags,
        source="common_commands",
        commands=common_commands,
        precedence=GeneratorSettingsPrecedence.WEIGHTS,
    ))

    return ca


def Starlight_Pythia8_Common_Cfg(flags,
                                ShowerCfg=Pythia8_A14_NNPDF23LO_Common_Cfg,
                                Commands=[],
                                safety=1.1,
                                **kwargs):
    """
    Fragment for setting up Starlight with Pythia8 for final state radiation.
    By deafault use Pythia8_A14_NNPDF23LO_Common_Cfg.
    """

    # Increase the requested number of events with a safety factor
    # to allow for failures in showering stage
    kwargs.setdefault("maxevents", _get_nevents(flags, safety))

    # Write the output to lhe file as input for Pythia8
    kwargs.setdefault("lheOutput", True)

    # Setup base Starlight configuration
    ca = StarlightBaseCfg(flags, **kwargs)

    # Add Pythia8
    ca.merge(StarlightPythia8BaseCfg(
        flags,
        ShowerCfg,
        Commands=Commands
    ))

    return ca


def Starlight_EvtGen_Common_Cfg(flags, **kwargs):
    """
    Fragment for setting up Starlight with EvtGen for VM decays.
    """

    # Configure base Starlight
    ca = StarlightBaseCfg(
        flags,
        suppressVMdecay = True,
        **kwargs
    )

    # Add EvtGen on top of Starlight
    from EvtGen_i.EvtGenConfig import EvtGenCfg
    ca.merge(EvtGenCfg(
        flags,
        auxfiles=["inclusiveP8DsDPlus.pdt"],
        pdtFile="inclusiveP8DsDPlus.pdt",
        setVMtransversePol=True
    ))

    return ca
