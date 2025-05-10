#!/usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from CoolConvUtilities.AtlCoolLib import indirectOpen

class LArRunInfo:
    "Wrapper class to hold LAr run configuration information"
    def __init__(self,nSamples,gainType,latency,firstSample,format,runType):
        self._nSamples = nSamples
        self._gainType = gainType
        self._latency = latency
        self._firstSample = firstSample
        self._format = format
        self._runType = runType

    def nSamples(self):
        "Number of samples readout from FEB"
        return self._nSamples

    def gainType(self):
        "gainType: 0=auto,1=L,2=M,3=H,4=LM,5=LH,6=ML,7=MH,8=HL,9=HM,10=LMH,11=LHM,12=MLH,13=MHL,14=HLM,15=HML"
        return self._gainType

    def latency(self):
        "latency between l1 trigger and readout"
        return self._latency

    def firstSample(self):
        "firstsample"
        return self._firstSample

    def format(self):
        "format:0=Transparent, 1=Format 1, 2=Format 2"
        return self._format

    def runType(self):
        "runType: 0=RawData, 1=RawDataResult, 2=Result"
        return self._runType

    def stringFormat(self):
       if (self._format == 0) :
           return 'transparent'
       if (self._format == 1) :
           return 'Format1'
       if (self._format == 2) :
           return 'Format2'

    def stringRunType(self):
       if (self._runType ==0) :
           return 'RawData'
       if (self._runType ==1) :
           return 'RawDataResult'
       if (self._runType ==2) :
           return 'Result'


def getLArFormatForRun(run,quiet=False,connstring="COOLONL_LAR/CONDBR2"):
    from AthenaCommon.Logging import logging
    mlog_LRF = logging.getLogger( 'getLArRunFormatForRun' )

    mlog_LRF.info("Connecting to database %s", connstring)

    mlog_LRF.info("Found LAr info for run %i",run)
    runDB=indirectOpen(connstring)
    if (runDB is None):
        mlog_LRF.error("Cannot connect to database %s",connstring)
        raise RuntimeError("getLArFormatForRun ERROR: Cannot connect to database %s",connstring)
    format=None
    nSamples=None
    gainType=None
    runType=None
    latency=None
    firstSample=None
    try:
        folder=runDB.getFolder('/LAR/Configuration/RunLog')
        runiov=run << 32
        obj=folder.findObject(runiov,0)
        payload=obj.payload()
        format=payload['format']
        nSamples=ord(payload['nbOfSamples'])
        gainType=payload['gainType'] 
        runType=payload['runType']
        latency=ord(payload['l1aLatency'])
        firstSample=ord(payload['firstSample'])
    except Exception:
        mlog_LRF.warning("No information in /LAR/Configuration/RunLog for run %i", run)
        #mlog_LRF.warning(e)
        return None
    runDB.closeDatabase()
    mlog_LRF.info("Found info for run %i", run)
    return  LArRunInfo(nSamples,gainType,latency,firstSample,format,runType)

class LArDTRunInfo:
    "Wrapper class to hold LAr DT run configuration information"
    def __init__(self,streamTypes, streamLengths, streamTypesPEB, streamLengthsPEB, timing, adccalib, fw):
        self._sTypes = streamTypes
        self._sLengths = streamLengths
        self._sTypesPEB = streamTypesPEB
        self._sLengthsPEB = streamLengthsPEB
        self._tim = timing
        self._adcc = adccalib
        self._fwversion = fw

    def streamTypes(self):
           return self._sTypes

    def streamLengths(self):
           return self._sLengths

    def streamTypesPEB(self):
           return self._sTypesPEB

    def streamLengthsPEB(self):
           return self._sLengthsPEB

    def timing(self):
           return self._tim

    def ADCCalib(self):
           return self._adcc

    def FWversion(self):
           return self._fwversion

