# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os
pbeastDefaultServer = 'https://pc-atlas-www.cern.ch' if os.getenv('PBEAST_SERVER_HTTPS_PROXY', '').startswith('atlasgw') else 'https://atlasop.cern.ch'
pbeastServer = os.getenv('PBEAST_SERVER', pbeastDefaultServer)

import libpbeastpy; pbeast = libpbeastpy.ServerProxy(pbeastServer)
import logging; log = logging.getLogger("DCSCalculator2.variable")

from itertools import chain, combinations

from DQUtils.events import process_iovs
from DQUtils.general import timer
from DQUtils.sugar import IOVSet, RANGEIOV_VAL, RunLumi, TimestampType, define_iov_type, make_iov_type

from DCSCalculator2 import config
from DCSCalculator2.consts import RED, YELLOW
from DCSCalculator2.libcore import map_channels
from DCSCalculator2.subdetector import DCSC_DefectTranslate_Subdetector
from DCSCalculator2.variable import DefectIOV, GoodIOV, DCSC_Variable

class DCSC_Variable_With_Mapping(DCSC_Variable):
    def __init__(self, folder, evaluator, *, mapping=None, **kwargs):
        super().__init__(folder, evaluator, **kwargs)
        self.mapping = mapping
    
    def read(self, query_range, folder_base, folder_name):
        result = super().read(query_range, folder_base, folder_name)
        if self.mapping is not None:
            result = map_channels(result, self.mapping, folder_name)
        return result

class DCSC_Merged_Variable(DCSC_Variable):
    def __init__(self, folders, evaluator, *, mapping={}, **kwargs):
        folder_merge = ','.join(folders)
            
        super().__init__(folder_merge, evaluator, **kwargs)
        self.folder_names = folders
        self.mapping = mapping
    
    def get_variable(self, name):
        for var in self.variables:
            if var.folder_name == name:
                return var
        raise RuntimeError("Folder '%s' not found" % name)

    def read(self, query_range, folder_base, folder_names):
        var_iovs = []
        folders = folder_names.split(',')

        for folder in folders:
            iovs = super().read(query_range, folder_base, folder)
            if folder in self.mapping:
                iovs = map_channels(iovs, self.mapping[folder], folder)
            var_iovs.append(iovs)

        iovs = self.merge_input_variables(*var_iovs)

        if config.opts.check_input_time:
            self.print_time_info(iovs)
            
        if log.isEnabledFor(logging.INFO):
            input_hash = hash(iovs)
            self.input_hashes.append(input_hash)
            #log.info("  -> Input hash: % 09x (len=%i)", input_hash, len(iovs))
            
        return iovs

    def merge_input_variables(self, *inputs):
        type_bases = [iovs.iov_type for iovs in inputs]
        base_names = [base.__name__[:-4] for base in type_bases]
        attributes = [base.lower() for base in base_names]
        clazz = make_iov_type('MERGER_OF_' + '_AND_'.join(base_names), tuple(['channel'] + attributes))

        inputs_by_channel = [iovs.by_channel for iovs in inputs]
        all_channels = sorted(set(y for x in inputs_by_channel for y in x.keys()))

        result = []
        for channel in all_channels:
            c_inputs = [x[channel] for x in inputs_by_channel]
            result.extend(iov for iov in self.merge_inputs(clazz, channel, *c_inputs))

        return IOVSet(result, iov_type=clazz, origin=self.folder_names)

    def merge_inputs(self, clazz, channel, *inputs):
        result = [clazz(since, until, channel, *states) for since, until, states in process_iovs(*inputs)]
        return IOVSet(result, iov_type=clazz, origin=self.folder_names)

@define_iov_type
def PBeastIOV(channel, value):
    "Stores the value of an object's attribute queried from pBeast"

