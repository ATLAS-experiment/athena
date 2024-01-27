# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def InitializeGeometryParameters(dbGeomCursor):
   """Read version name, layout and dbm from AtlasCommon table.

   dbGeomCursor: AtlasGeoDBInterface instance
   """

   dbId, dbCommon, dbParam = dbGeomCursor.GetCurrentLeafContent("AtlasCommon")

   params = {"Run" : "UNDEFINED",
             "GeoType" : "UNDEFINED"}

   if len(dbId)>0:
      key = dbId[0]
      if "CONFIG" in dbParam :
         params["Run"] = dbCommon[key][dbParam.index("CONFIG")]
      if "GEOTYPE" in dbParam :
         params["GeoType"] = dbCommon[key][dbParam.index("GEOTYPE")]

   return params

def InitializeGeometryParameters_SQLite(sqliteDbReader):
   """Read version name, layout and dbm from AtlasCommon table in SQLite

   sqliteDbReader: AtlasGeoDBInterface_SQLite instance
   """

   dbData = sqliteDbReader.GetData("AtlasCommon")

   params = {"Run" : "UNDEFINED",
             "GeoType" : "UNDEFINED"}

   if dbData:
      if "CONFIG" in dbData[0].keys():
         params["Run"] = dbData[0]["CONFIG"]
      if "GEOTYPE" in dbData[0].keys():
         params["GeoType"] = dbData[0]["GEOTYPE"]

   return params


def InitializeLuminosityDetectorParameters(dbGeomCursor):
   """Read luminosity detectors from the DB

   dbGeomCursor: AtlasGeoDBInterface instance
   """

   dbId, dbCommon, dbParam = dbGeomCursor.GetCurrentLeafContent("LuminositySwitches")

   params = {"BCMPrime" : False,
             "PLR" : False}

   if len(dbId):
      key = dbId[0]
      if "BCMPRIME" in dbParam :
         params["BCMPrime"] = dbCommon[key][dbParam.index("BCMPRIME")] == 1
      if "PLR" in dbParam :
         params["PLR"] = dbCommon[key][dbParam.index("PLR")] == 1

   return params

def InitializeLuminosityDetectorParameters_SQLite(sqliteDbReader):
   """Read luminosity detectors from LuminositySwitches table in the SQLite DB

   sqliteDbReader: AtlasGeoDBInterface_SQLite instance
   """

   dbData = sqliteDbReader.GetData("LuminositySwitches")

   params = {"BCMPrime" : False,
             "PLR" : False}

   if dbData:
      if "BCMPRIME" in dbData[0].keys():
         params["BCMPrime"] = dbData[0]["BCMPRIME"] == 1
      if "PLR" in dbData[0].keys():
         params["PLR"] = dbData[0]["PLR"] == 1

   return params

def SetupLocalSqliteGeometryDb(geometryFilePath,geometryTag):
    """ Configure the reading of local SQLite Geometry Database file

    This process consists of three steps
    1. It is necessary to create ./Geometry directory
    2. Place in this directory a symlink to the geometryFilePath with the name 'geometryTag.db'
    3. Prepend '.' to the DATAPATH environment
    """

    import os
    if not os.path.exists("Geometry"):
        os.mkdir("Geometry")
    linkName = geometryTag + ".db"
    linkPath = os.path.join("Geometry",linkName)
    if not os.path.exists(linkPath):
        os.symlink(geometryFilePath,linkPath)
    if 'DATAPATH' in os.environ.keys():
        os.environ['DATAPATH']='.:'+ os.environ['DATAPATH']
    else:
        os.environ['DATAPATH']='.'

    return

