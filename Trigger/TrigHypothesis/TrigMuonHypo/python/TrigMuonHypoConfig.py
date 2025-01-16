# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


# import Hypo Algs/Tools
from AthenaConfiguration.ComponentFactory import CompFactory # tools are imported from the factory, (NewJO)

# import monitoring
from TrigMuonHypo.TrigMuonHypoMonitoring import (
    TrigmuCombHypoMonitoring,
    TrigMuonEFHypoMonitoring,
    TrigMuonEFIdtpHypoMonitoring,
    TrigL2MuonOverlapRemoverMonitoringMucomb,
    TrigMuonEFInvMassHypoMonitoring,
    TrigMuonEFIdtpInvMassHypoMonitoring,
    TrigMuonEFTrackIsolationMonitoring
)

monitorAll = False #should only be true for local debugging to have histograms from all chains

# other imports
from AthenaCommon.SystemOfUnits import GeV

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigMuonHypoConfig')

muFastThresholds = {
    # added for Run3 low-mu 2mu3
    '3GeV_v22a'              : [ [0,9.9], [ 0.1 ] ],
    # 2011a tuning + 2015 tuning
    '4GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  3.38,  1.25,  3.17,  3.41] ],
    '4GeV_barrelOnly_v15a'   : [ [0,1.05,1.5,2.0,9.9], [  3.38, 1000., 1000., 1000.] ],
    '6GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  5.17,  3.25,  4.69,  5.14] ],
    '8GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  6.63,  5.17,  6.39,  6.81] ],
    '10GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  8.28,  6.35,  7.19,  8.58] ],
    #not optimized: ATR-20049
    '11GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  8.99,  6.62,  7.40,  9.32] ],
    '13GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 10.42,  7.16,  7.81, 10.80] ],
    '14GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 11.15,  7.58,  8.43, 11.61] ],
    '15GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 11.31, 10.52, 12.00, 13.24] ],
    '18GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 14.33,  9.45, 10.96, 14.35] ],
    '20GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.10,  3.25,  2.69,  5.14] ], # new
    '22GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.20,  3.25,  4.90,  5.00] ], # new
    '23GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.20,  3.25,  4.90,  5.00] ], # new
    '24GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  4.80,  3.25,  4.50,  5.10] ], # new
    '26GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.50,  3.25,  4.45,  4.40] ], # new
    '28GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.70,  3.25,  4.50,  5.40] ], # new
    '30GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.00,  4.00,  4.30,  5.55] ], # new
    '32GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.00,  6.50,  4.20,  6.00] ], # new
    '34GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.40,  7.20,  4.60,  9.00] ], # new
    '35GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.30,  5.40,  4.80,  5.00] ], # new
    '36GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  4.20,  3.90,  4.30,  5.00] ], # new
    '40GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.16,  6.20,  5.50, 10.00] ], # new
    '45GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  3.20,  9.80,  5.30, 10.70] ], # new
    '48GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  4.00,  7.30,  5.50,  9.50] ], # new
    '50GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  1.30,  8.10,  4.50,  9.50] ], # new
    '60GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 21.20, 25.38, 29.54] ],
    '80GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 21.20, 25.38, 29.54] ],
    '40GeV_uptoEC2_v15a'     : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 21.20, 25.38, 1000.] ],
    '40GeV_barrelOnly_v15a'  : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 1000., 1000., 1000.] ],
    '50GeV_barrelOnly_v15a'  : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 1000., 1000., 1000.] ],
    '60GeV_barrelOnly_v15a'  : [ [0,1.05,1.5,2.0,9.9], [ 21.13, 1000., 1000., 1000.] ],
    }

muCombThresholds = {
    # added for Run3 low-mu 2mu3
    '3GeV_v22a'              : [ [0,1.05,1.5,2.0,9.9], [  2.82,  2.76,  2.64,  2.64] ],
    # original + 2015 tuning
    '2GeV_v15a'              : [ [0,9.9],              [ 2.000] ],
    '3GeV_v15a'              : [ [0,9.9],              [ 3.000] ],
    '4GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  3.86,  3.77,  3.69,  3.70] ],
    '5GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  4.9,  4.8,  4.8,  4.8] ],
    '6GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  5.87,  5.79,  5.70,  5.62] ],
    '7GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  6.8,  6.7,  6.7,  6.6] ],
    '8GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  7.80,  7.72,  7.59,  7.46] ],
    '10GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  9.73,  9.63,  9.45,  9.24] ],
    '11GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 10.8, 10.4, 10.6, 10.6] ],
    '12GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 11.7, 11.3, 11.4, 11.5] ],
    '13GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 12.62, 12.48, 12.24, 11.88] ],
    '14GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 13.57, 13.44, 13.21, 12.77] ],
    '15GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 14.50, 14.00, 14.00, 14.50] ],
    '16GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 15.47, 15.09, 14.98, 15.08] ], # Lidia - extrapolated not optimized
    '18GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 17.41, 17.27, 16.95, 16.25] ],
    '20GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 16.50, 19.19, 18.80, 17.00] ], # new 
    '22GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.70, 21.07, 19.13, 18.80] ], # new 
    '23GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.70, 21.07, 19.13, 18.80] ], # new 
    '24GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.10, 21.07, 18.60, 19.35] ], # new 
    '25GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 24.20, 23.20, 23.20, 22.60] ],
    '26GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 21.50, 21.07, 20.22, 19.65] ], # new 
    '27GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 26.20, 25.10, 25.10, 24.40] ],
    '28GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.30, 21.07, 20.77, 20.00] ], # new 
    '30GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.80, 21.00, 21.00, 20.10] ], # new 
    '32GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 16.50, 22.50, 20.00, 21.50] ], # new 
    '34GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 18.40, 24.10, 21.45, 23.00] ], # new 
    '35GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 17.40, 25.90, 23.40, 23.80] ], # new 
    '36GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 16.90, 25.60, 24.20, 24.00] ], # new
    '38GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 35.80, 35.40, 35.40, 33.60] ],
    '40GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 13.20, 27.90, 27.30, 27.60] ], # new
    '40GeV_slow_v15a'        : [ [0,1.05,1.5,2.0,9.9], [ 40.00, 40.00, 40.00, 40.00] ],
    '45GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 17.70, 33.10, 31.60, 33.30] ], # new
    '48GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 14.00, 33.80, 34.20, 35.90] ], # new
    '50GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  8.30, 33.55, 32.40, 37.70] ], # new
    '60GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 45.0, 45.0, 45.0, 45.0] ],
    '60GeV_slow_v15a'        : [ [0,1.05,1.5,2.0,9.9], [ 47.0, 47.0, 47.0, 47.0] ],
    '70GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 49.0, 49.0, 49.0, 49.0] ],
    '80GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 56.0, 56.0, 56.0, 56.0] ],
    '100GeV_v15a'            : [ [0,1.05,1.5,2.0,9.9], [ 70.0, 70.0, 70.0, 70.0] ],

    '5GeV_v16'             : [ [0,1.05,1.5,2.0,9.9], [  4.9,  4.8,  4.8,  4.8] ], 
    '8GeV_v16'             : [ [0,1.05,1.5,2.0,9.9], [  7.8,  7.7,  7.7,  7.7] ],
    '10GeV_v16'            : [ [0,1.05,1.5,2.0,9.9], [  9.8,  9.5,  9.6,  9.7] ],
    '15GeV_v16'             : [ [0,1.05,1.5,2.0,9.9], [ 14.50, 14.00, 14.00, 14.50] ],
    '20GeV_v16'             : [ [0,1.05,1.5,2.0,9.9], [ 19.31, 19.19, 18.80, 17.95] ],
    '25GeV_v16'            : [ [0,1.05,1.5,2.0,9.9], [ 24.2, 23.2, 23.2, 22.6] ],
    '30GeV_v16'            : [ [0,1.05,1.5,2.0,9.9], [ 29.0, 28.0, 28.0, 27.0] ],
    '50GeV_v16'            : [ [0,1.05,1.5,2.0,9.9], [ 40.0, 40.0, 40.0, 40.0] ],
    }

