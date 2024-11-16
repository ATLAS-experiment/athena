# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from collections import OrderedDict as odict
from dataclasses import dataclass
from enum import Enum

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Base.ThresholdType import ThrType
from .FexThresholdParameters import eta_dependent_cuts

class ValueWithEtaDependence(object):
    """
        Class to encode a working point with eta dependence
        - Initialised with a list of prioritised cut values
        - Converts to a mapping from eta bin to highest priority cut
        - Logic should match that in ValueWithEtaDependence
          defined in L1ThresholdBase.h
    """

    def __init__(
        self,
        variable: str,
        working_point: str,
        values_with_prio: list,
        eta_range: list = (-49,49,1),
    ):
        self.variable = variable
        self.working_point = working_point
        self.name =  f"{working_point}.{variable}"
        self.values_with_prio = values_with_prio
        self.eta_range = eta_range
        self.eta_to_value = {}
        # Get value with highest priority for which
        # etabin lies in [etamin,etamax)
        for etabin in range(*eta_range):
            current_priority = -1
            for v in values_with_prio:
                priority_value = None
                if (
                    etabin >= v["etamin"]
                    and etabin < v["etamax"]
                ):
                    if v["priority"] == current_priority:
                        raise ValueError(f"{v} overlaps another value {priority_value} with same priority {current_priority}")
                    if v["priority"] > current_priority:
                        current_priority = v["priority"]
                        priority_value = v
                        self.eta_to_value[etabin] = v[self.variable]

# Facilitate enforcement of an ordering principle for working points:
# T >= M >= L in all eta bins, to avoid ambiguity in outcome from
# alg implementations [ATR-27796]
def leq_all_eta(lhs: ValueWithEtaDependence, rhs: ValueWithEtaDependence):
    for (k,v) in lhs.eta_to_value.items():
        if v>rhs.eta_to_value[k]:
            log.error(f"{lhs.name} ({v}) > {rhs.name} ({rhs.eta_to_value[k]}) for eta bin {k}")
            return False
    return True

# Enforce the ordering principle above either for eta-dependent/independent cuts
# Treat as a single uniform value if conf does not have etamin,etamax fields
def validate_ordering(var, wpl, wpg, conf):
    if "etamin" in conf[wpl][0]:
        _lesser = ValueWithEtaDependence(var,wpl,conf[wpl])
        _greater = ValueWithEtaDependence(var,wpg,conf[wpg])
        assert leq_all_eta(_lesser,_greater), f"Working point ordering violated for {_lesser.name}, {_greater.name}"
    else:
        assert len(conf[wpl])==1 and len(conf[wpg])==1, (
            f"Unsupported comparison between eta-dependent and eta-independent WPs {wpl}.{var}, {wpg}.{var}"
        )
        assert conf[wpl][0][var] <= conf[wpg][0][var], f"Working point ordering violated: {wpl}.{var} > {wpg}.{var}"

# eFEX conversions based on https://indico.cern.ch/event/1026972/contributions/4312070/attachments/2226175/3772176/Copy%20of%20Reta_Threshold_Setting.pdf
# ATR-23596
def eFEXfwToFloatConversion(fw,bitshift):
    decimal = 1/(1+fw/pow(2,bitshift))
    return float("{:.3f}".format(decimal))

def eFEXfwToFloatConversion_wstot(fw,bitshift):
    decimal = pow(2,bitshift)/fw
    return float("{:.3f}".format(decimal))

def eTAUfwToFloatConversion_bdt(fw):
    decimal = fw/4096
    return float("{:.2f}".format(decimal))

def eFEXfwToFloatConversion_minIsoEt(fw):
    decimal = fw * 100.0 # To MeV units
    return float("{:.3f}".format(decimal))

# jFEX conversion based on ATR-21235
def jFEXfloatToFWConversion(decimal):
    fw = round((1-decimal)/decimal)
    return fw

def cTAUfwToFlowConversion(fw):
    decimal = fw/1024
    return float("{:.2f}".format(decimal))

def getTypeWideThresholdConfig(ttype, do_HI_tob_thresholds=False, do_eFex_BDT_Tau=True):
    if isinstance(ttype, str):
        ttype = ThrType[ttype]

    if ttype == ThrType.MU:
        return getConfig_MU()
    if ttype == ThrType.eEM:
        return getConfig_eEM(do_HI_tob_thresholds)
    if ttype == ThrType.jEM:
        return getConfig_jEM()
    if ttype == ThrType.eTAU:
        return getConfig_eTAU(do_eFex_BDT_Tau, do_HI_tob_thresholds)
    if ttype == ThrType.cTAU:
        return getConfig_cTAU(do_eFex_BDT_Tau)
    if ttype == ThrType.jTAU:
        return getConfig_jTAU(do_HI_tob_thresholds)
    if ttype == ThrType.jJ:
        return getConfig_jJ(do_HI_tob_thresholds)
    if ttype == ThrType.jLJ:
        return getConfig_jLJ()
    if ttype == ThrType.gJ:
        return getConfig_gJ()
    if ttype == ThrType.gLJ:
        return getConfig_gLJ()
    if ttype == ThrType.jXE:
        return getConfig_jXE()
    if ttype == ThrType.jTE:
        return getConfig_jTE()
    if ttype == ThrType.gXE:
        return getConfig_gXE(do_HI_tob_thresholds)
    if ttype == ThrType.gTE:
        return getConfig_gTE()
    if ttype == ThrType.EM:
        return getConfig_EM(do_HI_tob_thresholds)
    if ttype == ThrType.TAU:
        return getConfig_TAU(do_HI_tob_thresholds)
    if ttype == ThrType.JET:
        return getConfig_JET()
    if ttype == ThrType.XS:
        return getConfig_XS()    
    return odict()


