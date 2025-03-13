# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import BeamType


def createITkConfigFlags():
    itkcf = AthConfigFlags()

    # take geometry XML files from local instance rather than Detector Database, for development
    itkcf.addFlag("ITk.Geometry.AllLocal", False)
    itkcf.addFlag("ITk.Geometry.PixelLocal",
                  lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
    itkcf.addFlag("ITk.Geometry.PixelFilename",
                  "ITKLayouts/Pixel/ITkPixel.gmx")
    itkcf.addFlag("ITk.Geometry.PixelClobOutputName", "")
    itkcf.addFlag("ITk.Geometry.StripLocal",
                  lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
    itkcf.addFlag("ITk.Geometry.StripFilename",
                  "ITKLayouts/Strip/ITkStrip.gmx")
    itkcf.addFlag("ITk.Geometry.StripClobOutputName", "")
    itkcf.addFlag("ITk.Geometry.BCMPrimeLocal",
                  lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
    itkcf.addFlag("ITk.Geometry.BCMPrimeFilename",
                  "ITKLayouts/Pixel/BCMPrime.gmx")
    itkcf.addFlag("ITk.Geometry.BCMPrimeClobOutputName", "")
    itkcf.addFlag("ITk.Geometry.PLRLocal",
                  lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
    itkcf.addFlag("ITk.Geometry.PLRFilename", "ITKLayouts/PLR/PLR.gmx")
    itkcf.addFlag("ITk.Geometry.PLRClobOutputName", "")
    itkcf.addFlag("ITk.Geometry.DictionaryLocal",
                  lambda prevFlags: prevFlags.ITk.Geometry.AllLocal)
    itkcf.addFlag("ITk.Geometry.DictionaryFilename",
                  "ITKLayouts/IdDictInnerDetector_ITK_LOCAL.xml")
    itkcf.addFlag("ITk.Geometry.isLocal", lambda prevFlags:
                  prevFlags.ITk.Geometry.PixelLocal or
                  prevFlags.ITk.Geometry.StripLocal or
                  prevFlags.ITk.Geometry.BCMPrimeLocal or
                  prevFlags.ITk.Geometry.PLRLocal)
    # The following flag is (3 Oct 2023) unused for now, the charge calibration
    # constants are hardcoded in the alg implementation
    # InnerDetector/InDetConditions/PixelConditionsAlgorithms/src/ITkPixChargeCalibAlg.cxx
    itkcf.addFlag("ITk.Conditions.PixelChargeCalibTag", "ChargeCalib-MC21-01")
    itkcf.addFlag("ITk.Conditions.PixelChargeCalibFile", "")
    itkcf.addFlag("ITk.Conditions.PixelOfflineCalibTag",
                  "PixelITkError_v5_ATLAS-P2-RUN4-01")
    itkcf.addFlag("ITk.Conditions.PixelOfflineCalibFile", "")

    # Turn on running of PRD MultiTruthMaker
    itkcf.addFlag("ITk.doTruth", lambda prevFlags: prevFlags.Input.isMC)

    # Turn on SCT_ModuleVetoSvc, allowing it to be configured later
    itkcf.addFlag("ITk.doStripModuleVeto", False)
    # Path to the JSON file to mask the modules
    itkcf.addFlag("ITk.JsonPathStripModuleVeto", "")
    # Path to the JSON file to mask the modules for Pixel.
    # A non-empty string activates the veto automatically 
    itkcf.addFlag("ITk.JsonPathPixelModuleVeto", "")
    # Enable check for dead modules and FEs
    itkcf.addFlag("ITk.checkDeadPixelsOnTrack", True)
    # defines if the X1X mode is used for the offline or not
    itkcf.addFlag("ITk.selectStripIntimeHits", lambda prevFlags:
                  not(prevFlags.Beam.Type is BeamType.Cosmics))

    # Save cluster information to Derivation
    itkcf.addFlag("ITk.DAODStorePixel", lambda prevFlags:
                  prevFlags.Detector.EnableITkPixel)
    itkcf.addFlag("ITk.DAODStoreStrip", lambda prevFlags:
                  prevFlags.Detector.EnableITkStrip)
    itkcf.addFlag("ITk.DAODStoreExtra", True)

    # Save SiHitCollections to RDO
    itkcf.addFlag("ITk.savePixelSiHits", lambda prevFlags:
                  prevFlags.BTagging.Trackless or
                  prevFlags.BTagging.savePixelHits)
    itkcf.addFlag("ITk.saveStripSiHits", lambda prevFlags:
                  prevFlags.BTagging.Trackless or
                  prevFlags.BTagging.saveSCTHits)

    # config flags for tracking geometry configuration
    from InDetConfig.TrackingGeometryFlags import createITkTrackingGeometryFlags
    itkcf.addFlagsCategory("ITk.trackingGeometry",
                           createITkTrackingGeometryFlags, prefix=True)

    return itkcf
