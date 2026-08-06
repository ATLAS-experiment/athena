#!/bin/env python

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys

from PyCool import cool



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


def getFolderTag(dbname: str, folderNames: str|list|tuple, globalTags: str|list|tuple):
    dbSvc = cool.DatabaseSvcFactory.databaseService() 
    db = dbSvc.openDatabase(dbname)
    if isinstance(folderNames, str): folderNames = [folderNames]
    if isinstance(globalTags, str): globalTags = [globalTags]
    tags = []
    for fn in folderNames:
        f = db.getFolder(fn)
        tags.append([])
        for gt in globalTags:
            try:
                tags[-1].append(_resolveTagFaster(f, gt))
            except Exception:
                print(f"Warning: could not resolve {gt} for folder {fn} in db {dbname}")
                tags[-1].append(None)
    db.closeDatabase()
    def _unnest_singles(x): return (x if isinstance(x, str) or x is None
                                      else _unnest_singles(x[0]) if len(x)==1 
                                      else tuple(_unnest_singles(y) for y in x))
    return _unnest_singles(tags)


def getCurrentFolderTag(dbname, folderName, ES=False, verbose=True):
    currentTag,nextTag=None,None
    #1. Get current and next global tags using resolver class in ~atlcond
    utils='/afs/cern.ch/user/a/atlcond/utils22'
    if utils not in sys.path: sys.path.append(utils)
    from CondUtilsLib.AtlCoolBKLib import resolveAlias
    resolver=resolveAlias()
    if(ES):
       currentGlobal=resolver.getCurrentES().replace("*","ST")
       nextGlobal=resolver.getNextES().replace("*","ST")
    else:   
       currentGlobal=resolver.getCurrent().replace("*","ST")
       nextGlobal=resolver.getNext().replace("*","ST")
    if verbose: print('currentGlobal: ',currentGlobal)
    #2. Open the DB to resolve this gobal tag for the given folder
    currentTag, nextTag = getFolderTag(dbname, folderName, (currentGlobal, nextGlobal))
    if currentTag is None:
        tmpGlobal="CONDBR2-BLKPA-2022-10" if "DBR2" in dbname else "COMCOND-BLKPA-RUN1-06"
        if verbose: print(f"resolving for the global {tmpGlobal}")
        currentTag = getFolderTag(dbname, folderName, tmpGlobal)
        if currentTag is None:
            if verbose: print('Also not working, giving up')
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
