# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import AthenaLogger
from PathResolver import PathResolver
import importlib
import os
import AthenaCommon.Utils.unixtools as unixtools

log = AthenaLogger(__name__)

#### Now import Data Prep config from other file
from FPGATrackSimConfTools import FPGATrackSimDataPrepConfig
from FPGATrackSimConfTools import FPGATrackSimAnalysisConfig

def getNSubregions(filePath):
    with open(PathResolver.FindCalibFile(filePath), 'r') as f:
        fields = f.readline()
        assert(fields.startswith('towers'))
        n = fields.split()[1]
        return int(n)

def getChi2CutNN2ndStage(region):
    chi2cut_l = [1.0,1.0,1.0, ### 0.0-0.6
                 1.0,1.0,1.0, ### 0.6-1.2
                 1.0,1.0,1.0, ### 1.2-1.8
                 1.0,1.0,1.0, ### 1.8-2.4
                 1.0,0.3,0.3, ### 2.4-3.0
                 0.3,0.3,0.3, ### 3.0-3.6
                 0.3,0.3] ### 3.6-4.0
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    eta_to_chi2 = {
        (0.0, 0.2): chi2cut_l[0], (0.2, 0.4): chi2cut_l[1], (0.4, 0.6): chi2cut_l[2],
        (0.6, 0.8): chi2cut_l[3], (0.8, 1.0): chi2cut_l[4], (1.0, 1.2): chi2cut_l[5],
        (1.2, 1.4): chi2cut_l[6], (1.4, 1.6): chi2cut_l[7], (1.6, 1.8): chi2cut_l[8],
        (1.8, 2.0): chi2cut_l[9], (2.0, 2.2): chi2cut_l[10], (2.2, 2.4): chi2cut_l[11],
        (2.4, 2.6): chi2cut_l[12], (2.6, 2.8): chi2cut_l[13], (2.8, 3.0): chi2cut_l[14],
        (3.0, 3.2): chi2cut_l[15], (3.2, 3.4): chi2cut_l[16], (3.4, 3.6): chi2cut_l[17],
        (3.6, 3.8): chi2cut_l[18], (3.8, 4.0): chi2cut_l[19]
    }
    return eta_to_chi2.get(abs_etaRange, 0.99) 

