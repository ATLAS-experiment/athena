#!/bin/env python

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

import ROOT
import sys
import time

from array import array
from PyCool import cool
from optparse import OptionParser
from math import fabs

def setAxisSizes(histos, tsize=0.05, lsize=0.05, zsize=0.05):
    for h in histos:
        h.GetXaxis().SetTitleSize(tsize)
        h.GetXaxis().SetLabelSize(lsize)
        h.GetYaxis().SetTitleSize(tsize)
        h.GetYaxis().SetLabelSize(lsize)
 
        if h.GetZaxis():
            h.GetZaxis().SetTitleSize(zsize)
            h.GetZaxis().SetLabelSize(zsize)
 
def setAxisTitles(histos, xtit, ytit, ztit="foo", xoff=0, yoff=0, zoff=0):
    for h in histos:
        h.GetXaxis().SetTitle(xtit)
        h.GetYaxis().SetTitle(ytit)
        if ztit != "foo":
            h.GetZaxis().SetTitle(ztit)
            
        if xoff!=0: h.GetXaxis().SetTitleOffset(xoff)
        if yoff!=0: h.GetYaxis().SetTitleOffset(yoff)
        if zoff!=0: h.GetZaxis().SetTitleOffset(zoff)


class L1CaloMap:
    Counter = 0
    def __init__(self,title,XaxisTitle="",YaxisTitle=""):
        
        self.nxbins = 66
        self.xbins = array('d',[-49.5,-44.5,-40.50,-36.5,-32.5,-31.5,-29.5,
                                -27.5,-25.5,-24.5,-23.5,-22.5,-21.5,-20.5,-19.5,
                                -18.5,-17.5,-16.5,-15.5,-14.5,-13.5,-12.5,-11.5,
                                -10.5,-9.5,-8.5,-7.5,-6.5,-5.5,-4.5,-3.5,
                                -2.5,-1.5,-0.5,0.5,1.5,2.5,3.5,4.5,5.5,6.5,
                                7.5,8.5,9.5,10.5,11.5,12.5,13.5,14.5,15.5,16.5,
                                17.5,18.5,19.5,20.5,21.5,22.5,23.5,24.5,26.5,28.5,
                                30.5,31.5,35.5,39.5,43.5,47.5])

        # Maintain a counter of instances to assign unique names,
        L1CaloMap.Counter += 1        
        hname = "GainTTsMap_%d" % L1CaloMap.Counter
        htitle = title
        self.h_1 = ROOT.TH2F(hname,htitle,self.nxbins,self.xbins,64,0.,64)
        setAxisTitles( [self.h_1], XaxisTitle, YaxisTitle )
        setAxisSizes( [self.h_1], 0.048, 0.040, 0.032)

    def Draw(self):
        self.h_1.SetStats(0)
        self.h_1.DrawCopy("colz")
        ROOT.gPad.RedrawAxis()

    def Fill(self,eta,phi,gain=1):

        if eta >= 32 or eta < -32:
            self.h_1.Fill(eta,phi+3.5,gain) 
            self.h_1.Fill(eta,phi+2.5,gain) 
            self.h_1.Fill(eta,phi+1.5,gain) 
            self.h_1.Fill(eta,phi+0.5,gain) 
        elif eta >= 25 or eta < -25:
            self.h_1.Fill(eta,phi+1.5,gain) 
            self.h_1.Fill(eta,phi+0.5,gain) 
        else:
            self.h_1.Fill(eta,phi+0.5,gain) 

    def SetMinimum(self,minimum):
        self.h_1.SetMinimum(minimum)

    def SetMaximum(self,maximum):
        self.h_1.SetMaximum(maximum)

