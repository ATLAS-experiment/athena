# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Exec.MaxEvents = 2
flags.lock()

cfg = MainServicesCfg(flags)
cfg.addEventAlgo(CompFactory.TrigConfMsgAlg())

import sys
sc = cfg.run()
sys.exit( not sc.isSuccess() )