def getWindowsNN2ndStage(region):

    fineIds_l = [[0, 1, 2, 3],   ### 0.0-0.2,
               [0, 1, 2, 3],   ### 0.2-0.4,
               [0, 1, 2, 3],   ### 0.4-0.6,
               [0, 1, 2, 3],   ### 0.6-0.8,
               [0, 1, 195, 2, 3],   ### 0.8-1.0,
               [0, 1, 10, 179, 196, 2, 3],   ### 1.0-1.2,
               [0, 1, 10, 11, 12, 180, 197, 198, 2, 3],   ### 1.2-1.4,
               [0, 1, 10, 11, 12, 13, 162, 181, 193, 197, 198, 199, 2],   ### 1.4-1.6,
               [0, 1, 10, 11, 12, 13, 14, 143, 15, 163, 182, 198, 199, 200, 201],   ### 1.6-1.8,
               [0, 10, 11, 12, 13, 14, 15, 181, 182, 183, 184, 198, 199, 200, 201, 202, 203],   ### 1.8-2.0,
               [0, 10, 11, 12, 13, 14, 146, 15, 160, 162, 164, 181, 182, 183, 184, 185, 200, 201, 202, 203, 204],   ### 2.0-2.2,
               [11, 12, 13, 14, 147, 15, 162, 163, 164, 165, 166, 183, 184, 185, 186, 187, 202, 203, 204, 205, 206],   ### 2.2-2.4,
               [12, 13, 14, 148, 149, 15, 163, 164, 165, 166, 167, 168, 184, 185, 186, 187, 188, 204, 205, 206, 207, 208],   ### 2.4-2.6,
               [150, 164, 165, 166, 167, 168, 169, 170, 186, 187, 188, 189, 190, 206, 207, 208, 209],   ### 2.6-2.8,
               [151, 152, 166, 167, 168, 169, 170, 171, 172, 188, 189, 190, 191, 208, 209, 210],   ### 2.8-3.0,
               [146, 147, 153, 154, 169, 170, 171, 172, 173, 174, 190, 191, 192, 210],   ### 3.0-3.2,
               [149, 153, 154, 155, 156, 171, 172, 173, 174, 175, 176, 191, 192],   ### 3.2-3.4,
               [134, 151, 152, 153, 154, 155, 156, 157, 174, 175, 176],   ### 3.4-3.6,
               [136, 153, 154, 155, 156, 157, 158, 159, 176],   ### 3.6-3.8,
               [132, 155, 156]]  ### 3.8-4.0,

    rWindows_l = [[25.5, 24.3, 21.2, 24.6, ], ### 0.0-0.2,
                  [24.6, 21.8, 18.8, 24.6, ], ### 0.2-0.4,
                  [24.9, 20.5, 20.2, 19.4, ], ### 0.4-0.6,
                  [23.4, 24.3, 23.1, 20.2, ], ### 0.6-0.8,
                  [18.5, 19.6, 28.5, 22.4, 20.5, ], ### 0.8-1.0,
                  [23.4, 22.0, 28.5, 27.0, 23.1, 20.0, 21.2, ], ### 1.0-1.2,
                  [41.0, 38.5, 47.0, 45.5, 49.5, 24.8, 41.5, 44.5, 39.5, 14.8, ], ### 1.2-1.4,
                  [41.5, 33.2, 44.5, 44.5, 46.5, 42.5, 37.2, 42.0, 23.2, 40.5, 42.5, 36.2, 45.5, ], ### 1.4-1.6,
                  [40.0, 44.5, 45.5, 44.0, 40.0, 42.0, 44.5, 42.0, 46.0, 2.5, 34.2, 42.0, 34.8, 40.0, 26.8, ], ### 1.6-1.8,
                  [43.5, 43.5, 36.8, 38.0, 36.2, 41.5, 44.5, 40.5, 35.2, 31.2, 48.5, 30.2, 32.8, 41.0, 25.8, 31.2, 46.0, ], ### 1.8-2.0,
                  [44.5, 45.5, 44.0, 30.2, 29.2, 38.5, 38.5, 43.0, 17.8, 43.5, 32.8, 39.0, 25.2, 34.8, 44.0, 43.0, 44.5, 43.0, 43.0, 45.0, 24.2, ], ### 2.0-2.2,
                  [33.8, 43.5, 38.5, 26.8, 36.2, 30.8, 42.5, 34.2, 36.2, 40.5, 39.5, 39.0, 42.5, 38.5, 41.0, 42.5, 36.8, 39.0, 43.5, 35.8, 45.0, ], ### 2.2-2.4,
                  [50.0, 38.5, 33.2, 31.2, 29.8, 33.8, 40.0, 42.5, 39.0, 24.2, 32.8, 32.8, 22.8, 34.2, 43.0, 30.8, 40.0, 43.0, 33.2, 30.8, 21.2, 35.2, ], ### 2.4-2.6,
                  [19.2, 47.0, 39.0, 40.0, 39.5, 30.8, 32.2, 39.0, 31.8, 36.8, 32.8, 40.5, 43.0, 45.5, 31.2, 44.5, 35.8, ], ### 2.6-2.8,
                  [36.8, 21.8, 27.2, 33.8, 43.0, 31.8, 35.2, 23.2, 26.8, 33.8, 42.0, 33.2, 18.2, 35.2, 39.0, 42.0, ], ### 2.8-3.0,
                  [45.0, 19.2, 32.2, 39.0, 38.5, 41.5, 24.2, 18.2, 36.8, 42.0, 38.5, 22.8, 40.5, 15.2, ], ### 3.0-3.2,
                  [20.2, 8.8, 25.4, 4.8, 24.2, 16.6, 26.2, 24.6, 26.6, 27.8, 6.8, 23.8, 17.4, ], ### 3.2-3.4,
                  [35.2, 24.6, 11.0, 16.2, 4.8, 5.6, 12.2, 14.6, 20.6, 29.0, 27.4, ], ### 3.4-3.6,
                  [25.0, 27.4, 17.4, 12.2, 10.6, 3.6, 6.0, 9.2, 24.6, ], ### 3.6-3.8,
                  [6.4, 18.2, 17.4, ]] ### 3.8-4.0,

    phiWindows_l = [[0.05, 0.08, 0.07, 0.08, ], ### 0.0-0.2,
                    [0.05, 0.07, 0.11, 0.19, ], ### 0.2-0.4,
                    [0.04, 0.08, 0.12, 0.14, ], ### 0.4-0.6,
                    [0.04, 0.07, 0.1, 0.12, ], ### 0.6-0.8,
                    [0.08, 0.05, 0.17, 0.07, 0.15, ], ### 0.8-1.0,
                    [0.2, 0.11, 0.32, 0.09, 0.13, 0.1, 0.11, ], ### 1.0-1.2,
                    [0.22, 0.09, 0.18, 0.14, 0.24, 0.18, 0.16, 0.18, 0.09, 0.13, ], ### 1.2-1.4,
                    [0.34, 0.07, 0.11, 0.15, 0.28, 0.23, 0.19, 0.22, 0.19, 0.21, 0.16, 0.23, 0.18, ], ### 1.4-1.6,
                    [0.15, 0.24, 0.19, 0.08, 0.04, 0.07, 0.11, 0.18, 0.13, 0.16, 0.21, 0.22, 0.22, 0.25, 0.22, ], ### 1.6-1.8,
                    [0.15, 0.17, 0.06, 0.03, 0.04, 0.03, 0.05, 0.15, 0.12, 0.16, 0.26, 0.24, 0.2, 0.15, 0.14, 0.18, 0.3, ], ### 1.8-2.0,
                    [0.43, 0.28, 0.26, 0.05, 0.02, 0.04, 0.24, 0.03, 0.2, 0.16, 0.18, 0.2, 0.22, 0.19, 0.18, 0.21, 0.27, 0.3, 0.27, 0.25, 0.24, ], ### 2.0-2.2,
                    [0.09, 0.15, 0.15, 0.01, 0.15, 0.03, 0.07, 0.14, 0.15, 0.18, 0.19, 0.2, 0.16, 0.13, 0.14, 0.09, 0.26, 0.15, 0.12, 0.21, 0.21, ], ### 2.2-2.4,
                    [0.34, 0.2, 0.13, 0.13, 0.18, 0.1, 0.22, 0.22, 0.17, 0.17, 0.18, 0.24, 0.24, 0.15, 0.15, 0.19, 0.28, 0.29, 0.19, 0.19, 0.19, 0.23, ], ### 2.4-2.6,
                    [0.06, 0.07, 0.09, 0.09, 0.1, 0.11, 0.07, 0.14, 0.15, 0.09, 0.12, 0.17, 0.17, 0.14, 0.14, 0.16, 0.18, ], ### 2.6-2.8,
                    [0.16, 0.18, 0.11, 0.21, 0.17, 0.12, 0.12, 0.13, 0.09, 0.11, 0.12, 0.15, 0.12, 0.07, 0.1, 0.09, ], ### 2.8-3.0,
                    [0.3, 0.16, 0.15, 0.16, 0.12, 0.18, 0.16, 0.12, 0.1, 0.07, 0.15, 0.13, 0.16, 0.19, ], ### 3.0-3.2,
                    [0.1, 0.04, 0.15, 0.1, 0.12, 0.11, 0.19, 0.19, 0.16, 0.1, 0.12, 0.12, 0.16, ], ### 3.2-3.4,
                    [0.1, 0.13, 0.06, 0.06, 0.04, 0.01, 0.02, 0.09, 0.07, 0.11, 0.14, ], ### 3.4-3.6,
                    [0.11, 0.14, 0.06, 0.05, 0.03, 0.01, 0.06, 0.06, 0.1, ], ### 3.6-3.8,
                    [0.09, 0.14, 0.09, ]] ### 3.8-4.0,

    zWindows_l = [[35.55, 54.6, 58.35, 63.6, ], ### 0.0-0.2,
                  [32.55, 49.2, 51.45, 63.6, ], ### 0.2-0.4,
                  [29.55, 60.6, 59.1, 67.35, ], ### 0.4-0.6,
                  [38.7, 58.35, 56.85, 68.25, ], ### 0.6-0.8,
                  [49.95, 56.85, 69.0, 66.6, 70.5, ], ### 0.8-1.0,
                  [62.1, 59.85, 69.75, 69.75, 70.5, 61.35, 68.25, ], ### 1.0-1.2,
                  [107.25, 113.75, 65.75, 77.0, 122.5, 98.5, 118.75, 121.25, 108.5, 102.25, ], ### 1.2-1.4,
                  [109.75, 82.0, 35.25, 43.0, 87.0, 60.5, 93.5, 111.0, 118.75, 106.0, 96.0, 92.25, 101.0, ], ### 1.4-1.6,
                  [106.0, 117.5, 60.5, 45.5, 40.5, 43.0, 35.25, 103.5, 72.0, 61.75, 91.0, 63.25, 92.25, 92.25, 108.5, ], ### 1.6-1.8,
                  [117.5, 97.25, 25.25, 22.75, 29.0, 56.75, 40.5, 91.0, 88.5, 29.0, 116.25, 99.75, 79.5, 84.5, 55.5, 77.0, 112.25, ], ### 1.8-2.0,
                  [111.0, 75.75, 51.75, 17.75, 17.75, 46.75, 108.5, 26.5, 108.5, 53.0, 44.25, 116.25, 63.25, 84.5, 83.25, 106.0, 94.75, 106.0, 88.5, 93.5, 35.25, ], ### 2.0-2.2,
                  [78.25, 64.5, 39.25, 19.0, 113.75, 17.75, 69.5, 70.75, 77.0, 91.0, 84.5, 115.0, 88.5, 53.0, 103.5, 98.5, 91.0, 93.5, 98.5, 50.5, 103.5, ], ### 2.2-2.4,
                  [50.0, 41.75, 63.25, 49.25, 93.5, 39.25, 116.25, 91.0, 55.5, 35.25, 87.0, 48.0, 50.0, 48.0, 78.25, 50.5, 93.5, 109.75, 43.0, 54.25, 17.75, 93.5, ], ### 2.4-2.6,
                  [29.0, 50.0, 36.5, 72.0, 68.25, 36.5, 46.75, 89.75, 97.25, 72.0, 43.0, 54.25, 117.5, 38.0, 19.0, 85.75, 59.25, ], ### 2.6-2.8,
                  [27.75, 73.25, 106.0, 55.5, 96.0, 72.0, 59.25, 50.5, 19.0, 78.25, 41.75, 68.25, 75.75, 101.0, 89.75, 97.25, ], ### 2.8-3.0,
                  [118.75, 51.75, 63.25, 113.75, 94.75, 91.0, 69.5, 17.75, 91.0, 44.25, 82.0, 39.25, 63.25, 53.0, ], ### 3.0-3.2,
                  [81.8, 6.0, 54.6, 20.2, 92.0, 75.8, 44.4, 70.8, 65.6, 24.2, 33.4, 46.4, 48.4, ], ### 3.2-3.4,
                  [59.6, 49.4, 65.6, 65.6, 9.0, 6.0, 9.0, 69.6, 75.8, 83.8, 71.8, ], ### 3.4-3.6,
                  [89.8, 81.8, 35.4, 24.2, 8.0, 7.0, 7.0, 54.6, 52.6, ], ### 3.6-3.8,
                  [84.8, 94.0, 49.4, ]] ### 3.8-4.0,


    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)


    eta_to_r = {
        (0.0, 0.2): rWindows_l[0], (0.2, 0.4): rWindows_l[1], (0.4, 0.6): rWindows_l[2],
        (0.6, 0.8): rWindows_l[3], (0.8, 1.0): rWindows_l[4], (1.0, 1.2): rWindows_l[5],
        (1.2, 1.4): rWindows_l[6], (1.4, 1.6): rWindows_l[7], (1.6, 1.8): rWindows_l[8],
        (1.8, 2.0): rWindows_l[9], (2.0, 2.2): rWindows_l[10], (2.2, 2.4): rWindows_l[11],
        (2.4, 2.6): rWindows_l[12], (2.6, 2.8): rWindows_l[13], (2.8, 3.0): rWindows_l[14],
        (3.0, 3.2): rWindows_l[15], (3.2, 3.4): rWindows_l[16], (3.4, 3.6): rWindows_l[17],
        (3.6, 3.8): rWindows_l[18], (3.8, 4.0): rWindows_l[19]
    }

    eta_to_phi = {
        (0.0, 0.2): phiWindows_l[0], (0.2, 0.4): phiWindows_l[1], (0.4, 0.6): phiWindows_l[2],
        (0.6, 0.8): phiWindows_l[3], (0.8, 1.0): phiWindows_l[4], (1.0, 1.2): phiWindows_l[5],
        (1.2, 1.4): phiWindows_l[6], (1.4, 1.6): phiWindows_l[7], (1.6, 1.8): phiWindows_l[8],
        (1.8, 2.0): phiWindows_l[9], (2.0, 2.2): phiWindows_l[10], (2.2, 2.4): phiWindows_l[11],
        (2.4, 2.6): phiWindows_l[12], (2.6, 2.8): phiWindows_l[13], (2.8, 3.0): phiWindows_l[14],
        (3.0, 3.2): phiWindows_l[15], (3.2, 3.4): phiWindows_l[16], (3.4, 3.6): phiWindows_l[17],
        (3.6, 3.8): phiWindows_l[18], (3.8, 4.0): phiWindows_l[19]
    }

    eta_to_z = {
        (0.0, 0.2): zWindows_l[0], (0.2, 0.4): zWindows_l[1], (0.4, 0.6): zWindows_l[2],
        (0.6, 0.8): zWindows_l[3], (0.8, 1.0): zWindows_l[4], (1.0, 1.2): zWindows_l[5],
        (1.2, 1.4): zWindows_l[6], (1.4, 1.6): zWindows_l[7], (1.6, 1.8): zWindows_l[8],
        (1.8, 2.0): zWindows_l[9], (2.0, 2.2): zWindows_l[10], (2.2, 2.4): zWindows_l[11],
        (2.4, 2.6): zWindows_l[12], (2.6, 2.8): zWindows_l[13], (2.8, 3.0): zWindows_l[14],
        (3.0, 3.2): zWindows_l[15], (3.2, 3.4): zWindows_l[16], (3.4, 3.6): zWindows_l[17],
        (3.6, 3.8): zWindows_l[18], (3.8, 4.0): zWindows_l[19]
    }

    eta_to_fineID = {
        (0.0, 0.2): fineIds_l[0], (0.2, 0.4): fineIds_l[1], (0.4, 0.6): fineIds_l[2],
        (0.6, 0.8): fineIds_l[3], (0.8, 1.0): fineIds_l[4], (1.0, 1.2): fineIds_l[5],
        (1.2, 1.4): fineIds_l[6], (1.4, 1.6): fineIds_l[7], (1.6, 1.8): fineIds_l[8],
        (1.8, 2.0): fineIds_l[9], (2.0, 2.2): fineIds_l[10], (2.2, 2.4): fineIds_l[11],
        (2.4, 2.6): fineIds_l[12], (2.6, 2.8): fineIds_l[13], (2.8, 3.0): fineIds_l[14],
        (3.0, 3.2): fineIds_l[15], (3.2, 3.4): fineIds_l[16], (3.4, 3.6): fineIds_l[17],
        (3.6, 3.8): fineIds_l[18], (3.8, 4.0): fineIds_l[19]
    }


    return eta_to_r.get(abs_etaRange, 30),eta_to_phi.get(abs_etaRange, 0.1),eta_to_z.get(abs_etaRange, 30),eta_to_fineID.get(abs_etaRange, 30)



