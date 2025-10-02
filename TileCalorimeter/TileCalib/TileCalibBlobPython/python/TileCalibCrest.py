#!/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# TileCalibCrest.py
# Sanya Solodkov <Sanya.Solodkov@cern.ch>, 2025-02-04
# Laura Sargsyan <Laura.Sargsyan@cern.ch>, 2025-09-16
# Siarhei Harkusha <Siarhei.Harkusha@cern.ch>, 2025-05-19
################################################################
"""
Python helper module for managing CREST DB connections and TileCalibBlobs.
"""

import os, cppyy, base64, json, time, datetime

from PyCool import cool # noqa: F401
Blob = cppyy.gbl.coral.Blob

from pycrest.api.crest_api import CrestApi

from TileCalibBlobObjs.Classes import TileCalibUtils, TileCalibDrawerCmt, \
     TileCalibDrawerInt, TileCalibDrawerOfc, TileCalibDrawerBch, \
     TileCalibDrawerFlt, TileCalibType
from TileCalibBlobPython import TileCalibTools

#=== get a logger
from TileCalibBlobPython.TileCalibLogger import TileCalibLogger, getLogger
log = getLogger("TileCalibCrest")


#======================================================================
#===
#=== Global helper functions
#===
#======================================================================

#
#______________________________________________________________________
#=== useful constants
MINRUN      = 0
MINLBK      = 0
MAXRUN      = (1<<31)-1
MAXLBK      = (1<<32)-1
MAXRUNLUMI  = (MAXRUN<<32)+MAXLBK
# empty Tile channel for storing laser partition variation. DO NOT CHANGE.
LASPARTCHAN = 43


