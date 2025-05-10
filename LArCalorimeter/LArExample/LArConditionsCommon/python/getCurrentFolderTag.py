#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys

from PyCool import cool
sys.path.append('/afs/cern.ch/user/a/atlcond/utils22')
from CondUtilsLib.AtlCoolBKLib import resolveAlias


def _resolveTagFaster(folder,tag):
    try:
        import cppyy
        # <folder> is at this point a pythonized shared_ptr<cool::IFolder>.
        # Doing an explicit cast to the base class cool::IHvsNode
        # significantly reduces the overhead from cling, for some reason.
        folder=cppyy.bind_object(cppyy.addressof(folder),
                                 getattr(cppyy.gbl,'cool::IHvsNode'))
        # this is in general not a safe cast, but IFolder is an abstract class
        # only inheriting from IHvsNode.
    except Exception: pass
    return folder.resolveTag(tag)


def getCurrentFolderTag(dbname,folderName,ES=False):
    currentTag,nextTag=None,None

    #1. Get current and next global tags using resolver class in ~atlcond
    resolver=resolveAlias()
    if(ES):
       currentGlobal=resolver.getCurrentES().replace("*","ST")
       nextGlobal=resolver.getNextES().replace("*","ST")
    else:   
       currentGlobal=resolver.getCurrent().replace("*","ST")
       nextGlobal=resolver.getNext().replace("*","ST")

    print('currentGlobal: ',currentGlobal)
    #2. Open the DB to resolve this gobal tag for the given folder
    dbSvc = cool.DatabaseSvcFactory.databaseService() 
    db = dbSvc.openDatabase(dbname)
    f=db.getFolder(folderName)
    try:
        currentTag=_resolveTagFaster(f,currentGlobal)
    except Exception:
        print('Warning: could not resolve ',currentGlobal,' in db: ',dbname)
        if "DBR2" in dbname:
           print('resolving for the global CONDBR2-BLKPA-2022-10')
           tmpGlobal='CONDBR2-BLKPA-2022-10'
        else:
           print('resolving for the global COMCOND-BLKPA-RUN1-06')
           tmpGlobal='COMCOND-BLKPA-RUN1-06'

        try:
           currentTag=_resolveTagFaster(f,tmpGlobal)
        except Exception:
           print('Also not working, giving up')
           pass
        pass

    if len(nextGlobal)>2:
        # NEXT exists, try to resolve it
        try:
            nextTag=_resolveTagFaster(f,nextGlobal)
        except Exception:
            pass
        pass
    
    db.closeDatabase()
    return (currentTag,nextTag)


if __name__=="__main__":
    if len(sys.argv)<3:
        sys.stderr.write("Usage: %s dbname folder\n" %  sys.argv[0].split("/")[-1])
        sys.exit(-1)

    dn=sys.argv[1]
    fn=sys.argv[2]
    if len(sys.argv)>3:
       estag=sys.argv[3]
    else:   
       estag=False

    currTag=getCurrentFolderTag(dn,fn,estag)[0]
    
    if currTag is None:
        sys.stderr.write("Failed to resolve current folder-level tag for folder %s in db %s\n" % (fn,dn))
        sys.exit(-1)

    print(currTag)
