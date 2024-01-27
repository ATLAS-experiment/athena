# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def InitializeGeometryParameters(dbGeomCursor):
    """Read geometry parameters for Pixel

    dbGeomCursor: AtlasGeoDBInterface instance
    """

    # ----------------------------------------------------------------------------
    # Read versionname, layout and dbm from PixelSwitches table
    dbId, dbSwitches, dbParam = dbGeomCursor.GetCurrentLeafContent("PixelSwitches")

    params = {"VersionName" : "UNDEFINED",
              "Layout" : "UNDEFINED",
              "DBM" : False }

    if dbId:
        key = dbId[0]
        if "VERSIONNAME" in dbParam:
            params["VersionName"] = dbSwitches[key][dbParam.index("VERSIONNAME")]
        if "LAYOUT" in dbParam :
            params["Layout"] = dbSwitches[key][dbParam.index("LAYOUT")]
        if "BUILDDBM" in dbParam :
            params["DBM"] = (dbSwitches[key][dbParam.index("BUILDDBM")] != 0)


    # ----------------------------------------------------------------------------
    # IBL layout
    params["IBL"] = (params["Layout"] == "IBL")
    params["IBLlayout"] = "noIBL"

    if params["IBL"]:
        dbId, dbLayers, dbParam = dbGeomCursor.GetCurrentLeafContent("PixelLayer")
        IBLStaveIndex = -1
        IBLgeoLayout = -1
        
        if dbId:
            key = dbId[0]
            if "STAVEINDEX" in dbParam and dbLayers[key][dbParam.index("STAVEINDEX")] not in ["NULL", None]:
                IBLStaveIndex = int(dbLayers[key][dbParam.index("STAVEINDEX")])

            if IBLStaveIndex > -1:
                dbId, dbStaves, dbParam = dbGeomCursor.GetCurrentLeafContent("PixelStave")

                if dbId and IBLStaveIndex <= len(dbStaves.keys()):
                    key = dbId[IBLStaveIndex]
                    if "LAYOUT" in dbParam and dbStaves[key][dbParam.index("LAYOUT")] not in ["NULL", None]:
                        IBLgeoLayout = int(dbStaves[key][dbParam.index("LAYOUT")])
                        if IBLgeoLayout in [3,4] : params["IBLlayout"] = "planar"
                        elif IBLgeoLayout in [5] : params["IBLlayout"] = "3D"

    return params

def InitializeGeometryParameters_SQLite(sqliteDbReader):
    """Read geometry parameters for Pixel from SQLite

    sqliteDbReader: AtlasGeoDBInterface_SQLite instance
    """

    # ----------------------------------------------------------------------------
    # Read versionname, layout and dbm from PixelSwitches table
    dbData = sqliteDbReader.GetData("PixelSwitches")

    params = {"VersionName" : "UNDEFINED",
              "Layout" : "UNDEFINED",
              "DBM" : False }

    if dbData:
        if "VERSIONNAME" in dbData[0].keys():
            params["VersionName"] = dbData[0]["VERSIONNAME"]
        if "LAYOUT" in dbData[0].keys():
            params["Layout"] = dbData[0]["LAYOUT"]
        if "BUILDDBM" in dbData[0].keys():
            params["DBM"] = (dbData[0]["BUILDDBM"] != 0)


    # ----------------------------------------------------------------------------
    # IBL layout
    params["IBL"] = (params["Layout"] == "IBL")
    params["IBLlayout"] = "noIBL"

    if params["IBL"]:
        dbData = sqliteDbReader.GetData("PixelLayer")
        IBLStaveIndex = -1
        IBLgeoLayout = -1

        if dbData:
            if "STAVEINDEX" in dbData[0].keys() and dbData[0]["STAVEINDEX"] not in ["NULL", None]:
                IBLStaveIndex = int(dbData[0]["STAVEINDEX"])

            if IBLStaveIndex > -1:
                dbData = sqliteDbReader.GetData("PixelStave")
                if dbData and IBLStaveIndex <= len(dbData):
                    if "LAYOUT" in dbData[IBLStaveIndex].keys() and dbData[IBLStaveIndex]["LAYOUT"] not in ["NULL", None]:
                        IBLgeoLayout = int(dbData[IBLStaveIndex]["LAYOUT"])
                        if IBLgeoLayout in [3,4] : params["IBLlayout"] = "planar"
                        elif IBLgeoLayout in [5] : params["IBLlayout"] = "3D"

    return params
