# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# This module defines mapping rules from nominal Phase-I L1Calo thresholds
# (target 50% efficiency point) to the actual cut that will be applied
# to the TOB ET. This allows tuning of the thresholds transparently via
# changes to the L1 menu, without adjusting the names of chains.
#
# The threshold mapping should be used for both multiplicity threshold
# definitions and for the threshold cuts on TOBs in topo algorithms.

threshold_mapping = {
    'eEM': { # TODO: Update when eFEX EM calibrations are applied
        # in pp menu (doHeavyIonTobThresholds=False) ptMinToTopo value is assigined to eEM1 and eEM2
        1:1.4,
        2:2,
        5:3.5,
        7:5.5,
        9:7,
        10:8,
        12:10,
        15:13,
        18:16,
        22:20,
        24:22,
        26:25,
        28:27,
    },
    'jEM': {
        20:15,
    },
    'eTAU': {
        12:8,
        20:12,
        30:20,
        35:25,
        40:30,
        60:40,
        80:60,
        140:100,
    },
    'jTAU': {
        20:12,
        30:20,
    },
    'cTAU': {
        12:8,
        20:12,
        30:20,
        35:25,
    },
    'jJ': {
        15:15,
        20:15,
        30:15,
        40:25,
        50:38,
        55:44,
        60:49,
        70:61,
        80:72,
        85:79,
        90:85,
        100:98,
        125:129,
        140:146,
        160:174,
        180:199,
        500:400,
    },
    'gJ':
    {
        20:20,
        30:30,
        40:40,
        50:50,
        100:100,
        160:160,
        400:400,
    },
    'jLJ':
    {
        60:60,
        80:80,
        100:100,
        120:120,
        140:140,
        160:160,
        180:180,
        200:200,
    },
    'gLJ':
    {
        80:80,
        100:100,
        140:140,
        160:160,
    },
    'jXE':
    {
        60:30,
        70:35,
        80:40,
        90:45,
        100:50, 
        110:55,
        120:60,
        500:300,
    },
    'gXENC':
    {
        70:30,
        80:40,
        100:50,
    },
#    'gXERHO':
#    {
#        70:30,
#        80:40,
#        100:50,
#    },
    'gXEJWOJ':
    {
        60:30,
        70:35, #ATR-28679: switched from 70:30, to 70:35 for adding 60:30 
        80:40,
        100:50,
        110:55,
        120:60,
        500:300, 
    },
}

def get_threshold_cut(threshold_type,threshold_val):
    # To support more generality in topo alg configs
    if threshold_val == 0:
        return 0
    # Handle prefix for different jet eta regions in L1Topo inputs
    # e.g. jJ, CjJ, FjJ, AjJ
    # May need to extend for other list types
    _threshold_type = threshold_type
    if 'jJ' in threshold_type and threshold_type[0] in ['A','C','F']:
        _threshold_type = threshold_type [1:]
    return threshold_mapping[_threshold_type][threshold_val]
