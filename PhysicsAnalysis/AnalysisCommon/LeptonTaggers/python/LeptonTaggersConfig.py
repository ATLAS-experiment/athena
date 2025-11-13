# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
lep_tag_log = logging.getLogger('LeptonTaggersConfig')

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaConfiguration.Enums import LHCPeriod


# TO DELETE:
#
# How LeptonTagger was called in MUON5:
#
# import LeptonTaggers.LeptonTaggersConfig as LepTagConfig
# if not hasattr(MUON5Seq,"Muons_decoratePromptLepton"):
#     LepTagConfig.ConfigureAntiKt4PV0TrackJets(MUON5Seq,"MUON1")
#     MUON5Seq += LepTagConfig.GetDecoratePromptLeptonAlgs()
#     MUON5Seq += LepTagConfig.GetDecorateImprovedPromptLeptonAlgs()
#
# MUON5SlimmingHelper.ExtraVariables += LepTagConfig.GetExtraPromptVariablesForDxAOD(onlyBDT=False)
# MUON5SlimmingHelper.ExtraVariables += LepTagConfig.GetExtraImprovedPromptVariablesForDxAOD()
# MUON5SlimmingHelper.ExtraVariables += ElectronsCPDetailedContent


def VtxFittingToolCfg(ConfigFlags, **kwargs) -> ComponentAccumulator:
    """
    Generate a vertex fitting tool.
    """
    acc = ComponentAccumulator()
    lep_tag_log.info("creating VtxFittingTool")

    kwargs.setdefault("doSeedVertexFit", False)

    kwargs.setdefault("DistToPriVtxName", "distToPriVtx")
    kwargs.setdefault("NormDistToPriVtxName", "normDistToPriVtx")
    kwargs.setdefault("DistToRefittedPriVtxName", "distToRefittedPriVtx")
    kwargs.setdefault("NormDistToRefittedPriVtxName", "normDistToRefittedPriVtx")
    kwargs.setdefault("DistToRefittedRmLepPriVtxName", "distToRefittedRmLepPriVtx")
    kwargs.setdefault("NormDistToRefittedRmLepPriVtxName",
                      "normDistToRefittedRmLepPriVtx")

    from TrkConfig.TrkVertexBilloirToolsConfig import FastVertexFitterCfg
    fast_vtx_fitter = acc.popToolsAndMerge(FastVertexFitterCfg(ConfigFlags))

    kwargs.setdefault("vertexFitterTool", fast_vtx_fitter)
    kwargs.setdefault("seedVertexFitterTool", fast_vtx_fitter)

    vertex_fitting_tool = CompFactory.Prompt.VertexFittingTool(
        name="VertexFittingTool", **kwargs
    )

    acc.setPrivateTools(vertex_fitting_tool)
    lep_tag_log.info("end of VtxFittingTool creation")
    return acc


def DecorateReFitPrimaryVertexCfg(
    ConfigFlags, name="PrimaryVertexReFitter",
    lepton_type="", **kwargs
) -> ComponentAccumulator:
    """
    CA to run the PrimaryVertexReFitter algorithm.
    """

    if lepton_type not in ['Electrons', 'Muons']:
        raise ValueError(f'DecorateReFitPrimaryVertex - unknown lepton type: "{lepton_type}"')

    acc = ComponentAccumulator()

    kwargs.setdefault("ReFitPriVtxName", f"RefittedPriVtx_{lepton_type}")
    kwargs.setdefault("LeptonContainerName", lepton_type)

    kwargs.setdefault("DistToRefittedPriVtxName", "distToRefittedPriVtx")
    kwargs.setdefault("NormDistToRefittedPriVtxName", "normDistToRefittedPriVtx")
    kwargs.setdefault("RefittedVtxWithoutLeptonLinkName",
                      f"RefittedPriVtxWithoutLepton_{lepton_type}")

    alg_name = f'PrimaryVertexReFitter_{lepton_type}_decoratePriVtx'

    kwargs.setdefault("VertexFittingTool", acc.popToolsAndMerge(
        VtxFittingToolCfg(ConfigFlags)
    ))

    the_alg = CompFactory.Prompt.PrimaryVertexReFitter(alg_name, **kwargs)

    acc.addEventAlgo(the_alg, primary = True)
    lep_tag_log.info("end of DecorateReFitPrimaryVertexCfg")
    return acc


