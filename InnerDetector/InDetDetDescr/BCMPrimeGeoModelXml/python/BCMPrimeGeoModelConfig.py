# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory


def BCMPrimeGeometryCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    bcmPrimeDetectorTool = CompFactory.BCMPrimeDetectorTool()
    bcmPrimeDetectorTool.DetectorName = "BCMPrime"
## PEDRO CHANGED THE FOLLOWING LINES
    if flags.ITk.Geometry.BCMPrimeLocal:
##      # Setting this filename triggers reading from local file rather than DB
      bcmPrimeDetectorTool.GmxFilename = flags.ITk.Geometry.BCMPrimeFilename
##    bcmPrimeDetectorTool.GmxFilename = "/home/purrejol/Documents/itk/ITKLayouts/ITKLayouts/data/BCM/BCMPrime.gmx"
    if flags.ITk.Geometry.BCMPrimeClobOutputName:
        bcmPrimeDetectorTool.ClobOutputName = flags.ITk.Geometry.BCMPrimeClobOutputName
    geoModelSvc.DetectorTools += [ bcmPrimeDetectorTool ]

    return acc