class L1CaloGeometryConvertor:

    def __init__(self):
        self.coolIdPath=ROOT.PathResolver.find_calib_file("TrigT1Calo/COOLIdDump_v1.txt")
        input = open(self.coolIdPath)
        self.list_of_channels_em={}
        self.list_of_channels_had={}

        for line in input.readlines():
            parts = line.split(' ')
            emCool = parts[4].rstrip()
            hadCool = parts[5].rstrip()
            self.list_of_channels_em[(parts[0],parts[1])]  = '0x'+emCool
            self.list_of_channels_had[(parts[0],parts[1])] = '0x'+hadCool
    
        input.close()

    def LoadReceiverPPMMap(self):
    
        self.receiver_to_ppm_map={}
        self.UNIX2COOL = 1000000000

        # get database service and open database
        dbSvc = cool.DatabaseSvcFactory.databaseService()
        dbString = 'oracle://ATLAS_COOLPROD;schema=ATLAS_COOLONL_TRIGGER;dbname=CONDBR2'
        try:
            db = dbSvc.openDatabase(dbString, False)
        except Exception as e:
            print ('Error: Problem opening database', e)
            sys.exit(1)

        folder_name = "/TRIGGER/Receivers/RxPpmIdMap"
        folder=db.getFolder(folder_name)
      
        startUtime = int(time.time())
        endUtime = int(time.time())
        startValKey = startUtime * self.UNIX2COOL
        endValKey = endUtime * self.UNIX2COOL
        chsel = cool.ChannelSelection(0,sys.maxsize)

        try:
            itr=folder.browseObjects(startValKey, endValKey, chsel)
        except Exception as e:
            print (e)
            sys.exit(1)

        for row in itr:
            ReceiverId = hex(int(row.channelId()))
            payload = row.payload()
            PPMId = hex(int(payload['ppmid']))
            self.receiver_to_ppm_map[ReceiverId]= PPMId
  
        # close database
        db.closeDatabase()

    def getPPMfromReceiver(self,ReceiverId):
        if ReceiverId in self.receiver_to_ppm_map:
            return self.receiver_to_ppm_map[ReceiverId]
        else:
            return None

    def getReceiverfromPPM(self,PPMId,strategy_string=None):
       
        ReceiverChannels = [item[0] for item in self.receiver_to_ppm_map.items() if item[1]==PPMId]      

        if strategy_string is None:
            print (" Warning! in getReceiverfromPPM no runtype given, using default!")
            return ReceiverChannels[0]

        if self.isPPMFCAL(PPMId) and self.isCoolHad(PPMId):     # pick correct FCAL23 channel

            if strategy_string == "GainOneOvEmbFcalHighEta":
                for channel in ReceiverChannels:
                    if self.getFCAL23RecEta(channel) == 'HighEta':
                        return channel                    
            if strategy_string == "GainOneOvEmecFcalLowEta":
                for channel in ReceiverChannels:
                    if self.getFCAL23RecEta(channel) == 'LowEta':
                        return channel

        elif self.isPPMOverlap(PPMId):
            if strategy_string == "GainOneOvEmbFcalHighEta":
                for channel in ReceiverChannels:
                    if self.getOverlapLayer(channel) == 'EMB':
                        return channel
            if strategy_string == "GainOneOvEmecFcalLowEta":
                for channel in ReceiverChannels:
                    if self.getOverlapLayer(channel) == 'EMEC':
                        return channel

        else:
            return ReceiverChannels[0]

    def getCoolEm(self,i_eta,i_phi):
        if (str(i_eta),str(i_phi)) in self.list_of_channels_em:
            cool = self.list_of_channels_em[(str(i_eta),str(i_phi))]
            cool.rstrip()
            cool.lstrip()
            return (cool)
        else:
            return ('')     
     
    def getCoolHad(self,i_eta,i_phi):
        if (str(i_eta),str(i_phi)) in self.list_of_channels_had:
            cool = self.list_of_channels_had[(str(i_eta),str(i_phi))]
            cool.rstrip()
            cool.lstrip()
            return (cool)
        else:
            return ('')         
     
    def isCoolEm(self,CoolId):
        return (CoolId in self.list_of_channels_em.values())

    def isCoolHad(self,CoolId):
        return (CoolId in self.list_of_channels_had.values())

    def getEtaBin(self,CoolId):
        if self.isCoolEm(CoolId):
            channel = [item[0] for item in self.list_of_channels_em.items() if item[1]==CoolId]
            return int(channel[0][0])
        elif self.isCoolHad(CoolId):
            channel = [item[0] for item in self.list_of_channels_had.items() if item[1]==CoolId]
            return int(channel[0][0])
        else:
            return -1

    def getPhiBin(self,CoolId):
        if self.isCoolEm(CoolId):
            channel = [item[0] for item in self.list_of_channels_em.items() if item[1]==CoolId]
            return int(channel[0][1])
        elif self.isCoolHad(CoolId):
            channel = [item[0] for item in self.list_of_channels_had.items() if item[1]==CoolId]
            return int(channel[0][1])
        else:
            return -1

    def getMissingReceiverChannels(self, channel_list):
        missing_channels= [channel for channel in self.receiver_to_ppm_map.keys() if channel not in channel_list]
        return missing_channels

    def getReceiverCMCP(self,ReceiverId):
        recI=int(ReceiverId,16)

        crate = recI/1024
        recI = recI - crate*1024

        module = recI/64
        recI = recI - module*64

        conn = recI/16
        recI = recI - conn*16

        pair = recI

        return [crate,module,conn,pair]
     
    def isPPMFCAL(self,CoolId): 
        eta_bin = self.getEtaBin(CoolId)  
        if eta_bin >= 32 or eta_bin <= -36:
            return True
        else:
            return False

    def isPPMOverlap(self,CoolId):
        eta_bin = self.getEtaBin(CoolId)  
        if self.isCoolEm(CoolId) is True and (eta_bin == 14 or eta_bin == -15):
            return True
        else:
            return False

    def getOverlapLayer(self,RecCoolId):
        ppm_id = self.getPPMfromReceiver(RecCoolId)

        if not self.isPPMOverlap(ppm_id):
            return None

        cabling = self.getReceiverCMCP(RecCoolId)
        if cabling[0] < 2:                 # unconnected channel has barrel crate nr.
            return 'Unconnected'
        elif cabling[2] == 0:
            return 'EMEC'
        elif cabling[2] == 2:
            return 'EMB'
        else:
            print ("Error in GetOverlapLayer, can't determine layer!")
            return None        

    def getFCAL23RecEta(self,RecCoolId):
        ppm_id = self.getPPMfromReceiver(RecCoolId)

        if (not self.isPPMFCAL(ppm_id)) or (not self.isCoolHad(ppm_id)):
            return None
        eta_bin = self.getEtaBin(ppm_id)  

        RecCoolInt = int(RecCoolId,16)
        if RecCoolInt%2 == 1:
            isRecOdd = True
        else:
            isRecOdd = False

        if eta_bin>0:
            if isRecOdd:
                return 'LowEta'
            else:
                return 'HighEta'
        else:
            if isRecOdd:
                return 'HighEta'
            else:
                return 'LowEta' 
        

