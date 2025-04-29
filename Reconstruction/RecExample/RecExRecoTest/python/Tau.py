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
    if flags.RecExRecoTest.doMC:
        flags.Input.Files = defaultTestFiles.RDO_RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
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
    if flags.RecExRecoTest.doMC:
        from tauRec.TauConfig import TauConfigTest
        TauConfigTest(flags)
    else:
        from RecJobTransforms.RecoSteering import RecoSteering
        acc = RecoSteering(flags)
        acc.run()