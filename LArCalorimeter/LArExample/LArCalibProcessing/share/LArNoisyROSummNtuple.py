#!/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

##=======================================================================================
## Name:        LArCellNtuple.py
##
## Description: Python script using  to dump the cells to the ntuple
##========================================================================================
import os,sys,getopt

# dumping class

import cppyy
from array import array

import ROOT
from ROOT import HWIdentifier, Identifier, IdentifierHash, TFile
from AthenaPython import PyAthena

class NoisyRO(PyAthena.Alg):
    def __init__(self,ofile,hv):
        super(NoisyRO,self).__init__()
        self.fout = ROOT.TFile(ofile,"RECREATE")
        self.addhv=hv

    def initialize (self):
        self.msg.debug("Doing NoisyRO init")
        self.sg = PyAthena.py_svc('StoreGateSvc')
        self.has_offID=False
        self.is_init=False
        return 1

    def execute (self):
        if self.detStore is None: 
           self.detStore = PyAthena.py_svc('DetectorStore')
           if self.detStore is None:
               print("Failed to get DetectorStore")
               return 0
        if not self.has_offID:
           self.offlineID = self.detStore['CaloCell_ID']       
        if self.offlineID is None:
               print("Failed to get CaloCell_ID")
               return 0
        EI = self.sg['EventInfo']
        if EI is None:
               self.msg.warning("Failed to get EventInfo")
               self.evtid=0
               self.bcid=0
        else: 
               self.evtid=EI.eventNumber()
               self.bcid=EI.bcid()

        if not self.is_init:
           self.msg.debug("Doing NoisyRO Ttree init")
           self.maxvec=100
           self.aevtid=array('L',[self.evtid])
           self.abcid=array('I',[self.bcid])
           self.badFEBPart=array('H',[0])
           self.badWFFEBPart=array('H',[0])
           self.SatMediumPart=array('H',[0])
           self.SatTightPart=array('H',[0])
           self.MNBLoosePart=array('H',[0])
           self.MNBTightPart=array('H',[0])
           self.MNBTight_PsVetoPart=array('H',[0])
           self.nbad = array('I',[0]) 
           self.idvec = array('i',self.maxvec*[-1]) 
           self.nMNBTight = array('I',[0]) 
           self.MNBTightvec = array('i',self.maxvec*[-1]) 
           self.nMNBLoose = array('I',[0]) 
           self.MNBLoosevec = array('i',self.maxvec*[-1]) 
           self.nMNBTight_PsVeto = array('I',[0]) 
           self.MNBTight_PsVetovec = array('i',self.maxvec*[-1]) 
           self.fout.cd()
           self.nt = ROOT.TTree("mytree","mytree")
           self.nt.SetDirectory(self.fout)
           self.nt.Branch("Event",self.aevtid,"Event/l")
           self.nt.Branch("BCID",self.abcid,"BCID/I")
           self.nt.Branch("badFEBPart",self.badFEBPart,"badFEBPart/s")
           self.nt.Branch("badWFFEBPart",self.badWFFEBPart,"badWFFEBPart/s")
           self.nt.Branch("SatMediumPart",self.SatMediumPart,"SatMediumPart/s")
           self.nt.Branch("SatTightPart",self.SatTightPart,"SatTightPart/s")
           self.nt.Branch("MNBLoosePart",self.MNBLoosePart,"MNBLoosePart/s")
           self.nt.Branch("MNBTightPart",self.MNBTightPart,"MNBTightPart/s")
           self.nt.Branch("MNBTight_PsVetoPart",self.MNBTight_PsVetoPart,"MNBTight_PsVetoPart/s")
           self.nt.Branch("nbad",self.nbad,"nbad/i")
           self.nt.Branch("bad_febs",self.idvec,"bad_febs[nbad]/I")
           self.nt.Branch("nMNBTight",self.nMNBTight,"nMNBTight/i")
           self.nt.Branch("MNBTight_febs",self.MNBTightvec,"MNBTight_febs[nbad]/I")
           self.nt.Branch("nMNBLoose",self.nMNBLoose,"nMNBLoose/i")
           self.nt.Branch("MNBLoose_febs",self.MNBLoosevec,"MNBLoose_febs[nbad]/I")
           self.nt.Branch("nMNBTight_PsVeto",self.nMNBTight_PsVeto,"nMNBTight_PsVeto/i")
           self.nt.Branch("MNBTight_PsVeto_febs",self.MNBTight_PsVetovec,"MNBTight_PsVeto_febs[nbad]/I")
           if self.addhv:
              self.hvLinesPart=array('H',[0])
              self.nhv = array('I',[0]) 
              self.hvLinesvec = array('i',self.maxvec*[-1]) 
              self.nt.Branch("hvLinesPart",self.hvLinesPart,"hvLinesPart/s")
              self.nt.Branch("nhv",self.nhv,"nhv/i")
              self.nt.Branch("hvLines_febs",self.hvLinesvec,"hvLines_febs[nbad]/I")

           self.is_init=True
           self.msg.debug(" Ttree init done")

        Nro = self.sg['LArNoisyROSummary']
        self.badFEBPart=Nro.BadFEBFlaggedPartitions()
        print("badFEBPart: ",self.badFEBPart," ",Nro.BadFEBFlaggedPartitions())
        self.badWFFEBPart[0]=Nro.BadFEB_WFlaggedPartitions()
        self.SatMediumPart[0]=Nro.SatMediumFlaggedPartitions()
        self.SatTightPart[0]=Nro.SatTightFlaggedPartitions()
        self.MNBLoosePart[0]=Nro.MNBLooseFlaggedPartitions()
        self.MNBTightPart[0]=Nro.MNBTightFlaggedPartitions()
        self.MNBTight_PsVetoPart[0]=Nro.MNBTight_PsVetoFlaggedPartitions()
        self.aevtid[0]=self.evtid    
        self.abcid[0]=self.bcid
        hwvec=Nro.get_noisy_febs()
        self.nbad[0]=hwvec.size()
        if self.nbad[0] >= self.maxvec:
           self.nbad[0]=self.maxvec
        for i in range(0,self.nbad[0]):
            self.idvec[i]=hwvec[i].get_identifier32().get_compact()

        hwvec=Nro.get_MNBTight_febs()
        self.nMNBTight[0]=hwvec.size()
        if self.nMNBTight[0] >= self.maxvec:
           self.nMNBTight[0]=self.maxvec
        for i in range(0,self.nMNBTight[0]):
            self.MNBTightvec[i]=hwvec[i].get_identifier32().get_compact()

        hwvec=Nro.get_MNBLoose_febs()
        self.nMNBLoose[0]=hwvec.size()
        if self.nMNBLoose[0] >= self.maxvec:
           self.nMNBLoose[0]=self.maxvec
        for i in range(0,self.nMNBLoose[0]):
            self.MNBLoosevec[i]=hwvec[i].get_identifier32().get_compact()

        hwvec=Nro.get_MNBTight_PsVeto_febs()
        self.nMNBTight_PsVeto[0]=hwvec.size()
        if self.nMNBTight_PsVeto[0] >= self.maxvec:
           self.nMNBTight_PsVeto[0]=self.maxvec
        for i in range(0,self.nMNBTight_PsVeto[0]):
            self.MNBTight_PsVetovec[i]=hwvec[i].get_identifier32().get_compact()

        if self.addhv:
           self.hvLinesPart[0]=Nro.HVlineFlaggedPartitions()
           hwvec=Nro.get_noisy_hvlines()
           self.nhv[0]=hwvec.size()
           if self.nhv[0] >= self.maxvec:
              self.nhv[0]=self.maxvec
           for i in range(0,self.nhv[0]):
               self.hvLinesvec[i]=hwvec[i].get_identifier32().get_compact()

        self.nt.Fill()
        return 1

    def finalize (self):
        self.fout.cd()
        self.nt.Write()
        self.fout.Close()
        return 1