def getPadding(region):
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = (
        [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)]
        if side else
        [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]
    )
    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    phi_pads = [0.01 , 0.008, 0.014, 0.016, 0.018, 0.014, 0.028, 0.036, 0.048, 0.06 , 0.068, 0.068, 0.07 , 0.07 , 0.034, 0.034, 0.038, 0.038, 0.036]
    eta_pads =  [0.036, 0.024, 0.036, 0.024, 0.1  , 0.078, 0.12 , 0.066, 0.078, 0.11 , 0.078, 0.054, 0.054, 0.072, 0.084, 0.078, 0.054, 0.036, 0.042]
    z0_pads =  [8.,  6.,  9.,  8., 50., 23., 50., 14., 39., 43., 33., 30., 35., 23., 30., 34., 29., 25., 28.]
    d0_pads =  [0.78, 0.6 , 1.4 , 1.4 , 1.3 , 1.1 , 1.9 , 2.3 , 2.8 , 2.7 , 2.6 , 2.6 , 2.  , 2.  , 0.96, 0.9 , 0.9 , 0.9 , 0.9 ]
    qpt_pads =  [0.0001, 0.0001, 0.0002, 0.0002, 0.0002, 0.0002, 0.0003, 0.0005, 0.0009, 0.0015, 0.0016, 0.0016, 0.0017, 0.0017, 0.0011, 0.0011, 0.0013, 0.0013, 0.0012]

    bin_edges = [(round(0.2 * i, 1), round(0.2 * (i + 1), 1)) for i in range(20)]

    eta_to_phi = dict(zip(bin_edges, phi_pads))
    eta_to_eta = dict(zip(bin_edges, eta_pads))
    eta_to_z0  = dict(zip(bin_edges, z0_pads))
    eta_to_d0  = dict(zip(bin_edges, d0_pads))
    eta_to_qpt = dict(zip(bin_edges, qpt_pads))

    #default padding values
    default_phiPad = 0.05
    default_etaPad = 0.05
    default_z0Pad  = 0.05
    default_d0Pad  = 0.05
    default_qptPad = 0.05

    phi_pad = eta_to_phi.get(abs_etaRange, default_phiPad)
    eta_pad = eta_to_eta.get(abs_etaRange, default_etaPad)
    z0_pad  = eta_to_z0.get(abs_etaRange, default_z0Pad)
    d0_pad  = eta_to_d0.get(abs_etaRange, default_d0Pad)
    qpt_pad = eta_to_qpt.get(abs_etaRange, default_qptPad)

    return {
        "phi": phi_pad,
        "eta": eta_pad,
        "z0": z0_pad,
        "d0": d0_pad,
        "qpt": qpt_pad
    }


