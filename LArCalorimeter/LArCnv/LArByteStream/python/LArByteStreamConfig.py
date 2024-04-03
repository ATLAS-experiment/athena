# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArRawDataContByteStreamToolCfg (flags,
                                     name="LArRawDataContByteStreamTool",
                                     InitializeForWriting = False,
                                     DSPRunMode = 4,
                                     RodBlockVersion = 0,
                                     **kwargs):
      acc = ComponentAccumulator()
      tool = CompFactory.LArRawDataContByteStreamTool(name, **kwargs)
      tool.InitializeForWriting = InitializeForWriting
      tool.DSPRunMode=DSPRunMode
      tool.RodBlockVersion=RodBlockVersion
      acc.addPublicTool(tool)

      extraOutputs = []

      if InitializeForWriting:
         from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
         acc.merge(CaloNoiseCondAlgCfg(flags, noisetype="totalNoise"))

         from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArFebRodMappingCfg
         acc.merge(LArOnOffIdMappingCfg(flags))
         acc.merge(LArFebRodMappingCfg(flags))

         extraOutputs = {
            ('CaloNoise', 'ConditionStore+totalNoise'),
            ('LArOnOffIdMapping', 'ConditionStore+LArOnOffIdMap'),
            ('LArFebRodMapping', 'ConditionStore+LArFebRodMap'),
            ('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager')
         }

      return acc, extraOutputs