def usage():
    print(sys.argv[0]+": Dump LArNoisyROSummary from pool file to ntuple")
    print("Options:")
    print("-i input file (default ESD.pool.root)")
    print("-o output file (default NoisyROSum.root)")
    print("-n number of events to dump (default -1)")
    print("-m MC pool file (default fals)")
    print("-v add HVline info (default fals)")
    print("--detdescr <DetDescrVersion>")
    print("-h Print this help text and exit")
        
try:
    opts,args=getopt.getopt(sys.argv[1:],"i:o:n:mvh",["help","detdescr="])
except Exception as e:
    usage()
    print(e)
    sys.exit(-1)

    
ifile='ESD.pool.root'
ofile="NoisyROSum.root"
nev=-1
mc=False
hv=False
from AthenaConfiguration.TestDefaults import defaultGeometryTags
detdescrtag=defaultGeometryTags.RUN2

for o,a in opts:
    if (o=="-i"): ifile=a
    if (o=="-o"): ofile=a
    if (o=="-n"): nev=int(a)
    if (o=="-m"): mc=True
    if (o=="-v"): hv=True
    if (o=="-h" or o=="--help"):
        usage()
        sys.exit(0)
    if (o=="--detdescr"):
        detdescrtag=a
    
#Don't let PyRoot open X-connections
sys.argv = sys.argv[:1] + ['-b'] 

from AthenaConfiguration.AllConfigFlags import initConfigFlags 
flags=initConfigFlags()
flags.Input.Files = [ifile]
flags.Input.isMC=mc
flags.IOVDb.DatabaseInstance="CONDBR2" 
flags.GeoModel.AtlasVersion = detdescrtag 
flags.LAr.doAlign=False

from AthenaConfiguration.DetectorConfigFlags import disableDetectors, allDetectors
disableDetectors(flags, allDetectors, toggle_geometry = True)
flags.Detector.EnableLAr = True
flags.Detector.EnableTile = True
flags.Detector.EnableCalo = True

from AthenaCommon.Constants import INFO
flags.Exec.OutputLevel=INFO
flags.lock()
flags.dump()

from RootUtils import PyROOTFixes  # noqa F401
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg=MainServicesCfg(flags)

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge(PoolReadCfg(flags))

from TileGeoModel.TileGMConfig import TileGMCfg
cfg.merge( TileGMCfg(flags) )
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
cfg.merge(LArGMCfg(flags))

cfg.addEventAlgo(NoisyRO(ofile,hv))

cfg.run(nev) 
