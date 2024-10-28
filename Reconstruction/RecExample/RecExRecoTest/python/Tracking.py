#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    # Setup flags with custom input-choice
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.addFlag('RecExRecoTest.doMC',  False, help='custom option for RexExRecoText to run data or MC test')
    flags.addFlag('RecExRecoTest.doESD', False, help='custom option for RexExRecoText to run on ESD. Not a recommended workflow')
    flags.fillFromArgs()
        
    # Use latest Data or MC
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags, defaultGeometryTags
    if flags.RecExRecoTest.doMC:
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        if flags.RecExRecoTest.doESD:
            flags.Input.Files = defaultTestFiles.ESD_RUN3_MC
        else:
            flags.Input.Files = defaultTestFiles.RDO_RUN3
            print("MC RDO inputs not currently supported for RecExRecoTest.Tracking, see ATLASRECTS-8110")
            exit(1)
    else:
        flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.lock()

    from InDetConfig.TrackRecoConfig import TrackRecoConfigTest
    TrackRecoConfigTest(flags)

