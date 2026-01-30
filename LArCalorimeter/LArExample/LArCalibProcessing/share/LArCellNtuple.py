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

class CellE(PyAthena.Alg):
    def __init__(self,ofile,isSC):
        super(CellE,self).__init__()
        self.fout = ROOT.TFile(ofile,"RECREATE")
        self.isSC = isSC

    def initialize (self):
        self.msg.debug("Doing CellE init")
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
           if not self.isSC:
              self.offlineID = self.detStore['CaloCell_ID']       
           else:
              self.offlineID = self.detStore['CaloCell_SuperCell_ID']       
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
           self.msg.debug("Doing CellE Ttree init")
           self.maxcells=self.offlineID.calo_cell_hash_max()
           self.amaxcells=array('i',[self.maxcells])
           self.aevtid=array('L',[self.evtid])
           self.abcid=array('I',[self.bcid])
           self.idvec = array('i',self.maxcells*[0]) # storing the identifiers
           self.calosamp = array('i',self.maxcells*[0]) # storing the calo sample
           self.posneg = array('i',self.maxcells*[0]) # storing the side
           self.region = array('i',self.maxcells*[0]) # storing the region
           self.etaidx = array('i',self.maxcells*[0]) # storing the eta bin
           self.phiidx = array('i',self.maxcells*[0]) # storing the phi bin
           self.encell =  array('d',self.maxcells*[0.]) # energies
           self.timecell =  array('d',self.maxcells*[0.]) # timing
           self.provcell =  array('H',self.maxcells*[0]) # provenance
           self.qcell =  array('H',self.maxcells*[0]) # quality
           self.fout.cd()
           self.nt = ROOT.TTree("mytree","mytree")
           self.nt.SetDirectory(self.fout)
           self.nt.Branch("ncell",self.amaxcells,"ncell/I") 
           self.nt.Branch("Event",self.aevtid,"Event/l")
           self.nt.Branch("BCID",self.abcid,"BCID/I")
           self.nt.Branch("OffID",self.idvec,"OffID[ncell]/i")
           self.nt.Branch("calosamp",self.calosamp,"calosamp[ncell]/I")
           self.nt.Branch("pos_neg",self.posneg,"pos_neg[ncell]/I")
           self.nt.Branch("region",self.region,"region[ncell]/I")
           self.nt.Branch("eta",self.etaidx,"eta[ncell]/I")
           self.nt.Branch("phi",self.phiidx,"phi[ncell]/I")
           self.nt.Branch("energy",self.encell,"energy[ncell]/D")
           self.nt.Branch("time",self.timecell,"time[ncell]/D")
           self.nt.Branch("prov",self.provcell,"prov[ncell]/s")
           self.nt.Branch("qual",self.qcell,"qual[ncell]/s")
           self.is_init=True
           self.msg.debug("CellE Ttree init done")

        if not self.isSC:   
           Cells = self.sg['AllCalo']
        else:   
           Cells = self.sg['SCell']
        nc = Cells.size()
        cellitr=0;
        for j in range(nc):
            c=Cells[j]
            id=c.ID()
            if self.offlineID.is_tile(id): continue
            if cellitr >= self.maxcells: continue
            self.idvec[cellitr]=id.get_identifier32().get_compact()
            self.calosamp[cellitr]=self.offlineID.calo_sample(id)
            self.region[cellitr]=self.offlineID.region(id)
            self.etaidx[cellitr]=self.offlineID.eta(id)
            self.posneg[cellitr]=self.offlineID.pos_neg(id)
            self.phiidx[cellitr]=self.offlineID.phi(id)
            self.encell[cellitr]=c.energy()
            self.timecell[cellitr]=c.time()
            self.provcell[cellitr]=c.provenance()
            self.qcell[cellitr]=c.quality()
            cellitr += 1
        self.aevtid[0]=self.evtid    
        self.abcid[0]=self.bcid
        self.amaxcells[0]=cellitr
        if self.amaxcells[0] > 0:
           self.nt.Fill()
        return 1

    def finalize (self):
        self.fout.cd()
        self.nt.Write()
        self.fout.Close()
        return 1


def usage():
    print(sys.argv[0]+": Dump cells from pool file to ntuple")
    print("Options:")
    print("-i input file (default ESD.pool.root)")
    print("-o output file (default cells.root)")
    print("-n number of events to dump (default -1)")
    print("-m MC pool file (default false)")
    print("-s is SC (default false)")
    print("--detdescr <DetDescrVersion>")
    print("-h Print this help text and exit")
        
try:
    opts,args=getopt.getopt(sys.argv[1:],"i:o:n:msh",["help","detdescr="])
except Exception as e:
    usage()
    print(e)
    sys.exit(-1)

    
ifile='ESD.pool.root'
ofile="cells.root"
nev=-1
mc=False
sc=False
from AthenaConfiguration.TestDefaults import defaultGeometryTags
detdescrtag=defaultGeometryTags.RUN2

for o,a in opts:
    if (o=="-i"): ifile=a
    if (o=="-o"): ofile=a
    if (o=="-n"): nev=int(a)
    if (o=="-m"): mc=True
    if (o=="-s"): sc=True
    if (o=="-h" or o=="--help"):
        usage()
        sys.exit(0)
    if (o=="--detdescr"):
        detdescrtag=a
    
#Don't let PyRoot open X-connections
sys.argv = sys.argv[:1] + ['-b'] 

from AthenaConfiguration.AllConfigFlags import initConfigFlags 
flags=initConfigFlags()
flags.Input.Files = ifile.split(",")
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
if sc:
  from AthenaConfiguration.ComponentFactory import CompFactory
  cfg.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg())

cfg.addEventAlgo(CellE(ofile,sc))

cfg.run(nev) 
