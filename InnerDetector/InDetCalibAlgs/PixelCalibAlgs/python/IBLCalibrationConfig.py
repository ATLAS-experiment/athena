# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import sys

if __name__=="__main__":
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.IBLCalibrationConfig.',
                            description="""Calibration tool for IBL.\n\n
                            Example: python -m PixelCalibAlgs.IBLCalibrationConfig --folder "global/path/to/folder/" --thr "threshold_file" --totLowQ "totLowCharge_file" 
                                                                                   --totHisDis "totHisDisConfig_file" [--runCal --skipPlots]""")
    
    parser.add_argument('--folder'    , required=True, help="Directory path to the files")
    parser.add_argument('--thr'       , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" - THRESHOLD_SCAN (0Preset_full)")
    parser.add_argument('--totLowQ'   , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" - TOT_CALIB (0Preset_lowcharge) ")
    parser.add_argument('--totHisDis' , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" - TOT_CALIB (0Preset_lowcharge_HisDisConfig)")
    parser.add_argument('--tag'       , type=str, default="PixelChargeCalibration-DATA-RUN2-UPD4-28", help="Tag in order to read the DB")
    parser.add_argument('--runCal'    , action='store_true', help="Runs only the IBL Calibration layer")
    parser.add_argument('--skipPlots' , action='store_true', help="Skips the plotting step - Slower the running time")
    
    args = parser.parse_args()

    try:
        print("Running IBLCalibration layers..")
        command = 'IBLCalibration directory_path=' + args.folder + ' THR=' + args.thr + ' TOT_HISDIS=' + args.totHisDis + ' TOT_LOWQ=' + args.totLowQ
        print("Command: %s\n" % command)
        import subprocess
        stdout, stderr = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
        print("OUTPUT: \n%s" % (stdout.decode('ascii')))
        print("ERRORS: %s" % ("NONE" if stderr.decode('ascii')=='' else "\n"+stderr.decode('ascii')))
        if stderr:
            exit(1)
            
    except OSError as e:
        print("IBLCalibration.cxx execution CRASHED - Please check\n",e)
        sys.exit(e.errno) 
    

    
    # Use --runCal = True if you plan to run just the calibration
    if args.runCal:
        print("Jobs finished")
        exit(0)
    
    print("Creating Reference file..")
    # Downloads the last IOV
    command = 'MakeReferenceFile tagName=%s' % (args.tag)
    print("Command: %s\n" % command)
    (subprocess.Popen(command, shell=True)).communicate()
    print("Done\n")
    
    if args.skipPlots:
        print("Jobs finished")
        exit(0)

    from PixelCalibAlgs.CheckValues import CheckThresholdsIBL
    CheckThresholdsIBL("ChargeCalib_ToTbin1_FrtEnd2_"+args.totHisDis+".TXT")
    
    print("Validation new vs. previous calibration.")
    # Plots the old vs. new charge for all FE (includes IBL)
    from PixelCalibAlgs.EvoMonitoring import setupRunEvo
    setupRunEvo("ChargeCalib_ToTbin1_FrtEnd2_"+args.totHisDis+".TXT", args.tag+".log" )
    print("Done\n")
    
    
    print("Jobs finished")
    exit(0)
    