def parse_recipe(recipe,mux,mlog):
    # parse recipe string of the type at0_bcX-at1_bcY...
    typesMap={0:"ADC", 1:"RawADC", 2:"Energy", 3:"SelectedEnergy",15:"Invalid"}
    sTypes=[]
    sLengths=[]    
    for s,m in ["at0_bc",0],["at1_bc",1]:
       pos=recipe.find(s)
       if pos >=0:
          n=-1
          try:
             n=int(recipe[pos+6:pos+8])
          except Exception:
             try:
               n=int(recipe[pos+6:pos+7])
             except Exception:
               mlog.warning("could not decode %s",recipe[pos+6:])
          if n>=0:
             sLengths.append(n)
             if mux[m] in typesMap.keys():
                sTypes.append(typesMap[mux[m]])
             else:         
                sTypes.append(15)
          pass      
       pass
    return (sTypes,sLengths) 

def getLArDTInfoForRun(run,quiet=False,connstring="COOLONL_LAR/CONDBR2"):
    from AthenaCommon.Logging import logging
    mlog_LRF = logging.getLogger( 'getLArDTRunInfoForRun' )
    mlog_LRF.info("Connecting to database %s", connstring)

    runDB=indirectOpen(connstring)
    if (runDB is None):
        mlog_LRF.error("Cannot connect to database %s",connstring)
        raise RuntimeError("getLArFormatForRun ERROR: Cannot connect to database %s",connstring)
    mlog_LRF.info("Found DB")
    timing="LAR"
    adccalib=0
    mux=[]
    fw=0
    try:
        folder=runDB.getFolder('/LAR/Configuration/RunLogDT')
        runiov=run << 32
        obj=folder.findObject(runiov,0)
        payload=obj.payload()
        timing=payload['timing_configuration']
        recipe=payload['recipe_tdaq_A']
        recipePEB=payload['recipe_tdaq_B']
        mux.append(ord(payload['mux_setting_0_tdaq']))
        mux.append(ord(payload['mux_setting_1_tdaq']))
        adccalib=ord(payload['ADCCalibMode'])
        if run > 493743: # hardcoded, first run when this info was filled
            fw=ord(payload['ttype_mask_A'])
        mlog_LRF.info("Found DT info for run %i",run)
    except Exception:
        mlog_LRF.warning("No information in /LAR/Configuration/RunLogDT for run %i", run)
        mlog_LRF.warning("Using defaults: MUX0: ADC MUX1: ET_ID receipe: at0_bc5-at1_bc1_ts1-q")
        recipe="at0_bc5-at1_bc1_ts1-q"
        recipePEB=''
        mux.append(0)
        mux.append(3)

    runDB.closeDatabase()
    print(mux,fw)
    sTypes, sLengths = parse_recipe(recipe,mux,mlog_LRF)
    sTypesPEB, sLengthsPEB = parse_recipe(recipePEB,mux,mlog_LRF)
    return  LArDTRunInfo(sTypes, sLengths, sTypesPEB, sLengthsPEB, timing, adccalib, fw)

# command line driver for convenience
if __name__=='__main__':
    import sys
    if len(sys.argv)!=2:
        print("Syntax",sys.argv[0],'<run>')
        sys.exit(-1)
    run=int(sys.argv[1])
    myformat=getLArFormatForRun(run, connstring="COOLONL_LAR/CONDBR2")
    if (myformat is not None):
      print(" LAr run configuration: Nsamples:%d  GainType:%d  Latency:%d  FirstSample:%d  Format:%s  runType:%s" % (myformat.nSamples(),myformat.gainType(),myformat.latency(),myformat.firstSample(),myformat.stringFormat(),myformat.stringRunType()))
    else:
      print(" LAr run information not available")

    myformat1=getLArDTInfoForRun(run, connstring="COOLONL_LAR/CONDBR2")
    if (myformat1 is not None):
      print(" LAr DT run configuration: timing:%s  adccalib:%d" % (myformat1.timing(),myformat1.ADCCalib()))
      for i in range(0,len(myformat1.streamTypes())):
         print(" stream: %s  size: %d" % (myformat1.streamTypes()[i], myformat1.streamLengths()[i]))  
    else:
      print(" LAr DT run information not available")