def VtxItrMergingToolCfg(flags, name="VertexIterativeMergingTool",
                         lepton_name="", **kwargs) -> ComponentAccumulator:
    """
    Generate a VtxItrMergingTool configuration.
    """
    lep_tag_log.info("creating VtxItrMergingToolCfg configuration")

    acc = ComponentAccumulator()

    kwargs.setdefault("minFitProb", 0.03)

    tool_name = f'VtxIterFitMergingTool_{lepton_name}'

    vtxItrMergingTool = CompFactory.Prompt.VertexIterativeFitMergingTool(
        name=tool_name, **kwargs
    )

    acc.setPrivateTools(vtxItrMergingTool)

    return acc


def DecorateNonPromptVertexCfg(flags, name="DecorateNonPromptVertex",
                               lepton_name="", **kwargs) -> ComponentAccumulator:
    """
    Configure the non-prompt vertex decorator.
    """
    if lepton_name not in ['Electrons', 'Muons']:
        raise ValueError(f'DecorateNonPromptVertex - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    vertex_fitting_tool = acc.popToolsAndMerge(VtxFittingToolCfg(flags))

    kwargs.setdefault("VertexFittingTool",vertex_fitting_tool)

    kwargs.setdefault(
        "VertexMergingTool",
        acc.popToolsAndMerge(VtxItrMergingToolCfg(
            flags, lepton_name=lepton_name,
            VertexFittingTool=vertex_fitting_tool
        ))
    )

    kwargs.setdefault("LeptonContainerName", lepton_name)
    kwargs.setdefault("ReFitPriVtxContainerName", f"RefittedPriVtx_{lepton_name}")
    kwargs.setdefault("SVContainerName", f"SecVtxContainer_{lepton_name}")

    kwargs.setdefault("SecVtxLinksName", "SecVtxLinks")
    kwargs.setdefault("DeepMergedSecVtxLinksName", "DeepMergedSecVtxLinks")
    kwargs.setdefault("NoLeptonPriVtxLinkName", f"RefittedPriVtxWithoutLepton_{lepton_name}")
    kwargs.setdefault("IndexVectorName", "PromptLeptonInput_SecondaryVertexIndexVector")

    alg = CompFactory.Prompt.NonPromptLeptonVertexingAlg(
        name=f'NonPromptVtx_decorate_{lepton_name}',
        **kwargs
    )

    acc.addEventAlgo(alg, primary=True)

    return acc


def RNNToolCfg(flags, name="RNNTool",
               RNN_name="RNN_name", lepton_name="",
               **kwargs) -> ComponentAccumulator:
    """
    Configure the RNN tool.
    """
    lep_tag_log.info("calling RNNToolCfg with name"+RNN_name+" with lepton_name="+lepton_name)

    if lepton_name not in ["Electrons", "Muons"]:
        raise ValueError(f'RNNTool - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    #
    # Read configuration from AFS for this initial merge request, will switch to cvmfs with second request
    #
    if lepton_name == 'Electrons':
        kwargs.setdefault("configRNNVersion", 'InputData-2020-02-25/RNN/Electron')
        kwargs.setdefault("configRNNJsonFile",
                          'elecs_feb20_fullrun2_linear_ptraw_ntk5_model_ndense10_nhidden50_nepoch10_nbatch256_use_weights_nn-config.json')

    elif lepton_name == 'Muons':
        kwargs.setdefault("configRNNVersion", 'InputData-2020-02-25/RNN/Muon')
        kwargs.setdefault("configRNNJsonFile",
                          'muons_feb19_fullrun2_linear_ptraw_ntk5_model_ndense10_nhidden50_nepoch10_nbatch256_use_weights_nn-config.json')

    tool_rnn = CompFactory.Prompt.RNNTool(
        name=f'{RNN_name}_{lepton_name}_RNNTool', **kwargs
    )
    acc.setPrivateTools(tool_rnn)

    return acc


def DecoratePromptLeptonRNNCfg(flags, RNN_name="RNN_name", lepton_name="",
                               name="DecoratePromptLeptonRNN",
                               **kwargs) -> ComponentAccumulator:
    """
    Configure the prompt lepton RNN decorator.
    """
    lep_tag_log.info("calling DecoratePromptLeptonRNNCfg with name"+RNN_name+" with lepton_name="+lepton_name)
    if lepton_name not in ["Electrons", "Muons"]:
        raise ValueError(f'DecorateNonPromptVertex - unknown lepton type: "{lepton_name}"')

    acc = ComponentAccumulator()

    #
    # Prepare DecoratePromptLepton alg
    #
    kwargs.setdefault("inputContainerLepton", lepton_name)
    kwargs.setdefault("inputContainerTrack", 'InDetTrackParticles')
    kwargs.setdefault("inputContainerTrackJet", 'AntiKtVR30Rmax4Rmin02PV0TrackJets')
    kwargs.setdefault("inputContainerPrimaryVertices", 'PrimaryVertices')

    kwargs.setdefault("decorationPrefixRNN", 'PromptLeptonRNN_')

    kwargs.setdefault("debug", False)
    kwargs.setdefault("outputStream", 'out')

    kwargs.setdefault("toolRNN", acc.popToolsAndMerge(
        RNNToolCfg(flags, RNN_name=RNN_name, lepton_name=lepton_name)
    ))

    alg = CompFactory.Prompt.DecoratePromptLeptonRNN(
        name=f'{lepton_name}_decorate_RNN_{RNN_name}', **kwargs
    )
    acc.addEventAlgo(alg, primary=True)

    lep_tag_log.info('Decorate%s - prepared %s algorithm for: %s',
                     RNN_name, RNN_name, lepton_name)

    return acc


def getStringIntVars(BDT_name):
    """
    Get the integer variables for the BDT (I think)
    """
    int_vars = []

    if BDT_name == 'PromptLeptonImprovedVeto':
        int_vars += ['MVAXBin']
    elif (BDT_name == 'PromptLeptonImprovedVetoBARR'
          or BDT_name == 'PromptLeptonImprovedVetoECAP'):
        int_vars += ['MVAXBin', 'TrackJetNTrack']
    else:
        raise ValueError(f'getStringIntVars - unknown alg: "{BDT_name}"')

    return int_vars


def getStringFloatVars(BDT_name, part_type=''):
    """
    Get the float variables for the BDT (I think)
    """
    float_vars = []

    if BDT_name == 'PromptLeptonImprovedVeto':
        float_vars += ['topoetcone30rel',
                       'ptvarcone30_TightTTVA_pt500rel',
                       'PromptLeptonRNN_prompt',
                       'PtFrac',
                       'DRlj',
                       'CaloClusterERel',
                       'CandVertex_normDistToPriVtxLongitudinalBest']
    elif (BDT_name == 'PromptLeptonImprovedVetoBARR'
          or BDT_name == 'PromptLeptonImprovedVetoECAP'):
        float_vars += ['topoetcone30rel',
                       'ptvarcone30rel',
                       'PromptLeptonRNN_prompt',
                       'PtFrac',
                       'DRlj',
                       'CaloClusterSumEtRel',
                       'PtRel',
                       'CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx']
    else:
        raise ValueError(f'getStringFloatVars - unknown alg: "{BDT_name}"')

    return float_vars


def DecoratePLITCfg(
    flags, Tagger_name="", lepton_name="",
    **kwargs
) -> ComponentAccumulator:
    """
    Configure the PLIT decorator.
    """
    lep_tag_log.info("calling DecoratePLITCfg with name="+Tagger_name+" lepton_name="+lepton_name)

    acc = ComponentAccumulator()

    #
    # Prepare DecoratePromptLepton alg
    #
    kwargs.setdefault("LeptonContainerName", lepton_name)
    kwargs.setdefault("TracksContainerKey", "InDetTrackParticles")
    kwargs.setdefault("CaloClusterContainerKey", "egammaClusters")
    kwargs.setdefault("TaggerName", Tagger_name)

    # check if Run3 (or beyond) configs should be used 
    isRun3 = (flags.GeoModel.Run >= LHCPeriod.Run3)

    # Make sure the decorations have a unique name to avoid multiple tools writing the same name
    kwargs.setdefault("acc_trk_dr_lepton","dr_lepton"+lepton_name)
    kwargs.setdefault("acc_trk_electron_track","electron_track"+lepton_name)
    kwargs.setdefault("acc_trk_muon_track","muon_track"+lepton_name)
    kwargs.setdefault("acc_trk_dr_leptontrack","dr_leptontrack"+lepton_name)
    kwargs.setdefault("dec_trk_dr_lepton","dr_lepton"+lepton_name)
    kwargs.setdefault("dec_trk_electron_track","electron_track"+lepton_name)
    kwargs.setdefault("dec_trk_muon_track","muon_track"+lepton_name)
    kwargs.setdefault("dec_trk_dr_leptontrack","dr_leptontrack"+lepton_name)

    # path on calib area (found by path resolver
    # /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/
    if lepton_name == 'Electrons':
        if isRun3:
            kwargs.setdefault("ConfigPath", "IsolationSelection/PLIT/2025-04-25/")
            kwargs.setdefault("ConfigFileVersion", 'network_run3_electrons_barrel.onnx')
            kwargs.setdefault("ConfigFileVersion_endcap", 'network_run3_electrons_endcap.onnx')
        else:
            kwargs.setdefault("ConfigPath", "IsolationSelection/PLIT/2025-04-25/")
            kwargs.setdefault("ConfigFileVersion", 'network_run2_electrons_barrel.onnx')
            kwargs.setdefault("ConfigFileVersion_endcap", 'network_run2_electrons_endcap.onnx')
    elif lepton_name == 'Muons':
        if isRun3:
            kwargs.setdefault("ConfigPath", "IsolationSelection/PLIT/2025-04-25/")
            kwargs.setdefault("ConfigFileVersion", 'network_run3_muons.onnx')
        else:
            kwargs.setdefault("ConfigPath", "IsolationSelection/PLIT/2025-04-25/")
            kwargs.setdefault("ConfigFileVersion", 'network_run2_muons.onnx')
    else:
        raise ValueError(f'Decorate{Tagger_name} - unknown lepton type: "{lepton_name}"')

    alg = CompFactory.Prompt.DecoratePLIT(
        f'{lepton_name}_decorate{Tagger_name}', **kwargs
    )
    acc.addEventAlgo(alg, primary=True)

    lep_tag_log.info(
        'Decorate%s - prepared %s algorithm for: %s',
        Tagger_name, Tagger_name, lepton_name
    )

    return acc

def DecoratePromptLeptonImprovedCfg(
    flags, BDT_name="", lepton_name="", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets",
    veto_BDT_name=None,
    **kwargs
) -> ComponentAccumulator:
    """
    Configure the PromptLeptonImproved decorator.
    """
    lep_tag_log.info("calling DecoratePromptLeptonImprovedCfg with BDT_name="+BDT_name+" lepton_name="+lepton_name+" track_jet_name="+track_jet_name)
    #
    # Check track jet container is correct
    #
    if track_jet_name != 'AntiKtVR30Rmax4Rmin02PV0TrackJets':
        raise ValueError(f'Decorate{BDT_name} - unknown track jet collection: "{track_jet_name}"')

    acc = ComponentAccumulator()

    #
    # Prepare DecoratePromptLepton alg
    #
    kwargs.setdefault("LeptonContainerName", lepton_name)

    kwargs.setdefault("TrackJetContainerName", track_jet_name)
    kwargs.setdefault("PrimaryVertexContainerName", 'PrimaryVertices')
    kwargs.setdefault("ClusterContainerName", 'CaloCalTopoClusters')

    kwargs.setdefault("BDTName", BDT_name)
    kwargs.setdefault("InputVarDecoratePrefix", 'PromptLeptonImprovedInput_')
    kwargs.setdefault("PrintTime", False)

    #
    # Read configuration from AFS for this initial merge request, will switch to cvmfs with second request
    #
    if lepton_name == 'Electrons':
        kwargs.setdefault("MethodTitleMVA", f'BDT_Electron_{BDT_name}')
        kwargs.setdefault("ConfigFileVersion", 'InputData-2020-02-25/BDT/Electron/' + BDT_name)
        kwargs.setdefault("accessorRNNVars", ['PromptLeptonRNN_prompt'])
    elif lepton_name == 'Muons':
        kwargs.setdefault("MethodTitleMVA", f'BDT_Muon_{BDT_name}')
        kwargs.setdefault("ConfigFileVersion", 'InputData-2020-02-25/BDT/Muon/' + BDT_name)
        kwargs.setdefault("accessorRNNVars", ['PromptLeptonRNN_prompt'])
    else:
        raise ValueError(f'Decorate{BDT_name} - unknown lepton type: "{lepton_name}"')

    kwargs.setdefault("stringIntVars", getStringIntVars(BDT_name))
    kwargs.setdefault("stringFloatVars", getStringFloatVars(BDT_name))
    kwargs.setdefault("extraDecoratorFloatVars", ['RawPt'])
    kwargs.setdefault("extraDecoratorShortVars", ['CandVertex_NPassVtx'])
    kwargs.setdefault("vetoDecoratorFloatVars", ['PromptLeptonRNN_prompt'])
    kwargs.setdefault("vetoDecoratorShortVars", [])

    if veto_BDT_name:
        kwargs['vetoDecoratorFloatVars'] += ['RawPt']
        kwargs['vetoDecoratorFloatVars'] += getStringFloatVars(veto_BDT_name)
        kwargs['vetoDecoratorShortVars'] += ['CandVertex_NPassVtx']
        kwargs['vetoDecoratorShortVars'] += getStringIntVars(veto_BDT_name)

    kwargs.setdefault("leptonPtBinsVector", [10.0e3, 15.0e3, 20.0e3, 25.0e3, 32.0e3, 43.0e3, 100.0e3])


    #
    # Secondary vertex selection for the PromptLeptonImproved
    #
    kwargs.setdefault("VertexLinkName", 'DeepMergedSecVtxLinks')

    alg = CompFactory.Prompt.DecoratePromptLeptonImproved(
        f'{lepton_name}_decorate{BDT_name}', **kwargs
    )
    acc.addEventAlgo(alg, primary=True)

    lep_tag_log.info(
        'Decorate%s - prepared %s algorithm for: %s, %s',
        BDT_name, BDT_name, lepton_name, track_jet_name
    )

    return acc


def DecorateImprovedPromptLeptonAlgsCfg(
    ConfigFlags, name="DecorateImprovedPromptLeptonAlgs",
    lepton_type="", **kwargs
) -> ComponentAccumulator:
    """
    CA to decorate with PLIV input algorithms
    """
    valid_lepton_types = ["", "Electrons", "Muons"]
    if lepton_type not in valid_lepton_types:
        lep_tag_log.error("Requested lepton type: %s", lepton_type)
        lep_tag_log.error("Allowed lepton types: %s", valid_lepton_types)
        raise ValueError('DecorateImprovedPromptLeptonAlgsCfg - '
                         + f'unknown lepton type: "{lepton_type}"')

    acc = ComponentAccumulator()

    if lepton_type in ["", "Electrons"]:
        acc.merge(DecorateReFitPrimaryVertexCfg(ConfigFlags, lepton_type="Electrons"))
        acc.merge(DecorateNonPromptVertexCfg(ConfigFlags, lepton_name="Electrons"))
        acc.merge(DecoratePromptLeptonRNNCfg(ConfigFlags,
                                             RNN_name='PromptLeptonRNN',
                                             lepton_name='Electrons'))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVetoBARR",
            lepton_name="Electrons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVetoECAP",
            veto_BDT_name='PromptLeptonImprovedVetoBARR',
            lepton_name="Electrons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))

    if lepton_type in ["", "Muons"]:
        acc.merge(DecorateReFitPrimaryVertexCfg(ConfigFlags, lepton_type="Muons"))
        acc.merge(DecorateNonPromptVertexCfg(ConfigFlags, lepton_name="Muons"))
        acc.merge(DecoratePromptLeptonRNNCfg(ConfigFlags,
                                             RNN_name='PromptLeptonRNN',
                                             lepton_name='Muons'))
        acc.merge(DecoratePromptLeptonImprovedCfg(
            ConfigFlags, BDT_name="PromptLeptonImprovedVeto",
            lepton_name="Muons", track_jet_name="AntiKtVR30Rmax4Rmin02PV0TrackJets"
        ))

    return acc

def DecoratePLITAlgsCfg(
    ConfigFlags, name="DecoratePLITAlgsCfg",
    lepton_type="", **kwargs
) -> ComponentAccumulator:
    """
    CA to decorate with PLIT input algorithms
    """
    valid_lepton_types = ["", "Electrons", "Muons"]
    if lepton_type not in valid_lepton_types:
        lep_tag_log.error("Requested lepton type: %s", lepton_type)
        lep_tag_log.error("Allowed lepton types: %s", valid_lepton_types)
        raise ValueError('DecoratePLITAlgsCfg - '
                         + f'unknown lepton type: "{lepton_type}"')

    acc = ComponentAccumulator()

    if lepton_type in ["", "Electrons"]:
        acc.merge(DecoratePLITCfg(ConfigFlags, Tagger_name="PLIT", lepton_name="Electrons"))
      
    if lepton_type in ["", "Muons"]:
        acc.merge(DecoratePLITCfg(ConfigFlags, Tagger_name="PLIT", lepton_name="Muons"))

    return acc

#------------------------------------------------------------------------------
def GetExtraPromptVariablesForDxAOD(name='', addSpectators=False, onlyBDT=True):

    prompt_lep_vars = []

    #
    # Decorate lepton only with the BDT outputs when the onlyBDT flag is true.
    #
    # NOTE: The output score name for BDTname=LowPtPromptLeptonVeto is "LowPtPLV" instead "LowPtPromptLeptonVeto".
    #       This is to harmonize with the variable augmented in CP::IsolationLowPtPLVTool
    #
    if onlyBDT:
        if name == "" or name == "Electrons":
            prompt_lep_vars += ["Electrons.PromptLeptonVeto.PromptLeptonIso.LowPtPLV."]

        if name == "" or name == "Muons":
            prompt_lep_vars += ["Muons.PromptLeptonVeto.PromptLeptonIso.LowPtPLV."]

        return prompt_lep_vars


    prompt_vars  = "PromptLeptonVeto.PromptLeptonIso.LowPtPLV."
    prompt_vars += "PromptLeptonInput_TrackJetNTrack.PromptLeptonInput_sv1_jf_ntrkv."
    prompt_vars += "PromptLeptonInput_ip2.PromptLeptonInput_ip3."
    prompt_vars += "PromptLeptonInput_LepJetPtFrac.PromptLeptonInput_DRlj."
    prompt_vars += "PromptLeptonInput_PtFrac.PromptLeptonInput_PtRel."
    prompt_vars += "PromptLeptonInput_DL1mu.PromptLeptonInput_rnnip."
    prompt_vars += "PromptLeptonInput_topoetcone20rel.PromptLeptonInput_ptvarcone20rel."
    prompt_vars += "PromptLeptonInput_topoetcone30rel.PromptLeptonInput_ptvarcone30rel."

    prompt_vars += "PromptLeptonInput_SecondaryVertexIndexVector.PromptLeptonInput_SecondaryVertexIndexVectorInDet.PromptLeptonInput_SecondaryVertexIndexVectorMerge.PromptLeptonInput_SecondaryVertexIndexVectorDeepMerge."
    prompt_vars += "rhocen.rhofor.SecVtxLinks.RefittedPriVtxLink.RefittedPriVtxWithoutLeptonLink."


    secondaryvertex_vars = "SVType.trackParticleLinks.trackWeights.neutralParticleLinks.neutralWeights.SecondaryVertexIndex.SecondaryVertexIndexVectorInput.chiSquared.numberDoF.x.y.z.covariance.vertexType.energyFraction.mass.normDist.ntrk.distToPriVtx.normDistToPriVtx.distToRefittedPriVtx.normDistToRefittedPriVtx.distToRefittedRmLepPriVtx.normDistToRefittedRmLepPriVtx"

    if addSpectators :
        prompt_vars += "PromptLeptonInput_JetPt.PromptLeptonInput_JetEta.PromptLeptonInput_JetPhi.PromptLeptonInput_JetM."

    if name == "" or name == "Electrons":
        prompt_el_vars = "ptvarcone40.topoetcone20.topoetcone20ptCorrection.ptcone20_TightTTVA_pt500.ptcone20_TightTTVA_pt1000.ptvarcone20_TightTTVA_pt1000.ptvarcone30_TightTTVA_pt500.ptvarcone30_TightTTVA_pt1000.ptvarcone40_TightTTVALooseCone_pt500"

        prompt_lep_vars += ["Electrons." + prompt_vars + prompt_el_vars]
        prompt_lep_vars += ["SecVtxContainer_Electrons.%s" %secondaryvertex_vars]
        prompt_lep_vars += ["SecVtx_ConvVtxContainer_Electrons.%s" %secondaryvertex_vars]

    if name == "" or name == "Muons":
        prompt_mu_vars = "ET_Core.ET_EMCore.ET_HECCore.ET_TileCore.EnergyLoss.EnergyLossSigma.MeasEnergyLoss.MeasEnergyLossSigma.ParamEnergyLoss.ParamEnergyLossSigmaMinus.ParamEnergyLossSigmaPlus.neflowisol20.neflowisol30.neflowisol40.ptvarcone20_TightTTVA_pt500.ptvarcone30_TightTTVA_pt500.ptvarcone40_TightTTVA_pt500.ptvarcone20_TightTTVA_pt1000.ptvarcone30_TightTTVA_pt1000.ptvarcone40_TightTTVA_pt1000.caloExt_Decorated.caloExt_eta.caloExt_phi"

        prompt_lep_vars += ["Muons." + prompt_vars + prompt_mu_vars]
        prompt_lep_vars += ["SecVtxContainer_Muons.%s" %secondaryvertex_vars]

    return prompt_lep_vars

#------------------------------------------------------------------------------
def GetExtraImprovedPromptVariablesForDxAOD(name='', onlyBDT=False):

    prompt_lep_vars = []

    #
    # Decorate lepton only with the BDT outputs when the onlyBDT flag is true.
    #
    if onlyBDT:
        # Add lepton raw pT and pTBin as default which is needed for the PLIV working points.
        rawpt_vars ="PromptLeptonImprovedInput_MVAXBin.PromptLeptonImprovedInput_RawPt"

        if name == "" or name == "Electrons":
            prompt_lep_vars += ["Electrons.PromptLeptonImprovedVetoBARR.PromptLeptonImprovedVetoECAP.%s"%rawpt_vars]

        if name == "" or name == "Muons":
            prompt_lep_vars += ["Muons.PromptLeptonImprovedVeto.%s"%rawpt_vars]

        return prompt_lep_vars

    prompt_vars  = "PromptLeptonImprovedInput_MVAXBin.PromptLeptonImprovedInput_RawPt."
    prompt_vars += "PromptLeptonImprovedInput_PtFrac.PromptLeptonImprovedInput_DRlj."
    prompt_vars += "PromptLeptonImprovedInput_topoetcone30rel.PromptLeptonImprovedInput_ptvarcone30rel."

    if name == "" or name == "Electrons":
        # Add PromptLeptonTagger electron RNN and new inputs for PromptLeptonImprovedVetoBARR/PromptLeptonImprovedVetoECAP
        prompt_el_vars = "PromptLeptonRNN_prompt.PromptLeptonRNN_non_prompt_b.PromptLeptonRNN_non_prompt_c.PromptLeptonRNN_conversion."
        prompt_el_vars += "PromptLeptonImprovedVetoBARR.PromptLeptonImprovedVetoECAP.PromptLeptonImprovedInput_TrackJetNTrack.PromptLeptonImprovedInput_PtRel.PromptLeptonImprovedInput_CaloClusterSumEtRel.PromptLeptonImprovedInput_CandVertex_normDistToPriVtxLongitudinalBest_ThetaCutVtx"

        prompt_lep_vars += ["Electrons." + prompt_vars + prompt_el_vars]

    if name == "" or name == "Muons":
        # Add PromptLeptonTagger muon RNN and new inputs for PromptLeptonImprovedVeto
        prompt_mu_vars = "PromptLeptonRNN_prompt.PromptLeptonRNN_non_prompt_b.PromptLeptonRNN_non_prompt_c."
        prompt_mu_vars += "PromptLeptonImprovedVeto.PromptLeptonImprovedInput_ptvarcone30_TightTTVA_pt500rel.PromptLeptonImprovedInput_CaloClusterERel.PromptLeptonImprovedInput_CandVertex_normDistToPriVtxLongitudinalBest"

        prompt_lep_vars += ["Muons." + prompt_vars + prompt_mu_vars]

    return prompt_lep_vars

#------------------------------------------------------------------------------
def GetExtraPLITVariablesForDxAOD(name=''):

    prompt_lep_vars = []

    #
    # Decorate lepton with the score outputs
    #
    if name == "" or name == "Electrons":
        prompt_lep_vars += ["Electrons.PLIT_PLITel_pelxpromp", "Electrons.PLIT_PLITel_pnpxall"]

    if name == "" or name == "Muons":
        prompt_lep_vars += ["Muons.PLIT_TPLTmu_pmuxpromp", "Muons.PLIT_TPLTmu_pnpxall"]

    return prompt_lep_vars


# Script to run for testing the config
# from https://atlassoftwaredocs.web.cern.ch/athena/configuration/ca/
if __name__ == "__main__":
    # argument parsing - not using flags.fillFromArgs() since this is just a test app
    from argparse import ArgumentParser
    parser = ArgumentParser(description="Test the LeptonTaggersConfig")
    parser.add_argument("--toTest", type=str, choices=["DecorateImprovedPromptLeptonAlgs", "DecoratePLITAlgs"],
                        default="DecoratePLITAlgs",
                        help="The function to test")
    parser.add_argument("--filesInput", type=str, default=None,
                        help="The input files to use")
    parser.add_argument("--evtMax", type=int, default=10,
                        help="The number of events to process")
    parser.add_argument("--skipEvents", type=int, default=0,
                        help="The number of events to skip")
    parser.add_argument("--run", type=int, choices=[2, 3], default=3,
                        help="The LHC Run period")
    args = parser.parse_args()

    # import the flags and set them using the command line arguments
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    FLAGS = initConfigFlags()

    from AthenaConfiguration.Enums import ProductionStep
    FLAGS.Common.ProductionStep = ProductionStep.Derivation

    FLAGS.Exec.MaxEvents = args.evtMax
    FLAGS.Exec.SkipEvents = args.skipEvents

    # use one of the predefined files
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    FLAGS.Input.Files = args.filesInput.split(',') if args.filesInput else defaultTestFiles.AOD_RUN3_MC
    
    # set the run period
    FLAGS.GeoModel.Run = LHCPeriod.Run3 if args.run == 3 else LHCPeriod.Run2

    # # lock the flags
    FLAGS.lock()

    # create basic infrastructure
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    ACC = MainServicesCfg(FLAGS)

    # Pool file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    ACC.merge(PoolReadCfg(FLAGS))

    # Common augmentations here 
    from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
    trigger_lists_helper = TriggerListsHelper(FLAGS)
    from DerivationFrameworkPhys.PhysCommonConfig import PhysCommonAugmentationsCfg
    ACC.merge(PhysCommonAugmentationsCfg(FLAGS, TriggerListsHelper=trigger_lists_helper))

    if args.toTest == "DecorateImprovedPromptLeptonAlgs":
        ACC.merge(DecorateImprovedPromptLeptonAlgsCfg(FLAGS))
    elif args.toTest == "DecoratePLITAlgs":
        ACC.merge(DecoratePLITAlgsCfg(FLAGS))

    

    # debug printout
    ACC.printConfig(withDetails=True, summariseProps=True)

    # run the job
    status = ACC.run()

    # report the execution status (0 ok, else error)
    import sys
    sys.exit(not status.isSuccess())