class TileBlobReaderCrest(TileCalibLogger):
    """
    TileCalibBlobReader is a helper class, managing the details of CREST interactions for
    the user of TileCalibBlobs.
    """

    #____________________________________________________________________
    def __init__(self, db, folder='', tag='', run=None, lumi=0, modmin=0, modmax=275, copyBlob=False):
        """
        Input:
        - db    : server connection string or file name
        - folder: full folder path
        - tag   : The folder tag, e.g. \"UPD4-24\" or full tag
        - run   : Run number (if known)
        - lumi  : Lumi block number
        - modmin: Minimal module (COOL channel number)
        - modmax: Maximal module (COOL channel number)
        - copyBlob: save payload from CREST (Default:False, True to copy payload to json file)
        """
          #=== initialize base class
        TileCalibLogger.__init__(self,"TileBlobReader")
        self.payload = {}
        self.__db = db
        self.__folder = folder
        self.__tag = tag
        self.__copyBlob = copyBlob

        self.__iovList = []
        self.__iov = (-1,0)
        self.__commentBlob = None
        self.__drawerBlob = [None]*276
        self.__comment = None
        self.__drawer = [None]*276
        self.__modmin = modmin
        self.__modmax = modmax+1

        #=== try to open db
        self.__remote = (("http://" in db) or ("https://" in db) or ("CREST" in db))
        if self.__remote:
            if 'http' not in self.__db:
                self.__db = os.getenv(db,os.getenv('CREST_HOST',os.getenv('CREST_SERVER_PATH','http://crest-j23.cern.ch:8080/api-v5.0')))
            self.log().info('Host %s' , (self.__db))
            self.__api_instance = CrestApi(host=self.__db)
            socks = os.getenv('CREST_SOCKS', 'False')
            if socks == 'True': self.__api_instance.socks()
            self.__tag = self.getFolderTag(folder,None,tag)
            if run is not None and lumi is not None:
                log.info("Initializing for run %d, lumiblock %d", run,lumi)
                self.__getIov((run,lumi),True)
        else:
            self.log().info('File %s' , (self.__db))
            self.__iovList.append(((MINRUN,MINLBK),(MAXRUN, MAXLBK)))
            self.__iov = self.__runlumi2iov(self.__iovList[-1])
            try:
                with open(self.__db, 'r') as the_file:
                    jdata = json.load(the_file)
                    for chan in range(self.__modmin,self.__modmax):
                        self.__create_drawer(jdata[str(chan)][0],chan)
                    self.__create_comment(jdata['1000'][0])
            except Exception as e:
                self.log().critical( e )
                raise

    #____________________________________________________________________
    def getFolderTag(self, folder, prefix, globalTag):
        if globalTag=='CURRENT' or globalTag=='UPD4' or globalTag=='':
            globalTag=TileCalibTools.getAliasFromFile('Current')
            log.info("Resolved CURRENT globalTag to \'%s\'", globalTag)
        elif globalTag=='CURRENTES' or globalTag=='UPD1':
            globalTag=TileCalibTools.getAliasFromFile('CurrentES')
            log.info("Resolved CURRENT ES globalTag to \'%s\'", globalTag)
        elif globalTag=='NEXT':
            globalTag=TileCalibTools.getAliasFromFile('Next')
            log.info("Resolved NEXT globalTag to \'%s\'", globalTag)
        elif globalTag=='NEXTES':
            globalTag=TileCalibTools.getAliasFromFile('NextES')
            log.info("Resolved NEXT ES globalTag to \'%s\'", globalTag)
        globalTag=globalTag.replace('*','')
        if prefix is None:
            prefix = ''
            for f in folder.split('/'):
                prefix+=f.capitalize()
        else:
            prefix=prefix.strip('-').split('-')[0]
        if prefix.startswith('Calo'):
            prefix='CALO'+prefix[4:]
        if 'UPD1' in globalTag or 'UPD4' in globalTag or 'COND' not in globalTag:
            if prefix != '':
                tag=prefix+'-'+globalTag
                self.log().info("Resolved localTag \'%s\' to folderTag \'%s\'", globalTag,tag)
            elif folder!='' and not (globalTag.startswith('Tile') or globalTag.startswith('CALO')):
                tag = TileCalibUtils.getFullTag(folder, globalTag)
                if tag.startswith('Calo'):
                    tag='CALO'+tag[4:]
                self.log().info("Resolved localTag \'%s\' to folderTag \'%s\'", globalTag,tag)
            else:
                tag=globalTag
                self.log().info("Use localTag \'%s\' as is", tag)
        else:
            tag=None
            tags=self.__api_instance.find_global_tag_map(globalTag)
            if tags['size']==0:
                raise Exception( "globalTag %s not found" % (globalTag) )
            else:
                for i in range(tags['size']):
                    t=tags['resources'][i]['tag_name']
                    l=tags['resources'][i]['label']
                    if (prefix!='' and t.startswith(prefix)) or l==folder:
                        tag=t
                        self.log().info("Resolved globalTag \'%s\' to folderTag \'%s\'", globalTag,tag)
                        #taginfo = self.__api_instance.find_tag(name=tag)
                        #print(taginfo)
        return tag

    #____________________________________________________________________
    def getIovs(self,since,until):
        run_lumi1=str((since[0]<<32)+since[1]+1)
        run_lumi2=str((until[0]<<32)+until[1]+1)
        MAXRUNLUMI1=str(MAXRUNLUMI+1)
        iovs1=self.__api_instance.select_iovs(self.__tag,"0",run_lumi1,sort='id.since:DESC,id.insertionTime:DESC',size=1,snapshot=0)
        iovs2=self.__api_instance.select_iovs(self.__tag,run_lumi2,MAXRUNLUMI1,sort='id.since:ASC,id.insertionTime:DESC',size=1,snapshot=0)
        since1=0 if iovs1['size']==0 else iovs1['resources'][0]['since']
        until1=MAXRUNLUMI if iovs2['size']==0 else iovs2['resources'][0]['since']
        iovs=self.__api_instance.select_iovs(self.__tag,str(since1),str(until1),sort='id.since:ASC,id.insertionTime:DESC',size=999999,snapshot=0)
        iovList=[]
        if iovs['size']==0:
            raise Exception( "IOV for tag %s IOV [%s,%s] - (%s,%s) not found" % (self.__tag,since[0],since[1],until[0],until[1]) )
        else:
            for i in range(iovs['size']):
                iov=iovs['resources'][i]
                since=int(iov['since'])
                runS=since>>32
                lumiS=since&0xFFFFFFFF
                iovList.append((runS,lumiS))
        return iovList

    #____________________________________________________________________
    def __getIov(self,runlumi,dbg=False):
        run_lumi1=str((runlumi[0]<<32)+runlumi[1]+1)
        MAXRUNLUMI1=str(MAXRUNLUMI+1)
        iovs1=self.__api_instance.select_iovs(self.__tag,"0",run_lumi1,sort='id.since:DESC,id.insertionTime:DESC',size=1,snapshot=0)
        iovs2=self.__api_instance.select_iovs(self.__tag,run_lumi1,MAXRUNLUMI1,sort='id.since:ASC,id.insertionTime:DESC',size=1,snapshot=0)
        if iovs1['size']==0:
            raise Exception( "IOV for tag %s run,lumi (%s,%s) not found" % (self.__tag,runlumi[0],runlumi[1]) )
        else:
            iov=iovs1['resources'][0]
            since=int(iov['since'])
            runS=since>>32
            lumiS=since&0xFFFFFFFF
            until=MAXRUNLUMI if iovs2['size']==0 else iovs2['resources'][0]['since']
            runU=until>>32
            lumiU=until&0xFFFFFFFF
            hash=iov['payload_hash']
            if dbg:
                #self.log().info('Run,Lumi (%d,%d)' , runlumi)
                self.log().info('IOV [%d,%d] - (%d,%d)' , runS,lumiS,runU,lumiU)
                self.log().info('Insertion time %s' , iov['insertion_time'])
                self.log().info('Hash %s' , hash)
            payload = self.__api_instance.get_payload(hash=hash).decode('utf-8')
            jdata=json.loads(payload)
            #with open("payload.json", 'w') as the_file:
            #    the_file.write(payload)
            #    the_file.write('\n')
            if self.__copyBlob:
                self.payload = jdata
                return
            self.__iovList.append(((runS,lumiS),(runU, lumiU)))
            self.__iov = self.__runlumi2iov(self.__iovList[-1])
            for chan in range(self.__modmin,self.__modmax):
                try:
                    blob=jdata[str(chan)][0]
                except Exception:
                    blob=None
                self.__create_drawer(blob,chan)
            try:
                blob=jdata['1000'][0]
            except Exception:
                blob=None
            self.__create_comment(blob)
        return

    #____________________________________________________________________
    def __runlumi2iov(self,runlumi):
        since = (runlumi[0][0]<<32) + runlumi[0][1]
        until = (runlumi[1][0]<<32) + runlumi[1][1]
        return (since,until)

    #____________________________________________________________________
    def __checkIov(self,runlumi):
        point = (runlumi[0]<<32) + runlumi[1]
        inrange = point>=self.__iov[0] and point<self.__iov[1]
        return inrange

    #____________________________________________________________________
    def __make_blob(self,string):
        b = Blob()
        b.write(string)
        b.seek(0)
        return b

    #____________________________________________________________________
    def __create_comment(self,b64string):
        if b64string is None or len(b64string)==0:
            self.__commentBlob = None
            self.__comment = None
        else:
            blob1 = base64.decodebytes(bytes(b64string,'ascii'))
            self.__commentBlob = self.__make_blob(blob1)
            self.__comment = TileCalibDrawerCmt.getInstance(self.__commentBlob)
        return

    #____________________________________________________________________
    def __create_drawer(self,b64string,chan):
        if b64string is None or len(b64string)==0:
            self.__drawerBlob[chan] = None
            self.__drawer[chan] = None
            return
        blob1 = base64.decodebytes(bytes(b64string,'ascii'))
        self.__drawerBlob[chan] = self.__make_blob(blob1)
        cmt = TileCalibDrawerCmt.getInstance(self.__drawerBlob[chan])
        typeName = TileCalibType.getClassName(cmt.getObjType())
        del cmt
        #=== create calibDrawer depending on type
        if   typeName=='TileCalibDrawerFlt':
            self.__drawer[chan] = TileCalibDrawerFlt.getInstance(self.__drawerBlob[chan])
            self.log().debug( "typeName = Flt " )
        elif typeName=='TileCalibDrawerInt':
            self.__drawer[chan] = TileCalibDrawerInt.getInstance(self.__drawerBlob[chan])
            self.log().debug( "typeName = Int " )
        elif typeName=='TileCalibDrawerBch':
            self.__drawer[chan] = TileCalibDrawerBch.getInstance(self.__drawerBlob[chan])
            self.log().debug( "typeName = Bch " )
        elif typeName=='TileCalibDrawerOfc':
            self.__drawer[chan] = TileCalibDrawerOfc.getInstance(self.__drawerBlob[chan])
            self.log().debug( "typeName = Ofc " )
        else:
            raise Exception( "Invalid blob type requested: %s" % typeName )
        return

    #____________________________________________________________________
    def getDrawer(self,ros, mod, runlumi=None, dbg=False, useDefault=True):

        if self.__remote and runlumi is not None and not self.__checkIov(runlumi):
            self.__getIov(runlumi,dbg)

        if ros<0:
            chanNum = mod
        else:
            chanNum = TileCalibUtils.getDrawerIdx(ros,mod)

        if (chanNum>=0 and chanNum<len(self.__drawer)):
            drawer=self.__drawer[chanNum]
            if not useDefault and drawer is None:
                return 0
            while drawer is None:
                #=== no default at all?
                if ros==0 and drawer==0:
                    raise Exception('No default available')
                #=== follow default policy
                ros,drawer = self.getDefault(ros,drawer)
                chanNum = TileCalibUtils.getDrawerIdx(ros,drawer)
                drawer=self.__drawer[chanNum]
            return drawer
        elif (chanNum == 1000):
            return self.__comment
        else:
            raise Exception( "Invalid drawer requested: %s %s" % (ros,mod) )

    #____________________________________________________________________
    def getComment(self,runlumi=None):

        if self.__remote and runlumi is not None and not self.__checkIov(runlumi):
            self.__getIov(runlumi)
        if self.__comment is not None:
            return self.__comment.getFullComment()
        else:
            return "<no comment found>"

    #____________________________________________________________________
    def getDefault(self, ros, drawer):
        """
        Returns a default drawer number (among first 20 COOL channels) for any drawer in any partition
        """
        if ros==0:
            if drawer<=4 or drawer==12 or drawer>=20:
                drawer1=0
            elif drawer<12:
                drawer1=4
            else:
                drawer1=12
        elif ros==1 or ros==2:
            drawer1=4
        elif ros==3:
            OffsetEBA = [ 0, 0, 0, 0, 0, 0, 3, 2, #// Merged E+1: EBA07; Outer MBTS: EBA08
                          0, 0, 0, 0, 7, 6, 5, 7, #// D+4: EBA13, EBA16; Special D+4: EBA14; Special D+40: EBA15
                          7, 6, 6, 7, 0, 0, 0, 2, #// D+4: EBA17, EBA20; Special D+4: EBA18, EBA19; Outer MBTS: EBA24
                          3, 0, 0, 0, 0, 0, 0, 0, #// Merged E+1:  EBA25
                          0, 0, 0, 0, 0, 0, 1, 1, #// Inner MBTS + special C+10: EBA39, EBA40
                          1, 1, 2, 3, 0, 0, 0, 0, #// Inner MBTS + special C+10: EBA41, EBA42; Outer MBTS: EBA43; Merged E+1: EBA44
                          0, 0, 0, 0, 3, 2, 1, 1, #// Merged E+1: EBA53; Outer MBTS: EBA54; Inner MBTS + special C+10: EBA55, EBA56
                          1, 1, 0, 0, 0, 0, 0, 0] #// Inner MBTS + special C+10: EBA57, EBA58
            drawer1 = 12 + OffsetEBA[drawer]
        elif ros==4:
            OffsetEBC = [ 0, 0, 0, 0, 0, 0, 3, 2, #// Merged E-1: EBC07; Outer MBTS: EBC08
                          0, 0, 0, 0, 7, 6, 6, 7, # // D-4: EBC13, EBC16; Special D-4: EBC14, EBC15;
                          7, 5, 6, 7, 0, 0, 0, 2, #// D-4: EBC17, EBC20; Special D-40 EBC18; Special D-4: EBC19; Outer MBTS: EBC24
                          3, 0, 0, 3, 4, 0, 3, 4, #// Merged E-1:  EBC25, EBC28, EBC31; E-4': EBC29, EBC32
                          0, 4, 3, 0, 4, 3, 1, 1, #// E-4': EBC34, EBC37; Merged E-1: EBC35, EBC38; Inner MBTS + special C-10: EBC39, EBC40
                          1, 1, 2, 3, 0, 0, 0, 0, #// Inner MBTS + special C-10: EBC41, EBC42; Outer MBTS: EBC43; Merged E-1: EBC44
                          0, 0, 0, 0, 3, 2, 1, 1, #// Merged E-1: EBC53; Outer MBTS: EBC54; Inner MBTS + special C-10: EBC55, EBC56
                          1, 1, 0, 0, 0, 0, 0, 0] #// Inner MBTS + special C-10: EBC57, EBC58
            drawer1 = 12 + OffsetEBC[drawer]
        else:
            drawer1=0

        return (0,drawer1)