trigMuonEFSAThresholds = {
    # added for Run3 low-mu 2mu3
    '3GeV_v22a'              : [ [0,9.9], [ 1.0 ] ],
    #
    '0GeV'                   : [ [0,9.9],              [ 0.100 ] ],
    '2GeV'                   : [ [0,9.9],              [ 2.000 ] ],
    '3GeV'                   : [ [0,9.9],              [ 3.000 ] ],
    '4GeV'                   : [ [0,1.05,1.5,2.0,9.9], [  3.0,  2.5,  2.5,  2.5] ],
    '4GeV_barrelOnly'        : [ [0,1.05,1.5,2.0,9.9], [  3.0,1000.0,1000.0,1000.0]],
    '5GeV'                   : [ [0,1.05,1.5,2.0,9.9], [  4.6,  3.3,  4.0,  4.5] ],
    '6GeV'                   : [ [0,1.05,1.5,2.0,9.9], [  5.4,  4.5,  4.9,  5.3] ],
    '7GeV'                   : [ [0,1.05,1.5,2.0,9.9], [  6.3,  5.6,  5.6,  6.3] ],
    '8GeV'                   : [ [0,1.05,1.5,2.0,9.9], [  7.2,  6.7,  6.4,  7.3] ],
    '10GeV'                  : [ [0,1.05,1.5,2.0,9.9], [  8.9,  9.0,  8.4,  9.2] ],
    '11GeV'                  : [ [0,1.05,1.5,2.0,9.9], [  9.8, 10.1,  9.3, 10.1] ],
    '12GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 10.6, 11.0, 10.2, 11.0] ],
    '13GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 11.4, 12.0, 11.1, 12.0] ],
    '14GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 12.2, 13.0, 12.1, 13.0] ],
    '15GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 13.0, 14.0, 13.0, 14.0] ],
    '16GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 13.0, 14.0, 13.0, 14.0] ], # not optimized
    '18GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 15.7, 16.6, 15.4, 16.3] ],
    '20GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 18.20, 16.50, 17.00, 18.00] ], # new
    '22GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 20.20, 18.00, 17.00, 19.00] ], # new 
    '23GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 21.20, 18.75, 17.80, 19.60] ], # new 
    '24GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 22.20, 19.50, 18.60, 20.20] ], # new 
    '26GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 23.50, 21.00, 20.15, 22.00] ], # new
    '28GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 23.40, 21.00, 20.55, 22.50] ], # new
    '30GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 25.20, 22.10, 21.70, 24.40] ], # new 
    '32GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 27.05, 20.50, 22.30, 26.12] ], # new 
    '34GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 28.90, 22.00, 24.00, 27.88] ], # new
    '35GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 30.00, 20.90, 24.90, 28.85] ], # new 
    '36GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 30.50, 23.77, 26.40, 28.00] ], # new
    '40GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 33.00, 27.30, 30.50, 32.90] ], # new
    '40GeV_barrelOnly'       : [ [0,1.05,1.5,2.0,9.9], [ 31.50,1000.0,1000.0,1000.0] ],
    '40GeV_uptoEC2'          : [ [0,1.05,1.5,2.0,9.9], [ 31.50, 30.00, 28.50, 1000.0] ],
    '45GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 41.20, 31.90, 35.10, 37.50] ], # new
    '48GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 43.00, 34.45, 38.00, 40.30] ], # new
    '50GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 40.00, 36.30, 40.00, 42.10] ], # new
    '50GeV_barrelOnly'       : [ [0,1.05,1.5,2.0,9.9], [ 45.0,1000.0,1000.0,1000.0]],
    '60GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 54.0, 54.0, 54.0, 54.0] ],
    '60GeV_barrelOnly'       : [ [0,1.05,1.5,2.0,9.9], [ 54.0,1000.0,1000.0,1000.0]],
    '70GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 63.0, 63.0, 63.0, 63.0] ],
    '80GeV'                  : [ [0,1.05,1.5,2.0,9.9], [ 72.0, 72.0, 72.0, 72.0] ],
    '100GeV'                 : [ [0,1.05,1.5,2.0,9.9], [ 90.0, 90.0, 90.0, 90.0] ],
   }

