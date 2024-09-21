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
        flags.Input.Files = defaultTestFiles.RDO_RUN3
    else:
        flags.Input.Files = defaultTestFiles.RAW_RUN3_DATA24
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA
        flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    from egammaConfig.ConfigurationHelpers import egammaOnlyFromRaw
    egammaOnlyFromRaw(flags)
    flags.lock()

    from egammaConfig.egammaSteeringConfig import egammaSteeringConfigTest
    egammaSteeringConfigTest(flags)