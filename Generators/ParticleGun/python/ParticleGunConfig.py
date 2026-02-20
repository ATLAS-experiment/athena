# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


def ParticleGunBaseCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    result.addEventAlgo(pg)
    return result


def ParticleGun_flatpt_2particleCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.samplers.append(PG.ParticleSampler()) # add a second sampler
    pg.samplers[0].pid = (-13, 13) # cycle mu+-
    pg.samplers[0].mom = PG.PtEtaMPhiSampler(pt=[4000, 100000], eta=[1.0, 3.2]) # flat in pt and +ve eta
    pg.samplers[1].pid = (13, -13) # cycle mu-+
    pg.samplers[1].mom = PG.PtEtaMPhiSampler(pt=[4000, 100000], eta=[-3.2, -1.0]) # flat in pt and -ve eta
    result.addEventAlgo(pg)
    return result


def ParticleGun_SingleMuonBasicCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = 13
    pg.sampler.mom = PG.EEtaMPhiSampler(energy=10000, eta=[-1,1])
    result.addEventAlgo(pg)
    return result


def ParticleGun_SingleMuonCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = PG.CyclicSeqSampler([-13,13])
    pg.sampler.mom = PG.PtEtaMPhiSampler(pt=50000, eta=[-4,4])
    result.addEventAlgo(pg)
    return result


def ParticleGun_SingleElectronCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = PG.CyclicSeqSampler([-11,11])
    pg.sampler.mom = PG.PtEtaMPhiSampler(pt=10000, eta=[-3,3])
    result.addEventAlgo(pg)
    return result


def ParticleGun_SinglePionCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = PG.CyclicSeqSampler([-211,211])
    pg.sampler.mom = PG.PtEtaMPhiSampler(pt=50000, eta=[-4,4])
    result.addEventAlgo(pg)
    return result


def ALFA_SingleParticlePreInclude(flags):
    flags.BField.solenoidOn=False
    flags.BField.barrelToroidOn=False
    flags.BField.endcapToroidOn=False
    from SimulationConfig.SimEnums import VertexSource
    flags.Sim.VertexSource=VertexSource.AsGenerated
    flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_ALFA_SingleParticleCfg"
    from ForwardTransportSvc.ForwardTransportSvcConfig import ForwardTransportBeta90mPreInclude
    ForwardTransportBeta90mPreInclude(flags)


def ParticleGun_ALFA_SingleParticleCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = 2212
    pg.sampler.mom = PG.EEtaMPhiSampler(energy=3500000, eta=10)
    result.addEventAlgo(pg)
    return result


def ZDC_SingleParticlePreInclude(flags):
    from SimulationConfig.SimEnums import VertexSource
    flags.Sim.VertexSource=VertexSource.AsGenerated
    flags.Sim.GenerationConfiguration="ParticleGun.ParticleGunConfig.ParticleGun_ZDC_SingleParticleCfg"
    from ForwardTransportSvc.ForwardTransportSvcConfig import ForwardTransportBeta055mPreInclude
    ForwardTransportBeta055mPreInclude(flags)


def ParticleGun_ZDC_SingleParticleCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    pg.sampler.pid = PG.CyclicSeqSampler([2112, 22, 2112, 22])
    esampler = PG.CyclicSeqSampler([1360000, 500000, 1360000, 500000])
    thsampler = PG.CyclicSeqSampler([0, 0, PG.PI, PG.PI])
    pg.sampler.mom = PG.EThetaMPhiSampler(energy=esampler, theta=thsampler)
    result.addEventAlgo(pg)
    return result


def ParticleGun_TestBeam_SingleParticleCfg(flags):
    result = ComponentAccumulator()
    import ParticleGun as PG
    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)
    # 50 GeV pions
    #pg.sampler.pid = 211
    #pg.sampler.pos = PG.PosSampler(x=-27500, y=[-10,15], z=[-15,15], t=-27500)
    #pg.sampler.mom = PG.EEtaMPhiSampler(energy=50000, eta=0, phi=0)

    # 100 GeV electrons - use for sampling faction calculation
    #pg.sampler.pid = 11
    #pg.sampler.pos = PG.PosSampler(x=-27500, y=[-20,20], z=[-15,15], t=-27500)
    #pg.sampler.mom = PG.EEtaMPhiSampler(energy=100000, eta=0, phi=0)

    pg.sampler.pid = flags.TestBeam.BeamPID
    pg.sampler.pos = PG.PosSampler(
        x=flags.TestBeam.Xbeam,
        y=flags.TestBeam.Ybeam,
        z=flags.TestBeam.Zbeam,
        t=flags.TestBeam.Tbeam)
    pg.sampler.mom = PG.EEtaMPhiSampler(
        energy=flags.TestBeam.BeamEnergy,
        eta=0,
        phi=0)
    result.addEventAlgo(pg)
    return result


def _makeMomentumSampler(PG, samplerType="EEtaMPhi", **kwargs):
    """
    Factory for the various PG momentum samplers.
    kwargs are passed through to the corresponding sampler.
    """
    from math import pi

    if samplerType == "EEtaMPhi":
        # energy: float or sequence, eta/phi can be scalar or range
        return PG.EEtaMPhiSampler(
            energy=kwargs["energy"],
            eta=kwargs.get("eta"),
            mass=kwargs.get("mass", 0),
            phi=kwargs.get("phi", [0, 2*pi]),
        )

    if samplerType == "PtEtaMPhi":
        # pt: float or [min,max], eta: float or [min,max]
        return PG.PtEtaMPhiSampler(
            pt=kwargs["pt"],
            eta=kwargs.get("eta"),
            mass=kwargs.get("mass", 0),
            phi=kwargs.get("phi", [0, 2*pi]),
        )

    if samplerType == "EThetaMPhi":
        # energy, theta: can be scalars or cyclic samplers
        return PG.EThetaMPhiSampler(
            energy=kwargs["energy"],
            theta=kwargs["theta"],
            mass=kwargs.get("mass", 0),
            phi=kwargs.get("phi", [0, 2*pi]),
        )

    raise ValueError(f"Unknown samplerType '{samplerType}'")


def ParticleGun_SteeredSingleParticleCfg(
    flags,
    *,
    pid,
    samplerType="EEtaMPhi",
    randomStream="SINGLE",
    randomSeed=None,
    **kwargs,
):
    """
    Generic single-particle gun configuration steered via arguments.

    Parameters
    ----------
    pid : int or sequence
        PDG ID (or sampler) for the primary particle.
    samplerType : str
        Which momentum sampler to use: "EEtaMPhi", "PtEtaMPhi", "EThetaMPhi".
    randomStream : str
        Name of the random stream (defaults to "SINGLE").
    randomSeed : int or None
        Random seed (defaults to flags.Random.SeedOffset if None).
    momKwargs : dict
        Keyword arguments passed to the momentum sampler factory.
        E.g. for samplerType="EEtaMPhi":
            energy=50000, eta=0
        for samplerType="PtEtaMPhi":
            pt=[4000, 100000], eta=[-2.5, 2.5]
    """
    import ParticleGun as PG
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["ParticleGun"]), sequenceName=EvgenSequence.Generator.value)

    if randomSeed is None:
        randomSeed = flags.Random.SeedOffset

    pg = PG.ParticleGun(
        randomStream=randomStream,
        randomSeed=randomSeed,
    )

    # Select the particle type
    pg.sampler.pid = pid

    # Select the momentum sampler
    pg.sampler.mom = _makeMomentumSampler(
        PG,
        samplerType=samplerType,
        **kwargs,
    )

    ca.addEventAlgo(pg)

    return ca