efCombinerThresholds = {
    #
    # added for Run3 low-mu 2mu3
    '3GeV_v22a'              : [ [0,1.05,1.5,2.0,9.9], [  2.94,  2.91,  2.77,  2.72] ],
    # original + 2015 tuning
    '2GeV_v15a'              : [ [0,9.9], [2.000] ],
    '3GeV_v15a'              : [ [0,9.9], [3.000] ],
    '4GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  3.94,  3.91,  3.77,  3.72] ],
    '5GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  4.91,  4.86,  4.84,  4.83] ],
    '6GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  5.92,  5.86,  5.70,  5.64] ],
    '7GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  6.85,  6.77,  6.74,  6.74] ],
    '8GeV_v15a'              : [ [0,1.05,1.5,2.0,9.9], [  7.89,  7.81,  7.60,  7.53] ],
    '10GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [  9.84,  9.77,  9.54,  9.47] ],
    '11GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 10.74, 10.64, 10.58, 10.53] ],
    '12GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 11.70, 11.59, 11.53, 11.49] ],
    '13GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 12.80, 12.67, 12.43, 12.38] ],
    '14GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 13.75, 13.62, 13.38, 13.36] ],
    '15GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 14.63, 14.49, 14.42, 14.38] ],
    '16GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 15.65, 15.50, 15.39, 15.37] ], # Lidia - extrapolated not optimized
    '18GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 17.68, 17.51, 17.34, 17.34] ],
    '20GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 19.65, 19.50, 19.20, 19.30] ], # new 
    '22GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 21.64, 21.32, 21.30, 21.30] ], # new 
    '23GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 22.59, 22.27, 22.25, 22.27] ], # new 
    '24GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 23.55, 23.21, 23.20, 23.25] ], # new 
    '26GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 25.60, 25.15, 25.20, 25.20] ], # new
    '27GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 26.26, 26.12, 26.11, 26.02] ],
    '28GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 27.28, 27.09, 27.15, 27.20] ], # new 
    '30GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 29.20, 29.00, 29.00, 29.00] ], # new 
    '32GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 31.20, 30.95, 30.90, 30.84] ], # new 
    '34GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 33.16, 32.88, 32.80, 32.70] ], # new 
    '35GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 34.15, 33.82, 33.65, 33.65] ], # new 
    '36GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 35.23, 34.75, 34.50, 34.60] ], # new
    '38GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 36.87, 36.67, 36.55, 36.48] ],
    '40GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 38.80, 38.56, 38.40, 38.90] ], # new
    '45GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 43.75, 43.30, 43.10, 43.90] ], # new
    '48GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 46.50, 46.20, 46.00, 46.80] ], # new
    '50GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 48.00, 48.20, 48.00, 48.80] ], # new
    '60GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 54.00, 54.00, 54.00, 54.00] ],
    '70GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 63.00, 63.00, 63.00, 63.00] ],
    '80GeV_v15a'             : [ [0,1.05,1.5,2.0,9.9], [ 72.00, 72.00, 72.00, 72.00] ],
    '100GeV_v15a'            : [ [0,1.05,1.5,2.0,9.9], [ 90.00, 90.00, 90.00, 90.00] ],
    }


muFastThresholdsForECWeakBRegion = {
    #
    # added for Run3 low-mu 2mu3
    '3GeV_v22a'             : [  0.1,   0.1 ],
    # 2011a tuning + 2015 tuning
    '4GeV_v15a'             : [  2.72,  1.58],
    '4GeV_barrelOnly_v15a'  : [ 1000., 1000. ],
    '6GeV_v15a'             : [  3.91,  2.22],
    '8GeV_v15a'             : [  4.65,  3.26],
    '10GeV_v15a'            : [  5.96,  4.24],
    #not optimized: ATR-20049
    '11GeV_v15a'            : [  6.19,  4.37 ],
    '13GeV_v15a'            : [  6.65,  4.64 ],
    '14GeV_v15a'            : [  6.78,  5.03 ],
    '15GeV_v15a'            : [  7.61,  7.81 ],
    '18GeV_v15a'            : [  8.48,  7.26 ],
    '20GeV_v15a'            : [  8.63,  7.26 ],
    '22GeV_v15a'            : [  9.53,  7.77 ],
    '23GeV_v15a'            : [  9.53,  7.77 ],
    '24GeV_v15a'            : [  9.02,  8.31 ],
    '26GeV_v15a'            : [  9.89,  8.77 ],
    '28GeV_v15a'            : [  9.89,  8.77 ], # copy of 26
    '30GeV_v15a'            : [ 14.41, 17.43 ],
    '32GeV_v15a'            : [ 14.41, 17.43 ], # copy of 30
    '34GeV_v15a'            : [ 14.41, 17.43 ], # copy of 30
    '35GeV_v15a'            : [ 14.41, 17.43 ], # copy of 30
    '36GeV_v15a'            : [ 10.78, 10.66 ],
    '40GeV_v15a'            : [ 15.07, 18.02 ],
    '45GeV_v15a'            : [ 15.07, 18.02 ], # copy of 40
    '48GeV_v15a'            : [ 15.07, 18.02 ], # copy of 50
    '50GeV_v15a'            : [ 15.07, 18.02 ], # lixia, not optimized
    '60GeV_v15a'            : [ 15.07, 18.02 ],
    '80GeV_v15a'            : [ 15.07, 18.02 ],
    '40GeV_uptoEC2_v15a'    : [ 15.07, 18.02 ],
    '40GeV_barrelOnly_v15a' : [ 1000., 1000. ],
    '50GeV_barrelOnly_v15a' : [ 1000., 1000. ],
    '60GeV_barrelOnly_v15a' : [ 1000., 1000. ],
    }


# Working points for EF track isolation algorithm
# syntax is:
# 'WPname' : cut on 0.3 cone
# put < 0 for no cut
trigMuonEFTrkIsoThresholds = {
    'ivarloose'       : 0.16, #ivarloose
    'ivarmedium'      : 0.07, #ivarmedium
    'ivartight'       : 0.06, #ivartight
    'ivarverytight'   : 0.04,   #ivarverytight
    'iloosems'        : 3000.0 #ms-only iso
    }

trigMuonLrtd0Cut = {
    'd0loose' : 2.0,
    'd0medium' : 3.0,
    'd0tight' : 5.0
    }

#Possible dimuon mass window cuts
#Fomat is [lower bound, upper bound] in GeV
# <0 for no cut
trigMuonEFInvMassThresholds = {
    'invmJPsiOS' : [2.5, 4.3],
    'invmDimu'   : [1.5, 14.],
    'idZmumu'    : [50., 130.],
    'idJpsimumu' : [1., 5.],
}

