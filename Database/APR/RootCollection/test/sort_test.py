#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Run unit tests on SortedCollectionCreator.py
"""

# verbose output to see the collections contents in the log
from AthenaCommon import Logging
Logging.log.setLevel(0)

from CollectionSvc.SortedCollectionCreator import SortedCollectionCreator
sorter = SortedCollectionCreator(name="SortEvents")

# Read test_collection.root (RootCollection) created by ttree_rw_test, sort it and write as RootCollection
sorter.execute("test_collection.root",  outputCollection="PFN:sorted.root", sortAttribute="attr1", sortOrder="Descending")

# Read test_collection.rntup (RNTCollection) created by rntuple_rw_test, sort it and write as RNTCollection
sorter.execute("test_collection.rntup",  outputCollection="PFN:sorted.rntup", sortAttribute="attr1", sortOrder="Descending", outputCollectionType="RNTCollection")


