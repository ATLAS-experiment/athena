# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the MuonHitValAlg nTuple
# This file can be used with Digi_tf by specifying --postInclude MuonPRDTest.HitValAlgDigi.HitValAlgDigiCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC
def HitValAlgDigiCfg(flags, name = "MuonHitValAlg", outFile="MuonHitValAlg.digi.ntuple.root", **kwargs):
    kwargs.setdefault("doTruth", False)
    kwargs.setdefault("doMuEntry", False)

    kwargs.setdefault("doSimHits", False)
    kwargs.setdefault("doSDOs", True)
    kwargs.setdefault("doDigits", True)
    kwargs.setdefault("doRDOs", True)
    from AthenaConfiguration.Enums import ProductionStep
    prefix= flags.Overlay.BkgPrefix if flags.Common.ProductionStep is ProductionStep.PileUpPresampling else ""



    kwargs.setdefault("sTgcSdoKey", f"{prefix}sTGC_SDO")
    kwargs.setdefault("sTgcDigitKey", f"{prefix}sTGC_DIGITS")
    kwargs.setdefault("sTgcRdoKey", f"{prefix}sTGCRDO")

    kwargs.setdefault("MmSdoKey", f"{prefix}MM_SDO")
    kwargs.setdefault("MmDigitKey", f"{prefix}MM_DIGITS")
    kwargs.setdefault("MmRdoKey", f"{prefix}MMRDO")

    kwargs.setdefault("CSC_SDOContainerName", f"{prefix}CSC_SDO")
    kwargs.setdefault("CSC_DigitContainerName", f"{prefix}CSC_DIGITS")
    kwargs.setdefault("CSC_RDOContainerName", f"{prefix}CSCRDO")

    kwargs.setdefault( "MdtSdoKey", f"{prefix}MDT_SDO")
    kwargs.setdefault( "MdtDigitKey", f"{prefix}MDT_DIGITS")

    kwargs.setdefault("RpcSdoKey", f"{prefix}RPC_SDO")
    kwargs.setdefault("RpcDigitKey", f"{prefix}RPC_DIGITS")

    kwargs.setdefault("TgcSdoKey", f"{prefix}TGC_SDO")
    kwargs.setdefault("TgcDigitKey", f"{prefix}TGC_DIGITS")
    kwargs.setdefault("TgcRdoKey", f"{prefix}TGCRDO")


    from MuonPRDTest.MuonPRDTestCfg import AddHitValAlgCfg
    return AddHitValAlgCfg(flags, name = name, outFile=outFile, **kwargs)