class TileBlobWriterCrest(TileCalibLogger):
    """
    TileBlobWriterCrest is a helper class, managing the details of
    CREST interactions for the user of TileCalibBlobs.
    """

    #____________________________________________________________________
    def __init__(self, db, folderPath, calibDrawerType, isMultiVersionFolder=True):
        """
        Input:
        - db        : db should be a database connection
        - folderPath: full folder path to create or update
        """

        #=== initialize base class
        TileCalibLogger.__init__(self, "TileBlobWriter")

        #=== store db
        self.__db = db
        self.__folderPath = folderPath

        #=== create default vectors based on calibDrawerType
        self.__calibDrawerType = calibDrawerType
        if calibDrawerType in ['TileCalibDrawerFlt', 'Flt']:
            self.__TileCalibDrawer = TileCalibDrawerFlt
            self.__defVec = cppyy.gbl.std.vector('std::vector<float>')()
        elif calibDrawerType in ['TileCalibDrawerBch', 'Bch']:
            self.__TileCalibDrawer = TileCalibDrawerBch
            self.__defVec = cppyy.gbl.std.vector('std::vector<unsigned int>')()
        elif calibDrawerType in ['TileCalibDrawerInt', 'Int']:
            self.__TileCalibDrawer = TileCalibDrawerInt
            self.__defVec = cppyy.gbl.std.vector('std::vector<unsigned int>')()
        else:
            raise Exception("Unknown calibDrawerType: %s" % calibDrawerType)

        # Always all drawers should be written
        self.__drawerBlob = {drawerIdx:Blob() for drawerIdx in range(0, TileCalibUtils.max_draweridx())}
        self.__drawer = {}
    #____________________________________________________________________
    def register(self, since=(MINRUN,MINLBK), tag=""):
        """
        Registers the folder in the database.
        - since: lower limit of IOV
        - tag  : The tag to write to

        The interpretation of the 'since' inputs depends on their type:
        - tuple(int,int) : run and lbk number
        """

        jdata = {}
        for drawerIdx,blob in self.__drawerBlob.items():
            b64string = str(base64.b64encode(blob.read()), 'ascii')
            jdata[drawerIdx] = [b64string]

        (sinceRun, sinceLumi) = since

        if not self.__db or (self.__db and self.__db.endswith('.json')):
            # Writting into the json file
            fullTag = tag
            if self.__folderPath and not (tag.startswith('Tile') or tag.startswith('Calo')):
                fullTag = TileCalibUtils.getFullTag(self.__folderPath, tag)
            fileName = f"{fullTag}.{sinceRun}.{sinceLumi}.json"
            if self.__db:
                fileName = f'{self.__db[:-5]}.{fileName}'

            with open(fileName, 'w') as the_file:
                json.dump(jdata, the_file)
                the_file.write('\n')

        #=== print info
        self.log().info( 'Writting tag "%s"', fullTag)
        self.log().info( '... since             : [%s,%s]' , sinceRun, sinceLumi)
        self.log().info( '... with comment field: "%s"', self.getComment())
        self.log().info( '... into file         : %s' , fileName)

    #____________________________________________________________________
    def setComment(self, author, comment=None):
        """
        Sets a general comment in the comment channel.
        """
        drawerIdx = TileCalibUtils.getCommentChannel()
        commentBlob = self.__drawer.get(drawerIdx, None)
        if not commentBlob:
            commentBlob = Blob()
            self.__drawerBlob[drawerIdx] = commentBlob

        if isinstance(author, tuple) and len(author) == 3:
            tm = time.mktime(datetime.datetime.strptime(author[2], "%a %b %d %H:%M:%S %Y").timetuple())
            self.__drawer[drawerIdx] = TileCalibDrawerCmt.getInstance(commentBlob, author[0], author[1], int(tm))
        else:
            self.__drawer[drawerIdx] = TileCalibDrawerCmt.getInstance(commentBlob, author, comment)

    #____________________________________________________________________
    def getComment(self, split=False):
        """
        Returns the general comment (default if none is set)
        """
        drawerIdx = TileCalibUtils.getCommentChannel()
        comment = self.__drawer.get(drawerIdx, None)
        if comment:
            if split:
                return (comment.getAuthor(), self.__comment.getComment(), self.__comment.getDate())
            else:
                return comment.getFullComment()
        else:
            return "<No general comment!>"

    #____________________________________________________________________
    def getDrawer(self, ros, drawer, calibDrawerTemplate=None):
        """
        Returns a TileCalibDrawer object of requested type
        for the given ROS and drawer.
        """

        try:
            drawerIdx = TileCalibUtils.getDrawerIdx(ros, drawer)
            calibDrawer = self.__drawer.get(drawerIdx, None)

            if not calibDrawer:
                calibDrawer = self.__TileCalibDrawer.getInstance(self.__drawerBlob[drawerIdx], self.__defVec,0,0)
                self.__drawer[drawerIdx] = calibDrawer

            #=== clone if requested
            if calibDrawerTemplate:
                calibDrawer.clone(calibDrawerTemplate)

            return calibDrawer

        except Exception as e:
            self.log().critical( e )
            return None

    #____________________________________________________________________
    def zeroBlob(self, ros, drawer):
        """
        Resets blob size to zero
        """
        try:
            drawerIdx = TileCalibUtils.getDrawerIdx(ros, drawer)
            blob = self.__drawerBlob[drawerIdx]
            blob.resize(0)
        except Exception as e:
            self.log().critical( e )
            return None
