#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from CoolConvUtilities.AtlCoolLib import indirectOpen
import time
from functools import cache

class ParticleTypeDCSInfo:
    def __init__(self,beam1Type,beam2Type):
        self._beam1Type=beam1Type
        self._beam2Type=beam2Type

    def getBeam1Type(self):
        return self._beam1Type

    def getBeam2Type(self):
        return self._beam2Type

    
#Cache to avoid multiple loopups
            
@cache
def getTypeForRun(run,quiet=False):
    """Get the particle type in the LHC for a given run from the DCS database"""
   
    newdb=(run>=236107)

    # setup appropriate connection and folder parameters
    if newdb:
        dbname='CONDBR2'
        sorfolder='/TDAQ/RunCtrl/SOR'
    else:
        dbname='COMP200'
        sorfolder='/TDAQ/RunCtrl/SOR_Params'

    if not quiet:
        print ("Reading particle type for run %i, %s" % (run,dbname))
        
    tdaqDB=indirectOpen('COOLONL_TDAQ/%s' % dbname)
    if tdaqDB is None:
        print ("ParticleTypeUtil ERROR: Cannot connect to COOLONL_TDAQ/%s" % dbname)
        return None
    
    sortime=0
    try:
        tdaqfolder=tdaqDB.getFolder(sorfolder)
        runiov=run << 32
        obj=tdaqfolder.findObject(runiov,0) #Cool channel 0
        payload=obj.payload()
        sortime=payload['SORTime']
    except Exception as e:
        print ("ParticleTypeUtil ERROR accessing folder %s" % sorfolder)
        print (e)
    tdaqDB.closeDatabase()

    if not quiet:
        print ("Start of run time:", sortime, time.ctime(sortime/1e9))

    dcsDB=indirectOpen('COOLOFL_DCS/%s' % dbname)
    if dcsDB is None:
        print ("ParticleTypeUtil ERROR: Cannot connect to COOLOFL_DCS/%s" % dbname)
        return None
    data=None
    try:
        f=dcsDB.getFolder('/LHC/DCS/FILLSTATE')
        obj=f.findObject(sortime,1) #Cool channel 1
        payload=obj.payload()
        beam1type=payload["BeamType1"]
        beam2type=payload["BeamType2"]
        data=ParticleTypeDCSInfo(beam1type,beam2type)
        if not quiet:
            print("Found FILLSTATE information valid from %s to %s" % (time.ctime(obj.since()/1e9),time.ctime(obj.until()/1e9)))
    except Exception as e:
        print ("ParticleTypeUtil ERROR getting information from /LHC/DCS/FILLSTATE")
        print (e)
    dcsDB.closeDatabase()  

    return data

if __name__=='__main__':
    import sys
    if len(sys.argv)<2:
        print ("Syntax",sys.argv[0],'<run>')
        sys.exit(1)
    run=int(sys.argv[1])
    info=getTypeForRun(run)
    print ("Particle Type information for run %i" % run)
    if info is not None:
        print ("Type (charge) of Beam 1 ",info.getBeam1Type())
        print ("Type (charge) of Beam 2 ",info.getBeam2Type())
    else:
        print ("Not available")
