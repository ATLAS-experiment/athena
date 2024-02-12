# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def InitializeGeometryParameters(dbGeomCursor):
    """Read muon geometry parameters

    dbGeomCursor: AtlasGeoDBInterface instance
    """
    dbId, dbSwitches, dbParam = dbGeomCursor.GetCurrentLeafContent("MuonSwitches")
    params = {"Layout" : "UNDEFINED",
              "HasCSC" : False,
              "HasSTGC" : True,
              "HasMM" : True,
              "HasMDT": True,
              "HasRPC": True,
              "HasTGC": True,
              "useR4Plugin": False}

    if len(dbId)>0:
        key=dbId[0]
        if "LAYOUTNAME" in dbParam:
            params["Layout"] = dbSwitches[key][dbParam.index("LAYOUTNAME")]
        if "HASCSC" in dbParam:
            params["HasCSC"] = (dbSwitches[key][dbParam.index("HASCSC")] != 0)
        if "HASSTGC" in dbParam:
            params["HasSTGC"] = (dbSwitches[key][dbParam.index("HASSTGC")] !=0)
        if "HASMM" in dbParam:
            params["HasMM"] = (dbSwitches[key][dbParam.index("HASMM")] != 0)

    return params

def InitializeGeometryParameters_SQLite(sqliteDbReader):
    """Read muon geometry parameters from SQLite

    sqliteDbReader: AtlasGeoDBInterface_SQLite instance
    """
    dbData = sqliteDbReader.GetData("MuonSwitches")
    params = {"Layout" : "UNDEFINED",
              "HasCSC" : False,
              "HasSTGC" : True,
              "HasMM" : True,
              "HasMDT": True,
              "HasRPC": True,
              "HasTGC": True,
              "useR4Plugin": False}

    if dbData:
        if "LAYOUTNAME" in dbData[0].keys():
            params["Layout"] = dbData[0]["LAYOUTNAME"]
        if "HASCSC" in dbData[0].keys():
            params["HasCSC"] = (dbData[0]["HASCSC"] != 0)
        if "HASSTGC" in dbData[0].keys():
            params["HasSTGC"] = (dbData[0]["HASSTGC"] !=0)
        if "HASMM" in dbData[0].keys():
            params["HasMM"] = (dbData[0]["HASMM"] != 0)
        if "HASMDT" in dbData[0].keys():
            params["HasMDT"] = (dbData[0]["HASMDT"] != 0)
        if "HASRPC" in dbData[0].keys():
            params["HasRPC"] = (dbData[0]["HASRPC"] != 0)
        if "HASTGC" in dbData[0].keys():
            params["HasTGC"] = (dbData[0]["HASTGC"] != 0)
        if "isMuonR4Plugin" in dbData[0].keys():
            params["useR4Plugin"] = (dbData[0]["isMuonR4Plugin"] != 0)
    return params
