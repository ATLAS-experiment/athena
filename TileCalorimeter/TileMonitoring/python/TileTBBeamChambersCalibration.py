#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileBeamChambersCalibration.py
@brief Python configuration of calibration for TileTB beam chambers
'''

def getBeamChambersCalibrations(run):
    '''
    This function returns TileTB beam chambers calbirations

    This function supposed to be used for beam chambers calibrations
    for the following algorithms: TileTBBeamMonitorAlgorithm, TileTBAANtuple
    '''

    calibrations = {}
    if run < 2110000:
        # It is not supposed to be used for these runs
        return calibrations

    calibrations['BC1Z']       = 17348.8
    calibrations['BC1Z_0']     = 17348.8
    calibrations['BC1Z_90']    = 15594.05
    calibrations['BC1Z_min90'] = 15571.8

    calibrations['BC2Z']       = 4404.2
    calibrations['BC2Z_0']     = 4420.7
    calibrations['BC2Z_90']    = 2649.45
    calibrations['BC2Z_min90'] = 2627.2

    if run <= 2110503:
        # September TB2021
        calibrations['BC1HorizontalOffset'] = calibrations['BC1X1'] = -0.156736
        calibrations['BC1HorizontalSlope']  = calibrations['BC1X2'] = -0.178455
        calibrations['BC1VerticalOffset']   = calibrations['BC1Y1'] = -0.452977
        calibrations['BC1VerticalSlope']    = calibrations['BC1Y2'] = -0.17734

        calibrations['BC2HorizontalOffset'] = calibrations['BC2X1'] =  2.88152
        calibrations['BC2HorizontalSlope']  = calibrations['BC2X2'] = -0.187192
        calibrations['BC2VerticalOffset']   = calibrations['BC2Y1'] = -1.79832
        calibrations['BC2VerticalSlope']    = calibrations['BC2Y2'] = -0.190846
    elif run <= 2200000:
        # November TB2021
        calibrations['BC1HorizontalOffset'] = calibrations['BC1X1'] = -0.0107769
        calibrations['BC1HorizontalSlope']  = calibrations['BC1X2'] = -0.178204
        calibrations['BC1VerticalOffset']   = calibrations['BC1Y1'] = -0.625063
        calibrations['BC1VerticalSlope']    = calibrations['BC1Y2'] = -0.178842

        calibrations['BC2HorizontalOffset'] = calibrations['BC2X1'] =  0.746517
        calibrations['BC2HorizontalSlope']  = calibrations['BC2X2'] = -0.176083
        calibrations['BC2VerticalOffset']   = calibrations['BC2Y1'] =  0.49177
        calibrations['BC2VerticalSlope']    = calibrations['BC2Y2'] = -0.18221
    elif run <= 2310000:
        # June TB2022
        calibrations['BC1HorizontalOffset'] = calibrations['BC1X1'] = -0.566211
        calibrations['BC1HorizontalSlope']  = calibrations['BC1X2'] = -0.0513049
        calibrations['BC1VerticalOffset']   = calibrations['BC1Y1'] =  1.15376
        calibrations['BC1VerticalSlope']    = calibrations['BC1Y2'] = -0.0514167

        calibrations['BC2HorizontalOffset'] = calibrations['BC2X1'] = -0.163869
        calibrations['BC2HorizontalSlope']  = calibrations['BC2X2'] = -0.0523055
        calibrations['BC2VerticalOffset']   = calibrations['BC2Y1'] = -0.0602012
        calibrations['BC2VerticalSlope']    = calibrations['BC2Y2'] = -0.0532108
    else:
        # July TB2023
        calibrations['BC1HorizontalOffset'] = calibrations['BC1X1'] =  2.465295
        calibrations['BC1HorizontalSlope']  = calibrations['BC1X2'] = -0.072135
        calibrations['BC1VerticalOffset']   = calibrations['BC1Y1'] =  2.127854
        calibrations['BC1VerticalSlope']    = calibrations['BC1Y2'] = -0.073442

        calibrations['BC2HorizontalOffset'] = calibrations['BC2X1'] = -0.317171
        calibrations['BC2HorizontalSlope']  = calibrations['BC2X2'] = -0.075384
        calibrations['BC2VerticalOffset']   = calibrations['BC2Y1'] =  1.875657
        calibrations['BC2VerticalSlope']    = calibrations['BC2Y2'] = -0.076717

    return calibrations


def updateBeamChambersCalibrations(alg, run):
    '''
    This function updates TileTB beam chambers calbirations for the algorithm

    This function supposed to be used for beam chambers calibrations
    for the following algorithms: TileTBBeamMonitorAlgorithm, TileTBAANtuple
    '''

    # Setup logs
    from AthenaCommon.Constants import INFO
    from AthenaCommon.Logging import logging
    log = logging.getLogger('updateBeamChambersCalibrations')
    log.setLevel(INFO)

    calibrations = getBeamChambersCalibrations(run)
    for k in calibrations:
        if hasattr(alg, k):
            log.info(f'{alg.name}.{k}={calibrations[k]}')
            setattr(alg, k, calibrations[k])
