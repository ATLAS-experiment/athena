#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

if __name__=="__main__":
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.PixelCalibrationConfig.',
                            description="""Calibration tool for pixel.\n\n
                            Example: python -m PixelCalibAlgs.PixelCalibrationConfig --folder "global/path/to/folder/" --thr "threshold_file" --thr_intime "intime_file" 
                                                                                     --tot "tot_file --layers [Blayer, L1, L2, disk] [--saveInfo]""")
    
    parser.add_argument('--folder'    , required=True, help="Directory path to the files")
    parser.add_argument('--thr'       , required=True, help="Threshold file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--thr_intime', required=True, help="Threshold intime file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--tot'       , required=True, help="Time over threshold file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--layers'    , required=True, nargs='+', choices={"Blayer","L1","L2","disk"}, help="What layers we should run to update the calibration.")
    parser.add_argument('--saveInfo'  , type=bool, default=False, help="Creates a root file with the fitting plots - Slower running time")
    
    args = parser.parse_args()
        
    import subprocess
    proc = []
    for layer in args.layers :
        command = 'PixelCalibration directory_path=' + args.folder + ' THR=' + args.thr + ' THRintime=' + args.thr_intime + ' TOT=' + args.tot + ' ' + layer + ' > log_' + layer
        print("%s\n" % command)
        proc.append(subprocess.Popen(command, shell=True))

    for l in range(len(args.layers)) :
        proc[l].communicate()


    # Work in progress... 
    # After all the processes are finished, we need to download the last IOV, modify the calibrated modules and upload
    # do we know if we need xchecks? if so the they can be implented here as well before the upload
    
    print("running Code")
    exit(0)
    