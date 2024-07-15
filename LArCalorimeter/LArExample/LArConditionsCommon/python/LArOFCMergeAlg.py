# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

##=================================================================================
## Name:        LArOFCMergeAlg.py
##
## Description: An athena algorithm to merge two LAr OFC blobs
##               
##==================================================================================

__author__="Pavol Strizenec <pavol@cern.ch>, based on example from Walter Lampl"
__doc__ = " An athena algorithm to merge two LAr OFC blobs"

from AthenaPython.PyAthena import StatusCode
import AthenaPython.PyAthena as PyAthena

import ROOT

from ROOT import Identifier, IdentifierHash

from PyCool import cool

from array import array
import struct


def patchChannel(idhelper, channel, patchdet, patchFT, patchChan, verbose=False ):
    chid = idhelper.channel_Id(IdentifierHash(channel))
    chidval = chid.get_identifier32().get_compact()
    name = idhelper.channel_name(chid)
    FT = idhelper.feedthrough_name(chid)
    
    if len(patchdet) > 0:
        if 0 in patchdet and idhelper.isEMBchannel(chid):
            if verbose: print(name, chidval, "is in EMB")
            return True
        if 1 in patchdet and idhelper.isEMECOW(chid):
            if verbose: print(name, chidval, "is in EMECOW")
            return True
        if 2 in patchdet and idhelper.isEMECIW(chid):
            if verbose: print(name, chidval, "is in EMECIW")
            return True
        if 3 in patchdet and idhelper.isHECchannel(chid):
            if verbose: print(name, chidval, "is in HEC")
            return True
        if 4 in patchdet and idhelper.isFCALchannel(chid):
            if verbose: print(name, chidval, "is in FCAL")
            return True
    if len(patchFT) > 0:
        if FT in patchFT:
            if verbose: print(name, chidval, "is in FT", FT)
            return True
    if len(patchChan) > 0:
        if chidval in patchChan:
            if verbose: print(name, chidval, "is in list of channels")
            return True

    return False