def getConfig_MU():
    confObj = odict()
    confObj["exclusionLists"] = odict()
    confObj["exclusionLists"]["rpcFeet"] = []
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B21"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B22"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B25"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B26"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B53"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B54"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B57"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )
    confObj["exclusionLists"]["rpcFeet"].append( odict([("sectorName", "B58"), ("rois",[8,9,10,11,16,17,18,19,20,21,22,23,28,29,30,31])]) )

    
    # roads from https://indico.cern.ch/event/1011425/contributions/4272884/
    confObj["roads"] = odict()
    confObj["roads"]["rpc"] = odict([(0,0), (4,1), (6,2), (8,3), (10,4), (12,5), (14,6)])
    confObj["roads"]["tgc"] = odict([(0,0)] + list(zip([3,4,5,6,7,8,9,10,11,12,13,14,15,18,20],list(range(1,16)))))

    # check that there is a unique assignment between roads and pt values
    for muonDet in ("rpc","tgc"):
        roads = []
        ptValues = []
        for ptValue in confObj["roads"][muonDet]: 
            if ptValue in ptValues:
                raise RuntimeError("Muon roads: pt value %s is duplicated, please fix.", str(ptValue) ) 
            else: 
                ptValues += [ptValue]
            road = confObj["roads"][muonDet][ptValue]
            if road !=0 and road in roads:
                raise RuntimeError("Muon roads: road %s is duplicated, please fix.", str(road) )
            else:
                roads += [road]
    if len(confObj["roads"]["rpc"])!=7 or len(confObj["roads"]["tgc"])!=16:
        raise RuntimeError("Muon roads: number of roads not as expected. TGC=%s (exp 16), RCP=%s (exp 7)", len(confObj["roads"]["tgc"]), len(confObj["roads"]["rpc"]) )

    return confObj