class GainReader:

    def __init__(self):

        self.measured_gains={}
        self.reference_gains={}
        self.measured_chi2={}
        self.measured_offset={}
        self.UNIX2COOL = 1000000000

        self.run_nr=None
        self.strategy=None

    def LoadGainsXml(self,name):
    
        input_file = open(name)

        for line in input_file.readlines():
            parts = line.split(' ')
            if parts[0] == '<Channel':
                list_cool=parts[1].split('\'')
                cool_id=list_cool[1]

                list_gain=parts[2].split('\'')
                gain=list_gain[1]
                self.measured_gains[cool_id]=gain

                list_offset=parts[3].split('\'')
                offset=list_offset[1]
                self.measured_offset[cool_id]=offset

                list_chi2=parts[4].split('\'')
                chi2=list_chi2[1]
                self.measured_chi2[cool_id]=chi2

        input_file.close()

    def LoadReferenceXml(self,name): 

        input_gains_reference = open(name)

        for line in input_gains_reference.readlines():
            parts = line.split(' ')
            if parts[0] == '<Channel':
                list_cool=parts[1].split('\'')
                cool_id=list_cool[1]
            
                list_gain=parts[2].split('\'')
                gain=list_gain[1]
                self.reference_gains[cool_id]=gain


    def LoadGainsSqlite(self,name):

        # get database service and open database
        dbSvc = cool.DatabaseSvcFactory.databaseService()

        dbString='sqlite://;schema='+name+';dbname=L1CALO'
        try:
            db = dbSvc.openDatabase(dbString, False)       
        except Exception as e:
            print ('Error: Problem opening database', e)
            sys.exit(1)

        folder_name = '/TRIGGER/L1Calo/V1/Results/EnergyScanResults'
        folder=db.getFolder(folder_name)

        startUtime = int(time.time())
        endUtime = int(time.time())
        startValKey = startUtime * self.UNIX2COOL
        endValKey = endUtime * self.UNIX2COOL
        chsel = cool.ChannelSelection(0,sys.maxsize)

        try:
            itr=folder.browseObjects(startValKey, endValKey, chsel)
        except Exception as e:
            print (e)
            sys.exit(1)

        for row in itr:
            CoolId = hex(int(row.channelId()))
            payload = row.payload()
            self.measured_gains[CoolId]  = payload['Slope']
            self.measured_chi2[CoolId]   = payload['Chi2']
            self.measured_offset[CoolId] = payload['Offset']

        folder_gen_name = '/TRIGGER/L1Calo/V1/Results/EnergyScanRunInfo'
        folder_gen=db.getFolder(folder_gen_name)

        try:
            itr=folder_gen.browseObjects(startValKey, endValKey, chsel)
            for row in itr:
                payload = row.payload()
                self.run_nr   = payload['RunNumber']
                self.strategy = payload['GainStrategy']
                if (self.strategy == ''): self.strategy='NA' 
            print ( ("Run nr. = %d, Strategy = %s") % (self.run_nr, self.strategy) )

        except Exception:                              # Doesn't seem to catch C++ exceptions :-(
            print ("Warning, in LoadGainsSqlite can't get runtype info! Hope this is not serious!")

        # close database
        db.closeDatabase()

    def LoadReferenceSqlite(self,name):

        # get database service and open database
        dbSvc = cool.DatabaseSvcFactory.databaseService()

        dbString='sqlite://;schema='+name+';dbname=L1CALO'
        try:
            db = dbSvc.openDatabase(dbString, False)        
        except Exception as e:
            print ('Error: Problem opening database', e)
            sys.exit(1)

        folder_name = '/TRIGGER/L1Calo/V1/Results/EnergyScanResults'
        folder=db.getFolder(folder_name)
       
        startUtime = int(time.time())
        endUtime = int(time.time())
        startValKey = startUtime * self.UNIX2COOL
        endValKey = endUtime * self.UNIX2COOL
        chsel = cool.ChannelSelection(0,sys.maxsize)

        try:
            itr=folder.browseObjects(startValKey, endValKey, chsel)
        except Exception as e:
            print (e)
            sys.exit(1)

        for row in itr:
            CoolId = hex(int(row.channelId()))
            payload = row.payload()
            self.reference_gains[CoolId]=payload['Slope']
  
        # close database
        db.closeDatabase()


    def LoadReferenceOracle(self,mapping_tool):

        # get database service and open database
        dbSvc = cool.DatabaseSvcFactory.databaseService()

        dbString = 'oracle://ATLAS_COOLPROD;schema=ATLAS_COOLONL_TRIGGER;dbname=CONDBR2'
        try:
            db = dbSvc.openDatabase(dbString, False)        
        except Exception as e:
            print ('Error: Problem opening database', e)
            sys.exit(1)

        folder_name = "/TRIGGER/Receivers/Factors/CalibGains"
        folder=db.getFolder(folder_name)
       
        startUtime = int(time.time())
        endUtime = int(time.time())
        startValKey = startUtime * self.UNIX2COOL
        endValKey = endUtime * self.UNIX2COOL
        chsel = cool.ChannelSelection(0,sys.maxsize)

        try:
            itr=folder.browseObjects(startValKey, endValKey, chsel)
        except Exception as e:
            print (e)
            sys.exit(1)

        for row in itr:
            ReceiverId = hex(int(row.channelId()))
            PPMId = mapping_tool.getPPMfromReceiver(ReceiverId)
            payload = row.payload()
            gain = payload['factor']

            if PPMId is not None:
                if self.strategy is None:                 #run type not known
                    self.reference_gains[PPMId]=gain
                else:
                    if mapping_tool.getReceiverfromPPM(PPMId,self.strategy) == ReceiverId:  # correct receiver?
                        #print ("Using receiver nr.", ReceiverId, "for PPM nr.",PPMId)
                        self.reference_gains[PPMId]=gain
                    #else:
                    #    print ("Skipping receiver nr.", ReceiverId, "for PPM nr.",PPMId)
   
        # close database
        db.closeDatabase()

    def getGain(self,coolId):
        if (coolId in self.measured_gains):
            return float(self.measured_gains[coolId])
        else:
            return ''
 
    def getChi2(self,coolId):
        if (coolId in self.measured_chi2):
            return float(self.measured_chi2[coolId])
        else:
            return ''

    def getOffset(self,coolId):
        if (coolId in self.measured_offset):
            return float(self.measured_offset[coolId])
        else:
            return ''

    def getReferenceGain(self,coolId):
        if (coolId in self.reference_gains):
            return float(self.reference_gains[coolId])
        else:
            return ''  

    def passesSelection(self,coolId):
        if ((coolId in self.measured_gains) and 
            (self.getGain(coolId) > 0.5 and self.getGain(coolId)<1.6) and
            #(self.getGain(coolId) > 0.5 and self.getGain(coolId)<2.1) and
            #(self.getOffset(coolId) > -2 and self.getOffset(coolId) < 2)):
            (self.getOffset(coolId) > -10 and self.getOffset(coolId) < 10)):
            return True
        else:
            return False


