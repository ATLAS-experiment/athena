# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from LArCellRec.LArCollisionTimeConfig import LArCollisionTimeCfg

def BackgroundAlgsCfg(flags):

  result=ComponentAccumulator()

  result.merge(LArCollisionTimeCfg(flags))

  haveMM   = flags.Detector.GeometryMM
  havesTGC = flags.Detector.GeometrysTGC
  haveNSW  = haveMM or havesTGC      

  isRun3  = haveNSW                 

  filler = CompFactory.BeamBackgroundFiller()

  if flags.Beam.Type is BeamType.Collisions:
    if isRun3:
      # Run 3: NSW segments
      filler.nswSegmentContainerKey = "NCB_TrackMuonSegments"
      filler.cscSegmentContainerKey = ""   # disable CSC
      filler.isRun3 = True

    else:
      # Run 2: CSC segments
      filler.cscSegmentContainerKey = "NCB_TrackMuonSegments"
      filler.nswSegmentContainerKey = ""   # disable NSW
      filler.isRun3 = False
  
  else:
    filler.nswSegmentContainerKey = ""
    filler.cscSegmentContainerKey = ""
    filler.isRun3 = isRun3 

  result.addEventAlgo(filler)

  result.addEventAlgo(CompFactory.BcmCollisionTimeAlg())

  result.addEventAlgo(CompFactory.BackgroundWordFiller(IsMC=flags.Input.isMC))

  return result

#No self-test __main__ here because these algos depend on many event data objects not stored in ESD, therefore they can't run stand-alone. 
