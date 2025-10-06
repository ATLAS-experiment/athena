#! /usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__author__ = "Marcin Nowak"
__doc__ = """
Test for the SortedCollectionCreator
"""

from AthenaCommon import Logging, Constants
Logging.log.setLevel(Constants.DEBUG)

import ROOT
pool = ROOT.pool
collSvc = pool.CollectionService()
collSvc.setMessageSvcQuiet()

primaryColl = { 'name' : 'EventNumber',  'type' : 'unsigned long' }
sampleCollName = 'sample_apr_collection'
desc = pool.CollectionDescription(sampleCollName, 'RootCollection')
desc.insertColumn( primaryColl['name'], primaryColl['type'] )

# Create a small collection to serve as input to the Sorter
coll = collSvc.create(desc)
row = pool.CollectionRowBuffer()
coll.initNewRow( row )
for ev in range(1000, 0, -200):
    row.attributeList()[primaryColl['name']].setValue[primaryColl['type']](ev)
    coll.insertRow( row )
coll.commit()
coll.close()

# Sort the example collection and write both TTree and RNTuple based sorted collections
outputCollNameTree =  "collection.tree"
outputCollNameRNTup = "collection.rntuple"
sampleCollName += '.root'
from CollectionUtilities.SortedCollectionCreator import SortedCollectionCreator
sorter = SortedCollectionCreator(name="SortEvents")
sorter.execute( sampleCollName, sortAttribute = primaryColl['name'],
                outputCollection = outputCollNameTree, outputCollectionType="RootCollection" )
sorter.executeInSubprocess( sampleCollName, sortAttribute = primaryColl['name'],
                outputCollection = outputCollNameRNTup, outputCollectionType="RNTCollection" )

# Read the collections in VERBOSE mode to see the content
Logging.log.setLevel(Constants.VERBOSE)
sorter.readInputCollections( [sampleCollName] )
sorter.readInputCollections( [outputCollNameTree+'.root'] )
sorter.readInputCollections( [outputCollNameRNTup+'.root'] )
