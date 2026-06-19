#! /usr/bin/env python

# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import os
# from ROOT import PathResolver
from PathResolver import PathResolver


def getScalefactor(tool_options):

    print('EgEfficiencyCorr_testEgEfficiencyCorrWithoutFile '+tool_options+' 2>&1')
    result = os.popen('EgEfficiencyCorr_testEgEfficiencyCorrWithoutFile ' +
                      tool_options+' 2>&1').read().strip()
    print(result)
    return result


stype = 'FullSim'
runno = 428648
model = 'TOTAL'
pT = 41212.1
eta = -0.94
other = '| grep SF'
eleid = 'MediumLH'

rangeofsim = ['FullSim'] # ,'AtlFast2'] # to be added later
rangeofrunno =  [runno] # 270000, 300000, 334320, 352183] # other runno to be added later

thesfs = set()
# first check: go through correlation models and see if the results are the same
print(' ==== CHECK I:  CORRELATION MODELS ==== ')
# we don't test COMBMCTOYS because it's subject to stat fluctuations
for thismodel in [model, 'SIMPLIFIED', 'FULL']:
    sf = getScalefactor(' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' %
                        (stype, runno, thismodel, eta, pT, eleid, other)).split(':')[5]
    if sf not in thesfs:
        thesfs.add(sf)
if len(thesfs) > 1:
    print('--------------------------------------------------')
    print(' the uncertainty models yield inconsistent results ', thesfs)
    print('--------------------------------------------------')
    exit()

# second check: check that are map file doesn't contain duplicates
themap = getScalefactor(' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' %
                        (stype, runno, model, eta, pT, eleid, '-l 0 | grep map')).split("'")[1]
thekeys = set()
print(' ==== CHECK II:  DEFAULT MAP ==== ')
for line in open(PathResolver.FindCalibFile(themap)).readlines():
    key = line.rstrip().split('=')[0]
    if key not in thekeys:
        thekeys.add(key)
    else:
        print('--------------------------------------------------')
        print(' there are duplicate keys in your map ', key)
        print('--------------------------------------------------')
        exit()
print('    ---> map looks good! ')

# third check: print all ID levels
print(' ==== CHECK III:  ID LVL ==== ')
for thisid in ['MediumLH', 'LooseBLayerLH', 'TightLH']:
    getScalefactor(' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' %
                   (stype, runno, model, eta, pT, thisid, other))

# fourth check: check a couple of pT, eta, runno and make sure they're different
print(' ==== CHECK IV:  PT/ETA/RUN ==== ')
for thissim in rangeofsim:
    for thisrunno in rangeofrunno :
        for thispT in [7421.4, pT, 12128482.9]:
            for thiseta in [-2.42, 0.94]:
                flags = (thissim, thisrunno, model,
                         thiseta, thispT, eleid, other)
                sf = getScalefactor(
                    ' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' % flags).split(':')[5]
                if sf not in thesfs:
                    thesfs.add(sf)
                else:
                    # we don't exit since this can be on purpose
                    print(
                        " we got a duplicate scale factor! "
                        "are you sure it\'s supposed to be there?")

# fifth check: also run a couple of reco, iso and trigger scale factors
print(' ==== CHECK IV:  RECO+ISO ==== ')
getScalefactor(' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' %
               (stype, runno, model, eta, pT, 'Reconstruction', other))
for thisother in [
    '-i Tight_VarRad'
]:
    for thissim in rangeofsim:
        for thisrunno in rangeofrunno :
            flags = (thissim, thisrunno, model, eta,
                     pT, "TightLH", thisother+other)
            getScalefactor(
                ' -t %s -r %d -c %s -e %1.2f -p %1.2f -d %s %s ' % flags)

print(' ==== DONE ==== ')
