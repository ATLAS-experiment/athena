# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def ITkStripGeoModelCfg(flags):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    ITkStripDetectorTool = CompFactory.ITk.StripDetectorTool()
    # ITkStripDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic #Will we need to do dynamic alignment for ITk?
    ITkStripDetectorTool.Alignable = flags.ITk.Geometry.stripAlignable
    ITkStripDetectorTool.AlignmentFolderName = flags.ITk.Geometry.alignmentFolder
    ITkStripDetectorTool.DetectorName = "ITkStrip"
    if flags.ITk.Geometry.StripLocal:
        # Setting this filename triggers reading from local file rather than DB
        ITkStripDetectorTool.GmxFilename = flags.ITk.Geometry.StripFilename
    if flags.ITk.Geometry.StripClobOutputName:
        ITkStripDetectorTool.ClobOutputName = flags.ITk.Geometry.StripClobOutputName
    geoModelSvc.DetectorTools += [ ITkStripDetectorTool ]

    # If we want to make eta overlap space points in strip endcaps, we need first to search for neighbour elements
    ITkStripDetectorTool.doEndcapEtaNeighbour = flags.ITk.doEndcapEtaNeighbour

    return acc


def ITkStripAlignmentCfg(flags):
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        return addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", flags.ITk.Geometry.alignmentFolder)
    else:
        from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripAlignCondAlgCfg
        return ITkStripAlignCondAlgCfg(flags)


def ITkStripSimulationGeometryCfg(flags):
    # main GeoModel config
    acc = ITkStripGeoModelCfg(flags)
    acc.merge(ITkStripAlignmentCfg(flags))
    return acc


def ITkStripReadoutGeometryCfg(flags):
    # main GeoModel config
    acc = ITkStripGeoModelCfg(flags)
    acc.merge(ITkStripAlignmentCfg(flags))
    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripDetectorElementCondAlgCfg
    acc.merge(ITkStripDetectorElementCondAlgCfg(flags))
    return acc