class EmPartitionPlots:
    Counter = 0  
    def __init__(self,name,nbins=40,minimum=0.,maximum=2.,XaxisTitle="",YaxisTitle=""):

        self.nPartitions=5
        self.ext  = ["all","00_15","15_25","25_32","32_50"]
        self.name = ["all","EMB","EMEC outer","EMEC Inner","FCAL 1"]

        # Maintain a counter of instances to assign unique histogram names
        EmPartitionPlots.Counter += 1
        self.his_partitions  = []

        for i_em_partition in range(0,self.nPartitions):
            hname = ("GainTTEm_%d"%EmPartitionPlots.Counter) + self.ext[i_em_partition]
            htitle = name+" for "+self.name[i_em_partition]       
            self.his_partitions.append(ROOT.TH1F(hname,htitle,nbins,minimum,maximum))

        setAxisTitles( self.his_partitions, XaxisTitle, YaxisTitle )
        setAxisSizes( self.his_partitions, 0.05, 0.04 )

    def get_partition_number(self,eta_bin):
        indem = -1
        if ( -9 <= eta_bin and eta_bin <= 8):                      
            indem = 1
        elif ((eta_bin>8 and eta_bin<=14) or (eta_bin>=-15 and eta_bin<-9)):  
            #elif ((eta_bin>8 and eta_bin<14)  or (eta_bin>-15 and eta_bin<-9)):     # cut out overlap
            indem = 1
        elif ((eta_bin>14 and eta_bin<=24) or (eta_bin>=-25 and eta_bin<-15)):
            indem = 2
        elif ((eta_bin>24 and eta_bin<=31) or (eta_bin>=-32 and eta_bin<-25)): 
            indem = 3
        elif ((eta_bin>31)          or  (eta_bin<-32)):           
            indem = 4

        return indem

    def Fill(self,eta_bin,gain):     
        partition=self.get_partition_number(eta_bin)

        if partition > 0:
            self.his_partitions[0].Fill(gain)
            self.his_partitions[partition].Fill(gain)
        else:
            print ("Warning in EmPartitionPlots, nonexisting partition!"  )

