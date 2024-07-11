# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DetDescrCnvSvcCfg(flags, **kwargs):
    kwargs.setdefault("IdDictName", "IdDictParser/ATLAS_IDS.xml")
    kwargs.setdefault("HasCSC", flags.Detector.GeometryCSC)
    kwargs.setdefault("HasSTgc", flags.Detector.GeometrysTGC)
    kwargs.setdefault("HasMM", flags.Detector.GeometryMM)
    kwargs.setdefault("HasTGC", flags.Detector.GeometryTGC)
    kwargs.setdefault("HasMDT", flags.Detector.GeometryMDT)
    kwargs.setdefault("HasRPC", flags.Detector.GeometryRPC)
    

    # Use GEOM DB to read InDet dictionary information 
    kwargs.setdefault("useGeomDB_InDet", not flags.GeoModel.SQLiteDB and (flags.Detector.GeometryITk or \
                                                                          flags.Detector.GeometryHGTD))   

    if (flags.Detector.GeometryITk or flags.Detector.GeometryHGTD) \
        and flags.ITk.Geometry.DictionaryLocal:
        kwargs.setdefault("IdDictFromRDB", False)
        kwargs.setdefault("InDetIDFileName", flags.ITk.Geometry.DictionaryFilename)
        kwargs.setdefault("MuonIDFileName", "IdDictParser/IdDictMuonSpectrometer_R.10.00.xml")
        kwargs.setdefault("LArIDFileName", "IdDictParser/IdDictLArCalorimeter_DC3-05-Comm-01.xml")
        kwargs.setdefault("TileIDFileName", "IdDictParser/IdDictTileCalorimeter.xml")
        kwargs.setdefault("CaloIDFileName", "IdDictParser/IdDictCalorimeter_L1Onl.xml")
        kwargs.setdefault("ForwardIDFileName", "IdDictParser/IdDictForwardDetectors_2010.xml")
    else:
        kwargs.setdefault("IdDictFromRDB", True)

    acc = ComponentAccumulator()
    service = CompFactory.DetDescrCnvSvc(**kwargs)
    acc.addService(service, create=True)
    acc.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",
                                                 CnvServices=[service.getName()]))
    return acc
