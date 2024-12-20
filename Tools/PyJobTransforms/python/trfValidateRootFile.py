#!/usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @Package PyJobTransforms.trfValidateRootFile
# @brief Functionality to test a Root file for corruption
# @author atlas-comp-transforms-dev@cern.ch
# @todo The main() CLI should migrate to @c scripts and this module just implement functions


import sys, os
import logging

from PyUtils import RootUtils
ROOT = RootUtils.import_root()
from ROOT import TFile, TTree, TDirectory, TStopwatch
from ROOT.Experimental import RNTupleReader
from PyUtils.PoolFile import isRNTuple

msg = logging.getLogger(__name__)

def checkBranch(branch):

    msg.debug('Checking branch %s ...', branch.GetName())

    nBaskets=branch.GetWriteBasket()

    msg.debug('Checking %s baskets ...', nBaskets)

    for iBasket in range(nBaskets):
        basket=branch.GetBasket(iBasket)
        if not basket:
            msg.warning('Basket %s of branch %s is corrupted.', iBasket, branch.GetName() )
            return 1

    listOfSubBranches=branch.GetListOfBranches()
    msg.debug('Checking %s subbranches ...', listOfSubBranches.GetEntries())
    for subBranch in listOfSubBranches:
        if checkBranch(subBranch)==1:
            return 1

    msg.debug('Branch %s looks ok.', branch.GetName())
    return 0    


def checkTreeBasketWise(tree):

    listOfBranches=tree.GetListOfBranches()

    msg.debug('Checking %s branches ...', listOfBranches.GetEntries())

    for branch in listOfBranches:
        if checkBranch(branch)==1:
            msg.warning('Tree %s is corrupted (branch %s ).', tree.GetName(), branch.GetName())
            return 1

    return 0


def checkTreeEventWise(tree, printInterval = 150000):

    nEntries=tree.GetEntries()

    msg.debug('Checking %s entries ...', nEntries)

    for i in range(nEntries):
        if tree.GetEntry(i)<0:
            msg.warning('Event %s of tree %s is corrupted.', i, tree.GetName())
            return 1

        # Show a sign of life for long validation jobs: ATLASJT-433
        if (i%printInterval)==0 and i>0:
            msg.info('Validated %s events so far ...', i)

    return 0

def checkNTupleEventWise(ntuple, printInterval = 150000):

    try:
        reader=RNTupleReader.Open(ntuple)
    except Exception as err:
        msg.warning('Could not open ntuple %s: %s', ntuple, err)
        return 1

    msg.debug('Checking %s entries ...', reader.GetNEntries())

    for i in reader:
        try:
            entry = reader.GetModel().CreateEntry()
            reader.LoadEntry(i, entry)
        except Exception as err:
            msg.warning('Event %s of ntuple %s is corrupted: %s', i, reader.GetDescriptor().GetName(), err)
            return 1

        # Show a sign of life for long validation jobs: ATLASJT-433
        if (i%printInterval)==0 and i>0:
            msg.info('Validated %s events so far ...', i)

    return 0

def checkNTupleFieldWise(ntuple):
    """Bulk read each top level field cluster by cluster.
    """
    from array import array
    try:
        from ROOT import RException
    except ImportError:
        from ROOT.Experimental import RException

    try:
        reader=RNTupleReader.Open(ntuple)
    except Exception as err:
        msg.warning('Could not open ntuple %r: %r', ntuple, err)
        return 1

    try:
        descriptor = reader.GetDescriptor()
        msg.debug(f"ntupleName={descriptor.GetName()}")

        model = reader.GetModel()
        fieldZero = model.GetFieldZero()
        subFields = fieldZero.GetSubFields()
        msg.debug(f"Top level fields number {subFields.size()}")
        for field in subFields:
            msg.debug(f"fieldName={field.GetFieldName()} typeName={field.GetTypeName()}")
            bulk = model.CreateBulk(field.GetFieldName())

            for clusterDescriptor in descriptor.GetClusterIterable():
                clusterIndex = ROOT.Experimental.RClusterIndex(clusterDescriptor.GetId(), 0)
                size = int(clusterDescriptor.GetNEntries())
                maskReq = array('b', (True for i in range(size)))
                msg.debug(f"    cluster #{clusterIndex.GetClusterId()}"
                          f" firstEntryIndex={clusterDescriptor.GetFirstEntryIndex()}"
                          f" nEntries={size}")
                values = bulk.ReadBulk(clusterIndex, maskReq, size)
                msg.debug(f"        values array at {values}")

    except RException as err:
        from traceback import format_exception
        msg.error("Exception reading ntuple %r\n%s", ntuple, "".join(format_exception(err)))
        return 1

    return 0

