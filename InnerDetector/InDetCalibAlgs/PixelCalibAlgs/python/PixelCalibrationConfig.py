# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
    parser.add_argument('--tag'       , type=str, default="PixelChargeCalibration-DATA-RUN2-UPD4-26", help="Tag in order to read the DB")
    
    args = parser.parse_args()
    
    import subprocess
    proc = []
    
    print("Running PixelCalibration layers..")
    # Executing layers
    for layer in args.layers :
        command = 'PixelCalibration directory_path=' + args.folder + ' THR=' + args.thr + ' THRintime=' + args.thr_intime + ' TOT=' + args.tot + ' ' + layer + ' > log_' + layer
        print("Command: %s\n" % command)
        proc.append(subprocess.Popen(command, shell=True))
    
    # Waiting to get the processes finished
    for l in range(len(args.layers)) :
        proc[l].communicate()
    print("Done\n")
    
    print("Merging calibration output...")
    from PixelCalibAlgs.FileMerger import MergeCalibFiles
    MergeCalibFiles(args.layers)
    print("Done\n")
    
    print("Creating Reference file..")
    # Downloads the last IOV
    command = 'MakeReferenceFile %s' % (args.tag)
    print("Command: %s\n" % command)
    (subprocess.Popen(command, shell=True)).communicate()
    print("Done\n")
    
    print("Updating last IOV and creating calibration candidate file..")
    # Updates last IOV with the new calibration
    from PixelCalibAlgs.Recovery import UpdateCalib
    UpdateCalib(args.tag)
    print("Done\n")
    
    
    # Work in progress... 
    # Check the calibrated modules (replace the bad ones with the previous calibration) and upload
    
    print("Jobs finished")
    exit(0)
    