def getMaxMissing_PathFinder(region, overRide=[]):
#    maxMissing = [3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4]
    maxMissing = [4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8]

    if (overRide != []):
        maxMissing = overRide
        
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)
    eta_to_maxMissing = {
        (0.0, 0.2): maxMissing[0], (0.2, 0.4): maxMissing[1], (0.4, 0.6): maxMissing[2],
        (0.6, 0.8): maxMissing[3], (0.8, 1.0): maxMissing[4], (1.0, 1.2): maxMissing[5],
        (1.2, 1.4): maxMissing[6], (1.4, 1.6): maxMissing[7], (1.6, 1.8): maxMissing[8],
        (1.8, 2.0): maxMissing[9], (2.0, 2.2): maxMissing[10], (2.2, 2.4): maxMissing[11],
        (2.4, 2.6): maxMissing[12], (2.6, 2.8): maxMissing[13], (2.8, 3.0): maxMissing[14],
        (3.0, 3.2): maxMissing[15], (3.2, 3.4): maxMissing[16], (3.4, 3.6): maxMissing[17],
        (3.6, 3.8): maxMissing[18], (3.8, 4.0): maxMissing[19]
    }
    return eta_to_maxMissing.get(abs_etaRange, 20)

def getMaxBranches_PathFinder(region, overRide=[]):

    maxBranches = [2,2,2, ### 0.0-0.6
                  2,2,2, ### 0.6-1.2,
                  2,2,2, ### 1.2-1.8
                  2,2,2, ### 1.8-2.4
                  2,2,2, ### 2.4-3.0
                  2,2,2, ### 3.0-3.6,
                  2,2] ### 3.6-4.0

    if (overRide != []):
        maxBranches = overRide

    if (len(maxBranches) == 1): return maxBranches[0]

    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]

    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)
    eta_to_maxBranches = {
        (0.0, 0.2): maxBranches[0], (0.2, 0.4): maxBranches[1], (0.4, 0.6): maxBranches[2],
        (0.6, 0.8): maxBranches[3], (0.8, 1.0): maxBranches[4], (1.0, 1.2): maxBranches[5],
        (1.2, 1.4): maxBranches[6], (1.4, 1.6): maxBranches[7], (1.6, 1.8): maxBranches[8],
        (1.8, 2.0): maxBranches[9], (2.0, 2.2): maxBranches[10], (2.2, 2.4): maxBranches[11],
        (2.4, 2.6): maxBranches[12], (2.6, 2.8): maxBranches[13], (2.8, 3.0): maxBranches[14],
        (3.0, 3.2): maxBranches[15], (3.2, 3.4): maxBranches[16], (3.4, 3.6): maxBranches[17],
        (3.6, 3.8): maxBranches[18], (3.8, 4.0): maxBranches[19]
    }
    return eta_to_maxBranches.get(abs_etaRange, 2)


