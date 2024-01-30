# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Define method to construct configures Sec Vtx Finder alg
# attempted by N Ribaric (@LancasterUNI) neza.ribaric@cern.ch

if __name__ == "__main__":
    import AthenaCommon.Constants as Lvl

    # import the flags and set them
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Exec.MaxEvents = 50

    # use one of the predefined files
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Input.isMC=True

    # lock the flags
    flags.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # add the algorithm to the configuration
    from InDetConfig.InDetSecVtxFinderConfig import InDetSecVtxFinderAlgCfg
    acc.merge(InDetSecVtxFinderAlgCfg(flags,
              name = "InDetSecVtxFinder",
              FinderTool = "ISV",
              useTrackParticles = True,
              inputTrackParticles = "InDetTrackParticles",
              outputSecondaryVertices = "RecoSecVtx",
              doVertexMerging = False,
              OutputLevel = Lvl.INFO))

    # Contents
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TRUTH0SlimmingHelper = SlimmingHelper("TRUTH0SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, ConfigFlags = flags)
    TRUTH0SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD:EventAuxInfo',
                                               'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                               'TruthVertices':'xAOD::TruthVertexContainer','TruthVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'TruthParticles':'xAOD::TruthParticleContainer','TruthParticlesAux':'xAOD::TruthParticleAuxContainer'} 

    TRUTH0SlimmingHelper.AllVariables = [ 'EventInfo',
                                          'TruthEvents', 
                                          'TruthVertices',
                                          'TruthParticles']

    # Metadata
    TRUTH0MetaDataItems = [ "xAOD::TruthMetaDataContainer#TruthMetaData", "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux." ]

    # Create output stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    TRUTH0ItemList = TRUTH0SlimmingHelper.GetItemList()
    TRUTH0ItemList+=["xAOD::VertexContainer#RecoSecVtx","xAOD::VertexContainer#RecoSecVtxAux."]
    acc.merge(OutputStreamCfg(flags, "SecondayVertexOutput", ItemList=TRUTH0ItemList))

    # debug printout
    acc.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = acc.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