# Monitoring groups to monitor
muonHypoMonGroups = ['muonMon:online']
idHypoMonGroups = ['idMon:t0','idMon:shifter']

def getThresholdsFromDict( chainDict ):
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='Muon' or i['signature']=='Bphysics']
    return sum( [ [part['threshold']]*int(part['multiplicity']) for part in cparts ], [])


def TrigMufastHypoAlgCfg(flags, name="UNSPECIFIED", **kwargs):
    return CompFactory.TrigMufastHypoAlg(name, **kwargs)

def TrigMufastHypoToolFromDict(flags, chainDict):

    chainPart = chainDict['chainParts'][0]

    doOverlapRemoval = False
    if chainPart['signature'] == 'Bphysics' or 'l2io' in chainPart['l2AlgInfo']:
        doOverlapRemoval = False
    elif int(chainPart['multiplicity']) > 1:
        doOverlapRemoval = True
    elif len(chainDict['signatures']) > 1 and not chainPart['extra']:
        doOverlapRemoval = True

    doMonitoring = monitorAll or any(group in muonHypoMonGroups for group in chainDict['monGroups'])

    config = TrigMufastHypoToolConfig(chainDict['chainName'], chainPart, doOverlapRemoval, doMonitoring)
    config.compile(flags)
    return config.tool()


class TrigMufastHypoToolConfig:
    def __init__(self, name, cpart, doOverlapRemoval = False, doMonitoring = False):

        from AthenaCommon.Logging import logging
        self.__log = logging.getLogger('TrigMufastHypoToolConfig')
        self.__name = name

        self.__threshold = int(cpart['threshold'])
        self.__multiplicity = int(cpart['multiplicity'])
        self.__isPassThrough = 'mucombTag' in cpart['extra']
        self.__isCalibration = 'muoncalib' in cpart['extra']
        self.__isBarrelOnly = '0eta105' in cpart['etaRange']
        self.__useGeV_v15a = any(x in cpart['addInfo'] for x in ['idperf', 'idtp', '3layersEC'])
        self.__doL2MT = 'l2mt' in cpart['l2AlgInfo']
        self.__doOverlapRemoval = doOverlapRemoval
        self.__doMonitoring = doMonitoring

        from AthenaConfiguration.ComponentFactory import CompFactory
        tool = CompFactory.TrigMufastHypoTool(name)
        self.__tool = tool

    def multiplicity(self):
        return self.__multiplicity

    def tool(self):
        return self.__tool

    def toolName(self):
        return self.__name

    def log(self):
        return self.__log

    def isPassThrough(self):
        return self.__isPassThrough

    def isCalibration(self):
        return self.__isCalibration

    def isBarrelOnly(self):
        return self.__isBarrelOnly

    def useGeV_v15a(self):
        return self.__useGeV_v15a or self.getThresholdValue() < 5

    def doMonitoring(self):
        return self.__doMonitoring

    def doL2MT(self):
        return self.__doL2MT

    def doOverlapRemoval(self):
        return self.__doOverlapRemoval

    def getThresholdValue(self):
        return self.__threshold

    def getThresholdName(self):
        threshold = str(self.getThresholdValue())
        key = '6GeV_v15a'
        if self.useGeV_v15a():
            key = threshold + 'GeV_v15a'
            if self.getThresholdValue() == 3:
                key = threshold + 'GeV_v22a'
        elif self.isBarrelOnly():
            key = threshold + 'GeV_barrelOnly_v15a'
        elif self.getThresholdValue() >= 20:
            key = threshold + 'GeV_v15a'
        return key

    def setOverlapRemoval(self):
        self.tool().ApplyOR = True
        # cut defintion
        self.tool().RequireDR       = True
        self.tool().RequireMass     = True
        self.tool().RequireSameSign = True
        # BB
        self.tool().DRThresBB       = 0.05
        self.tool().MassThresBB     = 0.20
        # BE
        self.tool().DRThresBE       = 0.05
        self.tool().MassThresBE     = 0.20
        # EE
        self.tool().EtaBinsEC       = [0.0, 1.9, 2.1, 9.9]
        self.tool().DRThresEC       = [0.06, 0.05, 0.05]
        self.tool().MassThresEC     = [0.20, 0.15, 0.10]

    def setL2MT(self):
        self.tool().ApplyOR = True
        # cut defintion
        self.tool().RequireDR       = True
        self.tool().RequireMass     = True
        self.tool().RequireSameSign = True
        # BB
        self.tool().DRThresBB       = 0.05
        self.tool().MassThresBB     = 0.20
        # BE
        self.tool().DRThresBE       = 0.05
        self.tool().MassThresBE     = 0.20
        # EE
        self.tool().EtaBinsEC       = [0.0, 1.9, 2.1, 9.9]
        self.tool().DRThresEC       = [0.06, 0.05, 0.05]
        self.tool().MassThresEC     = [0.20, 0.15, 0.10]

    def compile(self, flags):

        nt = self.multiplicity()
        if self.isCalibration():
            self.tool().AcceptAll = False
            self.tool().DoCalib = True
            self.tool().PtBins = [ [ 0.0, 2.5 ] ] * nt

        elif self.isPassThrough():
            self.tool().AcceptAll = True
            self.tool().PtBins = [ [-10000.,10000.] ]
            self.tool().PtThresholds = [ [ -1. * GeV ] ]
            self.tool().PtThresholdForECWeakBRegionA = [ 3. * GeV ]
            self.tool().PtThresholdForECWeakBRegionB = [ 3. * GeV ]

        else:
            self.log().debug('Set %d thresholds', nt)
            self.tool().AcceptAll = False
            self.tool().PtBins = [ [ 0.0, 2.5 ] ] * nt
            self.tool().PtThresholds = [ [ 5.49 * GeV ] ] * nt
            self.tool().PtThresholdForECWeakBRegionA = [ 3. * GeV ] * nt
            self.tool().PtThresholdForECWeakBRegionB = [ 3. * GeV ] * nt

            thvaluename = self.getThresholdName()

            for th in range(nt):
                try:
                    values = muFastThresholds[thvaluename]
                    self.tool().PtBins[th] = values[0]
                    self.tool().PtThresholds[th] = [ x * GeV for x in values[1] ]
                    self.log().debug('Configration of threshold[%d] %s', th, self.tool().PtThresholds[th])
                    self.log().debug('Configration of PtBins[%d] %s', th, self.tool().PtBins[th])
                    if thvaluename in muFastThresholdsForECWeakBRegion:
                        spThres = muFastThresholdsForECWeakBRegion[thvaluename]
                        self.tool().PtThresholdForECWeakBRegionA[th] = spThres[0] * GeV
                        self.tool().PtThresholdForECWeakBRegionB[th] = spThres[1] * GeV
                    else:
                        self.log().debug('No special thresholds for EC weak Bfield regions for %s. Copy EC1 for region A, EC2 for region B.', thvaluename)
                        spThres = values[0][1]
                        if thvaluename == '2GeV' or thvaluename == '3GeV':
                            self.tool().PtThresholdForECWeakBRegionA[th] = spThres[0] * GeV
                            self.tool().PtThresholdForECWeakBRegionB[th] = spThres[0] * GeV
                        else:
                            self.tool().PtThresholdForECWeakBRegionA[th] = spThres[1] * GeV
                            self.tool().PtThresholdForECWeakBRegionB[th] = spThres[2] * GeV

                        self.log().debug('Thresholds for A[%d]/B[%d] = %d/%d', th, th, self.tool().PtThresholdForECWeakBRegionA[th], self.tool().PtThresholdForECWeakBRegionB[th])

                except LookupError:
                    raise Exception('MuFast Hypo Misconfigured: threshold %r not supported' % thvaluename)

        if self.doL2MT():
            self.setL2MT()
        elif self.doOverlapRemoval():
            self.setOverlapRemoval()

        if self.doMonitoring():
            if self.doOverlapRemoval():
                from TrigMuonHypo.TrigMuonHypoMonitoring import TrigL2MuonOverlapRemoverMonitoringMufast
                self.tool().MonTool = TrigL2MuonOverlapRemoverMonitoringMufast(flags, 'TrigMufastHypoTool/' + self.toolName())
            else:
                from TrigMuonHypo.TrigMuonHypoMonitoring import TrigMufastHypoMonitoring
                self.tool().MonTool = TrigMufastHypoMonitoring(flags, 'TrigMufastHypoTool/' + self.toolName())