class HadPartitionPlots:
    Counter = 0  
    def __init__(self,name,nbins=40,minimum=0.,maximum=2.,XaxisTitle="",YaxisTitle=""):
     
        self.nPartitions=6
        self.ext = ["all","00_09","09_15","15_25","25_32","32_50"]
        self.name = ["all","Tile LB","Tile EB","HEC outer", "HEC inner","FCAL 2/3"]
       
        self.his_partitions  = []

        # Maintain a counter of instances to assign unique histogram names
        HadPartitionPlots.Counter += 1
        for i_had_partition in range(0,self.nPartitions):
            hname = ("GainTTHad_%d"%HadPartitionPlots.Counter) + self.ext[i_had_partition]
            htitle = name+" for "+self.name[i_had_partition]
            self.his_partitions.append(ROOT.TH1F(hname,htitle,nbins,minimum,maximum))       

        setAxisTitles( self.his_partitions, XaxisTitle, YaxisTitle )
        setAxisSizes( self.his_partitions, 0.05, 0.04 )
                     
    def get_partition_number(self,eta_bin):
        indhad = -1
        if ( -9 <= eta_bin and eta_bin <= 8):                      
            indhad = 1
        elif ((eta_bin>8 and eta_bin<=14)  or (eta_bin>=-15 and eta_bin<-9)):
            #elif ((eta_bin>8 and eta_bin<14)  or (eta_bin>-15 and eta_bin<-9)):    # cut out overlap  
            indhad = 2
        elif ((eta_bin>14 and eta_bin<=24) or (eta_bin>=-25 and eta_bin<-15)): 
            indhad = 3
        elif ((eta_bin>24 and eta_bin<=31) or (eta_bin>=-32 and eta_bin<-25)): 
            indhad = 4
        elif ((eta_bin>31)          or (eta_bin<-32)):           
            indhad = 5

        return indhad     
     
    def Fill(self,eta_bin,gain):
        partition=self.get_partition_number(eta_bin)

        if partition > 0:
            self.his_partitions[0].Fill(gain)
            self.his_partitions[partition].Fill(gain)
        else:
            print ("Warning in HadPartitionPlots, nonexisting partition!"  )
  

