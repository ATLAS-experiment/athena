# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import HIMode

def createEgammaConfigFlags():
    egcf = AthConfigFlags()

    # Do standard cluster-based egamma algorithm.
    egcf.addFlag("Egamma.doCentral",
                 lambda prevFlags: prevFlags.Detector.EnableCalo)

    egcf.addFlag("Egamma.doForward",
                 lambda prevFlags: prevFlags.Detector.EnableCalo
                 and prevFlags.Reco.HIMode is not HIMode.HI)

    # Run the GSF refitting/egamma Tracking it is calo seeded.
    egcf.addFlag("Egamma.doTracking",
                 lambda prevFlags: (
                     prevFlags.Detector.EnableID
                     or prevFlags.Detector.EnableITk)
                 and prevFlags.Detector.EnableCalo)

    # Build photon conversion vertices.
    egcf.addFlag("Egamma.doConversionBuilding",
                 lambda prevFlags: prevFlags.Egamma.doTracking)

    from egammaConfig.egammaPhotonConvFlags import (
        createEGammaPhotonConvFlags)
    egcf.addFlagsCategory("Egamma.PhotonConv",
                          createEGammaPhotonConvFlags, prefix=True)

    # Do egamma truth association when running on MC.
    egcf.addFlag("Egamma.doTruthAssociation",
                 lambda prevFlags: prevFlags.Input.isMC or
                 prevFlags.Overlay.DataOverlay)

    # Do e/gamma track thinning (Although we call the alg slimming...).
    egcf.addFlag("Egamma.doTrackThinning",
                 lambda prevFlags: prevFlags.Output.doWriteAOD and
                 prevFlags.Egamma.doTracking)

    # Keep egamma Cells in AOD. 
    # Should these not be an internal key if they are not written the xAOD?
    egcf.addFlag("Egamma.keepCaloCellsAOD",
                 lambda prevFlags: prevFlags.Egamma.doCentral or
                 prevFlags.Egamma.doForward)

    egcf.addFlag("Egamma.slimGSFTrkTracks",
                 lambda prevFlags: prevFlags.Egamma.doTracking)

    # Egamma runs in low <mu> mode (e.g UPC )
    egcf.addFlag("Egamma.doLowMu",  
                 lambda prevFlags: prevFlags.Reco.HIMode is HIMode.UPC)

    # The cluster corrections/calib.
    egcf.addFlag("Egamma.Calib.ClusterCorrectionVersion",
                 'v12phiflip_noecorrnogap')
    egcf.addFlag("Egamma.Calib.SuperClusterCorrectionVersion",
                 'v12phiflip_supercluster')
    egcf.addFlag("Egamma.Calib.MVAVersion", 'egammaMVACalib/offline/v7')

    # The input keys
    egcf.addFlag("Egamma.Keys.Input.CaloCells",
                 lambda prevFlags: prevFlags.HeavyIon.Egamma.SubtractedCells
                 if prevFlags.HeavyIon.Egamma.doSubtractedClusters
                 else 'AllCalo')
    egcf.addFlag("Egamma.Keys.Input.TopoClusters",
                 lambda prevFlags: prevFlags.HeavyIon.Egamma.UncalibCaloTopoCluster
                 if prevFlags.HeavyIon.Egamma.doSubtractedClusters
                 else 'CaloTopoClusters')
    egcf.addFlag("Egamma.Keys.Input.TruthParticles", 'TruthParticles')
    egcf.addFlag("Egamma.Keys.Input.TruthEvents", 'TruthEvents')
    egcf.addFlag("Egamma.Keys.Input.TrackParticles", 'InDetTrackParticles') # Input to GSF.

    # The topoclusters selected for egamma from the input topoclusters.
    egcf.addFlag("Egamma.Keys.Internal.EgammaTopoClusters",
                 lambda prevFlags: prevFlags.HeavyIon.Egamma.EgammaTopoCluster
                 if prevFlags.HeavyIon.Egamma.doSubtractedClusters
                 else 'egammaTopoClusters')
    egcf.addFlag("Egamma.Keys.Internal.ForwardTopoClusters",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Internal.EgammaTopoClusters + 'Fwd'))
    egcf.addFlag("Egamma.Keys.Internal.EgammaRecs", 'egammaRecCollection')
    egcf.addFlag("Egamma.Keys.Internal.PhotonSuperRecs",
                 'PhotonSuperRecCollection')
    egcf.addFlag("Egamma.Keys.Internal.ElectronSuperRecs",
                 'ElectronSuperRecCollection')
    egcf.addFlag("Egamma.Keys.Internal.PhotonSuperClusters",
                 'PhotonSuperClusters')
    egcf.addFlag("Egamma.Keys.Internal.ElectronSuperClusters",
                 'ElectronSuperClusters')

    egcf.addFlag("Egamma.Keys.Output.ConversionVertices",
                 'GSFConversionVertices')
    egcf.addFlag("Egamma.Keys.Output.ConversionVerticesSuppESD",
                 '-vxTrackAtVertex')
    egcf.addFlag("Egamma.Keys.Output.ConversionVerticesSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.ConversionVerticesSuppESD))

    egcf.addFlag("Egamma.Keys.Output.CaloClusters", 'egammaClusters')
    egcf.addFlag("Egamma.Keys.Output.CaloClustersSuppESD", '')
    egcf.addFlag("Egamma.Keys.Output.CaloClustersSuppAOD", '')
    egcf.addFlag("Egamma.Keys.Output.EgammaSuppAOD",
                 '-e033.-e011.-e333.-e335.-e337.-e377.'
                 '-isEMLoose.-isEMTight')

    # Remove GlobalFELinks - these are links between FlowElement (FE)
    # containers created in jet finding and electrons/photons.
    # Since these transient FE containers are not in the ESD/AOD,
    # we should not write out these links.
    egcf.addFlag("Egamma.Keys.Output.EgammaSuppESD",
                 '-ptconeCorrBitset.-ptconecoreTrackPtrCorrection.'
                 '-topoetconeCorrBitset.'
                 '-chargedGlobalFELinks.-neutralGlobalFELinks')

    egcf.addFlag("Egamma.Keys.Output.Electrons", 'Electrons')
    egcf.addFlag("Egamma.Keys.Output.ElectronsSuppESD",
                 lambda prevFlags: (prevFlags.Egamma.Keys.Output.EgammaSuppESD))
    egcf.addFlag("Egamma.Keys.Output.ElectronsSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.ElectronsSuppESD + '.' +
                     prevFlags.Egamma.Keys.Output.EgammaSuppAOD + '.'
                     "-EgammaCovarianceMatrix."
                     "-isEMLHLoose.-isEMLHTight.-isEMLHMedium.-isEMMedium"))

    egcf.addFlag("Egamma.Keys.Output.ForwardElectrons", 'ForwardElectrons')
    egcf.addFlag("Egamma.Keys.Output.ForwardElectronsSuppESD", '')
    egcf.addFlag("Egamma.Keys.Output.ForwardElectronsSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.ForwardElectronsSuppESD +
                     '-isEMTight.-isEMMedium.-isEMLoose'))

    egcf.addFlag("Egamma.Keys.Output.ForwardClusters",
                 'ForwardElectronClusters')
    egcf.addFlag("Egamma.Keys.Output.ForwardClustersSuppESD", '-SisterCluster')
    egcf.addFlag("Egamma.Keys.Output.ForwardClustersSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.ForwardClustersSuppESD))

    egcf.addFlag("Egamma.Keys.Output.Photons", 'Photons')
    egcf.addFlag("Egamma.Keys.Output.PhotonsSuppESD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.EgammaSuppESD +
                     '.-ptvarcone20.-ptvarcone30'))
    egcf.addFlag("Egamma.Keys.Output.PhotonsSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.PhotonsSuppESD + '.' +
                     prevFlags.Egamma.Keys.Output.EgammaSuppAOD))

    egcf.addFlag("Egamma.Keys.Output.GSFTrackParticles", 'GSFTrackParticles')
    egcf.addFlag("Egamma.Keys.Output.FwdGSFTrackParticles", 
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.GSFTrackParticles + 'Fwd'))
    egcf.addFlag("Egamma.Keys.Output.GSFTrackParticlesSuppESD", '')
    egcf.addFlag("Egamma.Keys.Output.GSFTrackParticlesSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.GSFTrackParticlesSuppESD))

    egcf.addFlag("Egamma.Keys.Output.TruthParticles", 'egammaTruthParticles')
    egcf.addFlag("Egamma.Keys.Output.TruthParticlesSuppESD", '')
    egcf.addFlag("Egamma.Keys.Output.TruthParticlesSuppAOD",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.TruthParticlesSuppESD))

    # Not written in AOD.
    egcf.addFlag("Egamma.Keys.Output.GSFTracks", 'GSFTracks')
    egcf.addFlag("Egamma.Keys.Output.FwdGSFTracks",
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.GSFTracks + 'Fwd'))

    # These are the clusters that are used to determine which cells to write 
    # out to AOD don't define SuppAOD because the whole container is suppressed.
    egcf.addFlag("Egamma.Keys.Output.EgammaLargeClusters", 'egamma711Clusters')
    egcf.addFlag("Egamma.Keys.Output.EgammaLargeClustersSuppESD", '')

    egcf.addFlag("Egamma.Keys.Output.EgammaLargeFWDClusters",
                 'egamma66FWDClusters')
    egcf.addFlag("Egamma.Keys.Output.EgammaLargeFWDClustersSuppESD", '')

    egcf.addFlag("Egamma.Keys.Output.TrkPartContainerName", 
                 'egammaSelectedTrackParticles')

    egcf.addFlag("Egamma.Keys.Output.FwdTrkPartContainerName", 
                 lambda prevFlags: (
                     prevFlags.Egamma.Keys.Output.TrkPartContainerName + 'Fwd'))
                     
    return egcf


if __name__ == "__main__":

    flags = createEgammaConfigFlags()
    flags.dump()
