# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

##=================================================================================
## Name:        LArBlobMergeAlg.py
##
## Description: An athena algorithm to merge two LAr blobs
##               
##==================================================================================

__author__="Pavol Strizenec <pavol@cern.ch>, based on example from Walter Lampl"
__doc__ = " An athena algorithm to merge two LAr blobs"

from AthenaPython.PyAthena import StatusCode
import AthenaPython.PyAthena as PyAthena

import ROOT

from ROOT import Identifier, IdentifierHash

from PyCool import cool

from array import array
import struct
import cppyy

def lenFactor(thelen):
    ''' Get the length of rows in the db, assuming that the number of entries will be divisible by some number of cells / SCs / other DB objects '''
    factor_cells = thelen/ 195072.
    factor_SC = thelen/34048.
    factor_LATOME = thelen/116.
    factor = None
    if factor_cells.is_integer():
        factor = int(factor_cells)
    if factor_SC.is_integer(): 
        factor = int(factor_SC)
    elif factor_LATOME.is_integer(): 
        factor = int(factor_LATOME)
    return factor

def payload_dict(payload, verbose=True):
    contents = {}
    for k in payload.keys():
        contents[k] = {}
        if isinstance(payload[k], cppyy.gbl.coral.Blob):
            factor = lenFactor(len(payload[k])/4)
            if verbose: print(f"{k} consists of {factor} item(s) per row")
            contents[k]["payload"] = payload[k]
            contents[k]["size"] = factor
        else:
            contents[k]["payload"] = payload[k]
            contents[k]["size"] = None
    return contents

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



class LArBlobMergeAlg(PyAthena.Alg):
    
    def __init__(self, name="LArBlobMergeAlg", **kw):
        ## init the base class
        kw['name'] = name
        super(LArBlobMergeAlg,self).__init__(**kw)

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

        contents = {}
        contents2 = {}
        try:
           folder=inconn.getFolder(self.folder)
           runiov=self.run << 32
           obj=folder.findObject(runiov,0)
           payload=obj.payload()

           contents.update(payload_dict(payload))
           
        except Exception as e:
           self.msg.warning(f"Could not decode {self.folder} from {self.indb}\n{e}")
           return StatusCode.Failure  

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
           contents2.update(payload_dict(payload))
        except Exception:
           self.msg.warning(f"Could not decode {self.folder} from {self.patchdb}")
           return StatusCode.Failure  

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

        for k in contents.keys():
            if isinstance(contents[k]["payload"], cppyy.gbl.coral.Blob):
                spec.extend( k, cool.StorageType.Blob16M )
            elif isinstance(contents[k]["payload"], int):
                spec.extend( k, cool.StorageType.UInt32 )
            else:
                self.msg.error(f"CANNOT WORK OUT THE DATATYPE OF {k}: {contents[k]} FOR STORAGE")
                return StatusCode.Failure

        data = cool.Record( spec )

        hash_max = self.onlineID.channelHashMax()
        fsize = 4
        
        btype=getattr(ROOT,"coral::Blob")
        outblob = {}
        vVec = {}
        blobvals = [ k for k in contents.keys() if isinstance( contents[k]["payload"], cppyy.gbl.coral.Blob) ]
        blobvals2 = [ k for k in contents2.keys() if isinstance( contents2[k]["payload"], cppyy.gbl.coral.Blob) ]

        if len(blobvals) != len(blobvals2):
            self.msg.error("The same blob payloads are not present in the two dbs!\n(1) {blobvals}\n(2) {blobvals2}")
            return StatusCode.Failure
            
        for bv in blobvals:
            vecLength = contents[bv]["size"]
            blobsize = vecLength*hash_max*fsize
            outblob[bv] = btype()
            outblob[bv].resize(blobsize) 

            if blobsize != len(contents[bv]["payload"]):
                self.msg.error(f'Wrong size {len(contents[bv]["payload"])} of the input blob, should be {blobsize}')
                return StatusCode.Failure
            
            vVec[bv] = ROOT.std.vector('float')(vecLength*hash_max)

            blobdata = contents[bv]["payload"].read()

            for i in range(0,hash_max):
                if vecLength > 1:
                    for iv in range(0, vecLength):
                        vVec[bv][i*vecLength + iv] = float(struct.unpack('f',blobdata[(i*vecLength + iv)*fsize:((i*vecLength+iv)*fsize+fsize)])[0])   
                else:
                    vVec[bv][i] = float(struct.unpack('f',blobdata[i*fsize:(i*fsize+fsize)])[0])   

            if blobsize != len(contents2[bv]["payload"]):
                self.msg.error(f'Wrong size {len(contents2[bv]["payload"])} of the input blob, should be {blobsize}')
                return StatusCode.Failure

            # patch what is needed from the second blob
            blobdata2 = contents2[bv]["payload"].read()
            vecLength2 = contents2[bv]["size"]
            if vecLength != vecLength2:
                self.msg.error(f"Item {bv} has a different shape in the base and patch dbs! {vecLength} & {vecLength2}")
                return StatusCode.Failure


            for i in range(0,hash_max):
                doPatch = patchChannel(self.onlineID, i, self.patchdet, self.patchFT, self.patchChan)
                if doPatch:
                    if vecLength > 1:
                        for iv in range(0, vecLength):
                            vVec[bv][i*vecLength + iv] = float(struct.unpack('f',blobdata2[(i*vecLength + iv)*fsize:((i*vecLength+iv)*fsize+fsize)])[0])   
                    else:
                        vVec[bv][i] = float(struct.unpack('f', blobdata2[i*fsize:(i*fsize+fsize)])[0])   
              
        #now trick with buffer
        writable_buf = { k: array('f') for k in vVec.keys() }
        buff = {}
        for key in vVec.keys():
            for i in range(0,vVec[key].size()):
                writable_buf[key].append(vVec[key][i])
            #and use buffers to fill blobs
            buff[key] = writable_buf[key].tobytes()

            for j in range(0, len(buff[key])):
                outblob[key][j] = buff[key][j]

            data[key] = outblob[key]

        tofill = [ k for k in contents.keys() if k not in outblob.keys() ]

        for tf in tofill:
            if contents[tf]["payload"] != contents2[tf]["payload"]:
                self.msg.warning(f'Setting {tf} to {contents[tf]["payload"]}, but beware: the value is {contents2[tf]["payload"]} in the patching db')
            data[tf] = contents[tf]["payload"]

        #and record data   
        outfolder.storeObject(self.iovSince, self.iovUntil, data, cool.ChannelId(0))   

        #=== close the database
        outconn.closeDatabase()
        
        return StatusCode.Success




    
