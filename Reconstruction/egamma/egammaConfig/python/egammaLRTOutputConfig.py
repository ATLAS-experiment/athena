# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

""" Prepare LRT EGamma output list
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaLRTOutputCfg(flags, name="LRTEGOutputList"):

    # This need some work form LRT community
    # ....
    acc = ComponentAccumulator()

    outFlags = flags.Egamma.Keys.Output

    toESD = []
    toAOD = []

    toESD += [
        f"xAOD::ElectronContainer#LRT{outFlags.Electrons}",
        f"xAOD::ElectronAuxContainer#LRT{outFlags.Electrons}"
        f"Aux.{outFlags.ElectronsSuppESD}"]
    toESD += [
        f"xAOD::CaloClusterContainer#LRT{outFlags.CaloClusters}",
        f"xAOD::CaloClusterAuxContainer#LRT{outFlags.CaloClusters}"
        f"Aux.{outFlags.CaloClustersSuppESD}"]
    toESD += [
        f"xAOD::CaloClusterContainer#LRT{outFlags.EgammaLargeClusters}",
        f"xAOD::CaloClusterAuxContainer#LRT{outFlags.EgammaLargeClusters}"
        f"Aux.{outFlags.EgammaLargeClustersSuppESD}"]
    toESD += [
        f"CaloClusterCellLinkContainer#LRT{outFlags.CaloClusters}"
        "_links"]
    toESD += [
        f"CaloClusterCellLinkContainer#LRT{outFlags.EgammaLargeClusters}"
        "_links"]
    toESD += [
        f"xAOD::TrackParticleContainer#LRT{outFlags.GSFTrackParticles}",
        f"xAOD::TrackParticleAuxContainer#LRT{outFlags.GSFTrackParticles}"
        f"Aux.{outFlags.GSFTrackParticlesSuppESD}"]
    if flags.Egamma.doTruthAssociation:
        toESD += [
            f"xAOD::TruthParticleContainer#LRT{outFlags.TruthParticles}",
            f"xAOD::TruthParticleAuxContainer#LRT{outFlags.TruthParticles}"
            f"Aux.{outFlags.TruthParticlesSuppESD}"]


    toAOD += [
        f"xAOD::ElectronContainer#LRT{outFlags.Electrons}",
        f"xAOD::ElectronAuxContainer#LRT{outFlags.Electrons}"
        f"Aux.{outFlags.ElectronsSuppAOD}"]
    toAOD += [
        f"xAOD::CaloClusterContainer#LRT{outFlags.CaloClusters}",
        f"xAOD::CaloClusterAuxContainer#LRT{outFlags.CaloClusters}"
        f"Aux.{outFlags.CaloClustersSuppAOD}"]
    toAOD += [
        f"CaloClusterCellLinkContainer#LRT{outFlags.CaloClusters}"
        "_links"]
    toAOD += [
        f"xAOD::TrackParticleContainer#LRT{outFlags.GSFTrackParticles}",
        f"xAOD::TrackParticleAuxContainer#LRT{outFlags.GSFTrackParticles}"
        f"Aux.{outFlags.GSFTrackParticlesSuppAOD}"]
    if flags.Egamma.doTruthAssociation:
        toAOD += [
            f"xAOD::TruthParticleContainer#LRT{outFlags.TruthParticles}",
            f"xAOD::TruthParticleAuxContainer#LRT{outFlags.TruthParticles}"
            f"Aux.{outFlags.TruthParticlesSuppAOD}"]

        

    if flags.Output.doWriteESD:
        from OutputStreamAthenaPool.OutputStreamConfig import addToESD
        acc.merge(addToESD(flags, toESD))

    if flags.Output.doWriteAOD:
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
        acc.merge(addToAOD(flags, toAOD))

    return acc
