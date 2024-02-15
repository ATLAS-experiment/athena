#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def SCT_CalibBsErrorToolCfg(flags, name = 'SCT_CalibBsErrorTool', **kwargs):
    acc = ComponentAccumulator()

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ByteStreamErrorsToolCfg
    kwargs.setdefault('SCT_ByteStreamErrorsTool',
                      acc.popToolsAndMerge(SCT_ByteStreamErrorsToolCfg(flags)))

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCT_CalibBsErrorTool(name, **kwargs))

    return acc


def SCT_CalibEventInfoCfg(flags, name = 'SCT_CalibEventInfo', **kwargs):
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCT_CalibEventInfo(name, **kwargs))

    return acc


def SCT_CalibHitmapToolCfg(flags, name = 'SCT_CalibHitmapTool', **kwargs):
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCT_CalibHitmapTool(name, **kwargs))

    return acc


def SCT_CalibLbToolCfg(flags, name = 'SCT_CalibLbTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('RDOContainer', 'SCT_RDOs')

    kwargs.setdefault('SCT_CalibEventInfo',
                      acc.popToolsAndMerge(SCT_CalibEventInfoCfg(flags)))

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCT_CalibLbTool(name, **kwargs))

    return acc


def SCT_CalibModuleListToolCfg(flags, name = 'SCT_CalibModuleListTool', **kwargs):
    acc = ComponentAccumulator()

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_MonitorConditionsToolCfg
    cond_kwargs = {}
    cond_kwargs.setdefault('FolderDb', '/SCT/Derived/Monitoring')
    cond_kwargs.setdefault('FolderTag', 'SctDerivedMonitoring-RUN2-UPD4-005')
    # In case of reprocessing an older run, the reference run has to be set manually, since it's not the last one uploaded to COOL
    #cond_kwargs.setdefault('Modifiers', '<forceRunNumber>364160</forceRunNumber>')
    cond_kwargs.setdefault('dbInstance', 'SCT_OFL')
    kwargs.setdefault('SCT_MonitorConditionsTool',
                      acc.popToolsAndMerge(SCT_MonitorConditionsToolCfg(flags, cond_kwargs = cond_kwargs)))

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCT_CalibModuleListTool(name, **kwargs))

    return acc


def SCTCalibWriteToolCfg(flags, name = 'SCTCalibWriteTool', **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault('BeginRun', int( flags.SCTCalib.RunNumber ))
    if flags.SCTCalib.DoNoisyStrip and flags.SCTCalib.NoisyUpdate:
        kwargs.setdefault('EndRun', -1) # Open IOV used for UPD4
    else:
        kwargs.setdefault('EndRun', int( flags.SCTCalib.RunNumber ))

    kwargs.setdefault('StreamName', 'SCTCalibStream')

    kwargs.setdefault('TagID4NoisyStrips', flags.SCTCalib.TagID4NoisyStrips)
    kwargs.setdefault('TagID4DeadStrips', flags.SCTCalib.TagID4DeadStrips)
    kwargs.setdefault('TagID4DeadChips', flags.SCTCalib.TagID4DeadChips)
    kwargs.setdefault('TagID4NoiseOccupancy', flags.SCTCalib.TagID4NoiseOccupancy)
    kwargs.setdefault('TagID4RawOccupancy', flags.SCTCalib.TagID4RawOccupancy)
    kwargs.setdefault('TagID4Efficiency', flags.SCTCalib.TagID4Efficiency)
    kwargs.setdefault('TagID4BSErrors', flags.SCTCalib.TagID4BSErrors)
    kwargs.setdefault('TagID4LorentzAngle', flags.SCTCalib.TagID4LorentzAngle)

    from AthenaCommon.Constants import DEBUG
    kwargs.setdefault('OutputLevel', DEBUG)

    # Setup for writing local COOL DB
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, streamName = 'SCTCalibStream'))

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.setPrivateTools(CompFactory.SCTCalibWriteTool(name, **kwargs))

    return acc


