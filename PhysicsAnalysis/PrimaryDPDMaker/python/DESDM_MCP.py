## Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

##-----------------------------------------------------------------------------
## Name: PerfDESDM_MS.py
##
## Description: This defines the content of the commissioning stream for
##
##-----------------------------------------------------------------------------
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def setupDESDMSkimmingToolsCfg(flags):
    result = ComponentAccumulator()
    from AthenaConfiguration.Enums import LHCPeriod
    isRun3 = flags.GeoModel.Run == LHCPeriod.Run3
    
    # ------------------------------------------------
    # All 'noalg' muon triggers ( to be re-checked at the start of run-3 )
    # ------------------------------------------------
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import TriggerSkimmingToolCfg
    MuonNoAlgTrig_EventSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags,  
                                                                name = "MuonNoAlgTrig_TriggerSkimmingTool",
                                                                TriggerListOR = [ "HLT_noalg_L1MU.*",
                                                                                  "HLT_noalg_L12MU.*",
                                                                                  "HLT_noalg_L1TGC_BURST",
                                                                                  "HLT_noalg_L1XE.*" ] if isRun3 else [
                                                                                  "HLT_noalg_L1MU4", 
                                                                                  "HLT_noalg_L1MU6", 
                                                                                  "HLT_noalg_L1MU10",
                                                                                  "HLT_noalg_L1MU11",
                                                                                  "HLT_noalg_L1MU15",
                                                                                  "HLT_noalg_L1MU20",
                                                                                  "HLT_noalg_L12MU4",
                                                                                  "HLT_noalg_L12MU6",
                                                                                  "HLT_noalg_L12MU10"]))
    ### The legacy configuration had a prescale tool of one here.. That's doing nothing

    # ------------------------------------------------
    # Orthogonal triggers (RPC needs)
    # ------------------------------------------------
    OrthoTrig_TriggerSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags, 
                                                              name = "OrthoTrig_TriggerSkimmingTool",
                                                              TriggerListOR = ["HLT_xe.*",
                                                                               "HLT_[0-9]?j[0-9]*",
                                                                               "HLT_j[0-9]*_(jes|lcw|nojcalib|sub|L1RD0|280eta320|320eta490).*",
                                                                               "HLT_[0-9]?j[0-9]*_b.*|j[0-9]*_[0-9]j[0-9]*_b.*",
                                                                               "HLT_tau.*",
                                                                               "HLT_[0-9]?e[0-9]*_(iloose|loose|medium|lhloose|lhmedium|lhtight|etcut)",
                                                                               "HLT_[0-9]?e[0-9]*_(iloose|loose|medium|lhloose|lhmedium|lhtight|etcut)_(iloose|nod0|HLTCalo|cu\
                                                                               td0dphideta|smooth|L1EM[0-9]*VH|L1EM[0-9]*)",
                                                                               "HLT_[0-9]?e[0-9]*_(iloose|loose|medium|lhloose|lhmedium|lhtight)_(iloose|nod0|HLTCalo|cutd0dph\
                                                                               ideta|smooth)_(HLTCalo|iloose|L1EM[0-9]*VH|L1EM[0-9]*)",
                                                                               "HLT_[0-9]?g[0-9]*_(loose|medium|tight|etcut)",
                                                                               "HLT_g[0-9]*_(loose|etcut)_(L1EM[0-9]*|L1EM[0-9]*VH)",
                                                                               "HLT_(e|g)[0-9]*_(loose|medium|lhmedium|tight)_g.*",
                                                                               "HLT_ht.*",
                                                                               "HLT_te.*",
                                                                               "HLT_xs.*",
                                                                               "HLT_mb.*"]  if not isRun3 else  [
                                                                               ### Run 3 triggers
                                                                               "HLT_xe.*",
                                                                               "HLT_[0-9]?j[0-9]*",
                                                                               "HLT_j[0-9]*_(pf_ftf|a10).*",
                                                                               "HLT_[0-9]?j[0-9]*_b.*|j[0-9]*_[0-9]j[0-9]*_b.*",
                                                                               "HLT_tau.*",
                                                                               "HLT_[0-9]?e[0-9]*_(lhvloose|lhloose|lhmedium|lhtight|etcut)",
                                                                               "HLT_[0-9]?g[0-9]*_(loose|medium|tight|etcut)",
                                                                               "HLT_(e|g)[0-9]*_(loose|medium|lhmedium|tight)_g.*",
                                                                               "HLT_ht.*",
                                                                               "HLT_te.*",
                                                                               "HLT_xs.*",
                                                                               "HLT_mb.*",
                                                                               "HLT_noalg_L1J.*",
                                                                               "HLT_noalg_L1TAU.*",
                                                                               "HLT_noalg_L1EM.*",
                                                                               "HLT_noalg_L1XE.*"] ))   
    # ------------------------------------------------
    # Offline orthogonal trigger selection (low pT)
    # ------------------------------------------------ 
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg   
    OrthoTrig_LowpTMuonEventStringSkimmingTool = result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                           name = "OrthoTrig_LowpTMuonEventStringSkimmingTool",
                                                                           expression = "(count(Muons.muonType == 0 && Muons.pt > 4*GeV && Muons.pt < 15*GeV) > 0)"))


    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import PrescaleToolCfg
    OrthoTrig_LowpTMuonPrescaleSkimmingTool =  result.getPrimaryAndMerge(PrescaleToolCfg(flags,
                                                                         name  = "OrthoTrig_LowpTMuonPrescaleSkimmingTool",
                                                                         Prescale = 10))
    #### Only write every 10th event from this stream
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationANDCfg
    OrthoTrig_LowpTMuonEventSkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                                     name="OrthoTrig_LowpTMuonEventSkimmingTool",
                                                                     FilterList=[OrthoTrig_LowpTMuonEventStringSkimmingTool, 
                                                                                 OrthoTrig_LowpTMuonPrescaleSkimmingTool]))
    
    
    # ------------------------------------------------
    # Offline orthogonal trigger selection (mid pT)

    OrthoTrig_MidpTMuonEventSkimmingTool = result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                     name = "OrthoTrig_MidpTMuonEventSkimmingTool",
                                                                     expression = "(count(Muons.muonType == 0 && Muons.pt >= 15*GeV) > 0)"))
    
    # ------------------------------------------------
    # Ortho trig events accepted if there is: 
    # [[a low pt (+ prescale) muon] OR [mid pt muon]] 
    # && [orthogonal trigger] event
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationORCfg
    OrthoTrig_pTMuonEventSkimmingTool=result.getPrimaryAndMerge(FilterCombinationORCfg(flags,
                                                                name="OrthoTrig_pTMuonEventSkimmingTool",
                                                                FilterList=[OrthoTrig_LowpTMuonEventSkimmingTool, 
                                                                            OrthoTrig_MidpTMuonEventSkimmingTool]))
    
    OrthoTrig_EventSkimmingTool=result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                         name="OrthoTrig_EventSkimmingTool",
                                                         FilterList=[OrthoTrig_pTMuonEventSkimmingTool, OrthoTrig_TriggerSkimmingTool]))

    # ------------------------------------------------
    # JPsi Trigger selection
    # ------------------------------------------------
    JPsiTrig_TriggerSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags,  
                                                             name = "JPsiTrig_TriggerSkimmingTool",
                                                             TriggerListOR = ["HLT_mu20_2mu0noL1_JpsimumuFS", 
                                                                              "HLT_mu18_2mu0noL1_JpsimumuFS", 
                                                                              "HLT_mu20_2mu4_JpsimumuL2",
                                                                              "HLT_mu18_2mu4_JpsimumuL2", 
                                                                              "HLT_mu6_bJpsi_Trkloose", 
                                                                              "HLT_mu18_bJpsi_Trkloose", 
                                                                              "HLT_mu4_bJpsi_Trkloose", 
                                                                              "HLT_mu20_msonly_mu6noL1_msonly_nscan05"] if not isRun3 else  [
                                                                              #### Run 3 menu 
                                                                              "HLT_2mu10_bJpsimumu",
                                                                              "HLT_mu11_mu6_bJpsimumu",
                                                                              "HLT_3mu4_bJpsi",
                                                                              "HLT_2mu6_bJpsimumu" ]))
    


    JPsiTrig_PrescaleSkimmingTool = result.getPrimaryAndMerge(PrescaleToolCfg(flags,   
                                                             name  = "JPsiTrig_PrescaleSkimmingTool",
                                                             Prescale = 2))

    # ------------------------------------------------
    # Jpsi trig events accepted if there is:
    # Jpsi trigger (+ prescale)
    JpsiTrig_EventSkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                           name="JpsiTrig_EventSkimmingTool",
                                                           FilterList=[JPsiTrig_PrescaleSkimmingTool, 
                                                                       JPsiTrig_TriggerSkimmingTool]))

    # ------------------------------------------------
    #For TGC, Muon HLT triggers:
    # ------------------------------------------------
    MuonHLTTrig_TriggerSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags,    
                                                                name = "MuonHLTTrig_TriggerSkimmingTool",
                                                                TriggerListOR =  ["HLT_mu.*"] ))

    
    # ------------------------------------------------
    # Dimuon selection
    MuonHLTTrig_DiMuonEventSkimmingTool =  result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                           name = "MuonHLTTrig_DiMuonEventSkimmingTool",
                                                                           expression = "( (count(Muons.muonType == 0 && Muons.pt > 25*GeV)  > 0) && (count(Muons.muonType == 0 && Muons.pt > 10*GeV)  > 1) )"))

    
    # ------------------------------------------------
    # Mid pT muon selection
    MuonHLTTrig_MidpTEventStringSkimmingTool = result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                         name = "MuonHLTTrig_MidpTEventStringSkimmingTool",
                                                                         expression = "(count(Muons.muonType == 0 && Muons.pt > 25*GeV) > 0)"))
    

    MuonHLTTrig_MidpTMuonPrescaleSkimmingTool = result.getPrimaryAndMerge(PrescaleToolCfg(flags,      
                                                                          name  = "MuonHLTTrig_MidpTMuonPrescaleSkimmingTool",
                                                                          Prescale = 2)) #okay with 2
    

    MuonHLTTrig_MidpTMuonEventSkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                                      name="MuonHLTTrig_MidpTMuonEventSkimmingTool",
                                                                      FilterList=[MuonHLTTrig_MidpTEventStringSkimmingTool, 
                                                                                  MuonHLTTrig_MidpTMuonPrescaleSkimmingTool]))
    # ------------------------------------------------
    # Muon HLT selection with combined logic
    MuonHLTTrig_pTMuonEventSkimmingTool=result.getPrimaryAndMerge(FilterCombinationORCfg(flags,
                                                                  name="MuonHLTTrig_pTMuonEventSkimmingTool",
                                                                  FilterList=[MuonHLTTrig_MidpTMuonEventSkimmingTool, 
                                                                              MuonHLTTrig_DiMuonEventSkimmingTool]))
    MuonHLTTrig_EventSkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags,
                                                              name="MuonHLTTrig_EventSkimmingTool",
                                                              FilterList=[MuonHLTTrig_pTMuonEventSkimmingTool, 
                                                                          MuonHLTTrig_TriggerSkimmingTool]))
    
    #### Asssemble everything together
    EventFilterTool=result.getPrimaryAndMerge(FilterCombinationORCfg(flags,
                                              name="DESDM_MCPEventFilterTool",
                                              FilterList=[MuonNoAlgTrig_EventSkimmingTool,
                                                          OrthoTrig_EventSkimmingTool, 
                                                          JpsiTrig_EventSkimmingTool, 
                                                          MuonHLTTrig_EventSkimmingTool]))
    result.addPublicTool(EventFilterTool, primary = True)
    return result


