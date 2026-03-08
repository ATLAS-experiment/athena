#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Run unit tests on SortedCollectionCreator.py
"""

# verbose output to see the collections contents in the log
from AthenaCommon import Logging
Logging.log.setLevel(0)

from CollectionSvc.SortedCollectionCreator import SortedCollectionCreator
sorter = SortedCollectionCreator(name="SortEvents")

# Read test_collection.root (RootCollection) created by ttree_rw_test, sort it and write as TTree
from PyUtils import PoolFile
sorter.execute("test_collection.ttree.root",  outputCollection="PFN:sorted.ttree.root", sortAttribute="attr1", sortOrder="Descending", outputCollectionType=PoolFile.PoolOpts.CollectionType.RootTTreeCollection)

# Read test_collection.rntup (RootCollection) created by rntuple_rw_test, sort it and write as RNTuple
sorter.execute("test_collection.rntup.root",  outputCollection="PFN:sorted.rntup.root", sortAttribute="attr1", sortOrder="Descending", outputCollectionType=PoolFile.PoolOpts.CollectionType.RootRNTupleCollection)
