# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PFlowTests/mc21_13p6TeV/mc21_13p6TeV.601589.PhPy8EG_A14_ttbar_hdamp258p75_nonallhadron.recon.ESD.e8485_s3986_r14060/ESD_100Events.pool.root"]
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.Calo.TopoCluster.addCalibrationHitDecoration=True
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    cfg.merge(GeoModelCfg(cfgFlags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(cfgFlags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(cfgFlags))

    from PFlowUtils.PFlowCalibHitDecoratorCfg import PFlowCalibHitDecoratorCfg
    cfg.merge(PFlowCalibHitDecoratorCfg(cfgFlags))

    #Rename existing decorations in input file, such that we can create new ones.
    from SGComps.AddressRemappingConfig import InputRenameCfg
    remaps = [
        InputRenameCfg ('xAOD::CaloClusterContainer','CaloCalTopoClusters.calclus_NLeadingTruthParticleBarcodeEnergyPairs','CaloCalTopoClusters.calclus_NLeadingTruthParticleBarcodeEnergyPairs_OLD'),
        InputRenameCfg ('xAOD::FlowElementContainer', 'JetETMissNeutralParticleFlowObjects.calpfo_NLeadingTruthParticleBarcodeEnergyPairs', 'JetETMissNeutralParticleFlowObjects.calpfo_NLeadingTruthParticleBarcodeEnergyPairs_OLD')
    ]

    for mapping in remaps:
        cfg.merge(mapping)

    cfg.run()
