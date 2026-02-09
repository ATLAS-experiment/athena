#!/bin/bash 
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# FCcmd_test.sh -- FileCatalog command line tool FCcmd test script


# switch to a unique name for the default catalog to avoid clashes
export POOL_CATALOG="testFCcmd.xml"
secondaryFC="testFCcmd.2.xml"
# start fresh
rm -f $POOL_CATALOG $secondaryFC

FCcmd register pfn -p myFile3 -g AAAAAAAA-BBBB-CCCC-DDDD-333333333333 -u $secondaryFC
FCcmd register pfn -p myFile1 -g AAAAAAAA-BBBB-CCCC-DDDD-111111111111
FCcmd list pfn
FCcmd register lfn -p myFile1 -l logical.name.1
FCcmd rename -p myFile1 -n myFile1.apr
FCcmd list pfn
FCcmd register pfn -p myFile2 -g AAAAAAAA-BBBB-CCCC-DDDD-222222222222
FCcmd list lfn
FCcmd list guid
FCcmd list guid -p myFile1.apr
FCcmd list guid -l logical.name.1
FCcmd list lfn -p myFile1.apr
FCcmd list guid -u $secondaryFC

