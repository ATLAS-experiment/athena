#!/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
import getpass
import math
import sys
_default_inputFile = "calonoise.txt"
_default_filename = "larnoisesqlite.db"


def usage():
   print ("Syntax for UPD4 open-end IoV noise update")
   print (" The first parameter is the run number of IoV start, the second parameter is the lumiblock number for IoV start")
   print (" The third and fourth parameter are the Run/lb for IoV end (if run is -1, uses open ended IoV)")
   print (" The fifth parameter is the upd4 tag name")
   print (" The sixth parameter is input text file name (default calonoise.txt)")
   print (" The seventh parameter is output sqlite file name (default caloSqlite.db)")
   print (" The eigth parameter is output DB  name (default CONDBR2)")
   print (" The nineth parameter is output folder name (default /LAR/NoiseOfl/CellNoise) ")
   print (" The tenth parameter is mu (default 30)")
   print (" The eleventh parameter is dt (default 25)")


def fillLArNoiseDB(iovSince, iovUntil, tag, inputFile=_default_inputFile, filename=_default_filename, 
                   dbname="CONDBR2", folderPath="/LAR/NoiseOfl/CellNoise", mu=-1, dt=-1):
   import ROOT
   from PyCool import cool
   from AthenaPython.PyAthena import StatusCode
   from CaloCondBlobAlgs import CaloCondTools, CaloCondLogger
   #=== values for the comment channel
   author   = getpass.getuser()
   comment  = "Updated noise values"
   #=== get a logger
   log = CaloCondLogger.getLogger("CaloNoiseWriter")

   #=== (re-)create the database
   dbSvc = cool.DatabaseSvcFactory.databaseService()
   try:
      db=dbSvc.openDatabase("sqlite://;schema="+filename+";dbname="+dbname, False)
   except Exception:
      db=dbSvc.createDatabase("sqlite://;schema="+filename+";dbname="+dbname)
      
   try:
      #=== creating folder specifications
      spec = cool.RecordSpecification()
      spec.extend( 'CaloCondBlob16M', cool.StorageType.Blob16M )
      fspec = cool.FolderSpecification(cool.FolderVersioning.MULTI_VERSION, spec)

      #=== create the folder
      folderTag  = tag
      log.info( "Filling COOL folder %s with tag %s", folderPath, folderTag )
      desc = CaloCondTools.getAthenaFolderDescr()
      try:
         folder = db.getFolder(folderPath)
      except Exception:
         log.warning("Folder %s not found, creating it...", folderPath)
         #folder = db.createFolder(folderPath, spec, desc, cool.FolderVersioning.MULTI_VERSION, True)
         folder = db.createFolder(folderPath, fspec, desc, True)
         
      #==================================================
      #=== Create the CaloCondBlobFlt objects
      #==================================================   
      #=== default a and b to be used for each gain 
      gainDefVec = ROOT.std.vector('float')()
      gainDefVec.push_back(0.) # a 
      gainDefVec.push_back(0.) # b 
      #=== three gains per channel for LAr
      #defVecLAr = g.std.vector('std::vector<float>')()
      defVecLAr = ROOT.std.vector('std::vector<float>')()
      defVecLAr.push_back(gainDefVec)
      defVecLAr.push_back(gainDefVec)
      defVecLAr.push_back(gainDefVec)
      #=== four "gains" per channel for Tile
      #defVecTile = g.std.vector('std::vector<float>')()
      defVecTile = ROOT.std.vector('std::vector<float>')()
      defVecTile.push_back(gainDefVec)
      defVecTile.push_back(gainDefVec)
      defVecTile.push_back(gainDefVec)
      defVecTile.push_back(gainDefVec)
      
      #=== system specific data: sysId  -> (nChannel, hash-offset, default-vector, name)
      systemDict = {  0 : (31872,      0, defVecLAr , 'EMEC, z<0'),
                     1 : (54784,  31872, defVecLAr , 'EMB , z<0'),
                     2 : (54784,  86656, defVecLAr , 'EMB , z>0'),
                     3 : (31872, 141440, defVecLAr , 'EMEC, z>0'),
                     16 : ( 5632,      0, defVecLAr , 'HEC'      ),
                     32 : ( 3524,      0, defVecLAr , 'FCAL'     ),
                     48 : ( 5184,      0, defVecTile, 'TILE'     ) 
                     }
      fltDict = {}
      for systemId, info in systemDict.items():
         if (systemId<48) :
            nChannel = info[0] 
            defVec   = info[2]
            sysName  = info[3]
            log.info("Creating BLOB for %s", sysName)
            data = cool.Record( spec )
            blob = data['CaloCondBlob16M']
            flt = ROOT.CaloCondBlobFlt.getInstance(blob)
            flt.init(defVec,nChannel,1,author,comment)
            fltDict[systemId] = [data,flt]
            mbSize = float(blob.size()) / 1024.
            log.info("---> BLOB size is %4.1f kB", mbSize)

      #=== read noise values from file
      lines = open(inputFile,"r").readlines()
      for line in lines:
         fields = line.split()
         if len(fields) < 5:
            log.info("---> wrong line length %d entries ", len(fields))
            continue
         pass
         systemId = int(fields[1])
         hash     = int(fields[2]) - systemDict[systemId][1]
         gain     = ROOT.CaloCondUtils.getDbCaloGain(int(fields[3]))
         noiseA   = float(fields[4])
         noiseB   = float(fields[5])
         flt = fltDict[systemId][1]
         if mu > 0 and dt > 0:
            # new normalization
            if dt > 25:
               noiseB /= math.sqrt(mu/53.*10.)
            else:
               noiseB /= math.sqrt(mu/29.*10.) 
         pass
         if mu == 0:
            noiseB = 0
         flt.setData(hash,gain,0,noiseA)
         flt.setData(hash,gain,1,noiseB)
         
      #=== write to DB
      for systemId, dataList in fltDict.items():
         if (systemId<48):
            sysName  = systemDict[systemId][3]
            log.info("Committing BLOB for %s", sysName)
            channelId = cool.ChannelId(systemId)
            log.info("Cool channel ID %s", channelId)
            data = dataList[0]
            folder.storeObject(iovSince, iovUntil, data, channelId, folderTag)
            sc = StatusCode.Success
   except Exception as e:
      log.fatal("Exception caught:")
      print (e)
      sc = StatusCode.Failure
   #=== close the database
   db.closeDatabase()
   return sc


