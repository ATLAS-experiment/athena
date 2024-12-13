# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArBadChannelTool.LArBadFebsConfig import LArKnownBadFebCfg, LArKnownMNBFebCfg
from AthenaConfiguration.Enums import ProductionStep

def LArNoisyROSummaryCfg(configFlags, **kwargs):

   result=ComponentAccumulator()

   isMC=configFlags.Input.isMC

   if not isMC:
      result.merge(LArKnownBadFebCfg(configFlags))
      result.merge(LArKnownMNBFebCfg(configFlags))
      result.addEventAlgo(CompFactory.LArHVlineMapAlg(keyOutput="LArHVNcells"))

   # now configure the algorithm
   LArNoisyROAlg,LArNoisyROTool=CompFactory.getComps("LArNoisyROAlg","LArNoisyROTool")
   if configFlags.Common.ProductionStep in [ProductionStep.PileUpPretracking, ProductionStep.MinbiasPreprocessing]:
        kwargs.setdefault('EventInfoKey', f"{configFlags.Overlay.BkgPrefix}EventInfo") 

   theLArNoisyROTool=LArNoisyROTool(CellQualityCut=configFlags.LAr.NoisyRO.CellQuality,
                                    BadChanPerFEB=configFlags.LAr.NoisyRO.BadChanPerFEB, 
                                    BadFEBCut=configFlags.LAr.NoisyRO.BadFEBCut,
                                    MNBLooseCut=configFlags.LAr.NoisyRO.MNBLooseCut,
                                    MNBTightCut=configFlags.LAr.NoisyRO.MNBTightCut,
                                    MNBTight_PsVetoCut=configFlags.LAr.NoisyRO.MNBTight_PsVetoCut,
                                    BadHVCut=configFlags.LAr.NoisyRO.BadHVCut,
                                    BadChanFracPerHVline=configFlags.LAr.NoisyRO.BadHVlineFrac,
                                    DoHVflag=not isMC
                                    )

   theLArNoisyROAlg=LArNoisyROAlg(isMC=isMC,Tool=theLArNoisyROTool, **kwargs)
   if not isMC:
      theLArNoisyROAlg.HVMapKey="LArHVNcells"
   result.addEventAlgo(theLArNoisyROAlg)
   
   toStore="LArNoisyROSummary#LArNoisyROSummary"
   from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
   result.merge(addToESD(configFlags,toStore))
   result.merge(addToAOD(configFlags,toStore))


   return result

