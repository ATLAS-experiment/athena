 # Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def HiggsAugmentationAlgsCfg(flags):
    acc = ComponentAccumulator()
    from DerivationFrameworkHiggs.FourLeptonVertexing import FourLeptonVertexerCfg
    acc.merge(FourLeptonVertexerCfg(flags))
    if flags.Input.isMC:
        from DerivationFrameworkHiggs.TruthCategoriesConfig import TruthCategoriesDecoratorCfg
        acc.merge(TruthCategoriesDecoratorCfg(flags))
    return acc

def setupHiggsSlimmingVariables(ConfigFlags, slimmingHelper):

    # extra variables needed for Higgs analyses
    if ConfigFlags.Input.isMC:      
        htxs_vars = [
            "HTXS_prodMode",
            "HTXS_errorCode",
            "HTXS_Stage0_Category",
            "HTXS_Stage1_Category_pTjet25",
            "HTXS_Stage1_Category_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet30",
            "HTXS_Stage1_FineIndex_pTjet25",
            "HTXS_Stage1_2_Category_pTjet25",
            "HTXS_Stage1_2_Category_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet30",
            "HTXS_Stage1_2_FineIndex_pTjet25",
            "HTXS_Stage1_2_Fine_Category_pTjet25",
            "HTXS_Stage1_2_Fine_Category_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet30",
            "HTXS_Stage1_2_Fine_FineIndex_pTjet25",
            "HTXS_Stage1_3_Category_pTjet25",
            "HTXS_Stage1_3_Category_pTjet30",
            "HTXS_Stage1_3_FineIndex_pTjet30",
            "HTXS_Stage1_3_FineIndex_pTjet25",
            "HTXS_Stage1_3_Fine_Category_pTjet25",
            "HTXS_Stage1_3_Fine_Category_pTjet30",
            "HTXS_Stage1_3_Fine_FineIndex_pTjet30",
            "HTXS_Stage1_3_Fine_FineIndex_pTjet25",
            "HTXS_Njets_pTjet25",
            "HTXS_Njets_pTjet30",
            "HTXS_isZ2vvDecay","HTXS_Higgs_pt"]
        for p4_var in ["HTXS_Higgs", "HTXS_V", "HTXS_V_jets25", "HTXS_V_jets30", "HTXS_Higgs_decay", "HTXS_V_decay" ]:
            htxs_vars += [p4_var+"_eta", p4_var+"_phi", p4_var+"_pt",p4_var+"_m"]
        slimmingHelper.ExtraVariables += ["EventInfo." + ".".join(htxs_vars)] 
    slimmingHelper.AppendToDictionary.update({
        'FourLeptonVertices':'xAOD::VertexContainer','FourLeptonVerticesAux':'xAOD::VertexAuxContainer'
    })
    slimmingHelper.ExtraVariables += ["FourLeptonVertices.trackParticleLinks.x.y.z.chiSquared.numberDoF.vertexType"]

    
 
