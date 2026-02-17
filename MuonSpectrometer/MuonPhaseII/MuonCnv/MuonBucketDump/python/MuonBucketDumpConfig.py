#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHitDumperCfg(flags, name="MuonHitDumper", **kwargs):
    result = ComponentAccumulator()
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    kwargs.setdefault("SpacePointKeys", spCont)
    result.addEventAlgo(CompFactory.MuonR4.MlHitDumperAlg(name, **kwargs))
    return result

def MuonBucketDumpCfg(flags, name="MuonBucketDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    kwargs.setdefault("isMC", flags.Input.isMC)
    
    # Optional calorimeter chain + dumper
    doCalo = bool(kwargs.pop("DoCaloDump", False))
    if doCalo:
        result.merge(CaloCellsDumperCfg(flags))
    
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    
    kwargs.setdefault("SpacePointKeys", spCont)
    
    the_alg = CompFactory.MuonR4.BucketDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonSegmentDumpCfg(flags, name="MuonSegmentDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    
    segCont = ""
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        segCont = "MuonSegmentsFromR4"
    
    kwargs.setdefault("SpacePointKeys", spCont)
    kwargs.setdefault("SegmentKeys", segCont)
    
    the_alg = CompFactory.MuonR4.SegmentDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def CaloCellsDumperCfg(flags, name="CaloCellsDumper", **kwargs):
    """
    Configure calorimeter reconstruction up to cells and towers and dump:
      - per-cell energy + position + identifier decoding
      - per-tower energy + (eta,phi) + a derived direction vector    Default input container key:
    Default input container key:
      - "AllCalo" (standard CaloCellContainer produced by CaloRecoCfg)
    If you want supercells instead, pass:
      CellContainerKey="SCell"  (or whatever your CaloRecoCfg produces in your setup)
    """
    result = ComponentAccumulator()

    # Make sure the reconstructed cell container exists from RDO
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    result.merge(CaloRecoCfg(flags))
    
    # Build calorimeter towers from cells (default container name is typically "CombinedTower")
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = result.getPrimaryAndMerge(CaloTowerMakerCfg(flags))

    kwargs.setdefault("CellContainerKey", "AllCalo")
    kwargs.setdefault("MinCellEnergyMeV", 0.0)
    kwargs.setdefault("MaxCells", -1)   # -1 => no cap
    
    kwargs.setdefault("TowerContainerKey", getattr(towerMaker, "TowerContainerName", "CombinedTower"))
    kwargs.setdefault("MinTowerEnergyMeV", 0.0)
    kwargs.setdefault("MaxTowers", -1)  # -1 => no cap

    result.addEventAlgo(CompFactory.MuonR4.CaloCellsDumperAlg(name, **kwargs))
    return result