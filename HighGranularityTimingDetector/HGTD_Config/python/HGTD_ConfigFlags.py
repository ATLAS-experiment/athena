# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class ClusteringStrategy(FlagEnum):
  SinglePad = "SinglePad"
  MultiPad = "MultiPad"

def createHGTD_ConfigFlags():
  hgtdcf = AthConfigFlags()

  hgtdcf.addFlag('HGTD.doMonitoring', False)
  hgtdcf.addFlag('HGTD.doActs', False)
  hgtdcf.addFlag('HGTD.outputAltirocRDO', False)
  hgtdcf.addFlag('HGTD.Acts.ClusteringStrategy', ClusteringStrategy.SinglePad, type=ClusteringStrategy)
  hgtdcf.addFlag('HGTD.useALTIROC_RDO',False)
  
  hgtdcf.addFlag("HGTD.Geometry.useGeoModelXml", True)
  hgtdcf.addFlag("HGTD.Geometry.isLocal", False)
  hgtdcf.addFlag("HGTD.Geometry.Filename", "HGTD.gmx")
  hgtdcf.addFlag("HGTD.Geometry.ClobOutputName", "")

  hgtdcf.addFlag("HGTD.trackingGeometry.passiveBarrelMatZbins", 100) # Number of z bins to be used for passive material layers
  hgtdcf.addFlag("HGTD.trackingGeometry.passiveBarrelMatPhiBins", 50) # Number of phi bins to be used for passive material layers
  hgtdcf.addFlag("HGTD.trackingGeometry.passiveEndcapMatRbins", 50) # Number of r bins to be used for passive material layers
  hgtdcf.addFlag("HGTD.trackingGeometry.passiveEndcapMatPhiBins", 50) # Number of phi bins to be used for passive material

  return hgtdcf