def FPGATrackSimBinnedHitsToolCfg_2nd(flags,name="FPGATrackSimBinnedHitsTool_2nd"):
    result = ComponentAccumulator()

    # This can probably be imported in the future from the analysis config, but for now it's here.
    ##NameWithRegion = ComponentAccumulator(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    # The second stge, like layer study alg, technically doesn't need a cuts file.
    # So for now allow the same override here I guess?
    cutfile = flags.Trigger.FPGATrackSim.SecondStage.CutFile
    if cutfile.endswith(".py"):
        abspath = cutfile  # assume it's a full path to a .py file
    else:
        relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, cutfile + ".py")
        abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))

    spec = importlib.util.spec_from_file_location("secondStageCuts", abspath)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load cut module from: {abspath}")

    cutmodule = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(cutmodule)
    cutset = cutmodule.cuts[flags.Trigger.FPGATrackSim.region]
    log.info("Running layer study using configured cuts file")
    log.info(cutset)

    # make the binned hits class
    BinnnedHits = CompFactory.FPGATrackSimBinnedHits(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimBinnedHits_2nd"))
    BinnnedHits.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))

    # TODO: we need a new flag for this!
    if flags.Trigger.FPGATrackSim.SecondStage.LayerMapFile:
        BinnnedHits.layerMapFile = flags.Trigger.FPGATrackSim.SecondStage.LayerMapFile
    else:
        relpath = os.path.join(flags.Trigger.FPGATrackSim.mapsDir, f"region{flags.Trigger.FPGATrackSim.region}_lyrmap_2nd.json")
        abspath = unixtools.find_datafile(relpath, pathlist=os.getenv("CALIBPATH").split(":"))
        BinnnedHits.layerMapFile = abspath

    # make the bintool class
    BinTool = CompFactory.FPGATrackSimBinTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimBinTool_2nd"))

    # Inputs for the BinTool
    binsteps=[]
    BinDesc=None
    if (cutset["parSet"]=="PhiSlicedKeyLyrPars"):
        BinDesc = CompFactory.FPGATrackSimKeyLayerBinDesc(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimKeyLayerBinDesc2nd"))
        BinDesc.rin=cutset["rin"]
        BinDesc.rout=cutset["rout"]

        # parameters for key layer bindesc are :"zR1", "zR2", "phiR1", "phiR2", "xm"
        step1 = CompFactory.FPGATrackSimBinStep(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimPhiBinning_2nd"))
        step1.parBins = [1,1,cutset["parBins"][2],cutset["parBins"][3],cutset["parBins"][4]]
        step2 = CompFactory.FPGATrackSimBinStep(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimFullBinning_2nd"))
        step2.parBins = cutset["parBins"]
        binsteps = [step1,step2]

        #resolution padding
        BinDesc.D0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["d0"]
        BinDesc.EtaPad=getPadding(flags.Trigger.FPGATrackSim.region)["eta"]
        BinDesc.QPtPad=getPadding(flags.Trigger.FPGATrackSim.region)["qpt"]
        BinDesc.PhiPad=getPadding(flags.Trigger.FPGATrackSim.region)["phi"]
        BinDesc.Z0Pad=getPadding(flags.Trigger.FPGATrackSim.region)["z0"]
        BinDesc.fieldCorrection=True
        BinDesc.fieldCorRegion=flags.Trigger.FPGATrackSim.region
    else:
        log.fatal("Unknown Binning Setup: ",cutset["parSet"])

    BinTool.BinDesc = BinDesc
    BinTool.Steps = binsteps

    # configure the padding around the nominal region
    BinTool.d0FractionalPadding =0.05
    BinTool.z0FractionalPadding =0.05
    BinTool.etaFractionalPadding =0.05
    BinTool.phiFractionalPadding =0.05
    BinTool.qOverPtFractionalPadding =0.05
    BinTool.parMin = cutset["parMin"]
    BinTool.parMax = cutset["parMax"]
    BinnnedHits.BinTool = BinTool

    result.setPrivateTools(BinnnedHits)

    return result

def getWindowCuts(region):
    binSize = 0.2
    side = (region >> 5) & 0x1
    etaBin = (region >> 6) & 0x1F
    etaRange = [round(binSize * etaBin, 1), round(binSize * (etaBin + 1), 1)] if side else [round(-binSize * (etaBin + 1), 1), round(-binSize * etaBin, 1)]
    abs_etaRange = tuple(round(abs(val), 1) for val in etaRange)

    # Default (very large windows)
    default_phi = [0]*5 + [0.75, 0.75, 1.5, 1.5, 3.24, 3.24, 4.5, 4.5]
    default_z   = [0]*5 + [2145, 2145, 3645, 3645, 4657.5, 4657.5, 8400, 8400]

    # phi windows (optimized! but can be more opimized? also 1250  is not here since it has not yet been tested)
    eta_to_phi = {(0.0, 0.2): [0.0, 0.0, 0.0, 0.0, 0.0, 0.089, 0.089, 0.021, 0.021, 0.028, 0.029, 0.041, 0.041],
        (0.2, 0.4): [0.0, 0.0, 0.0, 0.0, 0.0, 0.014, 0.014, 0.018, 0.020, 0.075, 0.075, 0.038, 0.039],
        (0.4, 0.6): [0.0, 0.0, 0.0, 0.0, 0.0, 0.112, 0.112, 0.035, 0.036, 0.05, 0.051, 0.075, 0.076],
        (0.6, 0.8): [0.0, 0.0, 0.0, 0.0, 0.0, 0.029, 0.029, 0.037, 0.038, 0.052, 0.053, 0.077, 0.078],
        (0.8, 1.0): [0.0, 0.0, 0.0, 0.0, 0.0, 0.031, 0.031, 0.081, 0.081, 0.054, 0.055, 0.079, 0.08],
        (1.0, 1.2): [0.0, 0.0, 0.0, 0.0, 0.0, 0.026, 0.026, 0.035, 0.035, 0.095, 0.105, 0.074, 0.074],
        (1.2, 1.4): [0.0, 0.0, 0.0, 0.0, 0.0, 0.047, 0.047, 0.06, 0.06, 0.084, 0.085, 0.1, 0.1],
        (1.4, 1.6): [0.0, 0.0, 0.0, 0.0, 0.0, 0.065, 0.066, 0.088, 0.089, 0.12, 0.12, 0.15, 0.15],
        (1.6, 1.8): [0.0, 0.0, 0.0, 0.0, 0.0, 0.098, 0.099, 0.14, 0.14, 0.16, 0.16, 0.21, 0.21],
        (1.8, 2.0): [0.0, 0.0, 0.0, 0.0, 0.0, 0.11, 0.15, 0.15, 0.19, 0.19, 0.24, 0.25, 0.0],
        (2.0, 2.2): [0.0, 0.0, 0.0, 0.0, 0.0, 0.11, 0.13, 0.17, 0.17, 0.21, 0.21, 0.0, 0.0],
        (2.2, 2.4): [0.0, 0.0, 0.0, 0.0, 0.0, 0.097, 0.11, 0.12, 0.17, 0.17, 0.0, 0.0, 0.0],
        (2.4, 2.6): [0.0, 0.0, 0.0, 0.0, 0.0, 0.096, 0.1, 0.11, 0.17, 0.17, 0.0, 0.0, 0.0],
        (2.6, 2.8): [0.0, 0.0, 0.0, 0.0, 0.0, 0.096, 0.11, 0.11, 0.12, 0.0, 0.0, 0.0, 0.0],
        (2.8, 3.0): [0.0, 0.0, 0.0, 0.0, 0.0, 0.049, 0.052, 0.068, 0.083, 0.0, 0.0, 0.0, 0.0],
        (3.0, 3.2): [0.0, 0.0, 0.0, 0.0, 0.0, 0.105, 0.049, 0.06, 0.06, 0.0, 0.0, 0.0, 0.0],
        (3.2, 3.4): [0.0, 0.0, 0.0, 0.0, 0.0, 0.051, 0.052, 0.055, 0.075, 0.0, 0.0, 0.0, 0.0],
        (3.4, 3.6): [0.0, 0.0, 0.0, 0.0, 0.0, 0.093, 0.093, 0.093, 0.062, 0.0, 0.0, 0.0, 0.0],
        (3.6, 3.8): [0.0, 0.0, 0.0, 0.0, 0.0, 0.065, 0.065, 0.065, 0.065, 0.0, 0.0, 0.0, 0.0]}

    # z windows (optimized! but can be more opimized? also 1250  is not here since it has not yet been tested)
    eta_to_z = {(0.0, 0.2): [0.0, 0.0, 0.0, 0.0, 0.0, 29.0, 29.0, 38.0, 38.0, 48.0, 49.0, 61.0, 61.0],
        (0.2, 0.4): [0.0, 0.0, 0.0, 0.0, 0.0, 22.0, 22.0, 30.0, 30.0, 113.0, 113.0, 52.0, 52.0],
        (0.4, 0.6): [0.0, 0.0, 0.0, 0.0, 0.0, 75.0, 75.0, 58.2, 58.2, 67.0, 67.0, 56.0, 56.0],
        (0.6, 0.8): [0.0, 0.0, 0.0, 0.0, 0.0, 27.0, 27.0, 23.0, 23.0, 44.0, 44.0, 82.0, 82.0],
        (0.8, 1.0): [0.0, 0.0, 0.0, 0.0, 0.0, 81.0, 81.0, 144.0, 175.0, 110.0, 110.0, 130.0, 130.0],
        (1.0, 1.2): [0.0, 0.0, 0.0, 0.0, 0.0, 43.0, 43.0, 121.0, 121.0, 244.0, 244.0, 73.0, 73.0],
        (1.2, 1.4): [0.0, 0.0, 0.0, 0.0, 0.0, 75.0, 76.0, 190.0, 190.0, 99.0, 100.0, 192.0, 192.0],
        (1.4, 1.6): [0.0, 0.0, 0.0, 0.0, 0.0, 254.0, 68.0, 325.0, 325.0, 192.0, 192.0, 159.0, 159.0],
        (1.6, 1.8): [0.0, 0.0, 0.0, 0.0, 0.0, 300.0, 300.0, 105.0, 105.0, 371.0, 371.0, 195.0, 195.0],
        (1.8, 2.0): [0.0, 0.0, 0.0, 0.0, 0.0, 133.0, 308.0, 308.0, 224.0, 169.0, 189.0, 189.0, 0.0],
        (2.0, 2.2): [0.0, 0.0, 0.0, 0.0, 0.0, 284.0, 242.0, 87.0, 87.0, 135.0, 135.0, 0.0, 0.0],
        (2.2, 2.4): [0.0, 0.0, 0.0, 0.0, 0.0, 32.0, 110.0, 110.0, 257.0, 257.0, 0.0, 0.0, 0.0],
        (2.4, 2.6): [0.0, 0.0, 0.0, 0.0, 0.0, 231.0, 329.0, 329.0, 132.0, 132.0, 0.0, 0.0, 0.0],
        (2.6, 2.8): [0.0, 0.0, 0.0, 0.0, 0.0, 170.0, 170.0, 170.0, 189.0, 0.0, 0.0, 0.0, 0.0],
        (2.8, 3.0): [0.0, 0.0, 0.0, 0.0, 0.0, 59.0, 219.0, 219.0, 219.0, 0.0, 0.0, 0.0, 0.0],
        (3.0, 3.2): [0.0, 0.0, 0.0, 0.0, 0.0, 230.0, 230.0, 230.0, 181.0, 0.0, 0.0, 0.0, 0.0],
        (3.2, 3.4): [0.0, 0.0, 0.0, 0.0, 0.0, 82.0, 235.0, 235.0, 235.0, 0.0, 0.0, 0.0, 0.0],
        (3.4, 3.6): [0.0, 0.0, 0.0, 0.0, 0.0, 108.0, 108.0, 108.0, 108.0, 0.0, 0.0, 0.0, 0.0],
        (3.6, 3.8): [0.0, 0.0, 0.0, 0.0, 0.0, 149.0, 149.0, 149.0, 149.0, 0.0, 0.0, 0.0, 0.0]}

    phi_window = eta_to_phi.get(abs_etaRange, default_phi)
    z_window = eta_to_z.get(abs_etaRange, default_z)

    return phi_window, z_window


def FPGATrackSimWindowExtensionToolCfg(flags,name="FPGATrackSimWindowExtensionTool"):
    result = ComponentAccumulator()
    FPGATrackSimWindowExtensionTool = CompFactory.FPGATrackSimWindowExtensionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    # these are services so we use getPrimaryAndMerge; tools (configured elsewhere) should use popToolsAndMerge
    FPGATrackSimWindowExtensionTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    FPGATrackSimWindowExtensionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    # Hardcoded settings for now, hook up to flags later...
    # This threshold is the number of *missing* hits allowed
    FPGATrackSimWindowExtensionTool.threshold = flags.Trigger.FPGATrackSim.hitThreshold

    # These MUST be of size equal to the full number of layers (13), though only the "new" layers
    # in the second stage are actually used.
    #removing the flag here since the function has built in defaults
    FPGATrackSimWindowExtensionTool.zWindow =   getWindowCuts(flags.Trigger.FPGATrackSim.region)[1]
    FPGATrackSimWindowExtensionTool.phiWindow = getWindowCuts(flags.Trigger.FPGATrackSim.region)[0]
    # If we're doing binning, i.e. genscan.
    if flags.Trigger.FPGATrackSim.ActiveConfig.genScan:
        FPGATrackSimWindowExtensionTool.doBinning = True
        BinningTool = result.getPrimaryAndMerge(FPGATrackSimBinnedHitsToolCfg_2nd(flags))
        BinningTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
        FPGATrackSimWindowExtensionTool.BinningTool = BinningTool

    # Cut the number of branches, equivalent to 610 option.
    if len(flags.Trigger.FPGATrackSim.maxBranches) == 1:
        FPGATrackSimWindowExtensionTool.maxHits = [1] * 5 + flags.Trigger.FPGATrackSim.maxBranches * 8
    elif len(flags.Trigger.FPGATrackSim.maxBranches) == 8:
        FPGATrackSimWindowExtensionTool.maxHits = [1] * 5 + flags.Trigger.FPGATrackSim.maxBranches
    else:
        raise RuntimeError(f"Failed to set maxhits with maxbranches = {flags.Trigger.FPGATrackSim.maxBranches}")

    # Other settings, shared with the first stage mostly. disable 2nd stage tracking for now.
    FPGATrackSimWindowExtensionTool.fieldCorrection =flags.Trigger.FPGATrackSim.ActiveConfig.fieldCorrection
    FPGATrackSimWindowExtensionTool.IdealGeoRoads = False # (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    FPGATrackSimWindowExtensionTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints
    FPGATrackSimWindowExtensionTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    FPGATrackSimWindowExtensionTool.addAllHits=flags.Trigger.FPGATrackSim.ActiveConfig.addAllHits
    result.setPrivateTools(FPGATrackSimWindowExtensionTool)
    return result

def FPGATrackSimNNPathfinderExtensionToolCfg(flags,name="FPGATrackSimNNPathfinderExtensionTool"):
    result = ComponentAccumulator()
    FPGATrackSimNNPathfinderExtensionTool = CompFactory.FPGATrackSimNNPathfinderExtensionTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))

    # these are services so we use getPrimaryAndMerge; tools (configured elsewhere) should use popToolsAndMerge
    FPGATrackSimNNPathfinderExtensionTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))

    # Hardcoded settings for now, hook up to flags later...
    if (flags.Trigger.FPGATrackSim.varyingThreshold):
        FPGATrackSimNNPathfinderExtensionTool.threshold = getMaxMissing_PathFinder(flags.Trigger.FPGATrackSim.region,flags.Trigger.FPGATrackSim.varyingHitThresholds)
    else:
        FPGATrackSimNNPathfinderExtensionTool.threshold = flags.Trigger.FPGATrackSim.hitThreshold

    windows = getWindowsNN2ndStage(flags.Trigger.FPGATrackSim.region)
    ### scale the windows now!
    for i in range(3):
        for j in range(len(windows[0])):
            windows[0][j] = windows[0][j] * flags.Trigger.FPGATrackSim.windowRScaling
            windows[1][j] = windows[1][j] * flags.Trigger.FPGATrackSim.windowPhiScaling
            windows[2][j] = windows[2][j] * flags.Trigger.FPGATrackSim.windowZScaling
    FPGATrackSimNNPathfinderExtensionTool.windowR = windows[0]
    FPGATrackSimNNPathfinderExtensionTool.windowPhi = windows[1]
    FPGATrackSimNNPathfinderExtensionTool.windowZ = windows[2]
    FPGATrackSimNNPathfinderExtensionTool.windowFineID = windows[3]
    FPGATrackSimNNPathfinderExtensionTool.lowPtValueWindowR = flags.Trigger.FPGATrackSim.lowPtvalueR
    FPGATrackSimNNPathfinderExtensionTool.lowPtRScaling = flags.Trigger.FPGATrackSim.lowPtWindowRScaling
    FPGATrackSimNNPathfinderExtensionTool.lowPtValueWindowZ = flags.Trigger.FPGATrackSim.lowPtvalueZ
    FPGATrackSimNNPathfinderExtensionTool.lowPtZScaling = flags.Trigger.FPGATrackSim.lowPtWindowZScaling
    FPGATrackSimNNPathfinderExtensionTool.missedHitRScaling = flags.Trigger.FPGATrackSim.missedHitRScaling
    FPGATrackSimNNPathfinderExtensionTool.missedHitZScaling = flags.Trigger.FPGATrackSim.missedHitZScaling
    FPGATrackSimNNPathfinderExtensionTool.missedHitPhiScaling = flags.Trigger.FPGATrackSim.missedHitPhiScaling
    FPGATrackSimNNPathfinderExtensionTool.lowPtValueWindowPhi = flags.Trigger.FPGATrackSim.lowPtvaluePhi
    FPGATrackSimNNPathfinderExtensionTool.lowPtPhiScaling = flags.Trigger.FPGATrackSim.lowPtWindowPhiScaling
    FPGATrackSimNNPathfinderExtensionTool.maxBranches = getMaxBranches_PathFinder(flags.Trigger.FPGATrackSim.region,flags.Trigger.FPGATrackSim.maxBranches)
    FPGATrackSimNNPathfinderExtensionTool.useCartesian = flags.Trigger.FPGATrackSim.NNCartesianCoordinates
    FPGATrackSimNNPathfinderExtensionTool.OutputRegion = str(flags.Trigger.FPGATrackSim.region)
    
    FPGATrackSimNNPathfinderExtensionTool.doOutsideIn = True
    if (flags.Trigger.FPGATrackSim.ActiveConfig.genScan):
        FPGATrackSimNNPathfinderExtensionTool.doOutsideIn = False
        FPGATrackSimNNPathfinderExtensionTool.predictionWindowLength = 4

    # Other settings
    FPGATrackSimNNPathfinderExtensionTool.OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    result.setPrivateTools(FPGATrackSimNNPathfinderExtensionTool)
    return result


