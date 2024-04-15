# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.PixelCalibrationConfig.',
                            description="""Calibration tool for pixel.\n\n
                            Example: python -m PixelCalibAlgs.PixelCalibrationConfig --folder "global/path/to/folder/" --thr "threshold_file" --thr_intime "intime_file" 
                                                                                     --tot "tot_file --layers [Blayer, L1, L2, disk] [--saveInfo --runCal --skipPlots]""")
    
    parser.add_argument('--folder'    , required=True, help="Directory path to the files")
    parser.add_argument('--thr'       , required=True, help="Threshold file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--thr_intime', required=True, help="Threshold intime file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--tot'       , required=True, help="Time over threshold file, format must be \"SCAN_SXXXXXXXXX\" ")
    parser.add_argument('--layers'    , required=True, nargs='+', choices={"Blayer","L1","L2","disk"}, help="What layers we should run to update the calibration.")
    parser.add_argument('--saveInfo'  , action='store_true', help="Creates a root file with the fitting plots - Slower running time")
    parser.add_argument('--runCal'    , action='store_true', help="Runs only the Pixel Calibration layers")
    parser.add_argument('--skipPlots' , action='store_true', help="Skips the plotting step. Takes less time")
    parser.add_argument('--tag'       , type=str, default="PixelChargeCalibration-DATA-RUN2-UPD4-26", help="Tag in order to read the DB")
    
    
    args = parser.parse_args()
    
    import subprocess
    proc = []
    
    print("Running PixelCalibration layers..")
    # Executing layers
    for layer in args.layers :
        extraArgs = "saveInfo" if args.saveInfo else ""
        command = 'PixelCalibration directory_path=' + args.folder + ' THR=' + args.thr + ' THRintime=' + args.thr_intime + ' TOT=' + args.tot + ' ' + layer + ' ' + extraArgs +' > log_' + layer
        print("Command: %s\n" % command)
        proc.append(subprocess.Popen(command, shell=True))
    
    # Waiting to get the processes finished
    for l in range(len(args.layers)) :
        proc[l].communicate()
    print("Done\n")
    
    # This is meant to run just one layer from a different calibration path and other layer from other path.
    # We need to merge after the output of PixelCalibration. Use --runCal = True if you plan to test or run other layer afterwards
    if args.runCal:
        print("Jobs finished")
        exit(0)
    
    print("Merging calibration output...")
    from PixelCalibAlgs.FileMerger import MergeCalibFiles
    # Sending an array of all the layer - the ones not present will be skipped and reported
    MergeCalibFiles(["Blayer", "L1", "L2", "disk"])
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

    if args.skipPlots:
        print("Jobs finished")
        exit(0)
    
    print("Validation new vs. previous calibration.")
    # Plots the old vs. new charge for all FE (includes IBL)
    # Meant to be used for pixel, but can be used for IBL in standalone (reading central DB using MakeReferenceFile.cxx step)
    from PixelCalibAlgs.EvoMonitoring import setupRunEvo
    setupRunEvo("FINAL_calibration_candidate.txt", args.tag+".log" )
    print("Done\n")
    
    
    print("Jobs finished")
    exit(0)
    