def getConfig_eEM(do_HI_tob_thresholds):
    confObj = odict()
    confObj["workingPoints"] = odict()
    bitshift_reta = 3
    bitshift_rhad = 3
    bitshift_wstot = 5   
    reta_fw_loose = 72
    reta_fw_medium = 92
    reta_fw_tight = 106
    rhad_fw_loose = 92
    rhad_fw_medium = 192
    rhad_fw_tight = 192
    wstot_fw_loose = 8
    wstot_fw_medium = 29
    wstot_fw_tight = 29 
    # based on https://indico.cern.ch/event/1035198/contributions/4378014/attachments/2251846/3820098/20210526_l1calo_TGM.pdf
    confObj["workingPoints"]["Loose"] = [
        odict([("reta_fw", reta_fw_loose), ("reta", eFEXfwToFloatConversion(reta_fw_loose,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_loose), ("rhad", eFEXfwToFloatConversion(rhad_fw_loose,bitshift_rhad)), 
               ("etamin", -49), ("etamax", -24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_loose), ("reta", eFEXfwToFloatConversion(reta_fw_loose,bitshift_reta)), 
               ("wstot_fw", wstot_fw_loose), ("wstot", eFEXfwToFloatConversion_wstot(wstot_fw_loose,bitshift_wstot)), 
               ("rhad_fw", rhad_fw_loose), ("rhad", eFEXfwToFloatConversion(rhad_fw_loose,bitshift_rhad)), 
               ("etamin", -24), ("etamax", 24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_loose), ("reta", eFEXfwToFloatConversion(reta_fw_loose,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_loose), ("rhad", eFEXfwToFloatConversion(rhad_fw_loose,bitshift_rhad)), 
               ("etamin",  24), ("etamax", 49), ("priority", 0)]),
        # More granular cuts from -24 to 25 are specified in FexThresholdParameters
        # with priority 2
    ]
    confObj["workingPoints"]["Medium"] = [
        odict([("reta_fw", reta_fw_medium), ("reta", eFEXfwToFloatConversion(reta_fw_medium,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_medium), ("rhad", eFEXfwToFloatConversion(rhad_fw_medium,bitshift_rhad)), 
               ("etamin", -49), ("etamax", -24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_medium), ("reta", eFEXfwToFloatConversion(reta_fw_medium,bitshift_reta)), 
               ("wstot_fw", wstot_fw_medium), ("wstot", eFEXfwToFloatConversion_wstot(wstot_fw_medium,bitshift_wstot)), 
               ("rhad_fw", rhad_fw_medium), ("rhad", eFEXfwToFloatConversion(rhad_fw_medium,bitshift_rhad)), 
               ("etamin", -24), ("etamax", 24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_medium), ("reta", eFEXfwToFloatConversion(reta_fw_medium,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_medium), ("rhad", eFEXfwToFloatConversion(rhad_fw_medium,bitshift_rhad)), 
               ("etamin",  24), ("etamax", 49), ("priority", 0)]),
        # More granular cuts from -24 to 25 are specified in FexThresholdParameters
    ]
    confObj["workingPoints"]["Tight"] = [
        odict([("reta_fw", reta_fw_tight), ("reta", eFEXfwToFloatConversion(reta_fw_tight,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_tight), ("rhad", eFEXfwToFloatConversion(rhad_fw_tight,bitshift_rhad)), 
               ("etamin", -49), ("etamax", -24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_tight), ("reta", eFEXfwToFloatConversion(reta_fw_tight,bitshift_reta)), 
               ("wstot_fw", wstot_fw_tight), ("wstot", eFEXfwToFloatConversion_wstot(wstot_fw_tight,bitshift_wstot)), 
               ("rhad_fw", rhad_fw_tight), ("rhad", eFEXfwToFloatConversion(rhad_fw_tight,bitshift_rhad)), 
               ("etamin", -24), ("etamax", 24), ("priority", 0)]),
        odict([("reta_fw", reta_fw_tight), ("reta", eFEXfwToFloatConversion(reta_fw_tight,bitshift_reta)), 
               ("wstot_fw", 0), ("wstot", 0), 
               ("rhad_fw", rhad_fw_tight), ("rhad", eFEXfwToFloatConversion(rhad_fw_tight,bitshift_rhad)), 
               ("etamin",  24), ("etamax", 49), ("priority", 0)]),
        # More granular cuts from -24 to 25 are specified in FexThresholdParameters
    ]
    confObj["ptMinToTopo"] = 0.6 if do_HI_tob_thresholds else 3
    confObj["maxEt"] = 60
    confObj["resolutionMeV"] = 100

    # Add any eta-dependent cuts that are defined for specific working points
    # with higher priority than the low-granularity values above
    eEM_eta_cuts = eta_dependent_cuts["eEM"]
    for wp in confObj["workingPoints"]:
        if wp in eEM_eta_cuts:
            # Check that all cut vector lengths are matching
            # the eta range
            etarange = eEM_eta_cuts[wp]["etarange"]
            stride = etarange[2]
            n_eta_bins = (etarange[1]-etarange[0]) / stride
            assert len(eEM_eta_cuts[wp]["rhad"]) == n_eta_bins
            assert len(eEM_eta_cuts[wp]["rhad"]) == n_eta_bins
            assert len(eEM_eta_cuts[wp]["wstot"]) == n_eta_bins
            for ieta, etalow in enumerate(range(*etarange)):
                reta_cut  = eEM_eta_cuts[wp]["reta"][ieta]
                rhad_cut  = eEM_eta_cuts[wp]["rhad"][ieta]
                wstot_cut = eEM_eta_cuts[wp]["wstot"][ieta]
                confObj["workingPoints"][wp].append(
                    odict([("reta_fw", reta_cut), ("reta", eFEXfwToFloatConversion(reta_cut,bitshift_reta)), 
                    ("wstot_fw", wstot_cut), ("wstot", wstot_cut),
                    ("rhad_fw", rhad_cut), ("rhad", eFEXfwToFloatConversion(rhad_cut,bitshift_rhad)), 
                    ("etamin", etalow), ("etamax", etalow+stride), ("priority", 2)])
                )

    # Check that FW values are integers
    for wp in confObj["workingPoints"]:
        for ssthr in confObj["workingPoints"][wp]:
            for ssthr_i in ssthr:
                if "_fw" in ssthr_i:
                     if not isinstance(ssthr[ssthr_i], int):
                          raise RuntimeError("Threshold %s in eEM configuration is not an integer!", ssthr_i )

    # Check that T >= M >= L [ATR-27796]
    for var in ["reta_fw","rhad_fw","wstot_fw"]:
        validate_ordering(var,"Loose","Medium",confObj["workingPoints"])
        validate_ordering(var,"Medium","Tight",confObj["workingPoints"])

    return confObj

def getConfig_jEM():

    iso_loose_float = 0.1 # PLACEHOLDER
    iso_medium_float = 0.1 # PLACEHOLDER
    iso_tight_float = 0.1 # PLACEHOLDER    
    frac_loose_float = 0.2 # PLACEHOLDER
    frac_medium_float = 0.2 # PLACEHOLDER
    frac_tight_float = 0.2 # PLACEHOLDER
    frac2_loose_float = 0.3 # PLACEHOLDER
    frac2_medium_float = 0.3 # PLACEHOLDER
    frac2_tight_float = 0.3 # PLACEHOLDER

    confObj = odict()
    confObj["workingPoints"] = odict()
    confObj["workingPoints"]["Loose"] = [
        odict([("iso_fw", jFEXfloatToFWConversion(iso_loose_float)), ("iso", iso_loose_float),
               ("frac_fw", jFEXfloatToFWConversion(frac_loose_float)), ("frac", frac_loose_float), 
               ("frac2_fw", jFEXfloatToFWConversion(frac2_loose_float)), ("frac2", frac2_loose_float), 
               ("etamin", -49), ("etamax", 49), ("priority", 0)]),
    ]
    confObj["workingPoints"]["Medium"] = [
        odict([("iso_fw", jFEXfloatToFWConversion(iso_medium_float)), ("iso", iso_medium_float), 
               ("frac_fw", jFEXfloatToFWConversion(frac_medium_float)), ("frac", frac_medium_float), 
               ("frac2_fw", jFEXfloatToFWConversion(frac2_medium_float)), ("frac2", frac2_medium_float), 
               ("etamin", -49), ("etamax", 49), ("priority", 0)]),
    ]
    confObj["workingPoints"]["Tight"] = [
        odict([("iso_fw", jFEXfloatToFWConversion(iso_tight_float)), ("iso", iso_tight_float), 
               ("frac_fw", jFEXfloatToFWConversion(frac_tight_float)), ("frac", frac_tight_float), 
               ("frac2_fw", jFEXfloatToFWConversion(frac2_tight_float)), ("frac2", frac2_tight_float), 
               ("etamin", -49), ("etamax", 49), ("priority", 0)]),
    ]
    confObj["ptMinToTopo1"] = 5 # PLACEHOLDER
    confObj["ptMinToTopo2"] = 5 # PLACEHOLDER
    confObj["ptMinToTopo3"] = 5 # PLACEHOLDER
    confObj["ptMinxTOB1"] = 5 # PLACEHOLDER
    confObj["ptMinxTOB2"] = 5 # PLACEHOLDER
    confObj["ptMinxTOB3"] = 5 # PLACEHOLDER
    confObj["maxEt"] = 50 # PLACEHOLDER
    confObj["resolutionMeV"] = 200

    # Check that FW values are integers
    for wp in confObj["workingPoints"]:
        for ssthr in confObj["workingPoints"][wp]:
            for ssthr_i in ssthr:
                if "_fw" in ssthr_i:
                     if not isinstance(ssthr[ssthr_i], int):
                          raise RuntimeError("Threshold %s in jEM configuration is not an integer!", ssthr_i )

    # Check that T >= M >= L [ATR-27796]
    for var in ["iso_fw","frac_fw","frac2_fw"]:
        validate_ordering(var,"Loose","Medium",confObj["workingPoints"])
        validate_ordering(var,"Medium","Tight",confObj["workingPoints"])

    return confObj


@dataclass
class L1Config_eTAU:
    # Working points here translate to cuts on both RCore (or BDT) and RHad
    # For individual thresholds, the RCore/BDT and RHad working points are
    # set independently, so the two variables are not coupled

    # The appropriate RCore or BDT cut values will be loaded into the RCore config variable,
    # depending on if the Trigger.L1.Menu.doeFexBDTTau flag is enabled

    # The eTAU TOB only has 2 WP bits, allowing None/Loose/Medium/Tight values

    # Heuristic eTAU RCore cuts
    # 8 bits (0 - 255), rCore > threshold -> pass
    # rCore = 1 - (3x2)/(9x2)
    rCore_fw_loose = 2
    rCore_fw_medium = 12
    rCore_fw_tight = 32

    # BDT eTAU score cuts
    # 12 bits (0 - 4095), BDT > 4 * threshold -> pass
    # CAREFUL!! THE THRESHOLDS HERE ARE MULTIPLIED BY 4 IN THE eFEX FIRMWARE!
    BDT_fw_loose = 221
    BDT_fw_medium = 224
    BDT_fw_tight = 225

    # RHad isolation cuts
    # 8 bits (0 - 255), rHad > threshold -> pass
    # Only used on HL/HM/HT L1 items
    # Independent of the rCore/BDT cut, available for both the heuristic (original) and BDT eTAU algorithm
    rHad_fw_loose = 32
    rHad_fw_medium = 72
    rHad_fw_tight = 152

    # Bitshift parameter, see https://indico.cern.ch/event/1026972/contributions/4312070/attachments/2226175/3772176/Copy%20of%20Reta_Threshold_Setting.pdf
    bitshift_rCore = 3
    bitshift_rHad = 3


    def __call__(self, do_eFex_BDT_Tau=True, do_HI_tob_thresholds=False) -> odict:
        # Load either RCore or BDT cut thresholds
        rCore_fw_loose = self.BDT_fw_loose if do_eFex_BDT_Tau else self.rCore_fw_loose
        rCore_fw_medium = self.BDT_fw_medium if do_eFex_BDT_Tau else self.rCore_fw_medium
        rCore_fw_tight = self.BDT_fw_tight if do_eFex_BDT_Tau else self.rCore_fw_tight

        confObj = odict()
        confObj["workingPoints"] = odict()
        confObj["workingPoints"]["Loose"] = [
            odict([("rCore", eTAUfwToFloatConversion_bdt(rCore_fw_loose) if do_eFex_BDT_Tau else eFEXfwToFloatConversion(rCore_fw_loose, self.bitshift_rCore)), ("rCore_fw", rCore_fw_loose), 
                   ("rHad", eFEXfwToFloatConversion(self.rHad_fw_loose, self.bitshift_rHad)), ("rHad_fw", self.rHad_fw_loose),
                  ]), 
        ]
        confObj["workingPoints"]["Medium"] = [
            odict([("rCore", eTAUfwToFloatConversion_bdt(rCore_fw_medium) if do_eFex_BDT_Tau else eFEXfwToFloatConversion(rCore_fw_medium, self.bitshift_rCore)), ("rCore_fw", rCore_fw_medium), 
                   ("rHad", eFEXfwToFloatConversion(self.rHad_fw_medium, self.bitshift_rHad)), ("rHad_fw", self.rHad_fw_medium), 
                 ]),
        ]
        confObj["workingPoints"]["Tight"] = [
            odict([("rCore", eTAUfwToFloatConversion_bdt(rCore_fw_tight) if do_eFex_BDT_Tau else eFEXfwToFloatConversion(rCore_fw_tight, self.bitshift_rCore)), ("rCore_fw", rCore_fw_tight), 
                   ("rHad", eFEXfwToFloatConversion(self.rHad_fw_tight, self.bitshift_rHad)), ("rHad_fw", self.rHad_fw_tight), 
                 ]),
        ]
        confObj["ptMinToTopo"] = 0.6 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["resolutionMeV"] = 100
        confObj["minIsoEt"] = 13.0 # Minimum Et for the BDT cut, in units of GeV (internally with 16-bit resolution, in units of 100 MeV)
        confObj["maxEt"] = 50 # Maximum Et for the RCore/BDT/RHad cuts, in units of GeV
        confObj["algoVersion"] = int(do_eFex_BDT_Tau)

        # Check that FW values are integers
        for wp in confObj["workingPoints"]:
            for ssthr in confObj["workingPoints"][wp]:
                for ssthr_i in ssthr:
                    if "_fw" in ssthr_i:
                         if not isinstance(ssthr[ssthr_i], int):
                              raise RuntimeError("Threshold %s in eTAU configuration is not an integer!", ssthr_i )
                         elif ssthr[ssthr_i] < 0:
                            raise RuntimeError("Threshold %s in eTAU configuration is negative!", ssthr_i )
                         
        # Check that T >= M >= L [ATR-27796]
        for var in ["rCore_fw","rHad_fw"]:
            validate_ordering(var,"Loose","Medium",confObj["workingPoints"])
            validate_ordering(var,"Medium","Tight",confObj["workingPoints"])

        return confObj

getConfig_eTAU = L1Config_eTAU()

@dataclass
class L1Config_cTAU:
    # cTAU isolation parameters (ATR-28621):
    # isolation: 12 bits (0 - 4095)
    # jTAUCoreScale: 11 bits (0 - 2047)
    # (jTAU.EtIso + isolation_jTAUCoreScale_fw/1024 * jTAU.Et) / eTAU.Et < isolation_fw/1024 -> pass

    # eTAU rCore/BDT WP: 2 bits (0 - 3)
    # We can only specify the WP of the rCore/BDT selection, as defined in L1Config_eTAU above

    # eTAU rHad WP: 2 bits (0 - 3)
    # We can only specify the WP of the rHad selection, as defined in L1Config_eTAU above

    class eTAUWP(Enum):
        NoSelection = 0
        Loose = 1
        Medium = 2
        Tight = 3
        # Same values as in the TrigConf::Selection::WP enum

        def rCoreMinCut(self, do_eFex_BDT_Tau=True) -> float:
            return 0.0 if self is self.NoSelection else getConfig_eTAU(do_eFex_BDT_Tau)['workingPoints'][self.name][0]['rCore']
        def rHadMinCut(self, do_eFex_BDT_Tau=True) -> float:
            return 0.0 if self is self.NoSelection else getConfig_eTAU(do_eFex_BDT_Tau)['workingPoints'][self.name][0]['rHad']

    # Generic L, M and T WPs, for the cTAUSpare1/2
    isolation_fw_loose: int = 410
    isolation_jTAUCoreScale_fw_loose: int = 0
    eTAU_rCoreMin_WP_fw_loose: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_loose: eTAUWP = eTAUWP.NoSelection

    isolation_fw_medium: int = 358
    isolation_jTAUCoreScale_fw_medium: int = 0
    eTAU_rCoreMin_WP_fw_medium: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_medium: eTAUWP = eTAUWP.NoSelection

    isolation_fw_tight: int = 307
    isolation_jTAUCoreScale_fw_tight: int = 0
    eTAU_rCoreMin_WP_fw_tight: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_tight: eTAUWP = eTAUWP.NoSelection

    # Dedicated M thresholds for the primary items:
    #cTAU12M (Medium12)
    isolation_fw_medium12: int = 400
    isolation_jTAUCoreScale_fw_medium12: int = 0
    eTAU_rCoreMin_WP_fw_medium12: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_medium12: eTAUWP = eTAUWP.NoSelection

    #cTAU20M (Medium20)
    isolation_fw_medium20: int = 600 + 550
    isolation_jTAUCoreScale_fw_medium20: int = 550
    eTAU_rCoreMin_WP_fw_medium20: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_medium20: eTAUWP = eTAUWP.NoSelection

    #cTAU30M (Medium30)
    isolation_fw_medium30: int = 600 + 550
    isolation_jTAUCoreScale_fw_medium30: int = 550
    eTAU_rCoreMin_WP_fw_medium30: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_medium30: eTAUWP = eTAUWP.NoSelection

    #cTAU35M (Medium35)
    isolation_fw_medium35: int = 600 + 550
    isolation_jTAUCoreScale_fw_medium35: int = 550
    eTAU_rCoreMin_WP_fw_medium35: eTAUWP = eTAUWP.NoSelection
    eTAU_rHadMin_WP_fw_medium35: eTAUWP = eTAUWP.NoSelection

    def __post_init__(self):
        # By default, duplicate the configs of isolation_fw_loose and isolation_fw_tight:
        for default_wp, wp_list in {'Loose': ['Loose12', 'Loose20', 'Loose30', 'Loose35'], 'Tight': ['Tight12', 'Tight20', 'Tight30', 'Tight35']}.items():
            for wp in wp_list:
                setattr(self, f'isolation_fw_{wp.lower()}', getattr(self, f'isolation_fw_{default_wp.lower()}'))
                setattr(self, f'isolation_jTAUCoreScale_fw_{wp.lower()}', getattr(self, f'isolation_jTAUCoreScale_fw_{default_wp.lower()}'))
                setattr(self, f'eTAU_rCoreMin_WP_fw_{wp.lower()}', getattr(self, f'eTAU_rCoreMin_WP_fw_{default_wp.lower()}'))
                setattr(self, f'eTAU_rHadMin_WP_fw_{wp.lower()}', getattr(self, f'eTAU_rHadMin_WP_fw_{default_wp.lower()}'))

    def __call__(self, do_eFex_BDT_Tau=True) -> odict:
        confObj = odict()
        confObj['workingPoints'] = odict()

        for wp in ['Loose', 'Medium', 'Tight', 'Loose12', 'Loose20', 'Loose30', 'Loose35', 'Medium12', 'Medium20', 'Medium30', 'Medium35', 'Tight12', 'Tight20', 'Tight30', 'Tight35']:
            confObj['workingPoints'][wp] = [
                odict([('isolation', cTAUfwToFlowConversion(getattr(self, f'isolation_fw_{wp.lower()}'))), ('isolation_fw', getattr(self, f'isolation_fw_{wp.lower()}')),
                       ('isolation_jTAUCoreScale', cTAUfwToFlowConversion(getattr(self, f'isolation_jTAUCoreScale_fw_{wp.lower()}'))), ('isolation_jTAUCoreScale_fw', getattr(self, f'isolation_jTAUCoreScale_fw_{wp.lower()}')),
                       ('eTAU_rCoreMin', getattr(self, f'eTAU_rCoreMin_WP_fw_{wp.lower()}').rCoreMinCut(do_eFex_BDT_Tau)), ('eTAU_rCoreMin_WP_fw', getattr(self, f'eTAU_rCoreMin_WP_fw_{wp.lower()}').value),
                       ('eTAU_rHadMin', getattr(self, f'eTAU_rHadMin_WP_fw_{wp.lower()}').rHadMinCut(do_eFex_BDT_Tau)), ('eTAU_rHadMin_WP_fw', getattr(self, f'eTAU_rHadMin_WP_fw_{wp.lower()}').value)]),
            ]

        confObj['resolutionMeV'] = 100

        # Check that FW values are integers
        for wp in confObj['workingPoints']:
            for ssthr in confObj['workingPoints'][wp]:
                for ssthr_i in ssthr:
                    if '_fw' in ssthr_i:
                         if not isinstance(ssthr[ssthr_i], int):
                              raise RuntimeError(f'Threshold {ssthr_i} in cTAU configuration is not an integer!')
                         elif ssthr[ssthr_i] < 0:
                            raise RuntimeError('Threshold {ssthr_i} in cTAU configuration is negative!')

        return confObj

getConfig_cTAU = L1Config_cTAU()


@dataclass
class L1Config_jTAU:
    # jTAU isolation cut:
    # 10 bits (0 - 1023)
    # jTAU.EtIso / jTAU.Et < isolation_fw/1024 -> pass

    isolation_fw_loose: int = 410
    isolation_fw_medium: int = 358
    isolation_fw_tight: int = 307


    def __call__(self,do_HI_tob_thresholds=False) -> odict:
        confObj = odict()
        confObj["workingPoints"] = odict()
        confObj["workingPoints"]["Loose"] = [
            odict([("isolation", cTAUfwToFlowConversion(self.isolation_fw_loose)), ("isolation_fw", self.isolation_fw_loose), 
                  ]),
        ]
        confObj["workingPoints"]["Medium"] = [
            odict([("isolation", cTAUfwToFlowConversion(self.isolation_fw_medium)), ("isolation_fw", self.isolation_fw_medium), 
                  ]),
        ]
        confObj["workingPoints"]["Tight"] = [
            odict([("isolation", cTAUfwToFlowConversion(self.isolation_fw_tight)), ("isolation_fw", self.isolation_fw_tight), 
                  ]),
        ]
        confObj["ptMinToTopo1"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["ptMinToTopo2"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["ptMinToTopo3"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["ptMinxTOB1"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["ptMinxTOB2"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["ptMinxTOB3"] = 1 if do_HI_tob_thresholds else 5 # PLACEHOLDER
        confObj["resolutionMeV"] = 200
        confObj["maxEt"] = 50 # PLACEHOLDER

        # Check that FW values are integers
        for wp in confObj["workingPoints"]:
            for ssthr in confObj["workingPoints"][wp]:
                for ssthr_i in ssthr:
                    if "_fw" in ssthr_i:
                         if not isinstance(ssthr[ssthr_i], int):
                              raise RuntimeError("Threshold %s in jTAU configuration is not an integer!", ssthr_i )
                         elif ssthr[ssthr_i] < 0:
                            raise RuntimeError("Threshold %s in jTAU configuration is negative!", ssthr_i )

        # Check that T >= M >= L [ATR-27796]
        # Ordering is inverted here: larger value is looser
        for var in ["isolation_fw"]:
            validate_ordering(var,"Medium","Loose",confObj["workingPoints"])
            validate_ordering(var,"Tight","Medium",confObj["workingPoints"])
        return confObj

getConfig_jTAU = L1Config_jTAU()


def getConfig_jJ(do_HI_tob_thresholds):
    confObj = odict()
    confObj["ptMinToTopo1"] = 5 if do_HI_tob_thresholds else 15
    confObj["ptMinToTopo2"] = 5 if do_HI_tob_thresholds else 15
    confObj["ptMinToTopo3"] = 5 if do_HI_tob_thresholds else 15
    confObj["ptMinxTOB1"] = 5 if do_HI_tob_thresholds else 15
    confObj["ptMinxTOB2"] = 5 if do_HI_tob_thresholds else 15
    confObj["ptMinxTOB3"] = 5 if do_HI_tob_thresholds else 15
    confObj["resolutionMeV"] = 200  
    confObj["seedThreshold1"] = -1 # signed, in GeV, negative values in practice mean no seed threshold. 
    confObj["seedThreshold2"] = -1 # Max value (HW constraint): (2^20)-1 * 25MeV = 26214.375 (GeV)
    confObj["seedThreshold3"] = -1 
    return confObj

def getConfig_jLJ():
    confObj = odict()
    confObj["ptMinToTopo1"] = 15 # PLACEHOLDER
    confObj["ptMinToTopo2"] = 15 # PLACEHOLDER
    confObj["ptMinToTopo3"] = 15 # PLACEHOLDER
    confObj["ptMinxTOB1"] = 15 # PLACEHOLDER
    confObj["ptMinxTOB2"] = 15 # PLACEHOLDER
    confObj["ptMinxTOB3"] = 15 # PLACEHOLDER
    confObj["resolutionMeV"] = 200
    return confObj

def getConfig_gJ():
    confObj = odict()
    confObj["ptMinToTopo1"] = 0 
    confObj["ptMinToTopo2"] = 0 
    confObj["resolutionMeV"] = 200
    return confObj

def getConfig_gLJ():
    confObj = odict()
    confObj["ptMinToTopo1"] = 6 
    confObj["ptMinToTopo2"] = 6 
    confObj["seedThrA"] = 20
    confObj["seedThrB"] = 20
    confObj["seedThrC"] = 20 
    confObj["rhoTowerMinA"] = -9.6 
    confObj["rhoTowerMinB"] = -9.6 
    confObj["rhoTowerMinC"] = -9.6 
    confObj["rhoTowerMaxA"] = 10 
    confObj["rhoTowerMaxB"] = 10 
    confObj["rhoTowerMaxC"] = 10 
    confObj["resolutionMeV"] = 200

    # Check that all values are integers in MeV
    for param in confObj:
        if int(confObj[param]*1000) != (confObj[param]*1000):
            raise RuntimeError("Param %s in gLJ configuration is not an integer in MeV! %d", param, confObj[param])
    return confObj

def getConfig_jXE():
    confObj = odict()
    confObj["resolutionMeV"] = 200
    return confObj

def getConfig_jTE():

    def convertTowerToEta(tower, module):  #ATR-21235
        boundaries1 = {0:16, 1:17, 2:18, 3:19, 4:20, 5:21, 6:22, 7:23, 8:24, 9:25, 10:27, 11:29, 12:31, 13:32, 14:49}
        boundaries2 = {0:8, 1:9, 2:10, 3:11, 4:12, 5:13, 6:14, 7:15, 8:16}
        boundaries3 = {0:0, 1:1, 2:2, 3:3, 4:4, 5:5, 6:6, 7:7, 8:8}
        if module==1:
            return boundaries1[tower]
        elif module==2:
            return boundaries2[tower]
        elif module==3:
            return boundaries3[tower]
        else:
            raise RuntimeError("getConfig_jTE::convertTowerToEta: module not recognised") 

    module1 = 9
    module2 = 4
    module3 = 4

    confObj = odict()
    confObj["etaBoundary1"] = convertTowerToEta(module1,1)
    confObj["etaBoundary1_fw"] = module1
    confObj["etaBoundary2"] = convertTowerToEta(module2,2)
    confObj["etaBoundary2_fw"] = module2
    confObj["etaBoundary3"] = convertTowerToEta(module3,3)
    confObj["etaBoundary3_fw"] = module3
    confObj["resolutionMeV"] = 200
    return confObj

def getConfig_gXE(do_HI_tob_thresholds):
    confObj = odict()
    confObj["seedThrA"] = 8 if do_HI_tob_thresholds else 80 #must be a multiple of 4
    confObj["seedThrB"] = 8 if do_HI_tob_thresholds else 80 #must be a multiple of 4
    confObj["seedThrC"] = 8 if do_HI_tob_thresholds else 80 #must be a multiple of 4
    confObj["XERHO_sigmaPosA"] = 3 
    confObj["XERHO_sigmaPosB"] = 3 
    confObj["XERHO_sigmaPosC"] = 3 
    confObj["XERHO_sigmaNegA"] = 8 
    confObj["XERHO_sigmaNegB"] = 8 
    confObj["XERHO_sigmaNegC"] = 8 
    confObj["XEJWOJ_a_A"] = 1003 
    confObj["XEJWOJ_a_B"] = 1003 
    confObj["XEJWOJ_a_C"] = 1003 
    confObj["XEJWOJ_b_A"] = 409 
    confObj["XEJWOJ_b_B"] = 409 
    confObj["XEJWOJ_b_C"] = 0 
    confObj["XEJWOJ_c_A"] = 0 
    confObj["XEJWOJ_c_B"] = 0 
    confObj["XEJWOJ_c_C"] = 0 
    confObj["resolutionMeV"] = 200
    return confObj

def getConfig_gTE():
    confObj = odict()
    confObj["resolutionMeV"] = 200
    return confObj


# LEGACY

def getConfig_EM(do_HI_tob_thresholds):
    confObj = odict()
    confObj["isolation"] = odict()
    confObj["isolation"]["HAIsoForEMthr"] = odict([ ( "thrtype", "HAIsoForEMthr" ), ("Parametrization", []) ])
    confObj["isolation"]["HAIsoForEMthr"]["Parametrization"] += [
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 1), ("mincut", 10), ("offset", -2), ("priority", 0), ("slope", 230), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 2), ("mincut",  0), ("offset",  0), ("priority", 0), ("slope",   0), ("upperlimit",  0)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 3), ("mincut", 10), ("offset", -2), ("priority", 0), ("slope", 230), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 4), ("mincut", 10), ("offset", -2), ("priority", 0), ("slope", 230), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 5), ("mincut", 10), ("offset", -2), ("priority", 0), ("slope", 230), ("upperlimit", 50)]),
    ]
    confObj["isolation"]["EMIsoForEMthr"] = odict([ ("thrtype", "EMIsoForEMthr" ), ("Parametrization", []) ])
    confObj["isolation"]["EMIsoForEMthr"]["Parametrization"] += [
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 1), ("mincut",  0), ("offset",   0), ("priority", 0), ("slope",  0), ("upperlimit",  0)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 2), ("mincut", 20), ("offset", -18), ("priority", 0), ("slope", 80), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 3), ("mincut", 20), ("offset", -18), ("priority", 0), ("slope", 80), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 4), ("mincut", 10), ("offset", -20), ("priority", 0), ("slope", 80), ("upperlimit", 50)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 5), ("mincut", 20), ("offset", -18), ("priority", 0), ("slope", 80), ("upperlimit", 50)]),
    ]
    confObj["ptMinToTopo"] = 8 if do_HI_tob_thresholds else 3
    confObj["resolutionMeV"] = 500
    return confObj