def TrigmuCombHypoAlgCfg(flags, name="UNSPECIFIED", **kwargs):
    return CompFactory.TrigmuCombHypoAlg(name, **kwargs)

def TrigmuCombHypoToolFromDict( flags, chainDict ):

    if 'idperf' in chainDict['chainParts'][0]['addInfo'] or 'idtp' in chainDict['chainParts'][0]['addInfo'] :
        thresholds = ['passthrough']
    else:
        thresholds = getThresholdsFromDict( chainDict )

    config = TrigmuCombHypoConfig()

    tight = False # can be probably decoded from some of the proprties of the chain, expert work

    acceptAll = False

    if 'mucombTag' in chainDict['chainParts'][0]['extra']:
        domuCombTag = True
    else:
        domuCombTag = False

    if chainDict['chainParts'][0]['signature'] == 'Bphysics':
        acceptAll = True

    tool = config.ConfigurationHypoTool( chainDict['chainName'], thresholds, tight, acceptAll, domuCombTag )

    if monitorAll:
        tool.MonTool = TrigmuCombHypoMonitoring(flags, "TrigmuCombHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            tool.MonTool = TrigmuCombHypoMonitoring(flags, "TrigmuCombHypoTool/"+chainDict['chainName'])

    d0cut=0.
    if 'd0loose' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0loose']
    elif 'd0medium' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0medium']
    elif 'd0tight' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0tight']
    tool.MinimumD0=d0cut  
    
    return tool


def TrigmuCombHypoToolwORFromDict( flags, chainDict ):

    if 'idperf' in chainDict['chainParts'][0]['addInfo'] or 'idtp' in chainDict['chainParts'][0]['addInfo'] :
       thresholds = ['passthrough']
    else:
       thresholds = getThresholdsFromDict( chainDict )

    config = TrigmuCombHypoConfig()

    tight = False # can be probably decoded from some of the proprties of the chain, expert work

    acceptAll = False
    if 'mucombTag' in chainDict['chainParts'][0]['extra']:
        domuCombTag = True
    else:
        domuCombTag = False

    tool = config.ConfigurationHypoTool( chainDict['chainName'], thresholds, tight, acceptAll, domuCombTag )

    if monitorAll:
        tool.MonTool = TrigL2MuonOverlapRemoverMonitoringMucomb(flags, "TrigmuCombHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            tool.MonTool = TrigL2MuonOverlapRemoverMonitoringMucomb(flags, "TrigmuCombHypoTool/"+chainDict['chainName'])

    # Overlap Removal
    tool.ApplyOR = True
    tool.RequireDR       = True
    tool.RequireMufastDR = True
    tool.RequireMass     = True
    tool.RequireSameSign = True
    tool.EtaBins         = [0, 0.9, 1.1, 1.9, 2.1, 9.9]
    tool.DRThres         = [0.002, 0.001, 0.002, 0.002, 0.002]
    tool.MufastDRThres   = [0.4,   0.4,   0.4,   0.4,   0.4]
    tool.MassThres       = [0.004, 0.002, 0.006, 0.006, 0.006]

    return tool


# muComb Hypo for L2 inside-out
def Trigl2IOHypoToolwORFromDict( chainDict ):

    thresholds = getThresholdsFromDict( chainDict )

    config = TrigmuCombHypoConfig()

    tight = False # can be probably decoded from some of the proprties of the chain, expert work

    acceptAll = False
    domuCombTag=False

    tool=config.ConfigurationHypoTool( chainDict['chainName'], thresholds, tight, acceptAll, domuCombTag )

    # Overlap Removal
    tool.ApplyOR = True
    tool.RequireDR       = True
    tool.RequireMufastDR = False
    tool.RequireMass     = True
    tool.RequireSameSign = True
    tool.EtaBins         = [0, 0.9, 1.1, 1.9, 2.1, 9.9]
    tool.DRThres         = [0.002, 0.001, 0.002, 0.002, 0.002]
    tool.MufastDRThres   = [0]
    tool.MassThres       = [0.004, 0.002, 0.006, 0.006, 0.006]

    return tool


# muComb Hypo for L2 multi-track SA mode
def Trigl2mtCBHypoToolwORFromDict( chainDict ):

    if 'idperf' in chainDict['chainParts'][0]['addInfo'] or 'idtp' in chainDict['chainParts'][0]['addInfo'] :
       thresholds = ['passthrough']
    else:
       thresholds = getThresholdsFromDict( chainDict )

    config = TrigmuCombHypoConfig()

    tight = False # can be probably decoded from some of the proprties of the chain, expert work

    acceptAll = False
    domuCombTag = False

    tool=config.ConfigurationHypoTool( chainDict['chainName'], thresholds, tight, acceptAll, domuCombTag )

    # Overlap Removal
    tool.ApplyOR = True
    tool.RequireDR       = True
    tool.RequireMufastDR = True
    tool.RequireMass     = True
    tool.RequireSameSign = True
    tool.EtaBins         = [0, 0.9, 1.1, 1.9, 2.1, 9.9]
    tool.DRThres         = [0.002, 0.001, 0.002, 0.002, 0.002]
    tool.MufastDRThres   = [0.4,   0.4,   0.4,   0.4,   0.4]
    tool.MassThres       = [0.004, 0.002, 0.006, 0.006, 0.006]

    return tool


class TrigmuCombHypoConfig(object):

    log = logging.getLogger('TrigmuCombHypoConfig')

    def ConfigurationHypoTool( self, thresholdHLT, thresholds, tight, acceptAll, domuCombTag ):

        tool = CompFactory.TrigmuCombHypoTool( thresholdHLT )
        tool.AcceptAll = acceptAll

        nt = len(thresholds)
        log.debug('Set %d thresholds', nt)
        tool.PtBins = [ [ 0.0, 2.5 ] ] * nt
        tool.PtThresholds = [ [ 5.83 * GeV ] ] * nt

        for th, thvalue in enumerate(thresholds):
            if thvalue == 'passthrough':
                tool.AcceptAll = True
                tool.PtBins[th] = [-10000.,10000.]
                tool.PtThresholds[th] = [ -1. * GeV ]
            else:
                # 15.03.2022: original flow, commented to allow use of the new HLT algo thresholds
                #if int(thvalue) >= 24:
                #    thvaluename = '22GeV_v15a'
                #else:
                #    thvaluename = thvalue + 'GeV_v15a'
                if domuCombTag:
                    thvaluename = thvalue + 'GeV_v16'
                else:
                    thvaluename = thvalue + 'GeV_v15a'

                if int(thvalue)==3:
                    thvaluename = thvalue + 'GeV_v22a'
                log.debug('Number of threshold = %d, Value of threshold = %s', th, thvaluename)

                try:
                    values = muCombThresholds[thvaluename]
                    tool.PtBins[th] = values[0]
                    tool.PtThresholds[th] = [ x * GeV for x in values[1] ]
                    if (tight is True):
                        tool.ApplyPikCuts        = True
                        tool.MaxPtToApplyPik      = 25.
                        tool.MaxChi2IDPik         = 3.5
                except LookupError:
                    raise Exception('MuComb Hypo Misconfigured: threshold %r not supported' % thvaluename)

        return tool

def TrigMuonEFHypoAlgCfg(flags, name="UNSPECIFIED", **kwargs):
    return CompFactory.TrigMuonEFHypoAlg(name, **kwargs)


def TrigMuonEFMSonlyHypoToolFromDict( flags, chainDict ) :
    thresholds = getThresholdsFromDict( chainDict )
    kwargs={}
    kwargs.setdefault("RequireSAMuons",True)
    if 'msonly' in chainDict['chainParts'][0]['msonlyInfo'] and 'noL1' not in chainDict['chainParts'][0]['extra']:
        kwargs.setdefault("RemoveOverlaps",True)


    if monitorAll:
        monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFMSonlyHypoTool/"+chainDict['chainName'])
        kwargs.setdefault("MonTool", monTool)
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFMSonlyHypoTool/"+chainDict['chainName'])
            kwargs.setdefault("MonTool", monTool)
    if '3layersEC' in chainDict['chainParts'][0]['addInfo']:
        kwargs.setdefault("RequireThreeStations", True)
        muonflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.SA.Muon')
        if muonflags.Muon.runCommissioningChain:
            kwargs.setdefault("RunCommissioningChain",True)
            

    if 'nscan10' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.1
       narrowscan = True
    elif 'nscan20' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.2
       narrowscan = True
    elif 'nscan30' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.3
       narrowscan = True
    elif 'nscan40' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.4
       narrowscan = True
    elif 'nscan' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.5
       narrowscan = True
    else:
       narrowscan = False
       conesize = 0.0

    kwargs.setdefault("ConeSize", conesize)
    kwargs.setdefault("NarrowScan", narrowscan)
    
    return TrigMuonEFHypoToolCfg( chainDict['chainName'], thresholds, doSA=True, **kwargs )

def TrigMuonEFMSonlyHypoToolFromName( flags, chainDict):
    #For full scan chains, we need to configure the thresholds based on all muons
    #in the chain to get the counting correct. 
    thresholds=[]
    chainName = chainDict["chainName"]
    hltChainName = chainName.rsplit("_L1",1)[0]
    cparts = hltChainName.split("_")

    if 'HLT' in hltChainName:
        cparts.remove('HLT')
    for part in cparts:
        if 'mu' in part:
            thrPart = part.split('mu')
            if not thrPart[0] or thrPart[0] == "0":
                mult = 1
            else:
                mult=thrPart[0]
            thr = thrPart[1]
            if 'noL1' in part:
                thr =thr.replace('noL1','')
            for i in range(1,int(mult)+1):
                thresholds.append(thr)

    monTool=None
    if monitorAll:
        monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFMSonlyHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFMSonlyHypoTool/"+chainDict['chainName'])

    if 'nscan10' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.1
       narrowscan = True
    elif 'nscan20' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.2
       narrowscan = True
    elif 'nscan30' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.3
       narrowscan = True
    elif 'nscan40' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.4
       narrowscan = True
    elif 'nscan' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.5
       narrowscan = True
    else:
       narrowscan = False
       conesize = 0.0


    return TrigMuonEFHypoToolCfg( chainDict['chainName'], thresholds, doSA=True, MonTool=monTool, RequireSAMuons=True, NarrowScan=narrowscan, ConeSize=conesize)

def TrigMuonEFHypoToolCfg(name, thresholds, doSA=False, **kwargs):


    log = logging.getLogger(name)

    nt = len(thresholds)
    log.debug('Set %d thresholds', nt)
    PtBins = [ [ 0.0, 2.5 ] ] * nt
    PtThresholds = [ [ 5.49 * GeV ] ] * nt
    passthrough=False
    for th, thvalue in enumerate(thresholds):
        if (thvalue=='passthrough'):
            passthrough = True
            PtBins[th] = [-10000.,10000.]
            PtThresholds[th] = [ -1. * GeV ]
        else:
            if "0eta105" in name:
                thvaluename = thvalue+ "GeV_barrelOnly"
            else:
                if int(thvalue)==3:
                    thvaluename = thvalue + 'GeV_v22a'
                else:
                    if doSA:
                        thvaluename = thvalue + 'GeV'
                    else:
                        thvaluename = thvalue + 'GeV_v15a'
            log.debug('Number of threshold = %d, Value of threshold = %s', th, thvaluename)

            if doSA:
                values = trigMuonEFSAThresholds[thvaluename]
            else:
                values = efCombinerThresholds[thvaluename]
            PtBins[th] = values[0]
            PtThresholds[th] = [ x * GeV for x in values[1] ]


    kwargs.setdefault("AcceptAll", passthrough)
    kwargs.setdefault("PtBins", PtBins)
    kwargs.setdefault("PtThresholds", PtThresholds)
    return CompFactory.TrigMuonEFHypoTool(name, **kwargs)


def TrigMuonEFCombinerHypoToolFromDict( flags, chainDict ) :
    if 'idperf' in chainDict['chainParts'][0]['addInfo'] or 'idtp' in chainDict['chainParts'][0]['addInfo']:
       thresholds = ['passthrough']
    else:
       thresholds = getThresholdsFromDict( chainDict )

    if 'muonqual' in chainDict['chainParts'][0]['addInfo']:
       muonquality = True
    else:
       muonquality = False

    if 'nscan10' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.1
       narrowscan = True
    elif 'nscan20' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.2
       narrowscan = True
    elif 'nscan30' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.3
       narrowscan = True
    elif 'nscan40' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.4
       narrowscan = True
    elif 'nscan' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.5
       narrowscan = True
    else:
       narrowscan = False
       conesize = 0.0

    if 'noL1' not in chainDict['chainParts'][0]['extra']:
        overlap=True
    else:
        overlap=False

    monTool=None
    if monitorAll:
        monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFCombinerHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFCombinerHypoTool/"+chainDict['chainName'])

    d0cut=0.
    if 'd0loose' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0loose']
    elif 'd0medium' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0medium']
    elif 'd0tight' in chainDict['chainParts'][0]['lrtInfo']:
        d0cut=trigMuonLrtd0Cut['d0tight']


    tool = TrigMuonEFHypoToolCfg(chainDict['chainName'], thresholds, doSA=False, MuonQualityCut=muonquality, MonTool=monTool,
                                 NarrowScan=narrowscan, RemoveOverlaps=overlap, ConeSize=conesize, MinimumD0=d0cut)

    return tool

def TrigMuonEFCombinerHypoToolFromName( flags, chainDict ):
    #For full scan chains, we need to configure the thresholds based on all muons
    #in the chain to get the counting correct. Currently a bit convoluted as
    #the chain dict is (improperly) overwritten when merging for FS chains.
    #Can probably improve this once serial merging is officially implemented
    thresholds=[]
    chainName = chainDict["chainName"]
    hltChainName = chainName.rsplit("_L1",1)[0]
    cparts = hltChainName.split("_")
    if 'HLT' in hltChainName:
        cparts.remove('HLT')
    for part in cparts:
        if 'mu' in part:
            thrPart = part.split('mu')
            if not thrPart[0] or thrPart[0] == "0":
                mult = 1
            else:
                mult=thrPart[0]
            thr = thrPart[1]
            if 'noL1' in part:
                thr =thr.replace('noL1','')
            for i in range(1,int(mult)+1):
                thresholds.append(thr)
    if 'nscan10' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.1
       narrowscan = True
    elif 'nscan20' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.2
       narrowscan = True
    elif 'nscan30' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.3
       narrowscan = True
    elif 'nscan40' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.4
       narrowscan = True
    if 'nscan' in chainDict['chainParts'][0]['addInfo']:
       conesize = 0.5
       narrowscan = True
    else:
       narrowscan = False
       conesize = 0.0

    if 'muonqual' in chainDict['chainParts'][0]['addInfo']:
       muonquality = True
    else:
       muonquality = False

    monTool=None
    if monitorAll:
        monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFCombinerHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            monTool = TrigMuonEFHypoMonitoring(flags, "TrigMuonEFCombinerHypoTool/"+chainDict['chainName'])

    tool = TrigMuonEFHypoToolCfg(chainDict['chainName'], thresholds, doSA=False, MuonQualityCut=muonquality, MonTool=monTool,
                                 NarrowScan=narrowscan, ConeSize=conesize)

    return tool



def TrigMuonEFTrackIsolationHypoToolFromDict( flags, chainDict ) :
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='Muon']
    if 'ivarperf' in chainDict['chainParts'][0]['isoInfo']:
        thresholds = 'passthrough'
    else:
        thresholds = cparts[0]['isoInfo']

    tool = TrigMuonEFTrackIsolationHypoCfg(flags, chainDict['chainName'], thresholds)


    if monitorAll:
        tool.MonTool = TrigMuonEFTrackIsolationMonitoring(flags, 'TrigMuonEFTrackIsolationHypoTool/'+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            tool.MonTool = TrigMuonEFTrackIsolationMonitoring(flags, 'TrigMuonEFTrackIsolationHypoTool/'+chainDict['chainName'])

    return tool

def TrigMuonEFTrackIsolationHypoCfg(flags, name, isoCut, **kwargs) :

        
    if(isoCut=='passthrough') :
        kwargs.setdefault("AcceptAll", True)

    else:
        ptcone03 = trigMuonEFTrkIsoThresholds[ isoCut ]
        
        kwargs.setdefault("PtCone02Cut", 0.0)
        kwargs.setdefault("PtCone03Cut", ptcone03)
        kwargs.setdefault("AcceptAll", False)

    if 'ms' in isoCut:
        kwargs.setdefault("RequireCombinedMuon", False)
        kwargs.setdefault("DoAbsCut", True)
    else:
        kwargs.setdefault("RequireCombinedMuon", True)
        kwargs.setdefault("DoAbsCut", False)


    if 'var' in isoCut :
        kwargs.setdefault("useVarIso", True)
    else :
        kwargs.setdefault("useVarIso", False)

    tool = CompFactory.TrigMuonEFTrackIsolationHypoTool(name, **kwargs)
    return tool

def TrigMuonEFTrackIsolationHypoAlgCfg(flags, name="TrigMuonEFTrackIsolationHypoAlg", **kwargs):
    return CompFactory.TrigMuonEFTrackIsolationHypoAlg(name, **kwargs)

def TrigMuonEFInvMassHypoToolFromDict( flags, chainDict ) :
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='Muon']
    #The invariant mass is specified at end of chain, so only shows up in the last chainPart
    thresholds = cparts[-1]['invMassInfo']
    if "os" in cparts[-1]['addInfo']:
        osCut=True
    else:
        osCut = False

    tool = TrigMuonEFInvMassHypoCfg( flags, chainDict['chainName'], thresholds, SelectOppositeSign=osCut )

    if monitorAll:
        tool.MonTool = TrigMuonEFInvMassHypoMonitoring(flags, "TrigMuonEFInvMassHypoTool/"+chainDict['chainName'])
    else:
        if any(group in muonHypoMonGroups for group in chainDict['monGroups']):
            tool.MonTool = TrigMuonEFInvMassHypoMonitoring(flags, "TrigMuonEFInvMassHypoTool/"+chainDict['chainName'])

    return tool

def TrigMuonEFInvMassHypoCfg(flags, toolName, thresholds, **kwargs):

        massWindow = trigMuonEFInvMassThresholds[thresholds]

        kwargs.setdefault("InvMassLow", massWindow[0])
        kwargs.setdefault("InvMassHigh", massWindow[1])
        kwargs.setdefault("AcceptAll", False)

        tool = CompFactory.TrigMuonEFInvMassHypoTool(toolName, **kwargs)
        return tool

def TrigMuonEFIdtpHypoToolFromDict( flags, chainDict ) :
    thresholds = getThresholdsFromDict( chainDict )
    monTool=None
    if any(group in idHypoMonGroups for group in chainDict['monGroups']):
        monTool = TrigMuonEFIdtpHypoMonitoring(flags, "TrigMuonEFIdtpHypoTool/"+chainDict['chainName'])
    tool = TrigMuonEFIdtpHypoCfg( chainDict['chainName'], thresholds, MonTool=monTool )
    return tool


def TrigMuonEFIdtpHypoCfg(name, thresholds, **kwargs):

    log = logging.getLogger(name)

    nt = len(thresholds)
    log.debug('Set %d thresholds', nt)
    ptBins = [ [ 0, 2.5 ] ] * nt
    ptThresholds = [ [ 5.49 * GeV ] ] * nt
    for th, thvalue in enumerate(thresholds):
        thvaluename = thvalue + 'GeV'
        if int(thvalue)==3:
            thvaluename = thvalue + 'GeV_v22a'
            log.debug('Number of threshold = %d, Value of threshold = %s', th, thvaluename)
        if (thvalue=='passthrough'):
            ptBins[th] = [-10000.,10000.]
            ptThresholds[th] = [ -1. * GeV ]
        else:
            values = trigMuonEFSAThresholds[thvaluename]
            ptBins[th] = values[0]
            ptThresholds[th] = [ x * GeV for x in values[1] ]


    if (thvalue=='passthrough'):
        kwargs.setdefault("AcceptAll", True)
    else:
        kwargs.setdefault("AcceptAll", False)

    kwargs.setdefault("PtBins", ptBins)
    kwargs.setdefault("PtThresholds", ptThresholds)


    return CompFactory.TrigMuonEFIdtpHypoTool( name, **kwargs )

def TrigMuonEFIdtpInvMassHypoToolFromDict(flags, chainDict) :
    cname = chainDict['chainName']
    if 'idZmumu' in cname : 
        thresholds = 'idZmumu'
    elif 'idJpsimumu' in cname :
        thresholds = 'idJpsimumu'
    else :
        log.warning("unknown chain name for IdtpInvmassHypo, chain name= %s, setting threshold of Z mass",cname)
        thresholds = 'idZmumu'

    tool = TrigMuonEFIdtpInvMassHypoCfg( chainDict['chainName'], thresholds )

    if any(group in idHypoMonGroups for group in chainDict['monGroups']):
        tool.MonTool = TrigMuonEFIdtpInvMassHypoMonitoring(flags, "TrigMuonEFIdtpInvMassHypoTool/"+chainDict['chainName'])

    return tool

def TrigMuonEFIdtpInvMassHypoCfg(name, thresholds, **kwargs) :
    
    massWindow = trigMuonEFInvMassThresholds[thresholds]
    kwargs.setdefault("InvMassLow", massWindow[0])
    kwargs.setdefault("InvMassHigh", massWindow[1])
    kwargs.setdefault("AcceptAll", False)

    return CompFactory.TrigMuonEFIdtpInvMassHypoTool(name, **kwargs)


def TrigMuonEFIdtpHypoAlgCfg(flags, name="TrigMuonEFIdtpHypoAlg", **kwargs):
    return CompFactory.TrigMuonEFIdtpHypoAlg(name, **kwargs)

def TrigMuonLateMuRoIHypoAlgCfg(flags, name="TrigMuRoIHypoAlg", **kwargs):
    return CompFactory.TrigMuonLateMuRoIHypoAlg(name, **kwargs)

def TrigMuonLateMuRoIHypoToolFromDict(flags, chainDict ) :
    tool = TrigMuonLateMuRoIHypoCfg(flags, chainDict['chainName'])
    return tool

def TrigMuonLateMuRoIHypoCfg(flags, name="TrigMuRoIHypoTool") :

    tool = CompFactory.TrigMuonLateMuRoIHypoTool(name, AcceptAll=False)
    return tool
