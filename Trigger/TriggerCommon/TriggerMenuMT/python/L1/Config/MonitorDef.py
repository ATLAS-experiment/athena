# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
The CTP monitors three different types of signals. In the XML file
they are listed in the section <TriggerCounterList>

1) type CTPIN

Almost each input threshold is monitored with one counter per
multiplicity it can have. E.g. there are 7 counters for the EM10VH
threshold: "1EM10VH" ... "7EM10VH"

Due to limitations of the CTPIN monitoring LUT (there are only 4 LUT
per CTPIN connector and a LUT has 8 bit input) only 2 3-bit thresholds
can be monitored per LUT, so max 8 3-bit thresholds per
connector. Hence JET thresholds 8 and 9 can not be monitored.


2) type CTPMON

This allows to monitor any combination of conditions built in the same
way as L1 Items. However, since we can also monitor L1 Items directly
(see 3)), there is little need for this type of monitoring. We use it to monitor simple conditions


3) type TBP, TAP, TAV

For each of these type 64 L1Items can be monitored independently

"""

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Base.MonCounters import CtpinCounter, CtpmonCounter

class MonitorDef:

    LOW_FREQ = 0
    HIGH_FREQ = 1


    # CTPIN counters
    # these are generated for all CTPIN signals except the two highest JET inputs on JET1 (see comment at start of file)
    @staticmethod
    def ctpinCounters( thresholds, connectors, ctpinConfig ):

        connectedCables = []
        for slotConnectors in ctpinConfig.values():
            for connName in slotConnectors.values():
                if connName:
                    connectedCables += [ connName ]

        counters = []
        for ctpinCableName in connectedCables:
            conn = connectors[ctpinCableName]
            for i, tl in enumerate(conn.triggerLines):
                if ctpinCableName == "JET1" and i==8:
                    break
                for mult in range(1, 2**tl.nbits):
                    counters += [ CtpinCounter(threshold=tl.name, multiplicity = mult) ]

        return counters




    # CTPMON counters
    # we only have a few for the moment
    @staticmethod
    def ctpmonCounters( thresholds, connectors ):

        counters = []

        cThr = {}
        cThr[1] = [ 
                   'AFP_NSA', 'AFP_NSC', 'AFP_FSA', 'AFP_FSC', 'AFP_FSA_TOF_T0', 'AFP_FSC_TOF_T0',
                   'AFP_FSA_TOF_T1', 'AFP_FSC_TOF_T1', 'AFP_FSA_TOF_T2', 'AFP_FSC_TOF_T2', 'AFP_FSA_TOF_T3', 'AFP_FSC_TOF_T3',
                    'MBTS_A0', 'MBTS_A1', 'MBTS_A2',  'MBTS_A3',  'MBTS_A4',  'MBTS_A5',  'MBTS_A6',  'MBTS_A7',
                    'MBTS_A8', 'MBTS_A9', 'MBTS_A10', 'MBTS_A11', 'MBTS_A12', 'MBTS_A13', 'MBTS_A14', 'MBTS_A15',
                    'MBTS_C0', 'MBTS_C1', 'MBTS_C2',  'MBTS_C3',  'MBTS_C4',  'MBTS_C5',  'MBTS_C6',  'MBTS_C7',
                    'MBTS_C8', 'MBTS_C9', 'MBTS_C10', 'MBTS_C11', 'MBTS_C12', 'MBTS_C13', 'MBTS_C14', 'MBTS_C15',
                    'BMA0', 'BMA1', 'BMA2', 'BMA3',
                   ]

        for mult in cThr:
            for thrName in cThr[mult]:
                counters += [ CtpmonCounter(thrName,1) ]

        return counters



    @staticmethod
    def applyItemCounter( menuName, items ):
        """
        this functions marks the items that should be monitored by setting the corresponding monitoring flags
        e.g. to "LF:000|HF:111" for high frequency monitoring of TBP, TAP, and TAV.
        """

        TBP=1
        TAP=2
        TAV=4

        monItems   = { 1 :[], 2: [], 3: [], 4: [], 5: [], 6: [], 7: [] }
        monItemsHF = { 1 :[], 2: [], 3: [], 4: [], 5: [], 6: [], 7: [] }

        # definitions hardcoded at the moment

        if 'HI' not in menuName:
            monItems[TBP|TAP|TAV] = [
                # L1Muon
                "L1_MU10BO", "L1_MU10BOM", "L1_MU12BOM", "L1_MU14FCH",
                "L1_MU3V", "L1_MU4BOM", "L1_MU5VF", "L1_MU8VFC",
                "L1_2MU3V", "L1_2MU3VF", "L1_2MU5VF",
                "L1_MU5VF_2MU3V", "L1_MU8VF_2MU5VF",
                "L1_3MU3V", "L1_MU5VF_3MU3VF", "L1_4MU3V",
                ## Legacy L1 items
                # L1Calo
                #"L1_TAU8", "L1_TAU20IM",
                #"L1_J100", 
                #"L1_J400",
                #"L1_XE35",
                # "L1_XE45",
                #"L1_XE50", 
                "L1_XE55", "L1_XE300",
                # "L1_XE45", "L1_XE50", "L1_XE55", "L1_XE300",
                # Combined
                #"L1_MU3V_J12",
                # L1Topo
                "L1_BPH-0DR3-eEM9jJ40_MU5VF", "L1_HT190-jJ40s5pETA21", "L1_LLP-RO-eEM", # Board 0 FPGA 0
                "L1_jMJJ-500-NFF",                                                # Board 0 FPGA 1
                "L1_SC111-CjJ40",                                                 # Board 1 FPGA 0
                "L1_BPH-0M9-eEM9-eEM7_MU5VF", "L1_BPH-0DR3-eEM9jJ40_2MU3V",
                "L1_JPSI-1M5-eEM9", "L1_JPSI-1M5-eEM15",
                "L1_jMJJ-700", "L1_LLP-NOMATCH-eEM",
                "L1_LAR-ZEE-eEM",

                ## Phase-I
                # L1Calo
                "L1_eEM5", "L1_eEM9", "L1_eEM12L",
                "L1_eEM18L", "L1_eEM18M", "L1_eEM24L",
                "L1_eEM26", "L1_eEM26L", "L1_eEM26M", "L1_eEM26T",
                "L1_eEM28M",
                "L1_2eEM18M", "L1_2eEM24L",
                "L1_eEM24L_3eEM12L",
                "L1_eTAU20M", "L1_eTAU30",
                "L1_eTAU60", "L1_eTAU80", "L1_eTAU140",
                "L1_jTAU20", "L1_jTAU30", "L1_jTAU30M",
                "L1_cTAU20M", "L1_cTAU35M",
                "L1_cTAU30M_2cTAU20M",
                "L1_jJ30", "L1_jJ40", "L1_jJ50",
                "L1_jJ60", "L1_jJ90", "L1_jJ125",
                "L1_jJ160", "L1_jJ500",
                "L1_jJ40p30ETA49", "L1_jJ50p30ETA49",
                "L1_jJ60p30ETA49", "L1_jJ90p30ETA49", "L1_jJ125p30ETA49",
                "L1_3jJ90", "L1_4jJ40", "L1_4jJ50",
                "L1_3jJ70p0ETA23", "L1_4jJ40p0ETA25", "L1_5jJ40p0ETA25",
                "L1_jJ140_3jJ60",
                "L1_jLJ80", "L1_jLJ120", "L1_jLJ140",
                "L1_jXE60", "L1_jXE70", "L1_jXE80", "L1_jXE90", "L1_jXE100",
                "L1_jXE110", "L1_jXE120", "L1_jXE500",
                "L1_jXEC100",
                "L1_jTE200",
                "L1_jTEC200", "L1_jTEFWD100", "L1_jTEFWDA100", "L1_jTEFWDC100",
                "L1_jJ85p0ETA21_3jJ40p0ETA25",
                "L1_gJ20p0ETA25", "L1_gJ20p25ETA49","L1_gJ20p0ETA25_EMPTY", "L1_gJ50p0ETA25",
                "L1_gJ100p0ETA25", "L1_gJ400p0ETA25", "L1_gLJ80p0ETA25",
                #"L1_gXERHO70", "L1_gXERHO100",
                "L1_gXENC70", "L1_gXENC100",
                "L1_gXEJWOJ60", "L1_gXEJWOJ70", "L1_gXEJWOJ80", "L1_gXEJWOJ100", "L1_gXEJWOJ110", "L1_gXEJWOJ120", "L1_gXEJWOJ500",
                "L1_gTE200",
                "L1_gMHT500",
                # Combined
                "L1_2eEM10L_MU8F", "L1_MU3V_jJ40",
                # L1Topo (Topo2 always in)
                "L1_BTAG-MU3VjJ40", "L1_BTAG-MU5VFjJ80",
                "L1_LAR-ZEE-eEM",
                "L1_JPSI-1M5-eEM9", "L1_JPSI-1M5-eEM15",
                "L1_BPH-0M9-eEM9-eEM7", "L1_BPH-0M9-eEM9-eEM7_MU5VF",
                "L1_cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M",
                "L1_DY-BOX-2MU3VF", "L1_DY-BOX-MU5VFMU3V",
                "L1_LFV-MU5VF",
                "L1_jMJJ-700",
                "L1_10DR-MU14FCH-MU5VF_EMPTY",
                "L1_BPH-0M10-3MU3V", "L1_BPH-0M10-3MU3VF",
                "L1_DPHI-M70-2eEM12M",
            ]

            topo3_monitems = [
                "L1_ZAFB-25DPHI-eEM18M",
                "L1_SC111-CjJ40",
                "L1_jMJJ-300-NFF", "L1_jMJJ-500-NFF",
                "L1_LFV-eEM10L-MU8VF", "L1_LFV-eEM15L-MU5VF",
                "L1_LLP-RO-eEM", "L1_LLP-NOMATCH-eEM",
            ]
            # Add triggers that are not in the MC menu
            if 'MC' not in menuName:
                monItems[TBP|TAP|TAV] += [
                    # Detector items
                    # "L1_ZDC_A", "L1_ZDC_C", "L1_ZDC_AND",
                    "L1_LUCID_A", "L1_LUCID_C",
                    "L1_CALREQ2",
                    "L1_TGC_BURST",
                    "L1_TRT_FILLED",
                    "L1_BPTX0_BGRP12", "L1_BPTX1_BGRP12",
                    "L1_NSW_MONITOR",
                    "L1_BCM_Wide", "L1_BCM_Wide_EMPTY", "L1_BCM_Wide_BGRP12",
                    "L1_BCM_2A_UNPAIREDB1", "L1_BCM_2C_UNPAIREDB1", "L1_BCM_2A_UNPAIREDB2", "L1_BCM_2C_UNPAIREDB2",
                    "L1_BCM_2A_FIRSTINTRAIN", "L1_BCM_2C_FIRSTINTRAIN",
                    "L1_BCM_2A_EMPTY", "L1_BCM_2C_EMPTY",
                    "L1_AFP_A_AND_C_TOF_T0T1",
                    "L1_AFP_FSA_BGRP12", "L1_AFP_FSC_BGRP12",
                    "L1_AFP_FSA_TOF_T0_BGRP12", "L1_AFP_FSA_TOF_T1_BGRP12", "L1_AFP_FSA_TOF_T2_BGRP12", "L1_AFP_FSA_TOF_T3_BGRP12",
                    "L1_AFP_FSC_TOF_T0_BGRP12", "L1_AFP_FSC_TOF_T1_BGRP12", "L1_AFP_FSC_TOF_T2_BGRP12", "L1_AFP_FSC_TOF_T3_BGRP12",
                    "L1_AFP_A_OR_C_EMPTY",
                    "L1_AFP_NSA_BGRP12", "L1_AFP_NSC_BGRP12",
                    "L1_AFP_A", "L1_AFP_C", "L1_AFP_A_AND_C",
                    "L1_MBTS_A", "L1_MBTS_C",
                    "L1_MBTS_1", "L1_MBTS_1_1", "L1_MBTS_2",
                    "L1_MBTS_1_A", "L1_MBTS_1_C", "L1_MBTS_4_A", "L1_MBTS_4_C",
                    "L1_MBTS_1_1_EMPTY",
                    "L1_ZB",
                    "L1_AFP_A_AND_C_TOF_jJ50",
                    # Phase-I
                    "L1_AFP_A_AND_C_TOF_T0T1_jJ90",
                    "L1_jJ500_LAR",

                    # Beam splashes
                    "L1_eEM22A", "L1_eEM22C",

                    # Other triggers disabled in MC
                    "L1_MU3VF", "L1_MU8F", "L1_MU8FC", "L1_MU8VF",
                    "L1_MU3VC", "L1_MU3EOF", "L1_MU4BO",
                    "L1_MU8FH", "L1_MU8EOF",
                    "L1_MU9VF", "L1_MU9VFC",
                    "L1_MU12FCH", "L1_MU14FCHR", "L1_MU14EOF",
                    "L1_MU15VFCH", "L1_MU15VFCHR",
                    "L1_MU18VFCH", "L1_MU20VFC",
                    "L1_2MU8F", "L1_2MU8VF",
                    "L1_2MU14FCH_OVERLAY",
                    #
                    "L1_jJ55", "L1_jJ80", "L1_jJ140", "L1_jJ180",
                    "L1_jJ30p0ETA25", "L1_jJ40p0ETA25",
                    "L1_jJ70p0ETA23", "L1_jJ55p0ETA23",
                    "L1_jJ80p0ETA25", "L1_jJ85p0ETA21",
                    "L1_jLJ180",
                    "L1_jEM20", "L1_jEM20M",
                    #
                    "L1_eEM7", "L1_eEM10L", "L1_eEM15",
                    "L1_eEM18", "L1_eEM22M", "L1_eEM24VM",
                    "L1_3eEM12L",
                    #
                    "L1_eTAU20L", "L1_eTAU35", "L1_eTAU40HM",
                ]

        else: # HI menu
            monItems[TBP|TAP|TAV] = [
                # Random
                "L1_RD0_FILLED",
                # Detector
                "L1_CALREQ0", "L1_CALREQ1", "L1_CALREQ2",
                "L1_BPTX0_BGRP12","L1_BPTX1_BGRP12",
                "L1_TGC_BURST",
                "L1_ZB",
                # Forward
                # AFP
                "L1_AFP_A", "L1_AFP_C",
                "L1_AFP_A_OR_C", "L1_AFP_A_AND_C",
                # AFP Calib
                "L1_AFP_FSA_BGRP12",
                "L1_AFP_FSA_TOF_T0_BGRP12",
                "L1_AFP_FSA_TOF_T1_BGRP12",
                "L1_AFP_FSA_TOF_T2_BGRP12",
                "L1_AFP_FSA_TOF_T3_BGRP12",
                "L1_AFP_FSC_BGRP12",
                "L1_AFP_FSC_TOF_T0_BGRP12",
                "L1_AFP_FSC_TOF_T1_BGRP12",
                "L1_AFP_FSC_TOF_T2_BGRP12",
                "L1_AFP_FSC_TOF_T3_BGRP12",
                "L1_AFP_NSA_BGRP12",
                "L1_AFP_NSC_BGRP12",
                # AFP combined
                "L1_AFP_A_AND_C_J12",
                "L1_AFP_A_AND_C_MBTS_2",
                "L1_AFP_A_AND_C_TOF_J20",
                "L1_AFP_A_AND_C_TOF_J30",
                "L1_AFP_A_AND_C_TOF_J50",
                "L1_AFP_A_AND_C_TOF_J75",
                "L1_AFP_A_AND_C_TOF_T0T1",
                "L1_AFP_A_AND_C_TOF_T0T1_J20",
                "L1_AFP_A_AND_C_TOF_T0T1_J30",
                "L1_AFP_A_AND_C_TOF_T0T1_J50",
                "L1_AFP_A_AND_C_TOF_T0T1_J75",
                "L1_AFP_A_AND_C_TOF_T0T1_jJ125",
                "L1_AFP_A_AND_C_TOF_T0T1_jJ50",
                "L1_AFP_A_AND_C_TOF_T0T1_jJ60",
                "L1_AFP_A_AND_C_TOF_T0T1_jJ90",
                "L1_AFP_A_AND_C_TOF_jJ125",
                "L1_AFP_A_AND_C_TOF_jJ50",
                "L1_AFP_A_AND_C_TOF_jJ60",
                "L1_AFP_A_AND_C_TOF_jJ90",
                "L1_AFP_A_AND_C_jJ20",
                "L1_AFP_A_AND_C_jJ30",
                "L1_AFP_A_OR_C_EMPTY",
                "L1_AFP_A_OR_C_FIRSTEMPTY",
                "L1_AFP_A_OR_C_J12",
                "L1_AFP_A_OR_C_MBTS_2",
                "L1_AFP_A_OR_C_UNPAIRED_ISO",
                "L1_AFP_A_OR_C_UNPAIRED_NONISO",
                "L1_AFP_A_OR_C_jJ20",
                "L1_AFP_A_OR_C_jJ30",
                # LUCID
                "L1_LUCID_A", "L1_LUCID_C",
                # ZDC
                # Basic inputs
                "L1_ZDC_BIT0", "L1_ZDC_BIT1", "L1_ZDC_BIT2",
                "L1_ZDC_COMB0", "L1_ZDC_COMB1", "L1_ZDC_COMB2", "L1_ZDC_COMB3",
                "L1_ZDC_COMB4", "L1_ZDC_COMB5", "L1_ZDC_COMB6", "L1_ZDC_COMB7",
                "L1_VZDC_A_VZDC_C", "L1_1ZDC_A_VZDC_C", "L1_VZDC_A_1ZDC_C",
                "L1_1ZDC_A_1ZDC_C", "L1_5ZDC_A_VZDC_C", "L1_VZDC_A_5ZDC_C",
                "L1_ZDC_1XOR5", "L1_5ZDC_A_5ZDC_C",
                #
                "L1_ZDC_A", "L1_ZDC_C", "L1_ZDC_A_C",
                "L1_ZDC_A_EMPTY", "L1_ZDC_A_UNPAIRED_NONISO",
                "L1_ZDC_C_EMPTY", "L1_ZDC_C_UNPAIRED_NONISO",
                "L1_ZDC_A_C_EMPTY", "L1_ZDC_A_C_UNPAIRED_NONISO",
                "L1_ZDC_XOR", "L1_ZDC_C_VZDC_A", "L1_ZDC_A_VZDC_C",
                # ZDC items in pp
                "L1_ZDC_A_AND_C",
                "L1_ZDC_A_AND_C_EMPTY", "L1_ZDC_A_AND_C_UNPAIRED_NONISO",
                "L1_ZDC_E1_AND_E1", "L1_ZDC_E2_AND_E2", "L1_ZDC_E2_AND_E3", "L1_ZDC_E3_AND_E3",
                "L1_ZDC_E1_AND_E2ORE3",
                "L1_ZDC_OR", "L1_ZDC_OR_EMPTY", "L1_ZDC_OR_UNPAIRED_NONISO",
                "L1_ZDC_XOR_E1_E3", "L1_ZDC_XOR_E2",
                # Vetos
                "L1_ZDC_A_VZDC_C_VTE200",
                "L1_VZDC_A_ZDC_C_VTE200",
                "L1_VZDC_A_VZDC_C_TE5_VTE200",
                # LHCF
                "L1_LHCF",
                # MBTS
                "L1_MBTS_A",
                "L1_MBTS_C",
                "L1_MBTS_1", "L1_MBTS_2", "L1_MBTS_1_1",
                "L1_MBTS_1_EMPTY", "L1_MBTS_1_UNPAIRED_ISO",
                "L1_MBTS_2_EMPTY", "L1_MBTS_2_UNPAIRED_ISO",
                "L1_MBTS_1_1_EMPTY", "L1_MBTS_1_1_UNPAIRED_ISO",
                "L1_MBTS_2_2", "L1_MBTS_3_3", "L1_MBTS_4_4",
                "L1_MBTS_1_VTE50", "L1_MBTS_1_1_VTE50", "L1_MBTS_1_VTE200",
                "L1_MBTS_1_ZDC_A_VZDC_C_VTE200",
                "L1_MBTS_1_VZDC_A_ZDC_C_VTE200",
                # LAr Zee
                "L1_LAR-ZEE-eEM",
                # Background
                "L1_BCM_Wide", "L1_BCM_Wide_EMPTY", "L1_BCM_Wide_BGRP12",
                "L1_BCM_2A_UNPAIREDB1", "L1_BCM_2C_UNPAIREDB1", "L1_BCM_2A_UNPAIREDB2", "L1_BCM_2C_UNPAIREDB2",
                "L1_BCM_2A_FIRSTINTRAIN", "L1_BCM_2C_FIRSTINTRAIN",
                "L1_BCM_2A_EMPTY", "L1_BCM_2C_EMPTY",
                # Muons
                "L1_MU3V", "L1_MU5VF",
                "L1_MU8F", "L1_MU8VF",
                "L1_MU14FCH",
                "L1_MU3V_EMPTY",
                "L1_MU3V_FIRSTEMPTY",
                "L1_2MU5VF_EMPTY",
                "L1_2MU3V", "L1_2MU5VF",
                "L1_3MU3V",
                "L1_2MU14FCH_OVERLAY",
                # Mu+X
                "L1_MU3V_J12",
                "L1_MU3V_VTE50",
                "L1_MU5VF_VTE50",
                "L1_MU5VF_AFP_A_OR_C",
                "L1_2MU3V_VTE50",
                # Legacy L1Calo
                # Tau
                "L1_TAU8", "L1_TAU12IM",
                "L1_TAU1_TE4_VTE200",
                "L1_2TAU1_VTE200",
                # Jet
                "L1_J30", "L1_J50",
                "L1_J20p31ETA49", "L1_J30p31ETA49",
                "L1_J12_EMPTY",
                "L1_J40_XE50",
                # TE
                "L1_TE3", "L1_TE4", "L1_TE5",
                "L1_TE20", "L1_TE50",
                "L1_TE100", "L1_TE200",
                #
                "L1_TE3_VTE50",
                "L1_TE5_VTE200",
                "L1_TE50_VTE200",
                #
                "L1_TE7p0ETA49",
                "L1_TE600p0ETA49",
                "L1_TE1500p0ETA49",
                "L1_TE3000p0ETA49",
                "L1_TE1500p0ETA49_OVERLAY",
                "L1_TE3000p0ETA49_OVERLAY",
                "L1_TE3500p0ETA49_OVERLAY",
                "L1_TE6500p0ETA49_OVERLAY",
                "L1_TE8000p0ETA49_OVERLAY",
                #
                "L1_ZDC_A_VZDC_C_TE5_VTE200",
                "L1_VZDC_A_ZDC_C_TE5_VTE200",
                #
                "L1_ZDC_A_VZDC_C_TE3_VTE200",
                "L1_VZDC_A_ZDC_C_TE3_VTE200",
                #
                "L1_VTE20", "L1_VTE50", "L1_VTE200",
                # TRT
                "L1_TRT_VTE50", "L1_TRT_VTE200", "L1_TRT_VTE20",
                # XE
                "L1_XE30", "L1_XE50",
                # Phase-I L1Calo
                "L1_eEM5", "L1_eEM9", "L1_eEM12", "L1_eEM15",
                "L1_eEM18", "L1_eEM18L",
                "L1_eEM26", "L1_eEM26M",
                "L1_2eEM18",
                #
                "L1_jJ20", "L1_jJ30", "L1_jJ40", "L1_jJ50",
                "L1_jJ55", "L1_jJ60", "L1_jJ90",
                "L1_jJ500", "L1_jJ500_LAR",
                "L1_jJ40p30ETA49", "L1_jJ50p30ETA49", "L1_jJ60p30ETA49",
                "L1_jJ90p30ETA49",
                #
                "L1_gLJ80p0ETA25", "L1_gXEJWOJ100",
                #
                "L1_jTE200",
                #
                "L1_eEM9_AFP_A_AND_C", #"L1_eEM9_AFP_A_OR_C",
                # Addtional L1 items added for 2022 Nov HI test run
                "L1_MBTS_1_VTE5", "L1_VTE5", 
                "L1_J12_VTE100", 
                "L1_J30_VTE200", "L1_J100_VTE200", 
                "L1_XE35_VTE200", "L1_XE50_VTE200",
                ]

            topo3_monitems = []

        monItems[TBP|TAP|TAV] += topo3_monitems


        monItemsHF[TBP|TAP|TAV] = [
           "L1_BCM_2A_FIRSTINTRAIN",
           "L1_BCM_2C_FIRSTINTRAIN",
           "L1_jJ30",
           "L1_MBTS_1", "L1_MBTS_2", "L1_MBTS_1_1",
           "L1_BCM_Wide",
        ]



        check = True
        if check:
            counts_LF_items = { TBP : set(), TAP : set(), TAV : set() }
            counts_HF_items = { TBP : set(), TAP : set(), TAV : set() }

            for k in range(1,8):

                if k & TBP:
                    counts_LF_items[TBP].update( monItems[k] )
                    counts_HF_items[TBP].update( monItemsHF[k] )

                if k & TAP:
                    counts_LF_items[TAP].update( monItems[k] )
                    counts_HF_items[TAP].update( monItemsHF[k] )

                if k & TAV:
                    counts_LF_items[TAV].update( monItems[k] )
                    counts_HF_items[TAV].update( monItemsHF[k] )

            counts_LF = dict( map(lambda x : (x[0],len(x[1])), counts_LF_items.items() ) )
            counts_HF = dict( map(lambda x : (x[0],len(x[1])), counts_HF_items.items() ) )

            lutsLF = int( (max(counts_LF.values())-1) / 8) + 1
            lutsHF = int( (max(counts_HF.values())-1) / 8) + 1

            maxLUTs = 32
            if lutsLF + lutsHF > maxLUTs:
                log.error("too many monitoring items are defined:")
                log.error("   low frequency  TBP: %i",counts_LF[TBP])
                log.error("                  TAP: %i",counts_LF[TAP])
                log.error("                  TAV: %i",counts_LF[TAV])
                log.error("   required LUTs: %i",lutsLF)
                log.error("   high frequency TBP: %i",counts_HF[TBP])
                log.error("                  TAP: %i",counts_HF[TAP])
                log.error("                  TAV: %i",counts_HF[TAV])
                log.error("   required LUTs: %i",lutsHF)
                log.error("   this menu requires %i monitoring LUTs while only %i are available", (lutsLF + lutsHF), maxLUTs)
                raise RuntimeError("Reduce the number of monitored items") 

        if 'AllCTPIn' not in menuName:
            MonitorDef.checkForNonExistingMonItems(items, monItems)

        # for each item set the monitor flags
        for item in items:

            itemName = item.name
            for k,l in monItems.items():

                if itemName in l:
                    item.addMonitor(k, MonitorDef.LOW_FREQ)


            for k,l in monItemsHF.items():

                if itemName in l:
                    item.addMonitor(k, MonitorDef.HIGH_FREQ)


    @staticmethod
    def checkForNonExistingMonItems(items, monItems):
        # check is based on item names
        allItemNames = [item.name for item in items]

        # unify all item names that are monitored
        allMonitorItems = set()
        for i in range(1,8):
            allMonitorItems.update(monItems[i])

        # register all monitems that don't exist in here
        nonExistingMonItems = []
        
        for monItem in allMonitorItems:
            if monItem not in allItemNames:
                nonExistingMonItems += [monItem]

        if len(nonExistingMonItems)>0:
            raise RuntimeError("These monitoring items are not part of the menu: %s" % ','.join(nonExistingMonItems))