def getConfig_TAU(do_HI_tob_thresholds):
    confObj = odict()
    confObj["isolation"] = odict()
    confObj["isolation"]["EMIsoForTAUthr"] =  odict([ ( "thrtype", "EMIsoForTAUthr" ), ("Parametrization", []) ])
    confObj["isolation"]["EMIsoForTAUthr"]["Parametrization"] += [
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 1), ("mincut", 0), ("offset", 30), ("priority", 0), ("slope", 100), ("upperlimit",  60)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 2), ("mincut", 0), ("offset", 20), ("priority", 0), ("slope", 100), ("upperlimit",  60)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 3), ("mincut", 0), ("offset", 15), ("priority", 0), ("slope", 100), ("upperlimit",  60)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 4), ("mincut", 0), ("offset", 40), ("priority", 0), ("slope",   0), ("upperlimit", 124)]),
        odict([ ("etamax", 49), ("etamin", -49), ("isobit", 5), ("mincut", 0), ("offset", 30), ("priority", 0), ("slope", 100), ("upperlimit",  60)])
    ]
    confObj["ptMinToTopo"] = 1 if do_HI_tob_thresholds else 8
    confObj["resolutionMeV"] = 500
    return confObj


def getConfig_JET():
    confObj = odict()
    confObj["ptMinToTopoLargeWindow"] = 12
    confObj["ptMinToTopoSmallWindow"] = 12
    return confObj


def getConfig_XS():
    confObj = odict()
    confObj["significance"] = odict()
    confObj["significance"]["xeMin"] = 11
    confObj["significance"]["xeMax"] = 63
    confObj["significance"]["teSqrtMin"] = 4
    confObj["significance"]["teSqrtMax"] = 63
    confObj["significance"]["xsSigmaScale"] = 1150
    confObj["significance"]["xsSigmaOffset"] = 1640
    return confObj