class LArOFCMergeAlg(PyAthena.Alg):
    
    def __init__(self, name="LArOFCMergeAlg", **kw):
        ## init the base class
        kw['name'] = name
        super(LArOFCMergeAlg,self).__init__(**kw)

        self.indb=kw.get('inputdb',"")
        self.patchdb=kw.get('patchingdb',"")
        self.outdb=kw.get('outputdb',"")
        self.folder=kw.get('fld',"")
        self.run=kw.get('runnum',999999)
        self.issc=kw.get('isSC',False)
        self.patchdet=kw.get('patchdet',[])
        self.patchFT=kw.get('patchFT',[])
        self.patchChan=kw.get('patchChan',[])
        self.nEvts=0
        
        self.msg.info(f"indb: {self.indb}, patchdb:{self.patchdb}, patchdet: {self.patchdet}, patchFT: {self.patchFT}, patchChan: {self.patchChan}, outdb: {self.outdb}, folder: {self.folder}, run: {self.run}, issc: {self.issc}") 
        return

    

    def initialize(self):
        ## note that we are using the python logging service
        ## and that the PyAthena.Alg base class has already initialized
        ## it for us

        # Get DetectorStore...

        self._detStore =  PyAthena.py_svc('StoreGateSvc/DetectorStore')
        if self._detStore is None:
            self.msg.error("Failed to get DetectorStore")
            return StatusCode.Failure

        self._condStore = PyAthena.py_svc('StoreGateSvc/ConditionStore')
        if (self._condStore is None):
            self.msg.error("Failed to get ConditionStore")
            return StatusCode.Failure

        # Get LArOnlineID helper class
        if self.issc:
           self.onlineID=self._detStore.retrieve("LArOnline_SuperCellID","LArOnline_SuperCellID")
        else:
           self.onlineID=self._detStore.retrieve("LArOnlineID","LArOnlineID")
        if self.onlineID is None:
            self.msg.error("Failed to get LArOnlineID")
            return StatusCode.Failure


        self.noid=Identifier()

    
        # this could be also be a paramter from outside:
        self.iovSince = 0
        self.iovUntil = cool.ValidityKeyMax

        return StatusCode.Success

        
    def execute(self):
        eid=ROOT.Gaudi.Hive.currentContext().eventID()

        try:
            if self.issc:
               condCont=self._condStore.retrieve("CondCont<LArOnOffIdMapping>","LArOnOffIdMapSC")
            else:
               condCont=self._condStore.retrieve("CondCont<LArOnOffIdMapping>","LArOnOffIdMap")
            self.larCabling=condCont.find(eid)
        except Exception:
            print("ERROR, failed to get LArCabling")
            return StatusCode.Failure
        
        if self.nEvts==0:
            self.nEvts+=1
            #Process one 'dummy' event to make sure all DB connections get closed
            #print ("Dummy event...")
            return StatusCode.Success

        self.onlineID.set_do_checks(True)
        #self.offlineID.set_do_checks(True)
        
        return StatusCode.Success


    def finalize(self):
        self.msg.info('finalizing...')
        return StatusCode.Success

    def stop(self): # here is all the code for the merging

        from CoolConvUtilities.AtlCoolLib import indirectOpen

        # reading main input blob
        inconn = indirectOpen(self.indb, True)
        if (inconn is None):
           self.msg.error(f"Cannot connect to database {self.indb}")
           raise RuntimeError(f"ERROR: Cannot connect to database {self.indb}")

        try:
           folder=inconn.getFolder(self.folder)
           runiov=self.run << 32
           obj=folder.findObject(runiov,0)
           payload=obj.payload()   
           nsampR = int(payload["nSamples"])
           versR  = int(payload["version"])
           blobRA = payload["OFCa"]
           blobRB = payload["OFCb"]
           blobRT = payload["TimeOffset"]
        except Exception:
           self.msg.warning(f"Could not decode {self.folder} from {self.indb}")
           return StatusCode.Failure  

        print("read input with nsamples %i"%nsampR)

        # reading second input blob
        if self.indb == self.patchdb:
           inconn2=inconn
        else:   
           inconn2 = indirectOpen(self.patchdb, True)
        if (inconn2 is None):
           self.msg.error(f"Cannot connect to database {self.patchdb}")
           raise RuntimeError(f"ERROR: Cannot connect to database {self.patchdb}")

        try:
           folder=inconn2.getFolder(self.folder)
           runiov=self.run << 32
           obj=folder.findObject(runiov,0)
           payload=obj.payload()   
           nsampR2 = int(payload["nSamples"])
           # versR2  = int(payload["version"])
           blobR2A = payload["OFCa"]
           blobR2B = payload["OFCb"]
           blobR2T = payload["TimeOffset"]
        except Exception:
           self.msg.warning(f"Could not decode {self.folder} from {self.patchdb}")
           return StatusCode.Failure  

        print("read second input with nsamples %i"%nsampR2)

        if nsampR != nsampR2:
           self.msg.warning(f"Non equal OFC lengths: {nsampR} vs. {nsampR2}, using lower")

        # create folder
        dbSvc = cool.DatabaseSvcFactory.databaseService()
        try:   
           outconn = dbSvc.openDatabase(self.outdb,False)
           outfolder = outconn.getFolder(self.folder)
        except Exception:
           #create one
           outconn = dbSvc.createDatabase(self.outdb)
           folder = inconn.getFolder(self.folder)
           fspec = folder.folderSpecification()
           from CaloCondBlobAlgs import CaloCondTools
           desc = CaloCondTools.getAthenaFolderDescr()
           outfolder = outconn.createFolder(self.folder, fspec, desc, True)
        
        # create new blobs
        spec = cool.RecordSpecification()
        spec.extend( "OFCa", cool.StorageType.Blob16M )
        spec.extend( "OFCb", cool.StorageType.Blob16M )
        spec.extend( "TimeOffset", cool.StorageType.Blob16M )
        spec.extend( "nSamples", cool.StorageType.UInt32 )
        spec.extend( "version", cool.StorageType.UInt32 )
        data = cool.Record( spec )

        hash_max = self.onlineID.channelHashMax()
        fsize = 4
        vecsize = nsampR*hash_max
        blobsize = vecsize*fsize
        print(blobsize, vecsize, hash_max, fsize)

        btype=getattr(ROOT,"coral::Blob")
        outbloba=btype()
        outblobb=btype()
        outblobt=btype()
        outbloba.resize(blobsize) 
        outblobb.resize(blobsize) 
        outblobt.resize(hash_max*fsize) 

        # now create vectors, which could be filled from input blobs 
        # (because we could not play here tricks with casting the pointers)
        vVeca = ROOT.std.vector('float')(vecsize) 
        vVecb = ROOT.std.vector('float')(vecsize) 
        vVect = ROOT.std.vector('float')(hash_max) 

        # fill from the input blob
        sa=blobRA.read()
        sb=blobRB.read()
        st=blobRT.read()
        if blobsize != len(blobRA):
           self.msg.error(f"Wrong size {len(blobRA)} of the input blob, should be {blobsize}")
           return StatusCode.Failure
        for i in range(0,hash_max):
            for isamp in range(0,nsampR):
              vVeca[i*nsampR + isamp] = float(struct.unpack('f',sa[(i*nsampR + isamp)*fsize:((i*nsampR+isamp)*fsize+fsize)])[0])   
              vVecb[i*nsampR + isamp] = float(struct.unpack('f',sb[(i*nsampR + isamp)*fsize:((i*nsampR+isamp)*fsize+fsize)])[0])   
            pass
            vVect[i] = float(struct.unpack('f',st[i*fsize:(i*fsize+fsize)])[0])   
        pass

        # patch what is needed from the second blob
        pa=blobR2A.read()
        pb=blobR2B.read()
        pt=blobR2T.read()
        if blobsize != len(blobR2A):
           self.msg.error(f"Wrong size {len(blobR2A)} of the patching blob, should be {blobsize}")
           return StatusCode.Failure
        for i in range(0,hash_max):

            doPatch = patchChannel(self.onlineID, i, self.patchdet, self.patchFT, self.patchChan)

            if doPatch:    
               for isamp in range(0,nsampR2):
                 vVeca[i*nsampR2 + isamp] = float(struct.unpack('f',pa[(i*nsampR + isamp)*fsize:((i*nsampR+isamp)*fsize+fsize)])[0])   
                 vVecb[i*nsampR2 + isamp] = float(struct.unpack('f',pb[(i*nsampR + isamp)*fsize:((i*nsampR+isamp)*fsize+fsize)])[0])   
               pass
               vVect[i] = float(struct.unpack('f',pt[i*fsize:(i*fsize+fsize)])[0])   
        pass

              
        #now trick with buffer
        writable_buf_a = array('f')
        writable_buf_b = array('f')
        writable_buf_t = array('f')
        for i in range(0,vVeca.size()):
           writable_buf_a.append(vVeca[i])
           writable_buf_b.append(vVecb[i])
        for i in range(0,vVect.size()):
           writable_buf_t.append(vVect[i])
        #and use buffers to fill blobs
        bta=writable_buf_a.tobytes()
        btb=writable_buf_b.tobytes()
        btt=writable_buf_t.tobytes()
        for j in range(0,len(bta)):
           outbloba[j]=bta[j]
           outblobb[j]=btb[j]
        for j in range(0,len(btt)):
           outblobt[j]=btt[j]
        data["OFCa"] = outbloba   
        data["OFCb"] = outblobb   
        data["TimeOffset"] = outblobt   
        data["nSamples"] = nsampR
        data["version"] = versR

        #and record data   
        outfolder.storeObject(self.iovSince, self.iovUntil, data, cool.ChannelId(0))   

        #=== close the database
        outconn.closeDatabase()
        
        return StatusCode.Success



