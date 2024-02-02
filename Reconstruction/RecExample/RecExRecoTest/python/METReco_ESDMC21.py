# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
    # Use latest MC21 tag to pick up latest muon folders apparently needed
    flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    # We have to set the production step, which PFFlow muon linking uses for autoconfiguration.
    from AthenaConfiguration.Enums import ProductionStep
    flags.Common.ProductionStep=ProductionStep.Derivation
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Setup calorimeter geometry, which is needed for jet reconstruction
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from JetRecConfig.JetRecoSteering import JetRecoSteeringCfg
    acc.merge(JetRecoSteeringCfg(flags))

    # We also need to build links between the newly
    # created jet constituents (GlobalFE)
    # and electrons,photons,muons and taus
    from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
    acc.merge(PFGlobalFlowElementLinkingCfg(flags))

    from METReconstruction.METRecCfg import METCfg
    acc.merge(METCfg(flags))

    acc.run(100)
