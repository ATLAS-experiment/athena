#!/usr/bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# This is a script that can be used to copy a number of COOL folders to a sqlite file for DCS Calculator testing

FOLDERS = { 
    'COOLONL_TDAQ/CONDBR2': ['/TDAQ/RunCtrl/DataTakingMode', '/TDAQ/OLC/LHC/SCANDATA',],
    "COOLONL_SCT/CONDBR2": ['/SCT/DAQ/Configuration/Module',],
    "COOLOFL_TILE/CONDBR2": ['/TILE/OFL02/STATUS/ADC',],
    "COOLOFL_INDET/CONDBR2": ['/Indet/Beampos',],
    'COOLONL_INDET/CONDBR2': ['/Indet/Onl/Beampos',],
    'COOLOFL_TRIGGER/CONDBR2': ['/TRIGGER/OFLLUMI/LumiAccounting',],
    'COOLONL_TRIGGER/CONDBR2': ['/TRIGGER/LUMI/LBLB',],

    'COOLOFL_DCS/CONDBR2': [
        '/AFP/DCS/STATION',
        '/AFP/DCS/SIT/LV',                                           
        '/AFP/DCS/SIT/HV',       
        '/AFP/DCS/SIT/HV_VOLTAGE_SET',
        '/AFP/DCS/TOF_TDC_CURRENT',
        '/AFP/DCS/TOF',
        '/AFP/DCS/TOF_PMT_VOLTAGE_SET',
        '/LAR/DCS/FSM',
        '/MDT/DCS/HV',
        '/MDT/DCS/LV',
        '/MDT/DCS/JTAG',
        '/PIXEL/DCS/FSMSTATUS',
        '/PIXEL/DCS/FSMSTATE',
        '/RPC/DCS/DQTOWERS_3',
        '/SCT/DCS/HV',
        '/SCT/DCS/CHANSTAT',
        '/TDQ/DCS/WIENER/LVL1',
        '/TILE/DCS/STATES',
        '/TGC/DCS/PSHVCHSTATE',
        '/TRT/DCS/HV/BARREL',
        '/TRT/DCS/HV/ENDCAPA',
        '/TRT/DCS/HV/ENDCAPC',
        '/EXT/DCS/MAGNETS/SENSORDATA',
        '/STG/DCS/HV',
        '/MMG/DCS/HV'
    ],
}

import subprocess
RUN = 456685
TARGET = 'sqlite://;schema=junk.db;dbname=CONDBR2'
for db, folders in FOLDERS.items():
    arg = ['AtlCoolCopy', db, TARGET, '-r', f'{RUN}',
                    '-bs', '10240', '-gt', '-create']
    for folder in folders:
        arg += ['-f', folder]
    subprocess.run(arg)