def setupAlignmentEventSkimmingToolCfg(flags,name = "AlignmentEventStringSkimmingTool", **kwargs ):
    kwargs.setdefault("expression", "(count(Muons.muonType == 0 && Muons.pt > 5*GeV) > 0)")
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    return xAODStringSkimmingToolCfg(flags,name, **kwargs)


def setupDESDMCPSkimmingAlgCfg(flags, name= "DESDMCPEventKernel", **kwargs):
    result = ComponentAccumulator()
    
    EventFilterTool = result.getPrimaryAndMerge(setupAlignmentEventSkimmingToolCfg(flags)) if flags.Muon.DESDM_MCP.doAlignmentFormat else \
                      result.getPrimaryAndMerge(setupDESDMSkimmingToolsCfg(flags))    
    kwargs.setdefault("SkimmingTools", [EventFilterTool])
    kwargs.setdefault("doChronoStat", flags.Concurrency.NumThreads <= 1)
    the_alg = CompFactory.DerivationFramework.DerivationKernel(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def DESDMCPOutputCfg(flags, **kwargs):
    result = ComponentAccumulator()
    from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory
    isRun3 = flags.GeoModel.Run == LHCPeriod.Run3

    container_items = ["xAOD::EventInfo#*", "xAOD::EventAuxInfo#*",
                       ### trigger navigation
                       "xAOD::TrigNavigation#TrigNavigation","xAOD::TrigNavigationAuxInfo#TrigNavigationAux.",
                       "xAOD::TrigDecision#xTrigDecision", "xAOD::TrigDecisionAuxInfo#xTrigDecisionAux.",
                       "xAOD::TrigCompositeContainer#HLTNav_Summary_OnlineSlimmed", "xAOD::TrigCompositeAuxContainer#HLTNav_Summary_OnlineSlimmedAux.",
                       "xAOD::TrigConfKeys#TrigConfKeys", "HLT::HLTResult#HLTResult_HLT",
                       ### Primary vertex collection
                       "xAOD::VertexContainer#PrimaryVertices",
                       "xAOD::VertexAuxContainer#PrimaryVerticesAux.-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV",
                       ## Tile containers to be checked 
                      "TileDigitsContainer#MuRcvDigitsCnt",
                      "TileRawChannelContainer#MuRcvRawChCnt",
                      "TileMuonReceiverContainer#TileMuRcvCnt",
                      #HLT                      
                      "xAOD::L2StandAloneMuonContainer#HLT_xAOD__L2StandAloneMuonContainer_MuonL2SAInfo",
                      "xAOD::L2StandAloneMuonAuxContainer#HLT_xAOD__L2StandAloneMuonContainer_MuonL2SAInfoAux.",

                      "xAOD::L2CombinedMuonContainer#HLT_xAOD__L2CombinedMuonContainer_MuonL2CBInfo",
                      "xAOD::L2CombinedMuonAuxContainer#HLT_xAOD__L2CombinedMuonContainer_MuonL2CBInfoAux.",

                      "xAOD::L2IsoMuonContainer#HLT_xAOD__L2IsoMuonContainer_MuonL2ISInfo",
                      "xAOD::L2IsoMuonAuxContainer#HLT_xAOD__L2IsoMuonContainer_MuonL2ISInfoAux.",
                      ]
    
    if flags.Muon.DESDM_MCP.doAlignmentFormat:
        container_items.append("xAOD::L2CombinedMuonContainer#HLT_MuonL2CBInfo")
        container_items.append("xAOD::L2CombinedMuonAuxContainer#HLT_MuonL2CBInfoAux.")

    if flags.Input.isMC:
        container_items +=["xAOD::TruthParticleContainer#MuonTruthParticles",
                           "xAOD::TruthParticleAuxContainer#MuonTruthParticlesAux."]

    l1_muon = ["LVL1MuonRoIs", "LVL1MuonRoIsBCp1", "LVL1MuonRoIsBCp2", "LVL1MuonRoIsBCm1","LVL1MuonRoIsBCm2",
                "HLT_xAOD__MuonRoIContainer_L1TopoMuon"]
    for l1 in l1_muon:
        container_items +=["xAOD::MuonRoIContainer#{roi}".format(roi = l1),
                           "xAOD::MuonRoIAuxContainer#{roi}Aux.".format(roi = l1)
                        ]

   
    xaod_muon = [ "Muons", 
                  "HLT_MuonsCB_RoI", 
                  "HLT_xAOD__MuonContainer_MuonEFInfo", 
                  "HLT_xAOD__MuonContainer_MuTagIMO_EF",
                  "HLT_xAOD__MuonContainer_eMuonEFInfo" ]
    for muon in xaod_muon:
        container_items+=["xAOD::MuonContainer#{muon}".format(muon = muon), 
                          "xAOD::MuonAuxContainer#{muon}Aux.".format(muon = muon)]
    
    
    trackParticleAuxExclusions="-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"

    if flags.Muon.DESDM_MCP.doAlignmentFormat:
       trackParticleAuxExclusions=""
    track_parts = ["MuonSpectrometerTrackParticles", 
                   "CombinedMuonTrackParticles", 
                   "ExtrapolatedMuonTrackParticles",
                   "InDetTrackParticles",  #Really want to skim/slim this guy //for ID Alignment
                   "MSOnlyExtrapolatedMuonTrackParticles",
                   ### Trigger
                   "HLT_xAOD__TrackParticleContainer_MuonEFInfo_CombTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_MuonEFInfo_ExtrapTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_MuTagIMO_EF_CombTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_MuTagIMO_EF_ExtrapTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_eMuonEFInfo_CombTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_eMuonEFInfo_ExtrapTrackParticles",
                   "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_MuonIso_EFID",
                   "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_FTF",
                   "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_Muon_EFID",
                   "HLT_xAOD__TrackParticleContainer_InDetTrigTrackingxAODCnv_MuonIso_FTF"]
    for trk in track_parts:
        container_items +=["xAOD::TrackParticleContainer#{trk}".format(trk = trk), 
                            "xAOD::TrackParticleAuxContainer#{trk}Aux.{excl}".format(trk = trk,
                                                                                     excl = trackParticleAuxExclusions )]
    segment_cont = ["MuonSegments", "UnAssocMuonSegments" ] + (["xAODNSWSegments"] if isRun3 else [])
    for seg in segment_cont:
        container_items +=["xAOD::MuonSegmentContainer#{seg}".format(seg = seg),
                           "xAOD::MuonSegmentAuxContainer#{seg}Aux.".format(seg =seg)]
    ### Prep data containers
    container_items += ["Muon::RpcPrepDataContainer#*", "Muon::TgcPrepDataContainer#*", "Muon::MdtPrepDataContainer#*"]
    ### Only add the SW containers in the corresponding geometry
    if not isRun3: container_items +=["Muon::CscPrepDataContainer#*", "Muon::CscStripPrepDataContainer#CSC_Measurements"]
    else: container_items +=["Muon::MMPrepDataContainer#*",
                            "Muon::sTgcPrepDataContainer#*",
                            "xAOD::NSWTPRDOContainer#*","xAOD::NSWTPRDOAuxContainer#*",
                            "xAOD::NSWMMTPRDOContainer#*","xAOD::NSWMMTPRDOAuxContainer#*",
                            "Muon::NSW_PadTriggerDataContainer#*", 
                            "Muon::NSW_TrigRawDataContainer#L1_NSWTrigContainer" ,
                            ]
    
    ### RPC trigger RDO containers
    container_items += ["MuCTPI_RDO#MUCTPI_RDO", "RpcPadContainer#RPCPAD", 
                        "Muon::RpcCoinDataContainer#RPC_triggerHits", 
                        "RpcSectorLogicContainer#RPC_SECTORLOGIC"]
    ### TGC trigger RDO containers
    container_items += ["Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionPriorBC",
                        "Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionNextBC",
                        "Muon::TgcCoinDataContainer#TrigT1CoinDataCollectionNextNextBC",
                        "Muon::TgcCoinDataContainer#TrigT1CoinDataCollection"]
    
    
    ### Segment containers
    trk_seg_cont = ["TrkMuonSegments", "UnAssocMuonTrkSegments"]+ (["TrackMuonNSWSegments"] if isRun3 else [])
    container_items += ["Trk::SegmentCollection#{seg}".format(seg = seg) for seg in trk_seg_cont]
    ### Track containers
    trk_cont = ["MuonSpectrometerTracks", "CombinedMuonTracks", "MSOnlyExtrapolatedTracks", "ExtrapolatedMuonTracks" ]
    container_items += ["TrackCollection#{trk}".format(trk = trk) for trk in trk_cont]
    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    kwargs.setdefault("ItemList", container_items)
    result.merge(OutputStreamCfg(flags, **kwargs))
    result.merge(
        SetupMetaDataForStreamCfg(
            flags,
            kwargs.get("streamName"),
            kwargs.get("AcceptAlgs"),
            createMetadata=[
                    MetadataCategory.ByteStreamMetaData,
                    MetadataCategory.CutFlowMetaData,
                    MetadataCategory.LumiBlockMetaData,
                    MetadataCategory.TriggerMenuMetaData,
            ],
        )
    )
    return result


def DESDM_MCPCfg(flags):
    if(flags.Muon.DESDM_MCP.doAlignmentFormat):
        from AthenaCommon.Logging import logging
        msg = logging.getLogger("Athena")
        msg.info("DESDM_MCP format will run with doAlignmentFormat True")
    result = ComponentAccumulator()
    StreamName = "DESDM_MCP"
    SeqName = "DESDMCPSequence"
    from AthenaCommon.CFElements import seqAND
    result.addSequence(seqAND("DESDMCPSequence"))
    result.merge(setupDESDMCPSkimmingAlgCfg(flags), sequenceName = SeqName)
    from DerivationFrameworkMuons.MuonsToolsConfig import AnalysisMuonThinningAlgCfg
    ####
    
    result.merge(AnalysisMuonThinningAlgCfg(flags, name = "DESDMCPMuonThinning",
                                            StreamName = "Stream{streamName}".format(streamName=StreamName),
                                            IdTrkFwdThinning = "",
                                            QualityWP = 4 ## Very loose muons
                                            ), sequenceName = SeqName)
    result.merge(DESDMCPOutputCfg(flags,
                    streamName = StreamName,
                    AcceptAlgs = ["DESDMCPMuonThinning", "DESDMCPEventKernel" ]),
                sequenceName = SeqName)    
    return result
