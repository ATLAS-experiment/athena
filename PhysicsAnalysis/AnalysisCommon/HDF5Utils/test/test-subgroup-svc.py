#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesMiniCfg
import sys

flags = initConfigFlags()
flags.lock()

acc = MainServicesMiniCfg(flags)
acc.setAppProperty('ExtSvcCreates', 'True', overwrite=True)

root_svc = CompFactory.H5FileSvc(path="test_groups.h5")
acc.addService(root_svc, primary=True)

svc_a = CompFactory.SubgroupSvc("SvcA", subgroup="group_a")
svc_a.parent = root_svc
acc.addService(svc_a)

svc_b = CompFactory.SubgroupSvc("SvcB", subgroup="subgroup_b")
svc_b.parent = svc_a
acc.addService(svc_b)

svc_c = CompFactory.SubgroupSvc("SvcC", subgroup="group_c")
svc_c.parent = root_svc
acc.addService(svc_c)

sc = acc.run(0)
sys.exit(not sc.isSuccess())
