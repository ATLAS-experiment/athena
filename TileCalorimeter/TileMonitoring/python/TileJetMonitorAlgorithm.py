#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileJetMonitorAlgorithm.py
@brief Python configuration of TileJetMonitorAlgorithm algorithm for the Run III
'''


def TileJetMonitoringConfig(flags, **kwargs):

    ''' Function to configure TileJetMonitorAlgorithm algorithm in the monitoring system.'''

    # Define one top-level monitoring algorithm. The new configuration
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChanToolCfg
    badChanTool = result.popToolsAndMerge( TileBadChanToolCfg(flags) )

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'TileMonitoring')

    # Adding an TileJetMonitorAlgorithm algorithm to the helper
    from AthenaConfiguration.ComponentFactory import CompFactory
    tileJetMonAlg = helper.addAlgorithm(CompFactory.TileJetMonitorAlgorithm, 'TileJetMonAlg')

    tileJetMonAlg.TileBadChanTool = badChanTool
    tileJetMonAlg.TriggerChain = ''

    if flags.Tile.doTimingHistogramsForGain in [0, 1]:
        kwargs.setdefault('Do1DHistograms', True)
        if flags.Tile.doTimingHistogramsForGain == 0:
            # Low Gain
            kwargs.setdefault('ChannelEnergyMin', 15000)
            kwargs.setdefault('ChannelEnergyMax', 50000)
            kwargs.setdefault('Gain', 0)

        else:
            # High Gain
            kwargs.setdefault('ChannelEnergyMin', 2000)
            kwargs.setdefault('ChannelEnergyMax', 4000)
            kwargs.setdefault('Gain', 1)

    if len(flags.Tile.doTimingHistogramsForCell) > 0:
        kwargs.setdefault('DoCellHistograms', True)

    for k, v in kwargs.items():
        setattr(tileJetMonAlg, k, v)

    DoEnergyProfiles = kwargs.get('DoEnergyProfiles', tileJetMonAlg._descriptors['DoEnergyProfiles'].default)

    Do1DHistograms = kwargs.get('Do1DHistograms', tileJetMonAlg._descriptors['Do1DHistograms'].default)
    DoEnergyDiffHistograms  = kwargs.get('DoEnergyDiffHistograms', tileJetMonAlg._descriptors['DoEnergyDiffHistograms'].default)

    DoCellHistograms = kwargs.get('DoCellHistograms', tileJetMonAlg._descriptors['DoCellHistograms'].default)

    from AthenaMonitoring.DQConfigFlags import DQDataType
    if flags.DQ.DataType not in (DQDataType.HeavyIon, DQDataType.Cosmics):

        jvtTool = CompFactory.JetVertexTaggerTool()
        jetContainer = kwargs.get('JetContainer', tileJetMonAlg._descriptors['JetContainer'].default)
        jvtTool.JetContainer = str(jetContainer)
        tileJetMonAlg.JVT = jvtTool

        jetCleaningTool = CompFactory.JetCleaningTool()
        jetCleaningTool.CutLevel = "LooseBad"
        jetCleaningTool.DoUgly = False
        jetCleaningTool.JetContainer = str(jetContainer)

        tileJetMonAlg.JetCleaningTool = jetCleaningTool
        result.addPublicTool(jetCleaningTool)

        jetPtMin = 20000
        jetTrackingEtaLimit = 2.4
        eventCleaningTool = CompFactory.ECUtils.EventCleaningTool()
        eventCleaningTool.JetCleaningTool = jetCleaningTool
        eventCleaningTool.PtCut = jetPtMin
        eventCleaningTool.EtaCut = jetTrackingEtaLimit
        eventCleaningTool.JvtDecorator = "passJvt"
        eventCleaningTool.OrDecorator = "passOR"
        eventCleaningTool.CleaningLevel = jetCleaningTool.CutLevel
        eventCleaningTool.JetContainer = str(jetContainer)
        eventCleaningTool.SuppressInputDependence = True
        eventCleaningTool.SuppressOutputDependence = True

        tileJetMonAlg.EventCleaningTool = eventCleaningTool
        tileJetMonAlg.JetTrackingEtaLimit = jetTrackingEtaLimit
        tileJetMonAlg.JetPtMin = jetPtMin

        tileJetMonAlg.DoEventCleaning = True
        tileJetMonAlg.DoJetCleaning = True

    else:

        tileJetMonAlg.DoEventCleaning = False
        tileJetMonAlg.DoJetCleaning = False

        if flags.Reco.EnableHI:
            if flags.Tracking.doUPC:
                tileJetMonAlg.JetContainer = 'AntiKt4EMPFlowJets'
            else:
                tileJetMonAlg.JetContainer = 'AntiKt4HIJets'

    # 1) Configure histogram with TileJetMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileJetMonAlg, 'TileJetMonExecuteTime', 'Tile/')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'Jet', type='TH1F',
                                     title = 'Time for execute TileJetMonAlg algorithm;time [#mus]',
                                     xbins = 300, xmin = 0, xmax = 300000)



    from TileMonitoring.TileMonitoringCfgHelper import addValueVsModuleAndChannelMaps, getPartitionName, getChannelsForCell
    runNumber = flags.Input.RunNumbers[0]


    # 2) Configure 2D histograms (profiles/maps) with Tile channel time vs module and channel per partion (DQ summary)
    channelTimeDQGroup = helper.addGroup(tileJetMonAlg, 'TileJetChanTimeDQ', 'Tile/Jet/')
    addValueVsModuleAndChannelMaps(channelTimeDQGroup, name = 'tileJetChanTime', title = 'Average time with jets',
                                   path = 'DQ', type = 'TProfile2D', value='time', run = str(runNumber))


    gains = ['LG', 'HG']
    partitions = ['LBA', 'LBC', 'EBA', 'EBC']

    # 3a) Configure 1D histograms with Tile channel time per partition
    channelTimeGroup = helper.addGroup(tileJetMonAlg, 'TileJetChanTime', 'Tile/Jet/ChanTime/')
    for partition in partitions:
        title = 'Partition ' + partition + ': Tile Channel Time;time [ns];N'
        name = 'channelTime' + partition
        path = partition
        channelTimeGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                         xbins = 600, xmin = -30.0, xmax = 30.0)

    # 3b) Configure 1D histograms with Tile channel time per partition for extended barrels without scintillators
    for partition in ['EBA', 'EBC']:
        title = 'Partition ' + partition + ': Tile Channel Time (without scintillators);time [ns];N'
        name = 'channelTime' + partition + '_NoScint'
        path = partition
        channelTimeGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                         xbins = 600, xmin = -30.0, xmax = 30.0)


    # Energy upper limits of the cell-time histograms
    energiesHG = [500, 1000, 2000, 4000, 6000, 8000, 10000, 13000, 16000, 20000]
    energiesLG = [25000, 30000, 40000, 50000, 65000, 80000]
    energiesALL = {'LG' : energiesLG, 'HG' : energiesHG}
    tileJetMonAlg.CellEnergyUpperLimitsHG = energiesHG
    tileJetMonAlg.CellEnergyUpperLimitsLG = energiesLG

    samples_LB = ['A', 'B', 'D', 'B9']
    samples_EB = ['A', 'B', 'C10', 'D4', 'E1', 'E2', 'E3', 'E4']

    # 4) Configure histograms with Tile cell time in energy slices per partition and gain
    cellTimeGroup = helper.addGroup(tileJetMonAlg, 'TileJetCellTime', 'Tile/Jet/CellTime/')
    if DoCellHistograms:
        selectedCellTimeGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelCellTime', 'Tile/Jet/SelectedCellTime/')
        selectedChanTimeGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelChanTime', 'Tile/Jet/SelectedChanTime/')
    for partition in partitions:
        for gain in gains:
            index = 0
            energies = energiesALL[gain]
            for index in range(0, len(energies) + 1):
                toEnergy = energies[index] if index < len(energies) else None
                fromEnergy = energies[index - 1] if index > 0 else None
                if not toEnergy:
                    title_suffix = ' > ' + str(fromEnergy) + ' MeV; time [ns]'
                elif not fromEnergy:
                    title_suffix = ' < ' + str(toEnergy) + ' MeV; time [ns]'
                else:
                    title_suffix = ' [' + str(fromEnergy) + ' .. ' + str(toEnergy) + ') MeV; time [ns]'

                # TD: add histograms per partition and per radial sampling
                if (partition == 'LBA') or (partition == 'LBC'):
                    for samp in range(0,len(samples_LB)):
                        name = 'Cell_time_' + partition + '_' + samples_LB[samp] + '_' + gain + '_slice_' + str(index)
                        title = 'Partition ' + partition + ', sampling ' + samples_LB[samp] + ': ' + gain + ' Tile Cell time in energy range' + title_suffix
                        cellTimeGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                      xbins = 600, xmin = -30.0, xmax = 30.0)
                else:
                    for samp in range(0,len(samples_EB)):
                        name = 'Cell_time_' + partition + '_' + samples_EB[samp] + '_' + gain + '_slice_' + str(index)
                        title = 'Partition ' + partition + ', sampling ' + samples_EB[samp] + ': ' + gain + ' Tile Cell time in energy range' + title_suffix
                        cellTimeGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                      xbins = 600, xmin = -30.0, xmax = 30.0)

                # Add histograms per selected individual cell
                if DoCellHistograms:
                    for module in flags.Tile.doTimingHistogramsForCell:
                        if partition in module:
                            for cell in flags.Tile.doTimingHistogramsForCell[module]:
                                name = 'Cell_time_' + module + '_' + cell + '_' + gain + '_slice_' + str(index)
                                title = 'Module ' + module + ', cell ' + cell + ': ' + gain + ' Tile Cell time in energy range' + title_suffix
                                selectedCellTimeGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                                    xbins = 600, xmin = -30.0, xmax = 30.0)

                                toEnergy_ch = energies[index] / 2 if index < len(energies) else None
                                fromEnergy_ch = energies[index - 1] / 2 if index > 0 else None
                                if not toEnergy_ch:
                                    title_suffix_ch = ' > ' + str(fromEnergy_ch) + ' MeV; time [ns]'
                                elif not fromEnergy_ch:
                                    title_suffix_ch = ' < ' + str(toEnergy_ch) + ' MeV; time [ns]'
                                else:
                                    title_suffix_ch = ' [' + str(fromEnergy_ch) + ' .. ' + str(toEnergy_ch) + ') MeV; time [ns]'

                                # Add histograms per channels of selected individual cell
                                for channel in getChannelsForCell(module, cell):
                                    name = 'Cell_time_' + module + '_' + cell + '_ch' + str(channel) + '_' + gain + '_slice_' + str(index)
                                    title = 'Module ' + module + ', cell ' + cell + ', channel ' + str(channel) + ': ' + gain + ' Tile Channel time in energy range' + title_suffix_ch
                                    selectedChanTimeGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                                        xbins = 600, xmin = -30.0, xmax = 30.0)

    if DoEnergyProfiles:

        # 5) Configure 1D histograms (profiles) with Tile cell energy profile in energy slices per partition and gain
        cellEnergyProfileGroup = helper.addGroup(tileJetMonAlg, 'TileJetCellEnergyProfile', 'Tile/Jet/CellTime/')
        if DoCellHistograms:
            selectedCellEnergyProfileGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelCellEnergyProfile', 'Tile/Jet/SelectedCellTime/')
            selectedChanEnergyProfileGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelChanEnergyProfile', 'Tile/Jet/SelectedChanTime/')
        for partition in partitions:
            for gain in gains:
                # TD: add profiles per partition and per sampling
                if (partition == 'LBA') or (partition == 'LBC'):
                    for samp in range(0,len(samples_LB)):
                        name = 'index_' + partition + '_' + samples_LB[samp] + '_' + gain
                        name += ',energy_' + partition + '_' + samples_LB[samp] + '_' + gain
                        name += ';Cell_ene_' + partition + '_' + samples_LB[samp] + '_' + gain + '_prof'
                        title = 'Partition ' + partition + ', sampling ' + samples_LB[samp] + ': ' + gain + ' Tile Cell energy profile;Slice;Energy [MeV]'
                        xmax = len(energiesALL[gain]) + 0.5
                        nbins = len(energiesALL[gain]) + 1
                        cellEnergyProfileGroup.defineHistogram(name, title = title, path = partition, type = 'TProfile',
                                                               xbins = nbins, xmin = -0.5, xmax = xmax)
                else:
                    for samp in range(0,len(samples_EB)):
                        name = 'index_' + partition + '_' + samples_EB[samp] + '_' + gain
                        name += ',energy_' + partition + '_' + samples_EB[samp] + '_' + gain
                        name += ';Cell_ene_' + partition + '_' + samples_EB[samp] + '_' + gain + '_prof'
                        title = 'Partition ' + partition + ', sampling ' + samples_EB[samp] + ': ' + gain + ' Tile Cell energy profile;Slice;Energy [MeV]'
                        xmax = len(energiesALL[gain]) + 0.5
                        nbins = len(energiesALL[gain]) + 1
                        cellEnergyProfileGroup.defineHistogram(name, title = title, path = partition, type = 'TProfile',
                                                               xbins = nbins, xmin = -0.5, xmax = xmax)

                # Add profiles per selected individual cell
                if DoCellHistograms:
                    for module in flags.Tile.doTimingHistogramsForCell:
                        if partition in module:
                            for cell in flags.Tile.doTimingHistogramsForCell[module]:
                                name = 'index_' + module + '_' + cell + '_' + gain
                                name += ',energy_' + module + '_' + cell + '_' + gain
                                name += ';Cell_ene_' + module + '_' + cell + '_' + gain + '_prof'
                                title = 'Module ' + module + ', cell ' + cell + ': ' + gain + ' Tile Cell energy profile;Slice;Energy [MeV]'
                                xmax = len(energiesALL[gain]) + 0.5
                                nbins = len(energiesALL[gain]) + 1
                                selectedCellEnergyProfileGroup.defineHistogram(name, title = title, path = partition, type = 'TProfile',
                                                                            xbins = nbins, xmin = -0.5, xmax = xmax)

                                # Add profiles per channels of selected individual cell
                                for channel in getChannelsForCell(module, cell):
                                    name = 'index_' + module + '_' + cell + '_ch' + str(channel) + '_' + gain
                                    name += ',energy_' + module + '_' + cell + '_ch' + str(channel) + '_' + gain
                                    name += ';Cell_ene_' + module + '_' + cell + '_ch' + str(channel) + '_' + gain + '_prof'
                                    title = 'Module ' + module + ', cell ' + cell + ', channel ' + str(channel) + ': ' + gain + ' Tile Channel energy profile;Slice;Energy [MeV]'
                                    xmax = len(energiesALL[gain]) + 0.5
                                    nbins = len(energiesALL[gain]) + 1
                                    selectedChanEnergyProfileGroup.defineHistogram(name, title = title, path = partition, type = 'TProfile',
                                                                                xbins = nbins, xmin = -0.5, xmax = xmax)

    else:

        # 6) Configure 1D histograms with Tile cell energy in energy slices per partition, gain and slice
        cellEnergyGroup = helper.addGroup(tileJetMonAlg, 'TileJetCellEnergy', 'Tile/Jet/CellTime/')
        if DoCellHistograms:
            selectedCellEnergyGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelCellEnergy', 'Tile/Jet/SelectedCellTime/')
            selectedChanEnergyGroup = helper.addGroup(tileJetMonAlg, 'TileJetSelChanEnergy', 'Tile/Jet/SelectedChanTime/')
        for partition in partitions:
            for gain in gains:
                energies = energiesALL[gain]
                for index in range(0, len(energies) + 1):
                    toEnergy = energies[index] if index < len(energies) else 2 * energies[index - 1]
                    fromEnergy = energies[index - 1] if index > 0 else -1000
                    name = 'Cell_ene_' + partition + '_' + gain + '_slice_' + str(index)
                    title = 'Partition ' + partition + ': ' + gain + ' Tile Cell Energy'
                    title += ' in energy range [' + str(fromEnergy) + ' .. ' + str(toEnergy) + ') MeV;Energy [MeV]'
                    cellEnergyGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                    xbins = 100, xmin = fromEnergy, xmax = toEnergy)
                    # TD: add histograms per partition
                    if (partition == 'LBA') or (partition == 'LBC'):
                        for samp in range(0,len(samples_LB)):
                            name = 'Cell_ene_' + partition + '_' + samples_LB[samp] + '_' + gain + '_slice_' + str(index)
                            title = 'Partition ' + partition + ', sampling ' + samples_LB[samp] + ': ' + gain + ' Tile Cell Energy'
                            title += ' in energy range [' + str(fromEnergy) + ' .. ' + str(toEnergy) + ') MeV;Energy [MeV]'
                            cellEnergyGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                            xbins = 100, xmin = fromEnergy, xmax = toEnergy)
                    else:
                        for samp in range(0,len(samples_EB)):
                            name = 'Cell_ene_' + partition + '_' + samples_EB[samp] + '_' + gain + '_slice_' + str(index)
                            title = 'Partition ' + partition + ', sampling ' + samples_EB[samp] + ': ' + gain + ' Tile Cell Energy'
                            title += ' in energy range [' + str(fromEnergy) + ' .. ' + str(toEnergy) + ') MeV;Energy [MeV]'
                            cellEnergyGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                            xbins = 100, xmin = fromEnergy, xmax = toEnergy)

                    # Add histograms per selected individual cell
                    if DoCellHistograms:
                        for module in flags.Tile.doTimingHistogramsForCell:
                            if partition in module:
                                for cell in flags.Tile.doTimingHistogramsForCell[module]:
                                    name = 'Cell_ene_' + module + '_' + cell + '_' + gain + '_slice_' + str(index)
                                    title = 'Module ' + module + ', cell ' + cell + ': ' + gain + ' Tile Cell Energy'
                                    title += ' in energy range [' + str(fromEnergy) + ' .. ' + str(toEnergy) + ') MeV;Energy [MeV]'
                                    selectedCellEnergyGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                                            xbins = 100, xmin = fromEnergy, xmax = toEnergy)

                                    # Add profiles per channels of selected individual cell
                                    for channel in getChannelsForCell(module, cell):
                                        name = 'Cell_ene_' + module + '_' + cell + '_ch' + str(channel) + '_' + gain + '_slice_' + str(index)
                                        title = 'Module ' + module + ', cell ' + cell + ', channel ' + str(channel) + ': ' + gain + ' Tile Channel Energy'
                                        title += ' in energy range [' + str(fromEnergy / 2) + ' .. ' + str(toEnergy / 2) + ') MeV;Energy [MeV]'
                                        selectedChanEnergyGroup.defineHistogram(name, title = title, path = partition, type = 'TH1F',
                                                                                xbins = 100, xmin = fromEnergy / 2, xmax = toEnergy / 2)

    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    if Do1DHistograms:

        # 7) Configure 1D histograms with Tile channel time per channel
        channelTime1DGroup = helper.addGroup(tileJetMonAlg, 'TileJetChanTime1D', 'Tile/Jet/ChanTime/')

        for ros in range(1, Tile.MAX_ROS):
            for module in range(0, Tile.MAX_DRAWER):
                for channel in range(0, Tile.MAX_CHAN):
                    moduleName = Tile.getDrawerString(ros, module)
                    title = 'Time in ' + moduleName + ' channel ' + str(channel) + ';time [ns];N'
                    name = moduleName + '_ch_' + str(channel) + '_1d'
                    path = getPartitionName(ros) + '/' + moduleName
                    channelTime1DGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                       xbins = 600, xmin = -30.0, xmax = 30.0)



    if DoEnergyDiffHistograms:

        # 7) Configure 1D histograms with Tile cell relative energy difference between two channels per even channel
        energyDiffGroup = helper.addGroup(tileJetMonAlg, 'TileJetEnergyDiff', 'Tile/Jet/EnergyDiff/')

        for ros in range(1, Tile.MAX_ROS):
            for module in range(0, Tile.MAX_DRAWER):
                for channel in range(0, Tile.MAX_CHAN):
                    if not channel % 2:
                        for gain in gains:
                            moduleName = Tile.getDrawerString(ros, module)
                            title = 'Tile Cell Energy difference in ' + moduleName + ' channel ' + str(channel) + ' ' + gain
                            title += ';#frac{ene1 - ene2}{ene1 + ene2}'
                            name = moduleName + '_enediff_' + gain + '_ch1_' + str(channel)
                            path = getPartitionName(ros) + '/' + moduleName
                            energyDiffGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                            xbins = 100, xmin = -1.0, xmax = 1.0)



    accumalator = helper.result()
    result.merge(accumalator)
    return result

if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD
    flags.Output.HISTFileName = 'TileJetMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    tileJetMonitorAccumulator  = TileJetMonitoringConfig(flags,
                                                         Do1DHistograms = True,
                                                         DoEnergyDiffHistograms = True,
                                                         DoCellHistograms = False)
    cfg.merge(tileJetMonitorAccumulator)
    #cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileJetMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
