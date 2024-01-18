#!/usr/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

## @Package PyJobTransforms.trfValidateRootFile
# @brief Functionality to test a Root file for corruption
# @author atlas-comp-transforms-dev@cern.ch
# @todo The main() CLI should migrate to @c scripts and this module just implement functions


import sys, os
import logging

from PyUtils import RootUtils
ROOT = RootUtils.import_root()
from ROOT import TFile, TTree, TDirectory, TStopwatch
from ROOT.Experimental import RNTuple, RNTupleReader

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
    except BaseException as err:
        msg.warning('Could not open ntuple %s: %s', ntuple, err)
        return 1

    msg.debug('Checking %s entries ...', reader.GetNEntries())

    for i in reader:
        try:
            reader.LoadEntry(i)
        except BaseException as err:
            msg.warning('Event %s of ntuple %s is corrupted: %s', i, reader.GetDescriptor().GetName(), err)
            return 1

        # Show a sign of life for long validation jobs: ATLASJT-433
        if (i%printInterval)==0 and i>0:
            msg.info('Validated %s events so far ...', i)

    return 0

def checkNTuplePageWise(ntuple):
    """Check each page column by column.
    """

    try:
        pageSource = ntuple.MakePageSource()
        pageSource.Attach()
    except Exception as err:
        msg.warning(f'Could not make page source for ntuple {ntuple!r}: {err}')
        return 1

    msg.debug(f'Checking pages of ntuple {pageSource.GetNTupleName()!r}')
    descriptor=pageSource.GetSharedDescriptorGuard().GetRef()
    for columnDesc in descriptor.GetColumnIterable():
        columnPhysicalId = columnDesc.GetPhysicalId()
        msg.debug(f"  Checking column {columnPhysicalId} ...")
        for cluster in descriptor.GetClusterIterable():
            if not cluster.ContainsColumn(columnPhysicalId):
                msg.debug(f"    Not in cluster {cluster.GetId()}")
                continue
            nElements = cluster.GetColumnRange(columnPhysicalId).fNElements.fValue
            idxInCluster = 0
            for (pageNo, pageInfo) in enumerate(cluster.GetPageRange(columnPhysicalId).fPageInfos):
                msg.debug(f"    Page {pageNo}, {pageInfo.fNElements} elements in cluster {cluster.GetId()}")
                buffer = bytearray(pageInfo.fLocator.fBytesOnStorage)
                sealedPage = ROOT.Experimental.Detail.RPageStorage.RSealedPage(buffer,\
                                                                               pageInfo.fLocator.fBytesOnStorage,\
                                                                               pageInfo.fNElements)
                try:
                    pageSource.LoadSealedPage(columnPhysicalId,\
                                              ROOT.Experimental.RClusterIndex(cluster.GetId(), idxInCluster),\
                                              sealedPage)
                except Exception as err:
                    msg.warning(f'Could not load SealedPage {sealedPage!r} for page {pageNo} in cluster {cluster.GetId()}'
                                f' of ntuple {pageSource.GetNTupleName()!r}: {err}')
                    return 1
                idxInCluster += pageInfo.fNElements
            if idxInCluster == nElements:
                msg.debug(f"    {nElements} elements in cluster {cluster.GetId()}")
            else:
                msg.warning(f"NTuple {descriptor.GetName()!r}, cluster {cluster.GetId()}, column {cluster.GetColumnRange(columnPhysicalId).fPhysicalColumnId}: inconsistent meta-data")
                return 1

    return 0

def checkDirectory(directory, the_type, requireTree):

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
            
            if the_type=='event':
                if checkTreeEventWise(the_object)==1:
                    return 1
            elif the_type=='basket':    
                if checkTreeBasketWise(the_object)==1:
                    return 1

            msg.debug('Tree %s looks ok.', the_object.GetName())    

        if isinstance(the_object,RNTuple):

            msg.debug('Checking ntuple of key %s ...', key.GetName())

            if the_type=='event':
                if checkNTupleEventWise(the_object)==1:
                    return 1
            elif the_type=='basket':
                if checkNTuplePageWise(the_object)==1:
                    return 1

            msg.debug('NTuple of key %s looks ok.', key.GetName())
            
        if isinstance(the_object, TDirectory):
            if checkDirectory(the_object, the_type, requireTree)==1:
                return 1

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

    if checkDirectory(file_handle, the_type, requireTree)==1:
        msg.warning("File %s is corrupted.", fileName)
        file_handle.Close()
        return 1

    file_handle.Close()
    msg.info("File %s looks ok.", fileName)

    if not isIMTEnabled and 'TRF_MULTITHREADED_VALIDATION' in os.environ and 'ATHENA_CORE_NUMBER' in os.environ:
        ROOT.ROOT.DisableImplicitMT()

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
    
