#
# File: share/xAODRootTestRead_t.py
# Author: snyder@bnl.gov
# Date: Jun 2014
# Purpose: Test reading xAOD objects directly from root.
#

import ROOT
import cppyy

ROOT.xAOD.TEvent


from AthenaCommon.Include import Include
include = Include(show = False)
include('DataModelRunTests/xAODRootTest.py')

is_rntuple = xAODInit()
if is_rntuple:
    ana = Analysis('xaoddata.rntup.root', is_rntuple = is_rntuple)
else:
    ana = Analysis('xaoddata.root', 'xaodroot.root')
ana.add (xAODTestRead(is_rntuple))
if not is_rntuple:
    # Copy/write doesn't work yet with RNTuple.
    ana.add (xAODTestCopy(writePrefix='copy_'))
ana.add (xAODTestDecor(decorName = 'dint2', offset=600))
##ana.add (xAODTestPDecor(decorName = 'dpint3', offset=700))
ana.run()
ana.finalize()
