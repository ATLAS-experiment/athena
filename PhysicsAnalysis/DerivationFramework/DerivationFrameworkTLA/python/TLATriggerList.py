# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# TODO: add all small-R jet, single-photon triggers used for Run 3!!

SupportSingleJetTriggers = [
     # additional triggers required for MC
                # j0 is used to have a chain that allows us to manually calculate EventShape.Density values
                "HLT_j0_perf_pf_subjesgscIS_ftf_L1RD0_FILLED",

                # single-jet support
                "HLT_j0_pf_ftf_L1RD0_FILLED",
                "HLT_j15_pf_ftf_L1RD0_FILLED",
                "HLT_j25_pf_ftf_L1RD0_FILLED",
                "HLT_j35_pf_ftf_L1RD0_FILLED",
                "HLT_j45_pf_ftf_preselj20_L1RD0_FILLED",
                "HLT_j45_pf_ftf_preselj20_L1J15",
                "HLT_j60_pf_ftf_preselj50_L1J20",
                "HLT_j85_pf_ftf_preselj50_L1J20",
                "HLT_j110_pf_ftf_preselj80_L1J30",
                "HLT_j175_pf_ftf_preselj140_L1J50",
                "HLT_j260_pf_ftf_preselj200_L1J75",
                "HLT_j360_pf_ftf_preselj225_L1J100",
                "HLT_j420_pf_ftf_preselj225_L1J100",
                "HLT_j0_L1RD0_FILLED",
                "HLT_j15_L1RD0_FILLED",
                "HLT_j25_L1RD0_FILLED",
                "HLT_j35_L1RD0_FILLED",
                "HLT_j45_preselj20_L1RD0_FILLED",
                "HLT_j45_preselj20_L1J15",
                "HLT_j60_preselj50_L1J20",
                "HLT_j85_preselj50_L1J20",
                "HLT_j110_preselj80_L1J30",
                "HLT_j175_preselj140_L1J50",
                "HLT_j260_preselj200_L1J75",
                "HLT_j360_preselj225_L1J100",
                "HLT_j420_L1J100",
                "HLT_j420_pf_ftf_L1J100",

]

SupportMultiJetTriggers = [
            # multi-jet 
                "HLT_2j250c_j120c_pf_ftf_presel2j180XXj80_L1J100",
                "HLT_3j200_pf_ftf_presel3j150_L1J100",
                "HLT_4j115_pf_ftf_presel4j85_L13J50",
                "HLT_5j70c_pf_ftf_presel5c50_L14J15",
                "HLT_5j85_pf_ftf_presel5j50_L14J15",
                "HLT_6j55c_pf_ftf_presel6j40_L14J15",
                "HLT_6j70_pf_ftf_presel6j40_L14J15",
                "HLT_7j45_pf_ftf_presel7j30_L14J15",
                "HLT_10j40_pf_ftf_presel7j30_L14J15",
                "HLT_3j200_L1J100",
                "HLT_4j120_L13J50",
                "HLT_5j70c_L14J15",
                "HLT_5j85_L14J15",
                "HLT_6j55c_L14J15",
                "HLT_6j70_L14J15",
                "HLT_7j45_L14J15",
                "HLT_10j40_L14J15",
                "HLT_6j35c_020jvt_pf_ftf_presel6c25_L14J15",
                "HLT_3j200_pf_ftf_L1J100",
                "HLT_6j35c_pf_ftf_presel6c25_L14J15",
]

SupportPhotonTriggers = [
     # support photon triggers
                "HLT_g25_loose_L1EM20VH",
                "HLT_g25_medium_L1EM20VH",
                "HLT_g20_tight_L1EM15VHI",
                "HLT_g25_loose_L1eEM24L",
                "HLT_g25_medium_L1eEM24L",
                "HLT_g20_tight_L1eEM18M",
]

PrimaryISRTLATriggers = [
                # Legacy Block (2022)
                "HLT_g35_loose_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g35_tight_3j25_PhysicsTLA_L1EM22VHI",
                "HLT_g35_tight_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g35_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g40_tight_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g45_tight_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g45_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g50_tight_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g50_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g60_tight_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",
                "HLT_g60_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1EM22VHI",

                # Phase1 Block (2023-)
                "HLT_g35_loose_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g35_tight_3j25_PhysicsTLA_L1eEM26M",
                "HLT_g35_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g35_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g40_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g45_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g45_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g50_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g50_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g60_tight_3j25_pf_ftf_PhysicsTLA_L1eEM26M",
                "HLT_g60_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM26M",

                        

                "HLT_g35_loose_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g35_tight_3j25_PhysicsTLA_L1eEM28M",
                "HLT_g35_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g35_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g40_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g45_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g45_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g50_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g50_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g60_tight_3j25_pf_ftf_PhysicsTLA_L1eEM28M",
                "HLT_g60_tight_noiso_3j25_pf_ftf_PhysicsTLA_L1eEM28M",

]

SupportTLATriggers = [
               "HLT_g35_tight_3j25_pf_ftf_L1EM22VHI",
]

PrimarySingleJetTLATriggers = [
                "HLT_j20_PhysicsTLA_L1J100",
                "HLT_j20_PhysicsTLA_L1jJ160",
                "HLT_j20_pf_ftf_preselj140_PhysicsTLA_L1J50",
                "HLT_j20_pf_ftf_preselj180_PhysicsTLA_L1J100",
                "HLT_j20_pf_ftf_preselj180_PhysicsTLA_L1jJ160",
                "HLT_j20_pf_ftf_preselj190_PhysicsTLA_L1J100",
                "HLT_j20_pf_ftf_preselj190_PhysicsTLA_L1jJ160",
                "HLT_j20_pf_ftf_preselj200_PhysicsTLA_L1J100",
                "HLT_j20_pf_ftf_preselj200_PhysicsTLA_L1jJ160",
]

PrimaryMultiJetTLATriggers = [

                "HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25",
                "HLT_j20_PhysicsTLA_L1HT190-J15s5pETA21",
                "HLT_j20_PhysicsTLA_L1HT190-jJ40s5pETA21",
                "HLT_j20_PhysicsTLA_L1J50_DETA20-J50J",
                "HLT_j20_PhysicsTLA_L1jJ90_DETA20-jJ90J",
                "HLT_j20_pf_ftf_preselcHT450_PhysicsTLA_L1HT190-J15s5pETA21",
                "HLT_j60_j45_j25_j20_pf_ftf_preselc60XXc45XXc25XXc20_PhysicsTLA_L1J45p0ETA21_3J15p0ETA25",
]

FTagPEBTLATriggers = [
    "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_3j20c_020jvt_bgn282_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40p0ETA25",
    "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_SHARED_2j20c_020jvt_bgn280_pf_ftf_presel2c20XX2c20bgtwo85_L1jJ85p0ETA21_3jJ40pE0TA25",
    "HLT_j75c_020jvt_j50c_020jvt_j25c_020jvt_j20c_020jvt_boffperf_pf_ftf_presel2c20XX2c20b85_L1jJ85p0ETA21_3jJ40p0ETA25",
    "HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_PhysicsTLA_L1jJ85p0ETA21_3jJ40p0ETA25",
    "HLT_2j20_2j20_pf_ftf_presel2c20XX2c20b85_FTagPEBTLA_L1jJ85p0ETA21_3jJ40p0ETA25",
]
