#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Constants import DEBUG, VERBOSE
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

flags = initConfigFlags()
flags.Exec.FPE = -2  # disable FPE auditing
flags.Exec.MaxEvents = 10
flags.lock()

cfg = MainServicesCfg(flags)

# Services
atMars = CompFactory.AtDSFMTGenSvc(
   OutputLevel=VERBOSE,
   EventReseeding=True,
   Seeds = ["MARS 1 2", "MARSOFF OFFSET 73 1 2", "MARS_INIT 3 4", "TEST 5 6"]
)
cfg.addService( atMars )

atLux = CompFactory.AtRanluxGenSvc(
   OutputLevel=VERBOSE,
   UseOldBrokenSeeding=False,
   EventReseeding=True,
   Seeds = [ "LUX 1 2", "LUXOFF OFFSET 73 1 2", "LUX_INIT LUXURY 0 3 4", "TEST 5 6"]
)
cfg.addService( atLux )

atRnd = CompFactory.AtRndmGenSvc(
   OutputLevel=VERBOSE,
   UseOldBrokenSeeding=False,
   EventReseeding=True,
   ReseedStreamNames = [ "TEST", "RNDM", "RNDMOFF" ],
   Seeds = [ "RNDM 11 12", "RNDMOFF OFFSET 73 11 12", "RNDM_INIT 13 14", "TEST 15 16"]
)
cfg.addService( atRnd )

atLuxNN = CompFactory.AtRanluxGenSvc(
   "LuxOld",
   OutputLevel=VERBOSE,
   UseOldBrokenSeeding=True,
   EventReseeding=False,
   Seeds = [ "LUX 1 2", "LUX_INIT LUXURY 0 3 4", "TEST 5 6"]
)
cfg.addService( atLuxNN )

atRndOR = CompFactory.AtRndmGenSvc(
   "RndNoReseed",
   OutputLevel=VERBOSE,
   UseOldBrokenSeeding=False,
   EventReseeding=False,
   Seeds = [ "RNDM 11 12", "RNDM_INIT 13 14", "TEST 15 16"]
)
cfg.addService( atRndOR )

# Testing algorithm
def testAlg(name, **kwargs):
   return CompFactory.TestRandomSeqAlg(name, OutputLevel=DEBUG, **kwargs)

cfg.addEventAlgo( testAlg("AtRanTestAlg", RndmSvc=atLux) )
cfg.addEventAlgo( testAlg("AtLuxWithOffTestAlg", RndmSvc=atLux, StreamName="LUXOFF") )
cfg.addEventAlgo( testAlg("AtLuxNoOffTestAlg", RndmSvc=atLux, StreamName="LUX") )
cfg.addEventAlgo( testAlg("AtRndTestAlg", RndmSvc=atRnd) )
cfg.addEventAlgo( testAlg("AtWithOffTestAlg", RndmSvc=atRnd, StreamName="RNDMOFF") )
cfg.addEventAlgo( testAlg("AtNoOffTestAlg", RndmSvc=atRnd, StreamName="RNDM") )
cfg.addEventAlgo( testAlg("OldTestAlg", RndmSvc=atLuxNN) )
cfg.addEventAlgo( testAlg("NoReseedTestAlg", RndmSvc=atRndOR) )
cfg.addEventAlgo( testAlg("MarsTestAlg", RndmSvc=atMars) )

# Run
import sys
sys.exit( cfg.run().isFailure() )
