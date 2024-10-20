#!/usr/bin/env athena.py --CA
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.addFlag('RecExRecoTest.doMC', False, help='custom option for RexExRecoText to run data or MC test')
    flags.fillFromArgs()

    # Use latest Data or MC
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags, defaultGeometryTags
    if flags.RecExRecoTest.doMC is True:
        # Currently not working on latest ESD, see ATLASRECTS-8111
        #flags.Input.Files = defaultTestFiles.ESD_RUN3_MC
        #flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC

        flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/ESD.16747874._000011_100events.pool.root"]
        from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
        setupDetectorFlags(flags)

    else:
        flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.lock()

    from MuonConfig.MuonReconstructionConfig import MuonReconstructionConfigTest
    MuonReconstructionConfigTest(flags)
