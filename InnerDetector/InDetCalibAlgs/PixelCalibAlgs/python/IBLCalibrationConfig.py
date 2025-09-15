# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import sys

def scanFullName(oname):
    
    if "-" in oname:
        nname = oname.replace("-", "")
        if nname.startswith("S"):
            return f"SCAN_S000{nname[1:]}"

    elif len(oname) == 15 and oname.startswith("SCAN_S000"):
        return oname

    else:
        print(f"ERROR: Scan name '{oname}' is wrong. Should have for example 'SCAN_S000092961' or 'S092-961' format")
        sys.exit(1) 

if __name__=="__main__":
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m PixelCalibAlgs.IBLCalibrationConfig',
                            description="""Calibration tool for IBL.\n\n
                            Example: python -m PixelCalibAlgs.IBLCalibrationConfig --folder "global/path/to/folder/" 
                            --thr "threshold_file" 
                            --totLowQ "totLowCharge_file" 
                            --totHisDis "totHisDisConfig_file" 
                            [--runCal --skipPlots]""")
    
    parser.add_argument('--folder'    , type=str, default="/eos/atlas/atlascerngroupdisk/det-pix/p1/scan-data/", help="Directory path to the files")
    parser.add_argument('--thr'       , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" or \"SXXX-XXX\" - THRESHOLD_SCAN (0Preset_full)")
    parser.add_argument('--totLowQ'   , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" or \"SXXX-XXX\" - TOT_CALIB (0Preset_lowcharge) ")
    parser.add_argument('--totHisDis' , required=True, help="Format must be \"SCAN_SXXXXXXXXX\" or \"SXXX-XXX\" - TOT_CALIB (0Preset_lowcharge_HisDisConfig)")
    parser.add_argument('--tag'       , type=str, default="PixelChargeCalibration-DATA-RUN2-UPD4-28", help="Tag in order to read the DB")
    parser.add_argument('--runCal'    , action='store_true', help="Runs only the IBL Calibration layer")
    parser.add_argument('--skipPlots' , action='store_true', help="Skips the plotting step - Slower the running time")
    
    args = parser.parse_args()

    args.thr       = scanFullName(args.thr)
    args.totHisDis = scanFullName(args.totHisDis)
    args.totLowQ   = scanFullName(args.totLowQ)

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
    