# Need to figure out if we have two output writers or somehow only one.
def FPGATrackSimSecondStageOutputCfg(flags,name="FPGATrackSimWriteOutputSecondStage"):
    result=ComponentAccumulator()
    FPGATrackSimWriteOutput = CompFactory.FPGATrackSimOutputHeaderTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    FPGATrackSimWriteOutput.InFileName = ["test.root"]
    FPGATrackSimWriteOutput.OutputTreeName = FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,"FPGATrackSimSecondStageTree")
    if not flags.Trigger.FPGATrackSim.writeAdditionalOutputData:
        FPGATrackSimWriteOutput.EventLimit = 0
    else:
        FPGATrackSimWriteOutput.EventLimit = flags.Trigger.FPGATrackSim.writeOutputEventLimit
    # RECREATE means that that this tool opens the file.
    # HEADER would mean that something else (e.g. THistSvc) opens it and we just add the object.
    FPGATrackSimWriteOutput.RWstatus = "HEADER"
    FPGATrackSimWriteOutput.THistSvc = CompFactory.THistSvc()
    result.setPrivateTools(FPGATrackSimWriteOutput)
    return result

def FPGATrackSimHoughRootOutputToolCfg(flags,name="FPGATrackSimHoughRootOutputTool"):
    result=ComponentAccumulator()
    HoughRootOutputTool = CompFactory.FPGATrackSimHoughRootOutputTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    HoughRootOutputTool.FPGATrackSimEventSelectionSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    HoughRootOutputTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    HoughRootOutputTool.THistSvc = CompFactory.THistSvc()
    HoughRootOutputTool.OutputRegion = str(flags.Trigger.FPGATrackSim.region)
    result.setPrivateTools(HoughRootOutputTool)
    return result