class TDAQC_Variable(DCSC_Variable):
    """
    A variable which reads data from pBeast.
    """
    TIME_RATIO = 1e3

    @staticmethod
    def timeCOOL2PBeast(timestamp):
        return int(timestamp/TDAQC_Variable.TIME_RATIO)
    
    @staticmethod
    def timePBeast2COOL(timestamp):
        return TimestampType(timestamp*TDAQC_Variable.TIME_RATIO)

    def __init__(self, query, evaluator, *, regex=False, mapping=dict(), force_mapping=False, empty_value=None):
        super().__init__(query, evaluator)
        self.regex = regex
        self.mapping = mapping
        self.force_mapping = force_mapping
        self.empty_value = empty_value
        self.query = query
        self.partition, self.className, self.attribute, self.path = query.split('.', 3)
        self.input_hashes = []
    
    def __repr__(self):
        return f"<TDAQCVariable {self.query}>"
    
    def read(self, query_range, query, *, regex=False):
        """
        Read the relevant data from pBeast for this variable, and convert them to COOL-like format
        """
        partition, className, attribute, path = query.split('.', 3)
        
        log.info(f"Querying pBeast object{'s using regex' if regex else ''} {query}")

        since, until = query_range
        since, until = TDAQC_Variable.timeCOOL2PBeast(since), TDAQC_Variable.timeCOOL2PBeast(until)
        
        query_data = pbeast.get_data(partition, className, attribute, path, regex, since, until)
        data = dict.fromkeys(self.mapping.keys()) if self.force_mapping else dict()
        if query_data:
            data.update(query_data[0].data)

        def instantiate(since, until, channel, value):
            if isinstance(value, list):
                value = tuple(value)
            return PBeastIOV(TDAQC_Variable.timePBeast2COOL(since), TDAQC_Variable.timePBeast2COOL(until), channel, value)

        iovs = []
        for channel, entries in data.items():
            if not entries:
                iovs.append(PBeastIOV(*query_range, channel, self.empty_value))
                continue
            first = entries[0]
            since = first.ts
            value = first.value
            for point in entries:
                if point.value == value:
                    continue
                iovs.append(instantiate(since, point.ts, channel, value))
                since = point.ts
                value = point.value
            last = entries[-1]
            iovs.append(instantiate(since, last.ts, channel, value))
        iovs = IOVSet(iovs, iov_type=PBeastIOV, origin=query)
            
        if log.isEnabledFor(logging.INFO):
            input_hash = hash(iovs)
            self.input_hashes.append(input_hash)
            log.info("  -> Input hash: % 09x (len=%i)", input_hash, len(iovs))
            
        return iovs

    def calculate_good_iovs(self, lbtime, subdetector):
        """
        Calculate LB-wise "good" states
        """

        self.subdetector = subdetector
        
        since, until = lbtime.first, lbtime.last
        if self.timewise_folder:
            query_range = RANGEIOV_VAL(since.since, until.until)
        else:
            query_range = RANGEIOV_VAL(RunLumi(since.Run, since.LumiBlock), 
                                       RunLumi(until.Run, until.LumiBlock))
        
        # Read the database
        iovs = self.read(query_range, self.query, regex=self.regex)
        # Decide the states of the input iovs
        iovs = self.make_good_iovs(iovs)
        # Apply a mapping for input channels if necessary
        iovs = self.map_input_channels(iovs)

        if self.timewise_folder and not config.opts.timewise:
            # we might already know the defect mapping
            with timer("Quantize %s (%i iovs over %i lbs)" % 
                       (self.query, len(iovs), len(lbtime))):
                # Quantize to luminosity block
                iovs = self.quantize(lbtime, iovs)

        self.iovs = iovs
        return self

    def make_good_iov(self, iov):
        """
        Determine if one input iov is good.
        """
        giov = GoodIOV(iov.since, iov.until, self.mapping.get(iov.channel, iov.channel), self.evaluator(iov))
        giov._orig_iov = iov
        return giov
    
