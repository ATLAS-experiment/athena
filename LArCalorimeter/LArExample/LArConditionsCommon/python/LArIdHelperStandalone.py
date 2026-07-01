# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from PyCool import cool
from CoolConvUtilities.AtlCoolLib import indirectOpen
import ROOT
import os,sys
from AthenaCommon.Logging import logging

def IdToNum(ident):
    return ident.get_identifier32().get_compact()

def NumToId(idnum):
    return ROOT.Identifier(ROOT.Identifier32(idnum))


class LArIdHelper:
    def __init__(self):
        self._msg=logging.getLogger("LArIdHelper")
        #Set up LArOnlineID helper class in standalone mode (from xml file)
        from ROOT import IdDictParser
        parser=IdDictParser()
        #Get xml files:
        xmlpath=None
        for dd in os.getenv('XMLPATH').split(os.pathsep):
            d=dd+"/IdDictParser/ATLAS_IDS.xml"
            if os.access(d,os.R_OK):
                xmlpath=dd
                break
        if not xmlpath:
            self._msg.error("unable to locate identifier dictionaries in $XMLPATH")
            sys.exit(-1)
               
        parser.register_external_entity("LArCalorimeter",xmlpath+"/IdDictParser/IdDictLArCalorimeter_DC3-05-Comm-01.xml")
        parser.register_external_entity("Calorimeter",xmlpath+"/IdDictParser/IdDictCalorimeter_L1Onl.xml")
        parser.register_external_entity("TileCalorimeter",xmlpath+"/IdDictParser/IdDictTileCalorimeter.xml")


        idd = parser.parse(xmlpath+"/IdDictParser/ATLAS_IDS.xml")
        from ROOT import LArOnlineID
        self._larOnlHelper=LArOnlineID()
        stat=self._larOnlHelper.initialize_from_dictionary(idd)
        if stat==1:
            self._msg.error("failed to init LArOnlineID")
            sys.exit(-1)


        from ROOT import LArEM_ID, LArHEC_ID, LArFCAL_ID, LArMiniFCAL_ID, TileID

        self._allHelpers=[]
        for subHelper in (LArEM_ID, LArHEC_ID, LArFCAL_ID, LArMiniFCAL_ID, TileID):
            helper=subHelper()
            helper.set_do_neighbours(False)
            stat=helper.initialize_from_dictionary(idd)
            if stat==1:
                self._msg.error("failed to init" + str(subHelper))
            else:
                self._allHelpers.append(helper)
            pass
        from ROOT import CaloCell_ID
        self._caloHelper=CaloCell_ID(*self._allHelpers)
        self._caloHelper.initialize_from_dictionary(idd)
        #Note: It's important that all sub-helpers are part of this class. The CaloCell_ID keeps pointers to the individual sub-helpers, 
        #but python's garbage collector doesn't know about this relationship. So it may delete the sub-helpers at any time.

        self._cabling=None
        return


    def larOnlHelper(self):
        return self._larOnlHelper

    def caloHelper(self):
        return self._caloHelper

    def getCabling(self):
        if self._cabling: return self._cabling

        lardb=indirectOpen("COOLONL_LAR/CONDBR2")
        f=lardb.getFolder("/LAR/Identifier/OnOffIdMap") 
        ptr=f.findObject(cool.ValidityKeyMax-1,cool.ChannelId(0),"LARIdentifierOnOffIdMap-RUN2-001")
        payload=ptr.payload()["OnlineHashToOfflineId"]
        on2off=payload.read()
        nChans=int(payload.size()/4)
        
        self._cabling=ROOT.LArOnOffIdMapping(self._larOnlHelper,self._caloHelper)
        onlHash2OflIdVec=self._cabling.getOnlHash2OflId()
        oflHash2OnlIdVec=self._cabling.getOflHash2OnId()


        i=0
        nConnected=0
        iChan=0
        while (iChan<nChans):
            idnum=int.from_bytes(on2off[i:i+4],'little')
            #self._msg.debug("From Blob[%i]=0x%x" % (iChan,idnum))
            oflId=ROOT.Identifier(ROOT.Identifier32(idnum))
            if oflId.is_valid():
                oflHash=self._caloHelper.calo_cell_hash(oflId)
                onlHash=ROOT.IdentifierHash(iChan)
                if oflHash.value()>=oflHash2OnlIdVec.size():
                    self._msg.error("invalid offline hash %i", oflHash.value())
                    return
              
                if onlHash.value()>=onlHash2OflIdVec.size():
                    self._msg.error("invalid offline hash %i", onlHash.value())
                    return
                nConnected+=1
                onlId=self._larOnlHelper.channel_Id(onlHash)
                onlHash2OflIdVec[iChan]=oflId
                oflHash2OnlIdVec[oflHash.value()]=onlId
                pass
            else: 
                #print ("idnum %i give invalid offline id 0x%x",(idnum,oflId.get_identifier32().get_compact()))
                pass
            i+=4
            iChan+=1
            pass

        self._msg.info("Found identifier mapping for %i connected channels", nConnected)
        
        return self._cabling


if __name__=="__main__":

    #usage example & self-test
    c=LArIdHelper()
    cc=c.getCabling()

    onlId1=NumToId(978347776)
    oflId1=IdToNum(cc.cnvToIdentifier(onlId1))
    assert(oflId1==740295168)

    
    #cell_id           (const int subCalo, 
    #                   const int barec_or_posneg, 
    #                   const int sampling_or_fcalmodule,
    #                   const int region_or_dummy,
    #                   const int eta,    
    #                   const int phi ) const;

    oflId2=c.caloHelper().cell_id(0, #LArEM
                                  -3, #EMECIW-C
                                  1, #layer 0
                                  0, #region 0
                                  1, #ieta
                                  0, #iphi
                                  )
    oflId2Num=IdToNum(oflId2)
    print (oflId2Num)
    assert(oflId2Num==740295168)
    
    onlId2=cc.createSignalChannelID(oflId2)
    assert(c.larOnlHelper().channel(onlId2)==103) #Channel on FrontEnd board
    assert(c.larOnlHelper().feedthrough(onlId2)==10) #Feed-though number
    
    onlId2Num=IdToNum(onlId2)
    assert(onlId2Num==978347776)
        
