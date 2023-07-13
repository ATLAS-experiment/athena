# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os
import os.path

##---
#inputDir = "/tank/awharton/ForLME/Data/data22_13p6TeV.periodAllYear.physics_Main.PhysCont.DAOD_PHYS.grp22_v02_p5632/"
#inputDir = "/home/atlas/lukem/QT-SecVtxTool/data/data22_13p6TeV.periodAllYear.physics_Main.PhysCont.DAOD_LLP1.grp22_v01_p5600/"
#inputDir =  "/home/atlas/lukem/QT-SecVtxTool/data/data22_13p6TeV.00427929.physics_Main.deriv.DAOD_FTAG1.f1246_m2112_p5444/"
#inputDir = "/tank/awharton/ForLME/Data/data22_13p6TeV.00440613.physics_Main.deriv.DAOD_FTAG1.f1321_m2153_p5444/"
#inputDir = "/tank/awharton/ForLME/Data/mc21_13p6TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.deriv.DAOD_FTAG1.e8453_s3873_r13829_p5654/"
inputDir = "/tank/awharton/ForLME/Data/mc21_13p6TeV.601237.PhPy8EG_A14_ttbar_hdamp258p75_allhad.deriv.DAOD_FTAG1.e8453_e8455_s3873_s3874_r13829_r13831_p5654/"

#inputDir = "/tank/awharton/ForLME/Data/data22_13p6TeV.periodAllYear.physics_Main.PhysCont.DAOD_PHYSLITE.grp22_v02_p5632/"
#inputDir = "/home/atlas/lukem/QT-SecVtxTool/data/data22_13p6TeV.00427884.physics_Main.deriv.DAOD_PHYS.r13928_p5279_p5514/"
#inputDir = "/home/atlas/lukem/QT-SecVtxTool/data/data22_13p6TeV.00427882.physics_Main.deriv.DAOD_PHYS.r13928_p5279_p5514/"
#inputDir = "/cvmfs/atlas.cern.ch/repo/tutorials/asg/cern-jun2022/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8357_s3802_r13508_p5057"
maxEvents = 4

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
    #from GNNVertexConstructor.GNNVertexConstructorToolConfig import GNNToolCfg
    #from GNNVertexConstructor.GNNVertexConstructorToolConfig import GNNVertexConstructorToolCfg

    acc.merge(GNNVertexConstructorAlgCfg(flags, name="LME_devAlg", OutputLevel=Lvl.DEBUG))

    ##---

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TRUTH0SlimmingHelper = SlimmingHelper("TRUTH0SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, ConfigFlags = flags)
    TRUTH0SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD:EventAuxInfo',
                                               'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                               'TruthVertices':'xAOD::TruthVertexContainer','TruthVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'TruthParticles':'xAOD::TruthParticleContainer','TruthParticlesAux':'xAOD::TruthParticleAuxContainer',
                                               'AntiKt4EMPFlowJets':'xAOD::JetContainer', 'AntiKt4EMPFlowJetsAux': 'xAOD::JetAuxContainer',
                                               'BTagging':'xAOD::BTaggingContainer', 'BTaggingAux': 'xAOD::BTaggingAuxContainer',
                                               'TrackParticle':'xAOD::TrackParticleContainer', 'TrackParticleAux': 'xAOD::TrackParticleAuxContainer'} 

    TRUTH0SlimmingHelper.AllVariables = [ 'EventInfo',
                                          'TruthEvents', 
                                          'TruthVertices',
                                          'TruthParticles',
                                          'AntiKt4EMPFlowJets',
                                          'BTagging',
                                          'TrackParticle']

    # Metadata
    #TRUTH0MetaDataItems = [ "xAOD::TruthMetaDataContainer#TruthMetaData", "xAOD::TruthMetaDataAuxContainer#TruthMetaDataAux." ]

    # Create output stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    TRUTH0ItemList = TRUTH0SlimmingHelper.GetItemList()
    TRUTH0ItemList+=["xAOD::TrackParticleContainer#DecoratedTrackParticles","xAOD::TrackParticleContainer#DecoratedTrackParticlesAux."]
    acc.merge(OutputStreamCfg(flags, "GNNVertexOutputDecoNew", ItemList=TRUTH0ItemList))


    acc.printConfig(withDetails=True, summariseProps=True)
    status = acc.run()
    
    print (status)

##---

if "__main__" == __name__:
    main()