class TDAQC_Multi_Channel_Variable(TDAQC_Variable):
    def make_good_iov(self, iov):
        """
        Determine if channels in one input iov are good.
        """
        giov = []
        for channel, goodness in self.evaluator(iov):
            current = GoodIOV(iov.since, iov.until, channel, goodness)
            current._orig_iov = iov
            giov.append(current)
        return giov
    
    def make_good_iovs(self, iovs):
        """
        Determine whether each iov signifies a good or bad state.
        """
        results = []
        for iov in iovs:
            results.append(self.make_good_iov(iov))
        return IOVSet(sum(zip(*results), ())) # Sort by channel first

class TDAQC_Bit_Flag_Variable(TDAQC_Multi_Channel_Variable):
    def make_good_iov(self, iov):
        """
        Determine if channels in one input iov are good.
        """
        giov = []
        for bit, channel in self.mapping.get(iov.channel, dict()).items():
            iov_value = (((iov.value >> bit) & 1) == 1) if iov.value is not None else None
            test_iov = PBeastIOV(iov.since, iov.until, channel, iov_value)
            current = GoodIOV(iov.since, iov.until, channel, self.evaluator(test_iov))
            current._orig_iov = iov
            giov.append(current)
        return giov
        
class TDAQC_Array_Variable(TDAQC_Multi_Channel_Variable):
    def make_good_iov(self, iov):
        """
        Determine if channels in one input iov are good.
        """
        giov = []
        for index, channel in self.mapping.get(iov.channel, dict()).items():
            iov_value = iov.value[index] if iov.value is not None else None
            test_iov = PBeastIOV(iov.since, iov.until, channel, iov_value)
            current = GoodIOV(iov.since, iov.until, channel, self.evaluator(test_iov))
            current._orig_iov = iov
            giov.append(current)
        return giov

# DCS channels
A_FAR_GARAGE, A_NEAR_GARAGE, C_FAR_GARAGE, C_NEAR_GARAGE = 101, 105, 109, 113
A_FAR_SIT_HV, A_NEAR_SIT_HV, C_FAR_SIT_HV, C_NEAR_SIT_HV =   1,   5,   9,  13
A_FAR_SIT_LV, A_NEAR_SIT_LV, C_FAR_SIT_LV, C_NEAR_SIT_LV =  21,  25,  29,  33
A_FAR_TOF_HV,                C_FAR_TOF_HV                =  51,       59
A_FAR_TOF_LV,                C_FAR_TOF_LV                =  71,       79

# TDAQ channels
TTC_RESTART        = 1000
STOPLESSLY_REMOVED = 5000

A_FAR_SIT_DISABLED, A_NEAR_SIT_DISABLED, C_FAR_SIT_DISABLED, C_NEAR_SIT_DISABLED = 1001, 1005, 1009, 1013
A_FAR_TOF_DISABLED,                      C_FAR_TOF_DISABLED                      = 1051,       1059

# Channel groups
GARAGE = [A_FAR_GARAGE, A_NEAR_GARAGE, C_FAR_GARAGE, C_NEAR_GARAGE]
SIT_HV = [A_FAR_SIT_HV, A_NEAR_SIT_HV, C_FAR_SIT_HV, C_NEAR_SIT_HV]
SIT_LV = [A_FAR_SIT_LV, A_NEAR_SIT_LV, C_FAR_SIT_LV, C_NEAR_SIT_LV]
TOF_HV = [A_FAR_TOF_HV,                C_FAR_TOF_HV]
TOF_LV = [A_FAR_TOF_LV,                C_FAR_TOF_LV]

SIT_DISABLED = [A_FAR_SIT_DISABLED, A_NEAR_SIT_DISABLED, C_FAR_SIT_DISABLED, C_NEAR_SIT_DISABLED]
TOF_DISABLED = [A_FAR_TOF_DISABLED,                      C_FAR_TOF_DISABLED]

# Thresholds
SIT_HV_DEAD_BAND = 0.05
TOF_HV_DEAD_BAND = 0.90

