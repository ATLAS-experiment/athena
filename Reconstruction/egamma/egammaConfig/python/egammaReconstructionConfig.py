# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the EGamma reconstruction.
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def egammaReconstructionCfg(flags, name="egammaReconstruction"):

    mlog = logging.getLogger(name)
    mlog.info('Starting EGamma reconstruction configuration')

    acc = ComponentAccumulator()
   
    # In case it has not be added upstream already
    from egammaAlgs.egammaTopoClusterCopierConfig import (
        egammaTopoClusterCopierCfg)
    acc.merge(egammaTopoClusterCopierCfg(flags))

    # Add e/gamma tracking algorithms
    if flags.Egamma.doTracking:
        
        if flags.Acts.GsfRefitActs:
            
            # === TODO === : Acts EM extrapolator is missing.
            
            # from egammaAlgs.egammaSelectedTrackCopyConfig import (
            #     egammaSelectedTrackCopyCfg)
            # acc.merge(egammaSelectedTrackCopyCfg(flags))
            
            from egammaAlgs.ActsEMBremCollectionBuilderConfig import (
                ActsEMBremCollectionBuilderCfg)
            acc.merge(ActsEMBremCollectionBuilderCfg(flags))
            
        else:
            from egammaAlgs.egammaSelectedTrackCopyConfig import (
                egammaSelectedTrackCopyCfg)
            acc.merge(egammaSelectedTrackCopyCfg(flags))
            
            from egammaAlgs.EMBremCollectionBuilderConfig import (
                EMBremCollectionBuilderCfg)
            acc.merge(EMBremCollectionBuilderCfg(flags))

        if (flags.Tracking.writeExtendedSi_PRDInfo or
            flags.Tracking.writeExtendedTRT_PRDInfo):
            from DerivationFrameworkInDet.InDetToolsConfig import (
                GSFTSOS_CommonKernelCfg)
            acc.merge(GSFTSOS_CommonKernelCfg(flags))

    # Add e/gamma conversion finding
    if flags.Egamma.doConversionBuilding:
        from egammaAlgs.EMVertexBuilderConfig import (
            EMVertexBuilderCfg)
        acc.merge(EMVertexBuilderCfg(flags))

    # Add algorithms to produce
    # xAOD Electrons and Photons
    if flags.Egamma.doCentral:
        from egammaAlgs.egammaRecBuilderConfig import (
            egammaRecBuilderCfg)
        acc.merge(egammaRecBuilderCfg(flags))

        from egammaAlgs.egammaSuperClusterBuilderConfig import (
            electronSuperClusterBuilderCfg, photonSuperClusterBuilderCfg)
        acc.merge(electronSuperClusterBuilderCfg(flags))
        acc.merge(photonSuperClusterBuilderCfg(flags))

        from egammaAlgs.xAODEgammaBuilderConfig import (
            xAODEgammaBuilderCfg)
        acc.merge(xAODEgammaBuilderCfg(flags))

        from egammaAlgs.egammaLargeClusterMakerAlgConfig import (
            egammaLargeClusterMakerAlgCfg)
        acc.merge(egammaLargeClusterMakerAlgCfg(flags))

    # Add calo seeded forward algorithms to produce xAOD Forward Electrons.
    if flags.Egamma.doForward:
        from egammaAlgs.egammaForwardBuilderConfig import (
            egammaForwardBuilderCfg)
        acc.merge(egammaForwardBuilderCfg(flags))

        from egammaAlgs.egammaLargeFWDClusterMakerAlgConfig import (
            egammaLargeFWDClusterMakerAlgCfg)
        acc.merge(egammaLargeFWDClusterMakerAlgCfg(flags))

    # Add truth association.
    if flags.Egamma.doTruthAssociation:
        from egammaAlgs.egammaTruthAssociationConfig import (
            egammaTruthAssociationCfg)
        acc.merge(egammaTruthAssociationCfg(flags))

    mlog.info("EGamma reconstruction configured")

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteESD = True  # To test the ESD parts.
    flags.Output.doWriteAOD = True  # To test the AOD parts.
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(egammaReconstructionCfg(flags))
    acc.printConfig(withDetails=True, printDefaults=True) 

    with open("egammareconstructionconfig.pkl", "wb") as f:
        acc.store(f)
