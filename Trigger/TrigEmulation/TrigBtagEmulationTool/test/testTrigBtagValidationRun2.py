#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def main():
    EvtMax = 100
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    inputFiles = defaultTestFiles.AOD_RUN2_MC

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Input.Files = inputFiles
    flags.Exec.MaxEvents = EvtMax
    flags.Common.MsgSuppression = False

    flags.lock()
    flags.dump()

    jetcontainer_a4tcemsubjes = ''
    jetcontainer_Split = 'HLT_xAOD__JetContainer_SplitJet'
    jetcontainer_GSC = 'HLT_xAOD__JetContainer_GSCJet'
    emulatedChains = []

    from Campaigns.Utils import Campaign, getMCCampaign
    campaign = getMCCampaign(flags.Input.Files)

    if campaign == Campaign.MC20a or flags.Input.DataYear == 2016:
        emulatedChains = ['HLT_5j70_L14J15', 'HLT_5j85_L14J15', 'HLT_7j45_L14J15',
                          'HLT_5j65_0eta240_L14J15', 'HLT_j35_boffperf_split', 'HLT_j45_boffperf_split_3j45_L13J20',
                          'HLT_j50_bmv2c2040_split_3j50_L14J15',
                          'HLT_2j35_bmv2c2060_split_2j35_L14J15.0ETA25', 'HLT_j100_2j55_bmv2c2060_split',
                          ]
        jetcontainer_a4tcemsubjes = 'HLT_xAOD__JetContainer_a4tcemsubjesFS'
        if jetcontainer_GSC not in flags.Input.Collections:
            jetcontainer_GSC = '' # GSC collection not saved in 2016 DAOD_PHYS

    elif campaign == Campaign.MC20d or flags.Input.DataYear == 2017:
        emulatedChains = ['HLT_5j70_L14J15', 'HLT_5j85_L14J15', 'HLT_7j45_L14J15',
                          'HLT_2j30_bmv2c1085_split_L12J15_XE55', 'HLT_2j35_bmv2c1050_split_3j35_boffperf_split',
                          'HLT_2j15_gsc35_bmv2c1040_split_2j15_gsc35_boffperf_split_L14J15.0ETA25',
                          'HLT_j110_gsc150_boffperf_split_2j35_gsc55_bmv2c1070_split_L1J85_3J30',
                          ]
        jetcontainer_a4tcemsubjes = 'HLT_xAOD__JetContainer_a4tcemsubjesISFS'

    elif campaign == Campaign.MC20e or flags.Input.DataYear == 2018:
        emulatedChains = ['HLT_3j50_gsc65_boffperf_split_L13J35.0ETA23',
                          'HLT_3j50_gsc65_bmv2c1085_split_L13J35.0ETA23',
                          'HLT_2j35_bmv2c1060_split_2j35_L14J15.0ETA25',
                          'HLT_j110_gsc150_boffperf_split_2j45_gsc55_bmv2c1070_split_L1J85_3J30',
        ]
        jetcontainer_a4tcemsubjes = 'HLT_xAOD__JetContainer_a4tcemsubjesISFS'


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc = MainServicesCfg( flags )
    acc.merge( PoolReadCfg( flags ) )

    from TrigBtagEmulationTool.TrigBtagEmulationToolConfig import TrigBtagValidationTestCfg
    acc.merge(TrigBtagValidationTestCfg(flags,
                                        toBeEmulatedTriggers = emulatedChains,
                                        InputChain_EMTopo = '',
                                        InputJetContainer_EMTopo = '',
                                        InputJetContainer_EMTopoPresel = '',
                                        InputJetContainer_PFlow = '',
                                        InputJetContainer_PFlowPresel = '',
                                        InputJetContainer_a4tcemsubjesJet = jetcontainer_a4tcemsubjes,
                                        InputJetContainer_SplitJet = jetcontainer_Split,
                                        InputJetContainer_GSCJet = jetcontainer_GSC,
                                        ))

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TrigBtagValidationConfig.pkl','wb') )

    acc.run()

if __name__ == "__main__":
    main()
