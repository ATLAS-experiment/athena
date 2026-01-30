#
# $Id$
#
# File: share/xAODRootTestRead2_t.py
# Author: snyder@bnl.gov
# Date: Jun 2014
# Purpose: Test reading xAOD objects directly from root.
#

import ROOT
import cppyy


from AthenaCommon.Include import Include
include = Include(show = False)
include('DataModelRunTests/xAODRootTest.py')

xAODInit()
ana = Analysis('xaodroot.root')
ana.add (xAODTestRead(False))
ana.add (xAODTestRead(False, readPrefix = 'copy_'))
ana.run()

