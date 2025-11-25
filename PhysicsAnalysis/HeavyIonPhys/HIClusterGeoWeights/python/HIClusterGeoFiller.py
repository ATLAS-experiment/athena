# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def HIClusterGeoFillerCfg(flags, **kwargs):
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  from AthenaConfiguration.ComponentFactory import CompFactory
  acc = ComponentAccumulator()

  # main algorithm
  kwargs.setdefault("EventInfoKey", "EventInfo")
  kwargs.setdefault("CaloClusterContainerKey", "HIClusters")
  kwargs.setdefault("HIEventShapeKey", "CaloSums")
  kwargs.setdefault("HistStream", "CLUSTERGEOFILLERSTREAM")
  kwargs.setdefault("minFCalET", 0.0)
  kwargs.setdefault("maxFCalET", 5.4)
  hiClusterGeoFiller = CompFactory.HIClusterGeo_HistoFiller("HIClusterGeo_HistoFiller", **kwargs)
  acc.addEventAlgo(hiClusterGeoFiller)

  # writing histograms into a root file
  thistSvc = CompFactory.THistSvc(Output=["CLUSTERGEOFILLERSTREAM DATAFILE='HIClusterGeo_HistoFiller.root' OPT='RECREATE'"])
  acc.addService(thistSvc)

  return acc


if __name__ == "__main__":
    """
    This macro will generate a new root file. There will be one histogram per lumiblock in the input files. 
    There will be no histograms for lumiblocks that are not present in the input files.
    To get the output:
        1) setup Athena:
            $ asetup Athena,main,latest,here 
        2) run this code:
            $ python -m HIClusterGeoWeights.HIClusterGeoFiller
        2.B) alternatively, specify the input file:
            $ python -m HIClusterGeoWeights.HIClusterGeoFiller --filesInput=some.input.file.AOD.root
        2.C) alternatively, submit the job to grid:
            $ lsetup panda
            $ pathena --trf "python -m HIClusterGeoWeights.HIClusterGeoFiller --filesInput=%IN" --inDS some.input.dataset --outDS user.$USER.some.output.dataset --extOutFile=HIClusterGeo_HistoFiller.root
        3) the new file is "HIClusterGeo_HistoFiller.root"
    The output file shall be fed to "makeHIResponse" macro. There shall be one file per each run. Thus, it might be
    necessary to merge several "HIClusterGeo_HistoFiller.root" files with "hadd".
    """
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()

    # for testing
    from os import listdir, path
    direc = "../storage/data18_hi.00367165.physics_PC.merge.AOD.f1030_m2048" # Pb+Pb 5.02TeV
    # direc = "../storage/data24_hi.00490182.physics_MinBias.merge.AOD.f1550_m2267" # Pb+Pb 5.36TeV
    if path.isdir(direc):
        # add files from the folder only if such folder exists; this prevents throwing an error from 'listdir' on grid
        flags.Input.Files = ["%s/%s" % (direc, x) for x in listdir(direc) if x[x.rfind(".") + 1:] in ["root", "1", "2"]]

    flags.Exec.MaxEvents=-1
    flags.Exec.SkipEvents=0
    flags.Concurrency.NumThreads=1

    # enables unit tests to switch only parts of reco such as (note the absence of spaces around the equal sign): 
    ### python -m HIClusterGeoWeights.HIClusterGeoFiller Exec.SkipEvents=15
    flags.fillFromArgs() 
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # to read from AOD
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # main task
    acc.merge(HIClusterGeoFillerCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    import sys
    sys.exit(acc.run().isFailure())
