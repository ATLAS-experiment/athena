# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import os
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaPoolUtilities.DumperConfig import Dumper, find_file
from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg


def TPCnvTest(infile, keys, useGeoModelSvc=False, useIOVDbSvc=False, doPixel=False, doSCT=False, doTRT=False, doLAr=False, doTile=False, doMuon=False, doTracks=False, configOnly=False, adjustMessageSvc=True):

    # Needed to prevent spurious root errors about streams in CreateRealData.
    import ROOT
    ROOT.GaudiPython.CallbackStreamBuf

    # Make sure we don't have a stale file catalog.
    if os.path.exists ('PoolFileCatalog.xml'):
        os.remove ('PoolFileCatalog.xml')

    if ('ATLAS_REFERENCE_TAG' not in globals() and
        'ATLAS_REFERENCE_TAG' in os.environ):
        ATLAS_REFERENCE_TAG = os.environ['ATLAS_REFERENCE_TAG'] # noqa: F841

    refpaths = [os.environ.get ('ATLAS_REFERENCE_DATA', None),
                '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art',
                '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs',
                '/afs/cern.ch/atlas/maxidisk/d33/referencefiles']

    if infile.startswith ('rtt:'):
        infile = infile[4:]
    infile = find_file (infile, refpaths)

    # Provide MC input
    flags = initConfigFlags()
    flags.Input.Files = [infile]
    flags.GeoModel.Run = LHCPeriod.Run1
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN1_2012
    if useGeoModelSvc:
        flags.GeoModel.Align.Dynamic = False
        flags.Detector.GeometryPixel = doPixel
        flags.Detector.GeometrySCT = doSCT
        flags.Detector.GeometryTRT = doTRT
        flags.Detector.GeometryLAr = doLAr
        flags.Detector.GeometryTile = doTile
        flags.Detector.GeometryMuon = doMuon
    flags.fillFromArgs()
    flags.lock()

    # Construct ComponentAccumulator
    acc = MainServicesCfg(flags)
    acc.setAppProperty('PrintAlgsSequence', False, overwrite=True)
    acc.merge(PoolReadCfg(flags))
    if useIOVDbSvc:
        acc.merge(IOVDbSvcCfg(flags))
    EventCnvSuperTool = None
    if useGeoModelSvc:
        if flags.Detector.GeometryPixel:
            from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
            acc.merge(PixelReadoutGeometryCfg(flags))
        if flags.Detector.GeometrySCT:
            from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
            acc.merge(SCT_ReadoutGeometryCfg(flags))
        if flags.Detector.GeometryTRT:
            from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
            acc.merge(TRT_ReadoutGeometryCfg(flags))
        if flags.Detector.GeometryITkPixel:
            from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
            acc.merge(ITkPixelReadoutGeometryCfg(flags))
        if flags.Detector.GeometryITkStrip:
            from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
            acc.merge(ITkStripReadoutGeometryCfg(flags))
        if flags.Detector.GeometryLAr:
            from LArGeoAlgsNV.LArGMConfig import LArGMCfg
            acc.merge(LArGMCfg(flags))
        if flags.Detector.GeometryTile:
            from TileGeoModel.TileGMConfig import TileGMCfg
            acc.merge(TileGMCfg(flags))
        if flags.Detector.GeometryMuon:
            from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
            acc.merge(MuonGeoModelCfg(flags))
        #acc.merge(ForDetGeometryCfg(flags))
        from AtlasGeoModel.GeoModelConfig import GeoModelCfg
        acc.merge(GeoModelCfg(flags))
        acc.getService("GeoModelSvc").IgnoreTagDifference = True
        if doTracks:
            # Doing this here as Trk.EventCnvSuperTool isn't part of all projects
            Trk_EventCnvSuperTool=CompFactory.Trk.EventCnvSuperTool
            EventCnvSuperTool = Trk_EventCnvSuperTool('EventCnvSuperTool', MaxErrorCount=10)
    acc.addEventAlgo(Dumper ('dumper', flags.Input.Files[0], keys, refpaths), 'AthAlgSeq')
    if adjustMessageSvc:
        acc.getService("MessageSvc").enableSuppression = True
        acc.getService("MessageSvc").Format = "% F%18W%S%7W%R%T %0W%M"
    if EventCnvSuperTool is not None:
        acc.addPublicTool(EventCnvSuperTool)
    if configOnly:
        f = open('new.pkl', 'wb')
        acc.store(f)
        f.close()
        return
    return acc.run(maxEvents=10)
