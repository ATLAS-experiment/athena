# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import  SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonSPId_2022_13p6TeV_00431493.root")
    parser.set_defaults(inputFile=[
                                   "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    ])
    args = parser.parse_args()
    from MuonSPId.muonSPIdDump import main
    main(args)