def NNTrackToolCfg(flags,name="FPGATrackSimNNTrackTool_2nd"):
    result=ComponentAccumulator()
    NNTrackTool = CompFactory.FPGATrackSimNNTrackTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    NNTrackTool.THistSvc = CompFactory.THistSvc()
    NNTrackTool.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    NNTrackTool.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    NNTrackTool.IdealGeoRoads = False
    NNTrackTool.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints and not flags.Trigger.FPGATrackSim.ActiveConfig.genScan
    NNTrackTool.SPRoadFilterTool = result.popToolsAndMerge(FPGATrackSimAnalysisConfig.SPRoadFilterToolCfg(flags,secondStage=True))
    NNTrackTool.Do2ndStageTrackFit = True
    NNTrackTool.useSectors = False
    NNTrackTool.useCartesian = flags.Trigger.FPGATrackSim.NNCartesianCoordinates
    result.setPrivateTools(NNTrackTool)
    return result


def FPGATrackSimTrackFitterToolCfg(flags,name="FPGATrackSimTrackFitterTool_2nd"):
    result=ComponentAccumulator()
    TF = CompFactory.FPGATrackSimTrackFitterTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    TF.GuessHits = flags.Trigger.FPGATrackSim.ActiveConfig.guessHits
    TF.IdealCoordFitType = flags.Trigger.FPGATrackSim.ActiveConfig.idealCoordFitType
    TF.FPGATrackSimBankSvc = result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))
    TF.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    TF.chi2DofRecoveryMax = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMax
    TF.chi2DofRecoveryMin = flags.Trigger.FPGATrackSim.ActiveConfig.chi2DoFRecoveryMin
    TF.doMajority = flags.Trigger.FPGATrackSim.ActiveConfig.doMajority
    TF.nHits_noRecovery = flags.Trigger.FPGATrackSim.ActiveConfig.nHitsNoRecovery
    TF.DoDeltaGPhis = flags.Trigger.FPGATrackSim.ActiveConfig.doDeltaGPhis
    TF.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    TF.IdealGeoRoads = (flags.Trigger.FPGATrackSim.ActiveConfig.IdealGeoRoads and flags.Trigger.FPGATrackSim.tracking)
    TF.useSpacePoints = flags.Trigger.FPGATrackSim.spacePoints
    TF.SPRoadFilterTool = result.popToolsAndMerge(FPGATrackSimAnalysisConfig.SPRoadFilterToolCfg(flags,secondStage=True))
    TF.Do2ndStageTrackFit = True
    result.setPrivateTools(TF)
    return result

