# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HICaloGeoExtractCfg(flags):
    acc = ComponentAccumulator()

    from CaloRec.CaloRecoConfig import CaloRecoCfg  
    acc.merge(CaloRecoCfg(flags))
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = acc.getPrimaryAndMerge(CaloTowerMakerCfg(flags))
    inputTowers = towerMaker.TowerContainerName

    extractCGC = CompFactory.ExtractCaloGeoConstants("ExtractCaloGeoConstants", InputTowerKey=inputTowers, HistStream="CALOGEOEXTRACTSTREAM")
    acc.addEventAlgo(extractCGC)

    acc.addService(CompFactory.THistSvc(Output=["CALOGEOEXTRACTSTREAM DATAFILE='cluster.geo.XXX.root' OPT='RECREATE'"]))

    return acc


if __name__ == "__main__":
    """
     This macro will generate a new root weight file with histograms "h3_w", "h3_eta", "h3_phi", and "h3_R" 
    that are stored in "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/HIJetCorrection/cluster.geo....root" files.
     It's based on the code from https://gitlab.cern.ch/atlas-physics/hi/jets/HICaloGeo/
     To have correct weights, one needs to assure:
        1) have consistent input file, conditions, and geometry
        2) have only 1 event processed
    To get the new file:
        1) setup Athena:
            $ asetup Athena,master,latest,here 
        2) run this code:
            $ python -m HIEventUtils.HICaloGeoExtract
        3) the new file is "cluster.geo.XXX.root"; rename it however is appropriate
        
     In the root weight file, there are also histograms "h3_eta_phi_response", "h3_eta_phi_offset", 
    and "h1_run_index". These are produced elsewhere.
    """

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    flags = initConfigFlags()

    ### input for Run2:
    # flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    # flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-RUN2-09" 
    # flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

    ### input for Run3:
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data22_hi/RAWFiles/data22_hi.00440101.physics_MinBias.daq.RAW/data22_hi.00440101.physics_MinBias.daq.RAW._lb0214._SFO-11._0001.data"]
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2022-09"
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    ### ### ###

    flags.Exec.MaxEvents=1 # always only 1 event!
    flags.Concurrency.NumThreads=1
    flags.Trigger.triggerConfig = "DB"
    flags.Reco.EnableHI = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    acc.merge(HICaloGeoExtractCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    import sys
    sys.exit(acc.run().isFailure())
