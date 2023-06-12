# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os
import os.path

##---

inputDir = "/home/atlas/lukem/mc20_13TeV.312939.PowhegPythia8EvtGen_ZH_H125_a35a35_4b_ctau100.recon.AOD.e7962_e5984_s3126_r13051_r13474/"
maxEvents = 10

##---


def getInputFiles():
    Files = [
        os.path.join(directoryPath, file)
        for directoryPath, directories, files in os.walk(inputDir)
        for file in files
    ]

    Files.sort()

    return Files


##---


def main():
    from AthenaCommon.Configurable import Configurable

    Configurable.configurableRun3Behavior = 1

    ##---

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = maxEvents
    flags.Input.Files = getInputFiles()
    flags.Input.isMC = True
    flags.lock()

    ##---

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    acc = MainServicesCfg(flags)

    ##---

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    acc.merge(PoolReadCfg(flags))

    ##---

    import AthenaCommon.Constants as Lvl
    from GNNVertexConstructor.GNNVertexConstructorToolConfig import GNNVertexConstructorAlgCfg
    #from GNNVertexConstructor.GNNVertexConstructorToolConfig import GNNVertexConstructorToolCfg

    acc.merge(GNNVertexConstructorAlgCfg(flags, name="LME_devAlg", OutputLevel=Lvl.DEBUG))

    ##---

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
    #TRUTH0MetaDataItems = [ "xAOD::TruthMetaDataContainer#TruthMetaData", "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux." ]

    # Create output stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    TRUTH0ItemList = TRUTH0SlimmingHelper.GetItemList()
    TRUTH0ItemList+=["xAOD::TrackParticleContainer#DecoratedTrackParticles","xAOD::TrackParticlesContainer#DecoratedTrackParticlesAux."]
    acc.merge(OutputStreamCfg(flags, "GNNVertexOutput", ItemList=TRUTH0ItemList))



    ##--

    acc.printConfig(withDetails=True, summariseProps=True)
    status = acc.run()
    
    print (status)

##---

if "__main__" == __name__:
    main()