def SCTCalibAlgCfg(flags, SORTime, EORTime, nLB, prefix, name = 'SCTCalibAlg', **kwargs):
    acc = ComponentAccumulator()

    #--- Run number
    kwargs.setdefault('RunNumber',   flags.SCTCalib.RunNumber)
    kwargs.setdefault('EventNumber', flags.SCTCalib.EventNumber)
    #--- Run stat/end time read from runInfo.txt
    kwargs.setdefault('RunStartTime', SORTime)
    kwargs.setdefault('RunEndTime',   EORTime)
    kwargs.setdefault('LBMax',        nLB)
    #--- Flag for ConditionsSvc, sorting, hitmaps, BSErros
    kwargs.setdefault('UseConfiguration', flags.SCTCalib.UseConfiguration) # True  in alg default
    kwargs.setdefault('UseCalibration',   flags.SCTCalib.UseCalibration)   # True  in alg default
    kwargs.setdefault('UseMajority',      flags.SCTCalib.UseMajority)      # True  in alg default
    kwargs.setdefault('UseBSError',       flags.SCTCalib.UseBSError)       # False in alg default
    kwargs.setdefault('DoHitMaps',        flags.SCTCalib.DoHitMaps)        # True  in alg default
    kwargs.setdefault('DoHitMapsLB',      flags.SCTCalib.DoHitMapsLB)      # True  in alg default
    kwargs.setdefault('ReadHitMaps',      flags.SCTCalib.ReadHitMaps)      # False in alg default
    kwargs.setdefault('DoBSErrors',       flags.SCTCalib.DoBSErrors)       # False  in alg default

    #--- Flags for input files
    kwargs.setdefault('ReadBS', flags.SCTCalib.ReadBS)
    #--- Input files
    if flags.SCTCalib.InputType == 'NTUP_TRKVALID':
        kwargs.setdefault('InputTrkVal', flags.Input.Files)
    elif flags.SCTCalib.InputType == 'HIST':
        kwargs.setdefault('InputHist',   flags.Input.Files)

    #--- Methods to run
    kwargs.setdefault('DoNoisyStrip',     flags.SCTCalib.DoNoisyStrip)      # True  in alg default
    kwargs.setdefault('DoHV',             flags.SCTCalib.DoHV)              # False in alg default
    if flags.SCTCalib.DoDeadStrip or flags.SCTCalib.DoQuietStrip:
        kwargs.setdefault('DoDeadStrip', True)
    else:
        kwargs.setdefault('DoDeadStrip', False)
    if flags.SCTCalib.DoDeadChip or flags.SCTCalib.DoQuietChip:
        kwargs.setdefault('DoDeadChip',  True)
    else:
        kwargs.setdefault('DoDeadChip',  False)
    kwargs.setdefault('DoNoiseOccupancy', flags.SCTCalib.DoNoiseOccupancy)  # False in alg default
    kwargs.setdefault('DoRawOccupancy',   flags.SCTCalib.DoRawOccupancy)    # False in alg default
    kwargs.setdefault('DoEfficiency',     flags.SCTCalib.DoEfficiency)      # False in alg default
    kwargs.setdefault('DoBSErrorDB',      flags.SCTCalib.DoBSErrorDB)       # False in alg default
    kwargs.setdefault('DoLorentzAngle',   flags.SCTCalib.DoLorentzAngle)    # False in alg default

    #--- Local DB
    kwargs.setdefault('WriteToCool', flags.SCTCalib.WriteToCool) # True in alg default

    #--- Properties for noisy strips
    kwargs.setdefault('LbsPerWindow',           flags.SCTCalib.LbsPerWindow)
    kwargs.setdefault('NoisyUpdate',            flags.SCTCalib.NoisyUpdate)
    kwargs.setdefault('NoisyWriteAllModules',   flags.SCTCalib.NoisyWriteAllModules)
    kwargs.setdefault('NoisyMinStat',           flags.SCTCalib.NoisyMinStat)
    kwargs.setdefault('NoisyStripAll',          flags.SCTCalib.NoisyStripAll)
    kwargs.setdefault('NoisyStripThrDef',       flags.SCTCalib.NoisyStripThrDef)
    kwargs.setdefault('NoisyStripThrOffline',   flags.SCTCalib.NoisyStripThrOffline)
    kwargs.setdefault('NoisyStripThrOnline',    flags.SCTCalib.NoisyStripThrOnline)
    kwargs.setdefault('NoisyWaferFinder',       flags.SCTCalib.NoisyWaferFinder)
    kwargs.setdefault('NoisyWaferWrite',        flags.SCTCalib.NoisyWaferWrite)
    kwargs.setdefault('NoisyWaferAllStrips',    flags.SCTCalib.NoisyWaferAllStrips)
    kwargs.setdefault('NoisyWaferThrBarrel',    flags.SCTCalib.NoisyWaferThrBarrel)
    kwargs.setdefault('NoisyWaferThrECA',       flags.SCTCalib.NoisyWaferThrECA)
    kwargs.setdefault('NoisyWaferThrECC',       flags.SCTCalib.NoisyWaferThrECC)
    kwargs.setdefault('NoisyWaferFraction',     flags.SCTCalib.NoisyWaferFraction)
    kwargs.setdefault('NoisyChipFraction',      flags.SCTCalib.NoisyChipFraction)
    kwargs.setdefault('NoisyUploadTest',        flags.SCTCalib.NoisyUploadTest)
    kwargs.setdefault('NoisyStripAverageInDB',  flags.SCTCalib.NoisyStripAverageInDB)
    kwargs.setdefault('NoisyModuleAverageInDB', flags.SCTCalib.NoisyModuleAverageInDB)
    kwargs.setdefault('NoisyStripLastRunInDB',  flags.SCTCalib.NoisyStripLastRunInDB)
    kwargs.setdefault('NoisyModuleList',        flags.SCTCalib.NoisyModuleList)
    kwargs.setdefault('NoisyModuleDiff',        flags.SCTCalib.NoisyModuleDiff)
    kwargs.setdefault('NoisyStripDiff',         flags.SCTCalib.NoisyStripDiff)

    #--- Properties for dead strips/chips
    kwargs.setdefault('DeadStripMinStat',      flags.SCTCalib.DeadStripMinStat)
    kwargs.setdefault('DeadStripMinStatBusy',  flags.SCTCalib.DeadStripMinStatBusy)
    kwargs.setdefault('DeadChipMinStat',       flags.SCTCalib.DeadChipMinStat)
    kwargs.setdefault('DeadStripSignificance', flags.SCTCalib.DeadStripSignificance)
    kwargs.setdefault('DeadChipSignificance',  flags.SCTCalib.DeadChipSignificance)
    kwargs.setdefault('BusyThr4DeadFinding',   flags.SCTCalib.BusyThr4DeadFinding)
    kwargs.setdefault('NoisyThr4DeadFinding',  flags.SCTCalib.NoisyThr4DeadFinding)
    kwargs.setdefault('DeadChipUploadTest',    flags.SCTCalib.DeadChipUploadTest)
    kwargs.setdefault('DeadStripUploadTest',   flags.SCTCalib.DeadStripUploadTest)
    if flags.SCTCalib.DoDeadStrip or flags.SCTCalib.DoDeadChip:
        kwargs.setdefault('DeadNotQuiet', True)
    else:
        kwargs.setdefault('DeadNotQuiet', False)
    kwargs.setdefault('QuietThresholdChip',    flags.SCTCalib.QuietThresholdChip)
    kwargs.setdefault('QuietThresholdStrip',   flags.SCTCalib.QuietThresholdStrip)

    #--- Properties for HIST
    kwargs.setdefault('NoiseOccupancyTriggerAware', flags.SCTCalib.NoiseOccupancyTriggerAware)
    kwargs.setdefault('NoiseOccupancyMinStat',      flags.SCTCalib.NoiseOccupancyMinStat)
    kwargs.setdefault('RawOccupancyMinStat',        flags.SCTCalib.RawOccupancyMinStat)
    kwargs.setdefault('EfficiencyMinStat',          flags.SCTCalib.EfficiencyMinStat)
    kwargs.setdefault('EfficiencyDoChips',          flags.SCTCalib.EfficiencyDoChips)
    kwargs.setdefault('BSErrorDBMinStat',           flags.SCTCalib.BSErrorDBMinStat)
    kwargs.setdefault('LorentzAngleMinStat',        flags.SCTCalib.LorentzAngleMinStat)
    kwargs.setdefault('LorentzAngleDebugMode',      flags.SCTCalib.LorentzAngleDebugMode)

    #--- Tags for XMLs : have to be consistent with SCTCalibWriteTool
    kwargs.setdefault('TagID4NoisyStrips',    flags.SCTCalib.TagID4NoisyStrips)
    kwargs.setdefault('TagID4DeadStrips',     flags.SCTCalib.TagID4DeadStrips)
    kwargs.setdefault('TagID4DeadChips',      flags.SCTCalib.TagID4DeadChips)
    kwargs.setdefault('TagID4NoiseOccupancy', flags.SCTCalib.TagID4NoiseOccupancy)

    #--- Output XMLs
    kwargs.setdefault('BadStripsAllFile',          prefix + 'BadStripsAllFile.xml')          # All NoisyStrips
    kwargs.setdefault('BadStripsNewFile',          prefix + 'BadStripsNewFile.xml')          # Newly found NoisyStrips
    kwargs.setdefault('BadStripsSummaryFile',      prefix + 'BadStripsSummaryFile.xml')      # Summary of NoisyStrips
    kwargs.setdefault('BadModulesFile',            prefix + 'BadModulesFile.xml')            # HVTrip
    if ( flags.SCTCalib.DoDeadChip or flags.SCTCalib.DoDeadStrip):
        kwargs.setdefault('DeadStripsFile',        prefix + 'DeadStripsFile.xml')            # DeadStrip
        kwargs.setdefault('DeadChipsFile',         prefix + 'DeadChipsFile.xml')             # DeadChip
        kwargs.setdefault('DeadSummaryFile',       prefix + 'DeadSummaryFile.xml')           # Summary of Dead Search
    if ( flags.SCTCalib.DoQuietChip or flags.SCTCalib.DoQuietStrip ):
        kwargs.setdefault('DeadStripsFile',        prefix + 'QuietStripsFile.xml')           # QuietStrip
        kwargs.setdefault('DeadChipsFile',         prefix + 'QuietChipsFile.xml')            # QuietChip
        kwargs.setdefault('DeadSummaryFile',       prefix + 'QuietSummaryFile.xml')          # Summary of Quiet Search
    kwargs.setdefault('NoiseOccupancyFile',        prefix + 'NoiseOccupancyFile.xml')        # NoiseOccupancy
    kwargs.setdefault('NoiseOccupancySummaryFile', prefix + 'NoiseOccupancySummaryFile.xml') # Summary of NoiseOccupancy
    kwargs.setdefault('LorentzAngleFile',          prefix + 'LorentzAngleFile.xml')          # LorentzAngle
    kwargs.setdefault('LorentzAngleSummaryFile',   prefix + 'LorentzAngleSummaryFile.xml')   # Summary of LorentzAngle

    kwargs.setdefault('RawOccupancySummaryFile',   prefix + 'RawOccupancySummaryFile.xml')   # Summary of RawOccupancy
    kwargs.setdefault('EfficiencyModuleFile',      prefix + 'EfficiencyModuleSummary.xml')   # Module Efficiency
    kwargs.setdefault('EfficiencyChipFile',        prefix + 'EfficiencyChipSummary.xml')     # Chip Efficiency
    kwargs.setdefault('EfficiencySummaryFile',     prefix + 'EfficiencySummaryFile.xml')     # Summary of Efficiency
    kwargs.setdefault('BSErrorSummaryFile',        prefix + 'BSErrorSummaryFile.xml')        # Summary of BS Errors
    kwargs.setdefault('BSErrorModuleFile',         prefix + 'BSErrorModuleSummary.xml')      # BS Errors for each module

    from AthenaCommon.Constants import WARNING
    kwargs.setdefault('OutputLevel',     WARNING) # DEBUG / INFO / WARNING / ERROR / FATAL
    kwargs.setdefault('AuditAlgorithms', True)


    # Configuring SCTCalibWriteTool
    kwargs.setdefault('SCTCalibWriteTool',
                      acc.popToolsAndMerge(SCTCalibWriteToolCfg(flags)))

    # Configuring SCT_ConfigurationConditionsTool
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConfigurationConditionsToolCfg
    kwargs.setdefault('SCT_ConfigurationConditionsTool',
                      acc.popToolsAndMerge(SCT_ConfigurationConditionsToolCfg(flags)))

    # Configuring SCT_ReadCalibDataTool
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ReadCalibDataToolCfg
    kwargs.setdefault('SCT_ReadCalibDataTool',
                      acc.popToolsAndMerge(SCT_ReadCalibDataToolCfg(flags)))

    # Configuring SCT_MajorityConditionsTool
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_MajorityConditionsCfg
    kwargs.setdefault('SCT_MajorityConditionsTool',
                      acc.popToolsAndMerge(SCT_MajorityConditionsCfg(flags)))

    # Configuring SCTCablingTool
    from SCT_Cabling.SCT_CablingConfig import SCT_CablingToolCfg
    kwargs.setdefault('SCT_CablingTool',
                      acc.popToolsAndMerge(SCT_CablingToolCfg(flags)))

    # Configuring SCT_CalibHitmapTool
    kwargs.setdefault('SCT_CalibHitmapTool',
                      acc.popToolsAndMerge(SCT_CalibHitmapToolCfg(flags)))

    # Configuring SCT_CalibLbTool
    kwargs.setdefault('SCT_CalibLbTool',
                      acc.popToolsAndMerge(SCT_CalibLbToolCfg(flags)))

    # Configuring SCT_CalibBsErrorTool
    kwargs.setdefault('SCT_CalibBsErrorTool',
                      acc.popToolsAndMerge(SCT_CalibBsErrorToolCfg(flags)))

    # Configuring SCT_CalibModuleListTool
    kwargs.setdefault('SCT_CalibModuleListTool',
                      acc.popToolsAndMerge(SCT_CalibModuleListToolCfg(flags)))

    # Configuring SCT_CalibEventInfo
    kwargs.setdefault('SCT_CalibEventInfo',
                      acc.popToolsAndMerge(SCT_CalibEventInfoCfg(flags)))

    from AthenaConfiguration.ComponentFactory import CompFactory
    acc.addEventAlgo(CompFactory.SCTCalib(name, **kwargs))

    return acc
