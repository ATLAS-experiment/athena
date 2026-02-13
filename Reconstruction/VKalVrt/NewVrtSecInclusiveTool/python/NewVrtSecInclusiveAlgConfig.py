# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Author: Vadim Kostyukhin vadim.kostyukhin@cern.ch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO

def NewVrtSecInclusiveAlgCfg(flags, algname="NVSI_Alg", **kwargs):

   acc = ComponentAccumulator()
   if "BVertexTool" not in kwargs:
      from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import SoftBFinderToolCfg
      kwargs.setdefault("BVertexTool", acc.popToolsAndMerge(SoftBFinderToolCfg(flags,FillHist=True)))

   kwargs.setdefault("BVertexContainerName","AllBVertices")

   acc.addEventAlgo(CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **kwargs))
   return acc

def NewVrtSecInclusiveAlgLLPCfg(flags, algname="NVSI", AugmentingVersionString="", **kwargs):
  
   acc = ComponentAccumulator()
   if "BVertexTool" not in kwargs:
      from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import DVFinderToolCfg
      kwargs.setdefault("BVertexTool", acc.popToolsAndMerge(DVFinderToolCfg(flags,FillHist=False,AugmentingVersionString=AugmentingVersionString)))

   kwargs.setdefault("BVertexContainerName","SecondaryVertices_"+algname)

   acc.addEventAlgo(CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **kwargs))
   return acc


def NewVrtSecInclusiveAlgTightCfg(flags, algname="NVSI_Alg_Tight", **kwargs):
  
   acc = ComponentAccumulator()
   from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import SoftBFinderToolCfg
   kwargs.setdefault("BVertexTool", acc.popToolsAndMerge(SoftBFinderToolCfg(flags,FillHist=False, IniV2T_v2tBDTCut=-0.3,FinV2T_v2tBDTCut=0.8,IniV2T_cosSVPVCut=0.4,FinV2T_cosSVPVCut=0.4,AugmentingVersionString='_SoftBTight')))
   kwargs.setdefault("OutputLevel", INFO)
   kwargs.setdefault("BVertexContainerName","NVSI_SecVrt_Tight")

   NVSI_Alg = CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **kwargs)
   acc.addEventAlgo(NVSI_Alg)
   return acc


def NewVrtSecInclusiveAlgMediumCfg(flags, algname="NVSI_Alg_Medium", **kwargs):
  
   acc = ComponentAccumulator()
   from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import SoftBFinderToolCfg
   kwargs.setdefault("BVertexTool", acc.popToolsAndMerge(SoftBFinderToolCfg(flags,FillHist=False, IniV2T_v2tBDTCut=-0.6,FinV2T_v2tBDTCut=0.2,IniV2T_cosSVPVCut=0.5,FinV2T_cosSVPVCut=0.5,AugmentingVersionString='_SoftBMedium')))
   kwargs.setdefault("OutputLevel", INFO)
   kwargs.setdefault("BVertexContainerName","NVSI_SecVrt_Medium")

   NVSI_Alg = CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **kwargs)
   acc.addEventAlgo(NVSI_Alg)
   return acc


def NewVrtSecInclusiveAlgLooseCfg(flags, algname="NVSI_Alg_Loose", **kwargs):
  
   acc = ComponentAccumulator()
   from NewVrtSecInclusiveTool.NewVrtSecInclusiveConfig import SoftBFinderToolCfg
   kwargs.setdefault("BVertexTool", acc.popToolsAndMerge(SoftBFinderToolCfg(flags,FillHist=False, IniV2T_v2tBDTCut=-0.4,FinV2T_v2tBDTCut=-0.3,IniV2T_cosSVPVCut=0.4,FinV2T_cosSVPVCut=0.4,AugmentingVersionString='_SoftBLoose')))
   kwargs.setdefault("OutputLevel", INFO)
   kwargs.setdefault("BVertexContainerName","NVSI_SecVrt_Loose")

   NVSI_Alg = CompFactory.Rec.NewVrtSecInclusiveAlg(algname, **kwargs)
   acc.addEventAlgo(NVSI_Alg)
   return acc
