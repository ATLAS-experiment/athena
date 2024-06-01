# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

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

   # now configure the algorithm
   LArNoisyROAlg,LArNoisyROTool=CompFactory.getComps("LArNoisyROAlg","LArNoisyROTool")
   if configFlags.Common.ProductionStep is ProductionStep.PileUpPretracking:
        kwargs.setdefault('EventInfoKey', "Bkg_EventInfo") 

   theLArNoisyROTool=LArNoisyROTool(CellQualityCut=configFlags.LAr.NoisyRO.CellQuality,
                                    BadChanPerFEB=configFlags.LAr.NoisyRO.BadChanPerFEB, 
                                    BadFEBCut=configFlags.LAr.NoisyRO.BadFEBCut,
                                    MNBLooseCut=configFlags.LAr.NoisyRO.MNBLooseCut,
                                    MNBTightCut=configFlags.LAr.NoisyRO.MNBTightCut,
                                    MNBTight_PsVetoCut=configFlags.LAr.NoisyRO.MNBTight_PsVetoCut
                                    )

   theLArNoisyROAlg=LArNoisyROAlg(isMC=isMC,Tool=theLArNoisyROTool, **kwargs)
   result.addEventAlgo(theLArNoisyROAlg)
   
   toStore="LArNoisyROSummary#LArNoisyROSummary"
   from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
   result.merge(addToESD(configFlags,toStore))
   result.merge(addToAOD(configFlags,toStore))


   return result