if __name__ == "__main__":
   if len(sys.argv)<6:
      usage()
      sys.exit(-1)
   runSince = sys.argv[1]
   lbkSince = sys.argv[2]
   runUntil = sys.argv[3]
   lbkUntil = sys.argv[4]
   tag = sys.argv[5]
   kwargs = {}
   if len(sys.argv)>6:
      kwargs["inputFile"]=sys.argv[6]

   if len(sys.argv)>7:
      kwargs["filename"]=sys.argv[7]

   if len(sys.argv)>8:
      kwargs["dbname"]=sys.argv[8]

   if len(sys.argv)>9:
      kwargs["folderPath"] = sys.argv[9]
 
   if len(sys.argv)>10:
      kwargs["mu"] = int(sys.argv[10])
      mu = -1

   if len(sys.argv)>11:
      kwargs["dt"] = int(sys.argv[11])

   print ("input:  ", kwargs.get("inputFile", _default_inputFile))
   print ("runUntil ", runUntil, lbkUntil)
   print ("output:", kwargs.get("filename", _default_filename))
   #=== IOV range
   from CaloCondBlobAlgs import CaloCondTools
   from PyCool import cool
   iovSince = CaloCondTools.iovFromRunLumi(runSince,lbkSince)
   iovUntil = cool.ValidityKeyMax
   print (" iovUntil max ",iovUntil)
   if int(runUntil) > 0:
      print (" use run number to define iobUntil ", runUntil)
      iovUntil = CaloCondTools.iovFromRunLumi(runUntil,lbkUntil)
   print (" iovSince ", iovSince)
   print (" iovUntil ", iovUntil)
   sc = fillLArNoiseDB(iovSince, iovUntil, tag, **kwargs)
   sys.exit(0 if sc.isSuccess() else -1)
