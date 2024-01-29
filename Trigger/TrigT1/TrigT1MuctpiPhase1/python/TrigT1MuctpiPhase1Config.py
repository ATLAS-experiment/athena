# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

#Muon RecRoiTools
from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import RPCRecRoiToolCfg, TGCRecRoiToolCfg

def TrigThresholdDecisionToolCfg(flags, name="TrigThresholdDecisionTool"):
  acc = ComponentAccumulator()
  tool = CompFactory.getComp("LVL1::TrigThresholdDecisionTool")(name)
  tool.RPCRecRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags))
  tool.TGCRecRoiTool = acc.popToolsAndMerge(TGCRecRoiToolCfg(flags))
  acc.setPrivateTools(tool)
  return acc


def MUCTPI_AthToolCfg(flags, name):
  acc = ComponentAccumulator()
  tool = CompFactory.getComp("LVL1MUCTPIPHASE1::MUCTPI_AthTool")(name)
  tool.RPCRecRoiTool = acc.popToolsAndMerge(RPCRecRoiToolCfg(flags))
  tool.TGCRecRoiTool = acc.popToolsAndMerge(TGCRecRoiToolCfg(flags))
  tool.TrigThresholdDecisionTool = acc.popToolsAndMerge(TrigThresholdDecisionToolCfg(flags))
  #tool.MUCTPI_xAODLocation = ["LVL1MuonRoIsBCm2", "LVL1MuonRoIsBCm1", "LVL1MuonRoIs", "LVL1MuonRoIsBCp1", "LVL1MuonRoIsBCp2"]
  # Create a logger:
  logger = logging.getLogger( "MUCTPI_AthTool" )

  # Set properties of the LUT overlap handling:
  tool.OverlapStrategyName = flags.Trigger.MUCTPI.OverlapStrategy
  
  # Decide which LUT to use, based on which run we are simulating:
  tool.LUTXMLFile = flags.Trigger.MUCTPI.LUTXMLFile 
  logger.info( "Configuring MuCTPI simulation with configuration file: %s", tool.LUTXMLFile )

  if flags.Trigger.doHLT:
      # Check the RoI EDM containers are registered in HLT outputs
      from TrigEDMConfig.TriggerEDM import recordable
      for key in tool.MUCTPI_xAODLocation:
        logger.info( "Configuring MuCTPI simulation with configuration outputs: %s", key )
        assert key==recordable(key), f'recordable() check failed for {key}'
  logger.info( "Configuring MuCTPI: post flags.Trigger.doHLT" )

  acc.setPrivateTools(tool)
  return acc


def MUCTPI_AthAlgCfg(flags):
  acc = ComponentAccumulator()
  alg = CompFactory.getComp("LVL1MUCTPIPHASE1::MUCTPI_AthAlg")(name="MUCTPI_AthAlg")
  alg.MUCTPI_AthTool = acc.popToolsAndMerge(MUCTPI_AthToolCfg(flags, name="MUCTPI_AthTool"))
  acc.addEventAlgo(alg)
  return acc