def FPGATrackSimOverlapRemovalToolCfg(flags,name="FPGATrackSimOverlapRemovalTool_2nd"):
    result=ComponentAccumulator()
    OR = CompFactory.FPGATrackSimOverlapRemovalTool(FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name))
    OR.ORAlgo = "Normal"
    OR.doFastOR =flags.Trigger.FPGATrackSim.ActiveConfig.doFastOR   
    OR.NumOfHitPerGrouping = 5
    OR.FPGATrackSimMappingSvc = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        OR.MinChi2 = getChi2CutNN2ndStage(flags.Trigger.FPGATrackSim.region)
    else:
        OR.MinChi2 = flags.Trigger.FPGATrackSim.ActiveConfig.secondChi2Cut

    if flags.Trigger.FPGATrackSim.ActiveConfig.hough:
        OR.nBins_x = flags.Trigger.FPGATrackSim.ActiveConfig.xBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.xBufferBins
        OR.nBins_y = flags.Trigger.FPGATrackSim.ActiveConfig.yBins + 2 * flags.Trigger.FPGATrackSim.ActiveConfig.yBufferBins
        OR.localMaxWindowSize = flags.Trigger.FPGATrackSim.ActiveConfig.localMaxWindowSize
        OR.roadSliceOR = flags.Trigger.FPGATrackSim.ActiveConfig.roadSliceOR

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimOverlapRemovalToolMonitoringCfg
    OR.MonTool = result.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolMonitoringCfg(flags))

    result.setPrivateTools(OR)
    return result

def prepareFlagsForFPGATrackSimSecondStageAlg(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag,keepOriginal=True)
    return newFlags

def FPGATrackSimSecondStageAlgCfg(inputFlags,name="FPGATrackSimSecondStageAlg",suffix="",**kwargs):

    flags = prepareFlagsForFPGATrackSimSecondStageAlg(inputFlags)

    result=ComponentAccumulator()

    
    theFPGATrackSimSecondStageAlg=CompFactory.FPGATrackSimSecondStageAlg(name=FPGATrackSimDataPrepConfig.nameWithRegionSuffix(flags,name),**kwargs)
    theFPGATrackSimSecondStageAlg.writeOutputData = flags.Trigger.FPGATrackSim.writeAdditionalOutputData
    theFPGATrackSimSecondStageAlg.tracking = flags.Trigger.FPGATrackSim.secondTracking
    theFPGATrackSimSecondStageAlg.DoMissingHitsChecks = flags.Trigger.FPGATrackSim.ActiveConfig.doMissingHitsChecks
    theFPGATrackSimSecondStageAlg.DoHoughRootOutput2nd = flags.Trigger.FPGATrackSim.ActiveConfig.houghRootoutput2nd
    theFPGATrackSimSecondStageAlg.DoNNTrack_2nd = flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd
    theFPGATrackSimSecondStageAlg.eventSelector = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimEventSelectionSvcCfg(flags))
    if flags.Trigger.FPGATrackSim.ActiveConfig.useVaryingChi2Cut and flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        theFPGATrackSimSecondStageAlg.TrackScoreCut = getChi2CutNN2ndStage(flags.Trigger.FPGATrackSim.region)
    else:
        theFPGATrackSimSecondStageAlg.TrackScoreCut = flags.Trigger.FPGATrackSim.ActiveConfig.secondChi2Cut

    theFPGATrackSimSecondStageAlg.doNNPathFinder = flags.Trigger.FPGATrackSim.doNNPathFinder
    
    FPGATrackSimMapping = result.getPrimaryAndMerge(FPGATrackSimDataPrepConfig.FPGATrackSimMappingCfg(flags))
    theFPGATrackSimSecondStageAlg.FPGATrackSimMapping = FPGATrackSimMapping
    theFPGATrackSimSecondStageAlg.passLowestChi2TrackOnly = flags.Trigger.FPGATrackSim.ActiveConfig.passLowestChi2TrackOnly

    # If tracking is set to False, don't configure the bank service
    if theFPGATrackSimSecondStageAlg.tracking and not flags.Trigger.FPGATrackSim.ActiveConfig.trackNNAnalysis2nd:
        result.getPrimaryAndMerge(FPGATrackSimAnalysisConfig.FPGATrackSimBankSvcCfg(flags))

    # Here, configure the window tool.
    if(flags.Trigger.FPGATrackSim.doNNPathFinder ):
        theFPGATrackSimSecondStageAlg.TrackExtensionTool = result.popToolsAndMerge(FPGATrackSimNNPathfinderExtensionToolCfg(flags))
    else:
        theFPGATrackSimSecondStageAlg.TrackExtensionTool = result.popToolsAndMerge(FPGATrackSimWindowExtensionToolCfg(flags))

    theFPGATrackSimSecondStageAlg.HoughRootOutputTool = result.popToolsAndMerge(FPGATrackSimHoughRootOutputToolCfg(flags))

    theFPGATrackSimSecondStageAlg.NNTrackTool = result.popToolsAndMerge(NNTrackToolCfg(flags))

    theFPGATrackSimSecondStageAlg.OutputTool = result.popToolsAndMerge(FPGATrackSimSecondStageOutputCfg(flags))
    theFPGATrackSimSecondStageAlg.TrackFitter_2nd = result.popToolsAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
    theFPGATrackSimSecondStageAlg.OverlapRemoval_2nd = result.popToolsAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))
    theFPGATrackSimSecondStageAlg.SetTruthParametersForTracks = flags.Trigger.FPGATrackSim.SetTruthParametersForTracks

    theFPGATrackSimSecondStageAlg.Spacepoints = flags.Trigger.FPGATrackSim.spacePoints

    from FPGATrackSimAlgorithms.FPGATrackSimAlgorithmConfig import FPGATrackSimSecondStageAlgMonitoringCfg
    theFPGATrackSimSecondStageAlg.MonTool = result.popToolsAndMerge(FPGATrackSimSecondStageAlgMonitoringCfg(flags))

    result.addEventAlgo(theFPGATrackSimSecondStageAlg)

    return result

if __name__ == "__main__":
    print("Running second stage separately is currently unsupported")