SIT_LV_CURRENT_LOW, SIT_LV_CURRENT_HIGH = 0.4, 0.6
TOF_HV_CURRENT_LOW                      = 600
TOF_LV_CURRENT_LOW                      = 1

def mapChannels(*mapseqArgs):
    return dict(chain(*[zip(channels, range(defectChannel, defectChannel + len(channels))) for channels, defectChannel in mapseqArgs]))

def mapTranslatorCounts(countMap):
    return {channel: range(channel, channel + count) for count, channels in countMap.items() for channel in channels}

def remove_None(value, default):
    return value if value is not None else default

class AFP(DCSC_DefectTranslate_Subdetector):
    folder_base = '/AFP/DCS'
    variables = [
        #######################################################################
        # DCS Defects
        #######################################################################

        # AFP_(A|C)_(FAR|NEAR)_IN_GARAGE
        DCSC_Variable_With_Mapping(
            'STATION',
            lambda iov: iov.inphysics is True,
            mapping = {1: C_FAR_GARAGE, 2: C_NEAR_GARAGE, 3: A_FAR_GARAGE, 4: A_NEAR_GARAGE}
        ),

        # AFP_(A|C)_(FAR|NEAR)_SIT_(PARTIALLY|NOT)_OPERATIONAL_LV
        DCSC_Variable_With_Mapping(
            'SIT/LV',
            lambda iov: SIT_LV_CURRENT_LOW <= remove_None(iov.current, 0) <= SIT_LV_CURRENT_HIGH,
            mapping = mapChannels(
                ([ 9, 10, 11, 12], A_FAR_SIT_LV ),
                ([13, 14, 15, 16], A_NEAR_SIT_LV),
                ([ 1,  2,  3,  4], C_FAR_SIT_LV ),
                ([ 5,  6,  7,  8], C_NEAR_SIT_LV)
            )
        ),

        # AFP_(A|C)_(FAR|NEAR)_SIT_(PARTIALLY|NOT)_OPERATIONAL_HV
        DCSC_Merged_Variable(
            ['SIT/HV', 'SIT/HV_VOLTAGE_SET'],
            lambda iov: -remove_None(iov.hv.voltage, 0) > iov.hv_voltage_set.voltageSet - SIT_HV_DEAD_BAND,
            mapping = {
                'SIT/HV': mapChannels(
                    ([ 6,  7,  1,  2], A_FAR_SIT_HV ),
                    ([ 8,  3,  4,  9], A_NEAR_SIT_HV),
                    ([10, 11, 12,  5], C_FAR_SIT_HV ),
                    ([13, 14, 15, 16], C_NEAR_SIT_HV)),
                'SIT/HV_VOLTAGE_SET': mapChannels(
                    ([ 1,  2,  3,  4], A_FAR_SIT_HV ),
                    ([ 5,  6,  7,  8], A_NEAR_SIT_HV),
                    ([ 9, 10, 11, 12], C_FAR_SIT_HV ),
                    ([13, 14, 15, 16], C_NEAR_SIT_HV))
            }
        ),

        # AFP_(A|C)_FAR_TOF_NOT_OPERATIONAL_LV
        DCSC_Variable_With_Mapping(
            'TOF_TDC_CURRENT',
            lambda iov: TOF_LV_CURRENT_LOW <= remove_None(iov.hptdc1_current, 0) and TOF_LV_CURRENT_LOW <= remove_None(iov.hptdc2_current, 0),
            mapping = {1: A_FAR_TOF_LV, 2: C_FAR_TOF_LV}
        ),

        # AFP_(A|C)_FAR_TOF_NOT_OPERATIONAL_HV
        DCSC_Merged_Variable(
            ['TOF', 'TOF_PMT_VOLTAGE_SET'],
            lambda iov: -remove_None(iov.tof.pmt_voltage, 0) > iov.tof_pmt_voltage_set.pmt_voltageSet - TOF_HV_DEAD_BAND and -remove_None(iov.tof.pmt_current, 0) > TOF_HV_CURRENT_LOW,
            mapping = {
                'TOF':                 {1: A_FAR_TOF_HV, 2: C_FAR_TOF_HV},
                'TOF_PMT_VOLTAGE_SET': {1: A_FAR_TOF_HV, 2: C_FAR_TOF_HV},
            }
        ),

        #######################################################################
        # TDAQ Defects
        #######################################################################

        # AFP_TTC_RESTART
        TDAQC_Variable(
            'ATLAS.RCStateInfo.state.RunCtrl.AFP',
            lambda iov: iov.value == 'RUNNING',
            mapping = {'RunCtrl.AFP': TTC_RESTART}
        ),

        # AFP_STOPLESSLY_REMOVED
        TDAQC_Array_Variable(
            'ATLAS.RODBusyIS.BusyEnabled.Monitoring.afp_rodBusy-VLDB/RODBusy',
            lambda iov: iov.value is not False,
            mapping = {'Monitoring.afp_rodBusy-VLDB/RODBusy': {6: STOPLESSLY_REMOVED}}
        ),

        # AFP_(A|C)_(FAR|NEAR)_(PARTIALLY|NOT)_OPERATIONAL_TDAQ
        TDAQC_Bit_Flag_Variable(
            'ATLAS.RceMonitoring.DisabledPerm.Monitoring.RceMonitoring_RCE[34]',
            lambda iov: iov.value is not True,
            regex = True,
            mapping = {
                'Monitoring.RceMonitoring_RCE3': mapChannels(([0, 2, 4, 6], A_NEAR_SIT_DISABLED), ([8, 10, 12, 14], A_FAR_SIT_DISABLED), ([9, 11], A_FAR_TOF_DISABLED)),
                'Monitoring.RceMonitoring_RCE4': mapChannels(([0, 2, 4, 6], C_NEAR_SIT_DISABLED), ([8, 10, 12, 14], C_FAR_SIT_DISABLED), ([9, 11], C_FAR_TOF_DISABLED)),
            }
        ),
    ]

    equality_breaker = 0.0001

    dead_fraction_caution = 0 + equality_breaker
    dead_fraction_bad = 0.25 + equality_breaker

    mapping = mapTranslatorCounts({
        1: [*GARAGE, TTC_RESTART, STOPLESSLY_REMOVED, *TOF_LV, *TOF_HV],
        2: TOF_DISABLED,
        4: [*SIT_DISABLED, *SIT_LV, *SIT_HV],
    })
        
    def merge_variable_states(self, states):
        """
        Merge input channel states across variables, taking the worst.
        
        Ignore configuration variables and variables without channel.
        """

        # Remove variables without channel
        states = [state for state in states if state and state.channel is not None]
                
        # More simplistic way of doing the above, but cannot handle config vars:
        return min(state.good for state in states) if len(states) > 0 else None

    @staticmethod
    def color_to_defect_translator(channel, defect_name, color, comment):
        def translator_core(iovs):
            return [DefectIOV(iov.since, iov.until, defect_name, True,
                            comment=comment(iov))
                    for iov in iovs if iov.channel == channel
                    and iov.Code == color]
        return translator_core
    
    @staticmethod
    def defect_combinator(channels, defect_name, code, comment):
        def combinator_core(iovs):
            result = []
            channel_diffs = [channels[0] - channel for channel in channels]
            channel_iovs = iovs.by_channel
            defect_iovs = [channel_iovs.get(channel) for channel in channels]
            for since, until, states in process_iovs(*defect_iovs):
                matched = [state for state in states if state.Code == code]
                if len(matched) < 2: continue # We need at least two defects
                matched_diffs = [diff for state,diff in zip(states, channel_diffs) if state.Code == code]
                bad_channels = [{iov.channel + diff for iov in state._orig_iovs if not iov.good} for state,diff in zip(matched,matched_diffs)]
                if all(a.issubset(b) or a.issuperset(b) for a,b in combinations(bad_channels, 2)): continue # We need that the defects have different origin
                result.append(DefectIOV(since, until, defect_name, True, comment=comment()))
            return result
        return combinator_core

    def __init__(self, *args, **kwargs):
        super(AFP, self).__init__(*args, **kwargs)
        self.translators = [
            AFP.color_to_defect_translator(*cdcc)
            for cdcc in [
                ###############################################
                # DCS Defects
                ###############################################
                (A_FAR_GARAGE,  'AFP_A_FAR_IN_GARAGE',  RED, AFP.comment_GARAGE),
                (A_NEAR_GARAGE, 'AFP_A_NEAR_IN_GARAGE', RED, AFP.comment_GARAGE),
                (C_FAR_GARAGE,  'AFP_C_FAR_IN_GARAGE',  RED, AFP.comment_GARAGE),
                (C_NEAR_GARAGE, 'AFP_C_NEAR_IN_GARAGE', RED, AFP.comment_GARAGE),

                (A_FAR_SIT_HV,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_HV',  YELLOW, AFP.comment_SIT_HV),
                (A_FAR_SIT_HV,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_HV',        RED,    AFP.comment_SIT_HV),
                (A_NEAR_SIT_HV, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_HV', YELLOW, AFP.comment_SIT_HV),
                (A_NEAR_SIT_HV, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_HV',       RED,    AFP.comment_SIT_HV),
                (C_FAR_SIT_HV,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_HV',  YELLOW, AFP.comment_SIT_HV),
                (C_FAR_SIT_HV,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_HV',        RED,    AFP.comment_SIT_HV),
                (C_NEAR_SIT_HV, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_HV', YELLOW, AFP.comment_SIT_HV),
                (C_NEAR_SIT_HV, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_HV',       RED,    AFP.comment_SIT_HV),

                (A_FAR_SIT_LV,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_LV',  YELLOW, AFP.comment_SIT_LV),
                (A_FAR_SIT_LV,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_LV',        RED,    AFP.comment_SIT_LV),
                (A_NEAR_SIT_LV, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_LV', YELLOW, AFP.comment_SIT_LV),
                (A_NEAR_SIT_LV, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_LV',       RED,    AFP.comment_SIT_LV),
                (C_FAR_SIT_LV,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_LV',  YELLOW, AFP.comment_SIT_LV),
                (C_FAR_SIT_LV,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_LV',        RED,    AFP.comment_SIT_LV),
                (C_NEAR_SIT_LV, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_LV', YELLOW, AFP.comment_SIT_LV),
                (C_NEAR_SIT_LV, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_LV',       RED,    AFP.comment_SIT_LV),

                (A_FAR_TOF_LV, 'AFP_A_FAR_TOF_NOT_OPERATIONAL_LV', RED, AFP.comment_TOF_LV),
                (C_FAR_TOF_LV, 'AFP_C_FAR_TOF_NOT_OPERATIONAL_LV', RED, AFP.comment_TOF_LV),

                (A_FAR_TOF_HV, 'AFP_A_FAR_TOF_NOT_OPERATIONAL_HV', RED, AFP.comment_TOF_HV),
                (C_FAR_TOF_HV, 'AFP_C_FAR_TOF_NOT_OPERATIONAL_HV', RED, AFP.comment_TOF_HV),

                ###############################################
                # TDAQ Defects
                ###############################################
                (TTC_RESTART,        'AFP_TTC_RESTART',        RED, AFP.comment_TTC_RESTART),
                (STOPLESSLY_REMOVED, 'AFP_STOPLESSLY_REMOVED', RED, AFP.comment_STOPLESSLY_REMOVED),

                (A_FAR_SIT_DISABLED,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_TDAQ',  YELLOW, AFP.comment_SIT_DISABLED),
                (A_FAR_SIT_DISABLED,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_TDAQ',        RED,    AFP.comment_SIT_DISABLED),
                (A_NEAR_SIT_DISABLED, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_TDAQ', YELLOW, AFP.comment_SIT_DISABLED),
                (A_NEAR_SIT_DISABLED, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_TDAQ',       RED,    AFP.comment_SIT_DISABLED),
                (C_FAR_SIT_DISABLED,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_TDAQ',  YELLOW, AFP.comment_SIT_DISABLED),
                (C_FAR_SIT_DISABLED,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_TDAQ',        RED,    AFP.comment_SIT_DISABLED),
                (C_NEAR_SIT_DISABLED, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_TDAQ', YELLOW, AFP.comment_SIT_DISABLED),
                (C_NEAR_SIT_DISABLED, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_TDAQ',       RED,    AFP.comment_SIT_DISABLED),

                (A_FAR_TOF_DISABLED, 'AFP_A_FAR_TOF_NOT_OPERATIONAL_TDAQ', RED, AFP.comment_TOF_DISABLED),
                (C_FAR_TOF_DISABLED, 'AFP_C_FAR_TOF_NOT_OPERATIONAL_TDAQ', RED, AFP.comment_TOF_DISABLED),
            ]
        ] + [
            AFP.defect_combinator(*cdcc)
            for cdcc in [
                ([A_FAR_SIT_LV,  A_FAR_SIT_DISABLED],  'AFP_A_FAR_SIT_NOT_OPERATIONAL',  YELLOW, AFP.comment_SIT_COMBINATION),
                ([A_NEAR_SIT_LV, A_NEAR_SIT_DISABLED], 'AFP_A_NEAR_SIT_NOT_OPERATIONAL', YELLOW, AFP.comment_SIT_COMBINATION),
                ([C_FAR_SIT_LV,  C_FAR_SIT_DISABLED],  'AFP_C_FAR_SIT_NOT_OPERATIONAL',  YELLOW, AFP.comment_SIT_COMBINATION),
                ([C_NEAR_SIT_LV, C_NEAR_SIT_DISABLED], 'AFP_C_NEAR_SIT_NOT_OPERATIONAL', YELLOW, AFP.comment_SIT_COMBINATION),
            ]
        ]
        
    ###########################################################################
    # DCS defects
    ###########################################################################

    @staticmethod
    def comment_GARAGE(iov):
        return 'Station not in physics'

    @staticmethod
    def comment_SIT_LV(iov):
        return AFP.comment_planes(iov, 'with too low current')

    @staticmethod
    def comment_SIT_HV(iov):
        return AFP.comment_planes(iov, 'out of nominal voltage')

    @staticmethod
    def comment_TOF_LV(iov):
        return 'ToF TDC with too low current'

    @staticmethod
    def comment_TOF_HV(iov):
        return 'ToF PMT out of nominal voltage'
    
    ###########################################################################
    # TDAQ defects
    ###########################################################################

    @staticmethod
    def comment_TTC_RESTART(iov):
        return 'AFP not in the RUNNING state'

    @staticmethod
    def comment_STOPLESSLY_REMOVED(iov):
        return 'AFP stoplessly removed from the run'
    
    @staticmethod
    def comment_SIT_DISABLED(iov):
        return AFP.comment_planes(iov, 'removed from readout')
    
    @staticmethod
    def comment_TOF_DISABLED(iov):
        return AFP.comment_device(iov, 'ToF TDC', 'removed from readout')
    
    ###########################################################################
    # Combination defects
    ###########################################################################
    
    @staticmethod
    def comment_SIT_COMBINATION():
        return "Combination of 'partially operational' defects"

    ###########################################################################
    # Comment templates
    ###########################################################################

    @staticmethod
    def comment_planes(iov, message):
        return AFP.comment_device(iov, 'SiT plane', message)

    @staticmethod
    def comment_device(iov, device, message):
        count = iov.NConfig - iov.NWorking
        if count != 1:
            device += 's'
        return f"{count} {device} {message}"
