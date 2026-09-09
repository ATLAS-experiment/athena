#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
import sys

flags = initConfigFlags()
flags.lock()

acc = MainServicesCfg(flags)

root_svc = CompFactory.H5FileSvc(path="test_must_new.h5")
acc.addService(root_svc, primary=True, create=True)

svc_first = CompFactory.SubgroupSvc("SvcFirst", subgroup="grp")
svc_first.parent = root_svc
acc.addService(svc_first, create=True)

# mustBeNew=True on a group that SvcFirst just created -> must fail
svc_dup = CompFactory.SubgroupSvc("SvcDup", subgroup="grp")
svc_dup.parent = root_svc
svc_dup.mustBeNew = True
acc.addService(svc_dup, create=True)

sc = acc.run(0)
# Pass the ctest when the run fails (as expected):
sys.exit(sc.isSuccess())
