# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from PyCool import cool
from AthenaCommon.Logging import logging


class FolderTagResolver:

    _globalTag=""

    _defaultSuffix="-RUN2-UPD3-00"

    def __init__(self,dbname="COOLOFL_LAR/CONDBR2"):
        dbSvc = cool.DatabaseSvcFactory.databaseService()
        self._db = dbSvc.openDatabase(dbname)
        self._msg=logging.getLogger('FolderTagResolver')
        return

    def __del__(self):
        self._db.closeDatabase()
        return

    def getFolderTag(self,foldername,globalTag=None):
        if globalTag is None:
            globalTag=self.__class__._globalTag

        self._msg.info("Looking up folder level tags in %s",foldername)  #using globalTag %s",foldername,globalTag)
        foldertag= ''.join(foldername.split('/')) +  self.__class__._defaultSuffix #default in case of failure


        try:
          folder=self._db.getFolder(foldername)
        except Exception:
          self._msg.warning("\tCould not find folder %s in database %s",foldername,self._db.databaseId())
          self._msg.warning("\tFalling back to default tag %s",foldertag)
          return foldertag

        
        taglist=folder.listTags()
        if len(taglist)==1:
            foldertag=taglist[0]
            self._msg.info("\tFound single tag %s",foldertag)
            return foldertag
        else:
            self._msg.info("\tTrying to resolve gobal tag %s",globalTag)
            try:
              foldertag=folder.resolveTag(globalTag)
              self._msg.info("\tResolved tag %s",foldertag)
              return foldertag
            except Exception:
                self._msg.warning("\tCould not find tag relation to %s",globalTag)
                self._msg.warning("\tFalling back to default tag %s",foldertag)
                pass
        return foldertag
          
            
    def getFolderTagSuffix(self,foldername,globalTag=None):
        ft=self.getFolderTag(foldername,globalTag)
        if '_mu' in ft: 
           p=ft.find("_mu") 
        else: 
           p=ft.find("-")
        if p==-1:
            return "-Default"
        else: 
            return ft[p:]
