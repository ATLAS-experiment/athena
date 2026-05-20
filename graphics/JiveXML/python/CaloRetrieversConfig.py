# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


def LArDigitRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.LArDigitRetriever(
                name="LArDigitRetriever",
                DoLArDigit=False,
                DoHECDigit=False,
                DoFCalDigit=False, **kwargs
            )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloFCalRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CaloFCalRetriever(
                name="CaloFCalRetriever",
                DoFCalCellDetails=False,
                DoBadFCal=False, **kwargs
            )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloLArRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CaloLArRetriever(
                name="CaloLArRetriever",
                DoLArCellDetails=False,
                DoBadLAr=False,
                LArlCellThreshold = 500 if flags.OnlineEventDisplays.BeamSplashMode else 50, **kwargs
            )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloHECRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CaloHECRetriever(
                name="CaloHECRetriever",
                DoHECCellDetails=False,
                DoBadHEC=False,
                HEClCellThreshold = 500 if flags.OnlineEventDisplays.BeamSplashMode else 50, **kwargs
            )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloClusterRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.CaloClusterRetriever(name = "CaloClusterRetriever",**kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloTileRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        tileDigitsContainer = "TileDigitsCnt"

        if flags.Tile.doOpt2:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif flags.Tile.doOptATLAS:
            tileRawChannelContainer = 'TileRawChannelFixed'
        elif flags.Tile.doFitCOOL:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif flags.Tile.doFit:
            tileRawChannelContainer = 'TileRawChannelFit'
        else:
            tileRawChannelContainer = 'TileRawChannelCnt'

    else:
        if "TileDigitsCnt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsCnt"
        elif "TileDigitsFlt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsFlt"

        if "TileRawChannelOpt2" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif "TileRawChannelFitCool" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif "TileRawChannelFit" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFit'
        elif "TileRawChannelCnt" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelCnt'

    the_tool = CompFactory.JiveXML.CaloTileRetriever(
            name = "CaloTileRetriever",
            TileDigitsContainer = tileDigitsContainer,
            TileRawChannelContainer = tileRawChannelContainer,
            DoTileCellDetails = False,
            DoTileDigit = False,
            DoBadTile = False, **kwargs
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloMBTSRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()

    if flags.Input.Format is Format.BS:
        tileDigitsContainer = "TileDigitsCnt"

        if flags.Tile.doOpt2:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif flags.Tile.doOptATLAS:
            tileRawChannelContainer = 'TileRawChannelFixed'
        elif flags.Tile.doFitCOOL:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif flags.Tile.doFit:
            tileRawChannelContainer = 'TileRawChannelFit'
        else:
            tileRawChannelContainer = 'TileRawChannelCnt'

    else:
        if "TileDigitsCnt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsCnt"
        elif "TileDigitsFlt" in flags.Input.Collections:
            tileDigitsContainer = "TileDigitsFlt"

        if "TileRawChannelOpt2" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelOpt2'
        elif "TileRawChannelFitCool" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFitCool'
        elif "TileRawChannelFit" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelFit'
        elif "TileRawChannelCnt" in flags.Input.Collections:
            tileRawChannelContainer = 'TileRawChannelCnt'

    the_tool = CompFactory.JiveXML.CaloMBTSRetriever(
            name = "CaloMBTSRetriever",
            TileDigitsContainer= tileDigitsContainer,
            TileRawChannelContainer = tileRawChannelContainer,
            DoMBTSDigits = False,  **kwargs
        )
    result.addPublicTool(the_tool, primary=True)
    return result


def CaloRetrieversCfg(flags, **kwargs):
    result = ComponentAccumulator()
    from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
    result.merge(LArADC2MeVCondAlgCfg (flags))

    from CaloJiveXML.CaloJiveXMLConf import JiveXML__LArDigitRetriever
    theLArDigitRetriever = JiveXML__LArDigitRetriever(name="LArDigitRetriever")
    theLArDigitRetriever.DoLArDigit = False
    theLArDigitRetriever.DoHECDigit = False
    theLArDigitRetriever.DoFCalDigit = False

    tools = []

    if (theLArDigitRetriever.DoLArDigit or theLArDigitRetriever.DoHECDigit or theLArDigitRetriever.DoFCalDigit):
        tools += [result.getPrimaryAndMerge(LArDigitRetrieverCfg(flags))]
    else:
        tools += [result.getPrimaryAndMerge(CaloFCalRetrieverCfg(flags))]
        tools += [result.getPrimaryAndMerge(CaloLArRetrieverCfg(flags))]
        tools += [result.getPrimaryAndMerge(CaloHECRetrieverCfg(flags))]

    tools += [result.getPrimaryAndMerge(CaloClusterRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(CaloTileRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(CaloMBTSRetrieverCfg(flags))]

    return result, tools
