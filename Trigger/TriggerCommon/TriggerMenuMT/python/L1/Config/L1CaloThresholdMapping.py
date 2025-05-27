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
        20:18,
        22:20,
        24:22,
        26:25,
        28:27,
        40:39,
    },
    'jEM': {
        20:14,
        25:20,
        35:30, # prospective Run 4 L1 item, ATR-30180
    },
    'eTAU': {
        # in pp menu (doHeavyIonTobThresholds=False) ptMinToTopo value is assigined to eTAU1
        1:1.4,
        12:7.7,
        20:10.1,
        30:17.7,
        35:23.2,
        40:29,
        50:35,
        55:37.5,
        60:40,
        70:50,
        80:60,
        140:100,
    },
    'jTAU': {
        # in pp menu (doHeavyIonTobThresholds=False) ptMinToTopo value is assigined to jTAU1
        1:1.4,
        20:10.1,
        30:17.7,
    },
    'cTAU': {
        12:7.7,
        20:10.1,
        30:17.7,
        35:23.2,
        50:35,
        55:37.5,
    },
    'jJ': {
        5:5,
        10:10,
        15:15,
        20:21,
        30:26,
        40:27,
        50:31,
        55:41,
        60:60,
        80:65,
        90:81,
        125:121,
        140:137,
        160:158,
        180:195,
        # Must be 400 to ensure threshold can be passed
        500:400,
    },
    'CjJ': { # 0ETA2[1,3,5]
        20:21,
        30:22,
        40:25,
        50:33,
        55:39,
        56:40, # prospective Run 4 L1 item, ATR-30180
        60:56,
        70:54,
        80:64,
        85:70,
        90:84,
        100:98,
    },
    'SCjJ': { # 0ETA2[1,3,5], mostly a copy of CjJ but decoupled to allow for independent configuration of L1Topo's SimpleCone algorithm
        10:15,
        20:21,
        30:22,
        40:25,
        50:33,
        55:39,
        60:56,
        70:54,
        80:64,
        85:70,
        90:84,
        100:98,
    },
    'FjJ': { # 30ETA49
        5:5,
        10:10,
        15:15,
        20:20,
        40:26,
        50:32,
        60:49,
        90:83,
        125:134,
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
        80:50,
        90:60, # prospective Run 4 L1 item, ATR-30180
        100:70,
        140:110,
        160:130,
    },
    'jXE':
    {
        60:30,
        70:34,
        80:38,
        90:45,
        100:48, 
        110:52,
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
    # For jJ thresholds, there are different maps for:
    # - jJ: 0-3.2 in eta
    # - CjJ: 0-2.X (X=1,3,5) in eta
    # - FjJ: 3.0-4.9 in eta
    # - AjJ: 0-4.9 in eta uses the default range (L1Topo triggers)
    _threshold_type = threshold_type
    if threshold_type == 'AjJ':
        _threshold_type = 'jJ'
    return threshold_mapping[_threshold_type][threshold_val]
