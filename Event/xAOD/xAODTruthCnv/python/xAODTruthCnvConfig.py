# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def GEN_EVNT2xAODCfg(flags, name="GEN_EVNT2xAOD", **kwargs):
    """Configure the EVNT->xAOD converter alg"""
    # Note that since this is usually used by the derivation framework
    # which handles the output stream itself (to allow the expansion to AuxDyn and slimming)
    # nothing is done with the stream here - this is done in the DF job options
    acc = ComponentAccumulator()

    # In case we find the special configuration setting, add the LHE particles
    if flags.Input.SpecialConfiguration.get("HasLHERecord", "False") == "True":
        kwargs.setdefault('xAODTruthLHEParticleContainerName', 'TruthLHEParticles')

    kwargs.setdefault('AODContainerName', 'GEN_EVENT')

    xAODTruthCnvAlg = CompFactory.xAODMaker.xAODTruthCnvAlg
    acc.addEventAlgo(xAODTruthCnvAlg(name,**kwargs))
    return acc

def GEN_AOD2xAODCfg(flags, name="GEN_AOD2xAOD", **kwargs):
    acc = ComponentAccumulator()

    writeInTimePileUpTruth=False

    # Use digiSteeringConf from metadata to check whether full-PU
    # truth should be written.
    if flags.Digitization.PileUp and flags.Digitization.DigiSteeringConf in ['StandardPileUpToolsAlg', 'StandardInTimeOnlyTruthPileUpToolsAlg', 'StandardInTimeOnlyGeantinoTruthPileUpToolsAlg']:
        writeInTimePileUpTruth = True

    kwargs.setdefault('WriteInTimePileUpTruth',  writeInTimePileUpTruth)
    kwargs.setdefault('AODContainerName', 'TruthEvent')

    algo = CompFactory.xAODMaker.xAODTruthCnvAlg(name, **kwargs)
    acc.addEventAlgo(algo, primary = True)

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
    toAOD = ["xAOD::TruthEventContainer#TruthEvents", "xAOD::TruthEventAuxContainer#TruthEventsAux.",
             "xAOD::TruthVertexContainer#TruthVertices", "xAOD::TruthVertexAuxContainer#TruthVerticesAux.",
             "xAOD::TruthParticleContainer#TruthParticles", "xAOD::TruthParticleAuxContainer#TruthParticlesAux.",
    ]

    if "xAOD::TruthParticleContainer#TruthPileupParticles" in flags.Input.TypedCollections:
        toAOD.append("xAOD::TruthParticleContainer#TruthPileupParticles")
        toAOD.append("xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux.")

    if writeInTimePileUpTruth:
        toAOD.append("xAOD::TruthPileupEventContainer#TruthPileupEvents")
        toAOD.append("xAOD::TruthPileupEventAuxContainer#TruthPileupEventsAux.")

    if flags.Input.SpecialConfiguration.get("HasLHERecord", "False") == "True":
        kwargs.setdefault('xAODTruthLHEParticleContainerName', 'TruthLHEParticles')
        toAOD.append("xAOD::TruthParticleContainer#TruthLHEParticles")
        toAOD.append("xAOD::TruthParticleContainer#TruthLHEParticlesAux.")

    toESD = []

    if flags.Output.doWriteESD:
        acc.merge(addToESD(flags, toAOD+toESD))
    if flags.Output.doWriteAOD:
        acc.merge(addToAOD(flags, toAOD))

    return acc

def HepMCTruthReaderCfg(flags, name="HepMCTruthReader"):
    acc = ComponentAccumulator()
    HepMCTruthReader = CompFactory.HepMCTruthReader(
        name               = name,
        HepMCContainerKey  = flags.HepMCContainerKey,
        Do4momPtEtaPhi     = flags.Do4momPtEtaPhi
    )
    acc.addEventAlgo(HepMCTruthReader)
    return acc

def xAODTruthReaderCfg(flags, name="xAODTruthReader"):
    acc = ComponentAccumulator()
    xAODTruthReader = CompFactory.xAODReader.xAODTruthReader(
        name                              = name,
        xAODTruthEventContainerKey       = flags.xAODTruthEventContainerKey,
        # xAODTruthPileupEventContainerKey = flags.xAODTruthPileupEventContainerKey,
        DoPUEventPrintout                 = flags.DoPUEventPrintout,
        Do4momPtEtaPhi                    = flags.Do4momPtEtaPhi
    )
    acc.addEventAlgo(xAODTruthReader)
    return acc