def checkDirectory(directory, the_type, requireTree, depth):

    from PyUtils import PoolFile
    nentries = None
    hasMetadata = False

    msg.debug('Checking directory %s ...', directory.GetName())

    listOfKeys=directory.GetListOfKeys()

    msg.debug('Checking %s keys ... ', listOfKeys.GetEntries())

    for key in listOfKeys:

        msg.debug('Looking at key %s ...', key.GetName())
        msg.debug('Key is of class %s.', key.GetClassName())

        the_object=directory.Get(key.GetName())
        if not the_object:
            msg.warning("Can't get object of key %s.", key.GetName())
            return 1

        if requireTree and not isinstance(the_object, TTree):
            msg.warning("Object of key %s is not of class TTree!", key.GetName())
            return 1

        if isinstance(the_object,TTree):

            msg.debug('Checking tree %s ...', the_object.GetName())

            if depth == 0:
                if PoolFile.PoolOpts.TTreeNames.EventData == the_object.GetName():
                    nentries = the_object.GetEntries()
                    msg.debug(f'  contains {nentries} events')
                elif PoolFile.PoolOpts.TTreeNames.MetaData == the_object.GetName():
                    hasMetadata = True
                    msg.debug('  contains MetaData')
            
            if the_type=='event':
                if checkTreeEventWise(the_object)==1:
                    return 1
            elif the_type=='basket':    
                if checkTreeBasketWise(the_object)==1:
                    return 1

            msg.debug('Tree %s looks ok.', the_object.GetName())    

        if isRNTuple(the_object):

            msg.debug('Checking ntuple of key %s ...', key.GetName())

            try:
                reader=RNTupleReader.Open(the_object)
            except Exception as err:
                msg.warning('Could not open ntuple %s: %s', the_object, err)
                return 1

            if depth == 0:
                if PoolFile.PoolOpts.RNTupleNames.EventData == reader.GetDescriptor().GetName():
                    nentries = reader.GetNEntries()
                    msg.debug(f'  contains {nentries} events')
                elif PoolFile.PoolOpts.RNTupleNames.MetaData == reader.GetDescriptor().GetName():
                    hasMetadata = True
                    msg.debug('  contains MetaData')

            if the_type=='event':
                if checkNTupleEventWise(the_object)==1:
                    return 1
            elif the_type=='basket':
                if checkNTupleFieldWise(the_object)==1:
                    return 1

            msg.debug('NTuple of key %s looks ok.', key.GetName())
            
        if isinstance(the_object, TDirectory):
            if checkDirectory(the_object, the_type, requireTree, depth + 1)==1:
                return 1

    # Only check if metadata object is available as in standard POOL files
    if depth == 0 and hasMetadata and checkNEvents(directory.GetName(), nentries)==1:
        return 1
    else:
        msg.debug('Directory %s looks ok.', directory.GetName())
        return 0


def checkFile(fileName, the_type, requireTree):

    msg.info('Checking file %s ...', fileName)

    isIMTEnabled = ROOT.ROOT.IsImplicitMTEnabled()
    if not isIMTEnabled and 'TRF_MULTITHREADED_VALIDATION' in os.environ and 'ATHENA_CORE_NUMBER' in os.environ:
        nThreads = int(os.environ['ATHENA_CORE_NUMBER'])
        msg.info(f"Setting the number of implicit ROOT threads to {nThreads}")
        ROOT.ROOT.EnableImplicitMT(nThreads)

    file_handle=TFile.Open(fileName)

    if not file_handle:
        msg.warning("Can't access file %s.", fileName)
        return 1

    if not file_handle.IsOpen():
        msg.warning("Can't open file %s.", fileName)
        return 1

    if file_handle.IsZombie():
        msg.warning("File %s is a zombie.", fileName)
        file_handle.Close()
        return 1

    if file_handle.TestBit(TFile.kRecovered):
        msg.warning("File %s needed to be recovered.", fileName)
        file_handle.Close()
        return 1

    if checkDirectory(file_handle, the_type, requireTree, 0)==1:
        msg.warning("File %s is corrupted.", fileName)
        file_handle.Close()
        return 1

    file_handle.Close()
    msg.info("File %s looks ok.", fileName)

    if not isIMTEnabled and 'TRF_MULTITHREADED_VALIDATION' in os.environ and 'ATHENA_CORE_NUMBER' in os.environ:
        ROOT.ROOT.DisableImplicitMT()

    return 0


def checkNEvents(fileName, nEntries):
    """Check consistency of number of events in file with metadata.

    fileName   name of file to check consistency of
    nEntries   number of events in fileName (e.g., obtained by examining event data object)
    return     0 in case of consistency, 1 otherwise
    """
    from PyUtils.MetaReader import read_metadata

    msg.debug('Checking number of events in file %s ...', fileName)

    meta = read_metadata(fileName, mode='lite')[fileName]
    msg.debug('  according to metadata: {0}'.format(meta["nentries"]))
    msg.debug('  according to event data: {0}'.format(nEntries))
    if meta["nentries"] and nEntries and meta["nentries"] != nEntries \
       or meta["nentries"] and not nEntries \
       or not meta["nentries"] and nEntries:
        msg.warning(f'  number of events ({nEntries}) inconsistent with metadata ({meta["nentries"]}) in file {fileName!r}.')
        return 1
    else:
        msg.debug("  looks ok.")
        return 0

def usage():
    print("Usage: validate filename type requireTree verbosity")
    print("'type'  must be either 'event' or 'basket'")
    print("'requireTree' must be either 'true' or 'false'")
    print("'verbosity' must be either 'on' or 'off'")

    return 2


def main(argv):

    clock=TStopwatch()
    
    argc=len(argv)

    if (argc!=5):
        return usage()

    fileName=argv[1]
    the_type=argv[2]
    requireTree=argv[3]
    verbosity=argv[4]


    if the_type!="event" and the_type!="basket":
        return usage()

    if requireTree=="true":
        requireTree=True
    elif requireTree=="false":
        requireTree=False
    else:
        return usage()

    if verbosity=="on":
        msg.setLevel(logging.DEBUG)
    elif verbosity=="off":
        msg.setLevel(logging.INFO)
    else:
        return usage()
  
    rc=checkFile(fileName,the_type, requireTree)
    msg.debug('Returning %s', rc)
    
    clock.Stop()
    clock.Print()

    return rc

    
if __name__ == '__main__':                

    ch=logging.StreamHandler(sys.stdout)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    msg.addHandler(ch)

    rc=main(sys.argv)
    sys.exit(rc)
    
