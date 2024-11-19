# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def InitializeGeometryParameters(dbGeomCursor):
   """Read version name, layout and dbm from AtlasCommon table.

   dbGeomCursor: AtlasGeoDBInterface instance
   """

   dbId, dbCommon, dbParam = dbGeomCursor.GetCurrentLeafContent("AtlasCommon")

   params = {"Run" : "UNDEFINED",
             "GeoType" : "UNDEFINED",
             "Detectors" : set(),
             "DetectorsConfigured": False}

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
             "GeoType" : "UNDEFINED",
             "Detectors" : set(),
             "DetectorsConfigured": False}

   if dbData:
      if "CONFIG" in dbData[0].keys():
         params["Run"] = dbData[0]["CONFIG"]
      if "GEOTYPE" in dbData[0].keys():
         params["GeoType"] = dbData[0]["GEOTYPE"]
   
   ### Read the set of the scheduled detector plugins from the meta-data
   dbData =  sqliteDbReader.GetData("AAHEADER")
   if dbData:
      if "PluginNames" in dbData[0].keys():
         plugins = dbData[0]["PluginNames"].split(";")
         print (plugins)
         detectors = set()
         ### BeamPipe
         if "Beampipe" in plugins:
            detectors.add("Bpipe")
         ### Inner detector: "Pixel", "SCT", "TRT", "BCM"
         if "Pixel" in plugins:
            detectors.add("Pixel")
            detectors.add("BCM")
         if "SCT" in plugins:
            detectors.add("SCT")
         if "TRT" in plugins:
            detectors.add("TRT")
         
         ### ITk: "ITkStrip", "ITkPixel", "ITk", "PLR", "BCMPrime"
         if "ITk" in plugins or "ITkStrip" in plugins:
            detectors.add("ITkStrip")
         if "ITk" in plugins or "ITkPixel" in plugins:
            detectors.add("ITkPixel")
         if "ITk" in plugins or "BCMPrime" in plugins:
            detectors.add("BCMPrime")
         if "ITk" in plugins or "PLR" in plugins:
            detectors.add("PLR")
         ### Calo: "LAr", "Tile"
         if "LAr" in plugins:
            detectors.add("LAr")
         if "Tile" in plugins:
            detectors.add("Tile")
         ### HGTD:  "HGTD"
         if "HGTD" in plugins:
            detectors.add("HGTD")
         ### Ignore muons as they write their own switches
         ### Muons:  "MTechPlugin", "Muon"
         params["Detectors"] = detectors
         params["DetectorsConfigured"] =True
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
