# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
'''
@author Elliot Lipeles - lipeles@cern.ch
@date Oct 10th, 2024
@brief This file specifies cuts for the FPGATrackSimGenScanTool
'''

# Cut Values for incremental L4 to L0 track building

NoCut = 10000000

# structure is dictionary of region and then cut
cuts = {    
    0: {
        "rin" : 30.0,
        "rout" : 300.0,
        "parSet" : "PhiSlicedKeyLyrPars",
        "parMin" : [-159.72599999999997, -132.192, 0.2162999990582466, 0.19339999257773163, -5.94],
        "parMax" : [171.72600000000006, 252.192, 0.5825999742746353, 0.6026000139191747, 5.9399999999999995],
        "parBins" : [20, 20, 5, 20, 3],
        "pairFilterDeltaPhiCut" : [NoCut,NoCut,NoCut,NoCut],
        "pairFilterDeltaEtaCut" : [NoCut,NoCut,NoCut,NoCut],
        "pairFilterPhiExtrapCut" : [7.5,4.0],
        "pairFilterEtaExtrapCut" : [11.2,10.0],
        "pairSetMatchPhiCut" : NoCut,
        "pairSetMatchEtaCut" : NoCut,
        "pairSetDeltaDeltaPhiCut" : NoCut, 
        "pairSetDeltaDeltaEtaCut" : NoCut, 
        "pairSetPhiCurvatureCut" : 0.00025,
        "pairSetEtaCurvatureCut" : 0.00010,
        "pairSetDeltaPhiCurvatureCut" : 0.00012,
        "pairSetDeltaEtaCurvatureCut" : NoCut, 
        "pairSetPhiExtrapCurvedCut" : [2.0,NoCut],  
        },
        3: {
        "rin" : 30.0,
        "rout" : 300.0,
        "parSet" : "PhiSlicedKeyLyrPars",
        "parMin" : [-54.09299999999998, 940.656, 0.2302720041632652, 0.2101899934336543, -5.559840098190307],
        "parMax" : [287.70300000000003, 1486.3439999999998, 0.5697279928565026, 0.595509973488748, 5.559840098190308], 
        "parBins" : [20, 20, 5, 20, 3],
        "pairFilterDeltaPhiCut" : [NoCut,NoCut,NoCut,NoCut],
        "pairFilterDeltaEtaCut" : [NoCut,NoCut,NoCut,NoCut],
        "pairFilterPhiExtrapCut" : [6.2,4.0],
        "pairFilterEtaExtrapCut" : [15.0,16.0],
        "pairSetMatchPhiCut" : NoCut,
        "pairSetMatchEtaCut" : NoCut,
        "pairSetDeltaDeltaPhiCut" : NoCut,
        "pairSetDeltaDeltaEtaCut" : NoCut,
        "pairSetPhiCurvatureCut" : 0.00023,
        "pairSetEtaCurvatureCut" : 0.00050,
        "pairSetDeltaPhiCurvatureCut" : 0.00055,
        "pairSetDeltaEtaCurvatureCut" : NoCut,
        "pairSetPhiExtrapCurvedCut" : [2.0,NoCut]
        },
}