def PlotCalibrationGains(input_file_name="",reference_file_name="",
                         isInputXml=False,isInputSqlite=False,
                         isRefXml=False,isRefSqlite=False,isRefOracle=False,
                         isTileOnly=False):

    ROOT.gROOT.SetBatch( True )    
    ROOT.gStyle.SetPalette(1)
    ROOT.gStyle.SetOptStat(111111)
    ROOT.gStyle.SetCanvasColor(10)

    ROOT.gStyle.SetPadTopMargin(0.12)
    ROOT.gStyle.SetPadBottomMargin(0.12)
    ROOT.gStyle.SetPadRightMargin(0.12)
    ROOT.gStyle.SetPadLeftMargin(0.12)
    
    canvas = ROOT.TCanvas('canvas','Gains',0,0,600,800)
    canvas.SetBatch(True)
  
    h_gains_em  = L1CaloMap("EM receiver gains","#eta bin","#phi bin")
    h_gains_had = L1CaloMap("HAD receiver gains","#eta bin","#phi bin")

    h_gains_em_fselect  = L1CaloMap("EM gains that failed selection","#eta bin","#phi bin")
    h_gains_had_fselect = L1CaloMap("HAD gains that failed selection","#eta bin","#phi bin")

    h_chi2_em  = L1CaloMap("EM fit Chi2","#eta bin","#phi bin")
    h_chi2_had = L1CaloMap("HAD fit Chi2","#eta bin","#phi bin")

    h_offset_em  = L1CaloMap("EM fit offsets","#eta bin","#phi bin")
    h_offset_had = L1CaloMap("HAD fit offsets","#eta bin","#phi bin")

    h_unfitted_em  = L1CaloMap("EM failed fits","#eta bin","#phi bin")
    h_unfitted_had = L1CaloMap("HAD failed fits","#eta bin","#phi bin")

    h_drifted_em  = L1CaloMap("EM TTs that drifted by more than 10%","#eta bin","#phi bin")
    h_drifted_had = L1CaloMap("HAD TTs that drifted by more than 10%","#eta bin","#phi bin")

    h_gains_em_reference  = L1CaloMap("EM (gain-reference)","#eta bin","#phi bin")
    h_gains_had_reference = L1CaloMap("HAD (gain-reference)","#eta bin","#phi bin")

    h_gains_em_reference_rel  = L1CaloMap("EM (gain-reference)/reference","#eta bin","#phi bin")
    h_gains_had_reference_rel = L1CaloMap("HAD (gain-reference)/reference","#eta bin","#phi bin")
    em_partition_gains = EmPartitionPlots(" EM gains",40,0.6,1.4,"gain","Entries")
    had_partition_gains = HadPartitionPlots(" HAD gains",40,0.5,2.5,"gain","Entries")

    em_partition_gains_ref = EmPartitionPlots(" EM (gains-reference)",40,-0.2,0.2,"gain-reference","Entries")
    had_partition_gains_ref = HadPartitionPlots(" HAD (gains-reference)",40,-1.,1.,"gain-reference","Entries")

    em_partition_gains_ref_rel = EmPartitionPlots(" EM (gains-reference)/reference",40,-0.2,0.2,"(gain-reference)/reference","Entries")
    had_partition_gains_ref_rel = HadPartitionPlots(" HAD (gains-reference)/reference",40,-1.,1.,"(gain-reference)/reference","Entries")

    threshold_change = 0.1

    geometry_convertor = L1CaloGeometryConvertor()
    receiver_gains    = GainReader()

    bad_gain_file = open('bad_gains.txt','w')
    drifted_towers_file = open('drifted_towers.txt','w')
  
    if isInputXml is True:
        print ("Taking input from xml file: ", input_file_name)
        receiver_gains.LoadGainsXml(input_file_name)
    elif isInputSqlite is True:
        print ("Taking input from Sqlite file: ", input_file_name)
        receiver_gains.LoadGainsSqlite(input_file_name)
    else:
        print ("No option for input file selected, assuming sqlite file energyscanresults.sqlite")
        receiver_gains.LoadGainsSqlite("energyscanresults.sqlite")
        
    if isRefXml is True:
        print ("Taking reference from Xml file: ",reference_file_name)
        receiver_gains.LoadReferenceXml(reference_file_name)
    elif isRefSqlite is True:
        print ("Taking reference from Sqlite file: ",reference_file_name)
        receiver_gains.LoadReferenceSqlite(reference_file_name)
    elif isRefOracle is True:
        print ("Taking reference from Oracle")
        geometry_convertor.LoadReceiverPPMMap()
        receiver_gains.LoadReferenceOracle(geometry_convertor)
    else:
        print (" No option for reference file, assuming Oracle")
        geometry_convertor.LoadReceiverPPMMap()
        receiver_gains.LoadReferenceOracle(geometry_convertor)
                
    for i_eta in range(-49,45):
        for i_phi in range(0,64):
            coolEm  = geometry_convertor.getCoolEm(i_eta,i_phi)
            coolHad = geometry_convertor.getCoolHad(i_eta,i_phi)
      
            if not coolEm == '':                      # there is a channel for this eta-phi      
                gain   = receiver_gains.getGain(coolEm)
                chi2   = receiver_gains.getChi2(coolEm)
                offset = receiver_gains.getOffset(coolEm)
                reference_gain = receiver_gains.getReferenceGain(coolEm)
                passes_selection = receiver_gains.passesSelection(coolEm)

                if (not gain == '') and (not reference_gain == ''):  # both  gains should be available

                    if gain == -1. :
                        h_unfitted_em.Fill(i_eta,i_phi)  
                        bad_gain_file.write('%i %i %s EM gain= %.3f \n' % (i_eta,i_phi,coolEm,float(gain))) 
                        h_gains_em_reference.Fill(i_eta,i_phi,-100.)
                        h_gains_em_reference_rel.Fill(i_eta,i_phi,-100.)
                    else:
                        h_gains_em.Fill(i_eta,i_phi,gain)
                        h_chi2_em.Fill(i_eta,i_phi,chi2)
                        h_offset_em.Fill(i_eta,i_phi,offset)   
                        em_partition_gains.Fill(i_eta,gain)         
                        em_partition_gains_ref.Fill(i_eta,gain-reference_gain)  
                        h_gains_em_reference.Fill(i_eta,i_phi,gain-reference_gain)

                        if passes_selection is False:
                            h_gains_em_fselect.Fill(i_eta,i_phi)
                            bad_gain_file.write('%i %i %s  EM gain= %.3f  chi2= %.3f offset= %.3f \n' %
                                                (i_eta,i_phi,coolEm,float(gain),float(chi2),float(offset))) 
                            #h_gains_em_reference.Fill(i_eta,i_phi,-100.)
                            #h_gains_em_reference_rel.Fill(i_eta,i_phi,-100.)

                        if reference_gain > 0:
                            em_partition_gains_ref_rel.Fill(i_eta,(gain-reference_gain)/reference_gain)  
                            h_gains_em_reference_rel.Fill(i_eta,i_phi,(gain-reference_gain)/reference_gain)
                            if fabs((gain-reference_gain)/reference_gain) > threshold_change:
                                h_drifted_em.Fill(i_eta,i_phi)
                                drifted_towers_file.write('%i %i %s  EM gain= %.3f   refGain= %.3f   (%.3f %%) \n' %
                                                          (i_eta,i_phi,coolEm,float(gain),float(reference_gain),
                                                           (gain-reference_gain)*100/reference_gain)) 


            if not coolHad == '':                    # there is a channel for this eta-phi
                gain = receiver_gains.getGain(coolHad)
                chi2   = receiver_gains.getChi2(coolHad)
                offset = receiver_gains.getOffset(coolHad)
                reference_gain = receiver_gains.getReferenceGain(coolHad)
                passes_selection = receiver_gains.passesSelection(coolHad)

                if (not gain == '') and (not reference_gain == ''):      # both gains should be available

                    if gain == -1. :
                        h_unfitted_had.Fill(i_eta,i_phi)  
                        bad_gain_file.write('%i %i %s HAD gain= %.3f \n' % (i_eta,i_phi,coolHad,float(gain)))
                        h_gains_had_reference.Fill(i_eta,i_phi,-100.)
                        h_gains_had_reference_rel.Fill(i_eta,i_phi,-100.)
                    else:
                        h_gains_had.Fill(i_eta,i_phi,gain)
                        h_chi2_had.Fill(i_eta,i_phi,chi2)
                        h_offset_had.Fill(i_eta,i_phi,offset)
                        had_partition_gains.Fill(i_eta,gain)
                        had_partition_gains_ref.Fill(i_eta,gain-reference_gain)
                        h_gains_had_reference.Fill(i_eta,i_phi,gain-reference_gain)

                        if passes_selection is False:
                            h_gains_had_fselect.Fill(i_eta,i_phi)
                            bad_gain_file.write( '%i %i %s  HAD gain= %.3f  chi2= %.3f offset= %.3f \n' %
                                                 (i_eta,i_phi,coolHad,float(gain),float(chi2),float(offset))) 
                            #h_gains_had_reference.Fill(i_eta,i_phi,-100.)
                            #h_gains_had_reference_rel.Fill(i_eta,i_phi,-100.)

                        if reference_gain > 0:
                            had_partition_gains_ref_rel.Fill(i_eta,(gain-reference_gain)/reference_gain)
                            h_gains_had_reference_rel.Fill(i_eta,i_phi,(gain-reference_gain)/reference_gain)
                            if fabs((gain-reference_gain)/reference_gain) > threshold_change:
                                h_drifted_had.Fill(i_eta,i_phi)
                                drifted_towers_file.write('%i %i %s  HAD gain= %.3f   refGain= %.3f   (%.3f %%) \n' %
                                                          (i_eta,i_phi,coolHad,float(gain),float(reference_gain),
                                                           (gain-reference_gain)*100/reference_gain))

    # start the plotting
    pdfFileName = "Gains.pdf"
    canvas.Print( pdfFileName + "[" )

    # new page: EM gains and drifts
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(1,2)
        canvas.cd(1)
        ROOT.gPad.SetLogy(0)
        h_gains_em.SetMinimum(0.6)
        h_gains_em.SetMaximum(1.4)
        h_gains_em.Draw()
        canvas.cd(2)
        h_drifted_em.Draw()
        canvas.Print( pdfFileName )

    # new page: EM gains that failed selection and failed fits
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(1,2)
        canvas.cd(1)
        ROOT.gPad.SetLogy(0)
        h_gains_em_fselect.Draw()
        canvas.cd(2)
        h_unfitted_em.Draw()
        canvas.Print( pdfFileName )

    # new page: HAD gains and drifts
    canvas.Clear()
    canvas.Divide(1,2)
    canvas.cd(1)
    ROOT.gPad.SetLogy(0)
    h_gains_had.SetMinimum(0.6)
    h_gains_had.SetMaximum(1.4)
    h_gains_had.Draw()
    canvas.cd(2)
    h_drifted_had.Draw()
    canvas.Print( pdfFileName )

    # new page: HAD gains that failed selection and failed fits
    canvas.Clear()
    canvas.Divide(1,2)
    canvas.cd(1)
    h_gains_had_fselect.Draw()
    canvas.cd(2)
    h_unfitted_had.Draw()
    canvas.Print( pdfFileName )
    
    # new page: EM gains relative references
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(1,2)
        canvas.cd(1)
        ROOT.gPad.SetLogy(0)
        h_gains_em_reference_rel.SetMinimum(-0.5)
        h_gains_em_reference_rel.SetMaximum(0.5)
        h_gains_em_reference_rel.Draw()
        canvas.cd(2)
        h_gains_em_reference_rel.SetMinimum(-0.1)
        h_gains_em_reference_rel.SetMaximum(0.1)
        h_gains_em_reference_rel.Draw()
        canvas.Print( pdfFileName )

    # new page: HAD gains relative references
    canvas.Clear()
    canvas.Divide(1,2)
    canvas.cd(1)
    ROOT.gPad.SetLogy(0)
    h_gains_had_reference_rel.SetMinimum(-0.5)
    h_gains_had_reference_rel.SetMaximum(0.5)
    h_gains_had_reference_rel.Draw()
    canvas.cd(2)    
    h_gains_had_reference_rel.SetMinimum(-0.1)
    h_gains_had_reference_rel.SetMaximum(0.1)
    h_gains_had_reference_rel.Draw()
    canvas.Print( pdfFileName )

    # new page: EM gains absolute references
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(1,2)
        canvas.cd(1)
        ROOT.gPad.SetLogy(0)
        h_gains_em_reference.SetMinimum(-0.5)
        h_gains_em_reference.SetMaximum(0.5)
        h_gains_em_reference.Draw()
        canvas.cd(2)
        ROOT.gPad.SetLogy(0)
        h_gains_em_reference.SetMinimum(-0.1)
        h_gains_em_reference.SetMaximum(0.1)
        h_gains_em_reference.Draw()
        canvas.Print( pdfFileName )

    # new page: HAD gains absolute references
    canvas.Clear()
    canvas.Divide(1,2)
    canvas.cd(1)
    ROOT.gPad.SetLogy(0)
    h_gains_had_reference.SetMinimum(-0.5)
    h_gains_had_reference.SetMaximum(0.5)
    h_gains_had_reference.Draw()
    canvas.cd(2)
    ROOT.gPad.SetLogy(0)
    h_gains_had_reference.SetMinimum(-0.1)
    h_gains_had_reference.SetMaximum(0.1)
    h_gains_had_reference.Draw()
    canvas.Print( pdfFileName )

    # new page: EM chi2 and offsets
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(1,2)
        canvas.cd(1)
        ROOT.gPad.SetLogy(0)
        h_chi2_em.SetMinimum(0.1)
        h_chi2_em.SetMaximum(100)
        h_chi2_em.Draw()
        canvas.cd(2)
        ROOT.gPad.SetLogy(0)
        h_offset_em.SetMinimum(-1.)
        h_offset_em.SetMaximum(1.)
        h_offset_em.Draw()
        canvas.Print( pdfFileName )

    # new page: HAD chi2 and offsets
    canvas.Clear()
    canvas.Divide(1,2)
    canvas.cd(1)
    ROOT.gPad.SetLogy(0)
    h_chi2_had.SetMinimum(0.1)
    h_chi2_had.SetMaximum(100)
    h_chi2_had.Draw()
    canvas.cd(2)
    ROOT.gPad.SetLogy(0)
    h_offset_had.SetMinimum(-1.)
    h_offset_had.SetMaximum(1.)
    h_offset_had.Draw()    
    canvas.Print( pdfFileName )

    # new page: EM 1D gains per layer
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(2,3)
        for i_p in range(0,em_partition_gains.nPartitions):
            canvas.cd(i_p+1)
            ROOT.gPad.SetLogy() if em_partition_gains.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
            em_partition_gains.his_partitions[i_p].Draw()
        canvas.Print( pdfFileName )

    # new page: HAD 1D gains per layer
    canvas.Clear()
    canvas.Divide(2,3)
    for i_p in range(0,had_partition_gains.nPartitions):
        if (isTileOnly and i_p>2): break
        canvas.cd(i_p+1)
        ROOT.gPad.SetLogy() if  had_partition_gains.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
        had_partition_gains.his_partitions[i_p].Draw()
    canvas.Print( pdfFileName )

    # new page: EM 1D gains absolute references per layer
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(2,3)
        for i_p in range(0,em_partition_gains_ref.nPartitions):
            canvas.cd(i_p+1)
            ROOT.gPad.SetLogy() if em_partition_gains_ref.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
            em_partition_gains_ref.his_partitions[i_p].Draw()  
        canvas.Print( pdfFileName )
    
    # new page: HAD 1D gains absolute references per layer
    canvas.Clear()
    canvas.Divide(2,3)
    for i_p in range(0,had_partition_gains_ref.nPartitions):
        if (isTileOnly and i_p>2): break
        canvas.cd(i_p+1)
        ROOT.gPad.SetLogy() if had_partition_gains_ref.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
        had_partition_gains_ref.his_partitions[i_p].Draw()  
    canvas.Print( pdfFileName )

    # new page: EM 1D gains relative references per layer
    if not isTileOnly:
        canvas.Clear()
        canvas.Divide(2,3)
        for i_p in range(0,em_partition_gains_ref_rel.nPartitions):
            canvas.cd(i_p+1)
            ROOT.gPad.SetLogy() if em_partition_gains_ref_rel.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
            em_partition_gains_ref_rel.his_partitions[i_p].Draw()  
        canvas.Print( pdfFileName )

    # new page: HAD 1D gains relative references per layer
    canvas.Clear()
    canvas.Divide(2,3)
    for i_p in range(0,had_partition_gains_ref_rel.nPartitions):
        if (isTileOnly and i_p>2): break
        canvas.cd(i_p+1)
        ROOT.gPad.SetLogy() if had_partition_gains_ref_rel.his_partitions[i_p].GetEntries()>0 else ROOT.gPad.SetLogy(0)
        had_partition_gains_ref_rel.his_partitions[i_p].Draw()  
    canvas.Print( pdfFileName )

    # Closing files
    canvas.Print( pdfFileName + "]" )
    bad_gain_file.close()
    drifted_towers_file.close()  
    print ("finished!")

    
if __name__ == "__main__":

    print ("Starting plot_gains_xml")

    parser = OptionParser()
    parser.add_option("-f","--InputFile",action="store",type="string",dest="input_file_name",help="Name of input file")
    parser.add_option("-r","--ReferenceFile",action="store",type="string",dest="reference_file_name",help="Name of reference file")
    parser.add_option("--InputXml",action="store_true",dest="isInputXml",help="Input is .xml file")
    parser.add_option("--InputSqlite",action="store_true",dest="isInputSqlite",help="Input is .sqlite file")
    parser.add_option("--RefXml",action="store_true",dest="isRefXml",help="Reference is .xml file")
    parser.add_option("--RefSqlite",action="store_true",dest="isRefSqlite",help="Reference is .sqlite file")
    parser.add_option("--RefOracle",action="store_true",dest="isRefOracle",help="Reference is from Oracle")
    parser.add_option("--TileOnly",action="store_true",dest="isTileOnly",help="Show only Tile",default=True)
    
    (options, args) = parser.parse_args()

    PlotCalibrationGains(options.input_file_name, options.reference_file_name, options.isInputXml,
                     options.isInputSqlite, options.isRefXml, options.isRefSqlite, options.isRefOracle,
                     options.isTileOnly)
