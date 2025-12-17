# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from AthenaCommon.SystemOfUnits import GeV

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("Pythia8Config")


def Pythia8BaseCfg(flags, name="Pythia8_i", **kwargs):
    """
    The main Pythia8 configuration fragment that sets up the Pythia8_i algorithm
    and returns a CA instance
    """
    # Default commands
    commands = kwargs.get("Commands", [])
    
    # Baseline P8 settings
    base_cmds = [
        "Main:timesAllowErrors = 500",
        "ParticleDecays:limitTau0 = on",
        "ParticleDecays:tau0Max = 10.0"
    ]
    commands.extend(base_cmds)

    # Collision energy
    if "CollisionEnergy" not in kwargs:
        kwargs["CollisionEnergy"] = flags.Beam.Energy * 2 / GeV

    # Extended settings
    if flags.Generator.PDGparams:
        ## Load parameters by including parameter dictionary 'parameters' in 'offline_dict'
        from EvgenProdTools.offline_dict import parameters

        ## Particle masses and widths
        particle_params = parameters.get("particles")
        if particle_params:
            for pdg_str, vals in particle_params.items():
                ## Only the top quark, the leptons and the bosons are applied
                pdg = int(pdg_str)
                if 6 <= pdg < 26:
                    commands.append(f"{pdg}:m0 = {vals['mass']}")
                    commands.append(f"{pdg}:mWidth = {vals['width']}")
        else:
            log.warning("Could not retrieve standard ATLAS particle parameters")

        ## SM electroweak parameters
        ew_params = parameters.get("EW_parameters")
        if ew_params:
            ## Only the parameters sin2thetaW and sin2thetaWbar are applied
            for key, val in ew_params.items():
                if key[1] in ('sin2thetaW', 'sin2thetaWbar'):
                    commands.append(f"StandardModel:{key[1]} = {val}")
        else:
            log.warning("Could not retrieve standard ATLAS EW parameters")
    else:
        ## Load basic parameters
        commands.extend([
            "6:m0 = 172.5",
            "23:m0 = 91.1876",
            "23:mWidth = 2.4952",
            "24:m0 = 80.399",
            "24:mWidth = 2.085",
            "StandardModel:sin2thetaW = 0.23113",
            "StandardModel:sin2thetaWbar = 0.23146",
        ])

    # Remove duplicate commands and update kwargs
    commands = list(dict.fromkeys(commands))
    kwargs["Commands"] = commands

    # Create CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.Pythia8_i("Pythia8_i", **kwargs)
    )

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["Pythia8"]), sequenceName=EvgenSequence.Generator.value)

    return ca


def Pythia8EvtGenBaseCfg(flags, **kwargs):
    """
    Fragment for setting up EvtGen on top of Pythia 8
    """
    # Custom settings
    auxfiles = ["inclusiveP8DsDPlus.pdt"]
    # FHerwig has problems with omega b* (5334), so not present in the base EvtGen fragment.
    whiteList = [-5334, 5334] 
    
    # Add custom EvtGenCfg
    from EvtGen_i.EvtGenConfig import EvtGenCfg
    ca = EvtGenCfg(
        flags,
        whiteList = whiteList,
        auxfiles = auxfiles
    )

    return ca


def Pythia8_A2_MSTW2008LO_Common_Cfg(flags, **kwargs):
    """
    Fragment for setting up A2 MSTW2008LO tune
    """
    # Defaults
    cmds = kwargs.get("Commands", [])
    
    # Tune parameters
    cmds.extend([
        "Tune:pp = 5",
        "MultipartonInteractions:bProfile = 4",
        "MultipartonInteractions:a1 = 0.03",
        "MultipartonInteractions:pT0Ref = 1.90",
        "MultipartonInteractions:ecmPow = 0.30",
        "SpaceShower:rapidityOrder = 0",
        "PDF:pSet = LHAPDF6:MSTW2008lo68cl",
        "ColourReconnection:range = 2.28"
    ])

    # Now call rapidity ordering
    cmds = ensureRapidityOrderMPI(cmds)

    # Update kwargs
    kwargs["Commands"] = list(dict.fromkeys(cmds))

    # Now get the base config
    ca = Pythia8BaseCfg(flags, **kwargs)

    # Broadcast tune to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Tune="A2 MSTW2008LO"), sequenceName=EvgenSequence.Generator.value)

    # Call the base config
    return ca


def Pythia8_A2_MSTW2008LO_EvtGen_Common_Cfg(flags, **kwargs):
    """
    Config for Py8 tune A2 with MSTW2008LO tune
    The default version of this includes EvtGen for standardised b fragmentation
    This tune is generally only used for pile up samples at the start of run 2 
    for high pT physics at the start of run 2 the A14 tune is more appropriate.  
    There are also more recent soft QCD tunes, such as Monash, 
    but A2 was a conservative choice for initial 13 TeV pile up
    """

    # Add Pythia 8 to CA with correct tune settings
    ca = Pythia8_A2_MSTW2008LO_Common_Cfg(flags, **kwargs)

    # Add EvtGen
    ca.merge(Pythia8EvtGenBaseCfg(flags, **kwargs))

    return ca


def ensureRapidityOrderMPI(cmds):
    """
    A function that ensures rapidity ordering is set
    """
    # if MPI‐ordering already explicitly set, do nothing
    if any("SpaceShower:rapidityOrderMPI" in c for c in cmds):
        return cmds

    # find the first rapidityOrder value
    for c in cmds:
        if "SpaceShower:rapidityOrder" in c and "MPI" not in c:
            val = c.split("=", 1)[-1].strip()
            cmds.append(f"SpaceShower:rapidityOrderMPI = {val}")
            break
    return cmds
