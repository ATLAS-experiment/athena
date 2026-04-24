#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaPython import PyAthena

flags = initConfigFlags()
flags.fillFromArgs()
flags.lock()

# Create ComponentAccumulator
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaRootComps.RootReadConfig import RootReadCfg

acc = MainServicesCfg(flags)
acc.merge( RootReadCfg(flags, tupleName="egamma;egamma_der") )
acc.addEventAlgo( CompFactory.Athena.RootAsciiDumperAlg("rootdumper") )

# Create ApplicationMgr
app = acc.createApp()
evLoop = PyAthena.py_svc('AthenaEventLoopMgr', iface='IEventProcessor')
evSeek = PyAthena.py_svc('AthenaEventLoopMgr', iface='IEventSeek')

# Initialize/start
app.initialize()
app.start()

# Seek events
evLoop.nextEvent(2)
evSeek.seek(12177); evLoop.nextEvent(evSeek.curEvent()+1)
evSeek.seek(24412); evLoop.nextEvent(evSeek.curEvent()+1)
evSeek.seek(24339); evLoop.nextEvent(evSeek.curEvent()+1)

# Exit cleanly
app.stop()
app.finalize()
app.terminate()
