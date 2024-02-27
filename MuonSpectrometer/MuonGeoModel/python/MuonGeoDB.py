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
        if "hasCsc" in dbData[0].keys():
            params["HasCSC"] = (dbData[0]["hasCsc"] != 0)
        if "hasSTGC" in dbData[0].keys():
            params["HasSTGC"] = (dbData[0]["hasSTGC"] !=0)
        if "hasMM" in dbData[0].keys():
            params["HasMM"] = (dbData[0]["hasMM"] != 0)
        if "hasMdt" in dbData[0].keys():
            params["HasMDT"] = (dbData[0]["hasMdt"] != 0)
        if "hasRpc" in dbData[0].keys():
            params["HasRPC"] = (dbData[0]["hasRpc"] != 0)
        if "hasTgc" in dbData[0].keys():
            params["HasTGC"] = (dbData[0]["hasTgc"] != 0)
        if "isMuonR4Plugin" in dbData[0].keys():
            params["useR4Plugin"] = (dbData[0]["isMuonR4Plugin"] != 0)
    return params
