#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def AddRecoMumuToolCfg(flags, name = "MuonTPRecoMumuTool", **kwargs):
    result = ComponentAccumulator()
    from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
    kwargs.setdefault("V0Tools", result.popToolsAndMerge(V0ToolsCfg(flags, "RecoMumuToolV0Tools"))) 
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    kwargs.setdefault("PVRefitter", result.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)))
    the_tool = CompFactory.DerivationFramework.Reco_mumu(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def AddJPsiVertexingFitterCfg(flags, prefix='', IdTrkContainer = "InDetTrackParticles", MuonContainer = "Muons"):
  result = ComponentAccumulator()
  from JpsiUpsilonTools.JpsiUpsilonToolsConfig import JpsiFinderCfg
  jpsi_finder_tool = result.popToolsAndMerge(JpsiFinderCfg(flags,  
                                           muAndMu = False,
                                           muAndTrack = False,
                                           TrackAndTrack = True,
                                           assumeDiMuons = True,    # If true, will assume dimu hypothesis and use PDG value for mu mass
                                           invMassUpper = 3500.0,
                                           invMassLower = 2700.0,
                                           Chi2Cut = 2000.,
                                           allChargeCombinations = True,
                                           oppChargesOnly = False,
                                           sameChargesOnly= False,
                                           trackThresholdPt = 2500,
                                           muonThresholdPt= 4000,
                                           atLeastOneComb = False,
                                           useCombinedMeasurement = False, # Only takes effect if combOnly=True  
                                           muonCollectionKey = MuonContainer,                                        
                                           TrackParticleCollection = IdTrkContainer,
                                           useMCPCuts                  = True))

  
  
  MuonTP_Reco_mumu = result.getPrimaryAndMerge(AddRecoMumuToolCfg(flags,
                                                  name                   = prefix+"MuonTP_Reco_mumu",
                                                  JpsiFinder             = jpsi_finder_tool,
                                                  OutputVtxContainerName = prefix+"JpsiCandidates",
                                                  PVContainerName        = "PrimaryVertices",
                                                  RefPVContainerName     = prefix+"RefittedPrimaryVertices",
                                                  RefitPV                = True,
                                                  MaxPVrefit             = 100000,
                                                  DoVertexType           = 7))
  result.addPublicTool(MuonTP_Reco_mumu)
  the_alg = CompFactory.DerivationFramework.DerivationKernel(prefix +"JPsiVertexFitKernel",
                                                    AugmentationTools = [MuonTP_Reco_mumu])
  result.addEventAlgo(the_alg, primary = True)
  return result

def MuonTPOniaSelToolCfg(flags,name = "MuonTP_Select_Jpsi2mumu", **kwargs):
  ## a/ augment and select Jpsi->mumu candidates
  result = ComponentAccumulator()
  from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
  v0_tools = result.popToolsAndMerge(V0ToolsCfg(flags, name+"JPsiSelV0Tools"))
  kwargs.setdefault("V0Tools", v0_tools)
  kwargs.setdefault("HypothesisName",         "Jpsi")
  kwargs.setdefault("VtxMassHypo",            3096.916)
  kwargs.setdefault("MassMin",                2700.0)
  kwargs.setdefault("MassMax",                3500.0)
  kwargs.setdefault("Chi2Max",                200)
  kwargs.setdefault("DoVertexType",           7)
  MuonTP_Select_Jpsi2mumu = CompFactory.DerivationFramework.Select_onia2mumu(name = name, **kwargs)
  result.addPublicTool(v0_tools)
  result.setPrivateTools(MuonTP_Select_Jpsi2mumu)                                 
  return result

def AddJPsiVertexingSelectionCfg(flags, prefix = ''):
  ## a/ augment and select Jpsi->mumu candidates
  result = ComponentAccumulator()
  MuonTP_Select_Jpsi2mumu =result.popToolsAndMerge(MuonTPOniaSelToolCfg(flags, prefix + "MuonTP_Select_Jpsi2mumu",
                                                                  InputVtxContainerName = prefix+"JpsiCandidates" ))
  result.addPublicTool(MuonTP_Select_Jpsi2mumu)
  the_alg = CompFactory.DerivationFramework.DerivationKernel(prefix +"JPsiVertexFitSelector",
                                                    AugmentationTools = [MuonTP_Select_Jpsi2mumu])
  result.addEventAlgo(the_alg, primary = True)
  return result

def AddMCPJPsiVertexFitCfg(flags, prefix = '', IdTrkContainer = "InDetTrackParticles", MuonContainer = "Muons"):
  result = ComponentAccumulator()
  result.merge(AddJPsiVertexingFitterCfg(flags, prefix = prefix, IdTrkContainer = IdTrkContainer, MuonContainer = MuonContainer))
  result.merge(AddJPsiVertexingSelectionCfg(flags, prefix = prefix))  
  return result



