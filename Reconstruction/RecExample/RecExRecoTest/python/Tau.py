#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    # Setup flags with custom input-choice
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.addFlag('RecExRecoTest.doMC', False, help='custom option for RexExRecoText to run data or MC test')
    flags.fillFromArgs()
        
    # Use latest Data or MC
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags, defaultGeometryTags
    if flags.RecExRecoTest.doMC is True:
        # Needs to be fixed to use latest ESDs, see ATLASRECTS-8112
        #flags.Input.Files = defaultTestFiles.ESD_RUN3_MC
        #flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
        flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    else:
        flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

        # Schedule Tau Reco flags for RecoSteering
        from tauRec.ConfigurationHelpers import StandaloneTauRecoFlags
        StandaloneTauRecoFlags(flags)
        flags.Jet.strictMode = False
    flags.lock()

    # Unify these two strategies? See ATLASRECTS-8112
    if flags.RecExRecoTest.doMC is True:
        from tauRec.TauConfig import TauConfigTest
        TauConfigTest(flags)
    else:
        from RecJobTransforms.RecoSteering import RecoSteering
        acc = RecoSteering(flags)
        acc.run()