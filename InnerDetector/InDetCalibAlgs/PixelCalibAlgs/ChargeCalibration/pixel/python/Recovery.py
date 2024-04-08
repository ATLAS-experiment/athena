# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import json

# Reading from the generated file in MakeReferenceFile
# It converts the DB payload into a python dictionary using json
def ReadDbFile(name):
    mydict = dict()
    iov = "non"
    with open(name) as fp:
        lines = fp.readlines()
        for line in lines:
            if(line.startswith('[\'')):
                # Removes ending
                line = line.rstrip(']\n')
                # Split to get just the dictionary in the second position
                arrayline = line.split(' : ')
                mydict = json.loads(arrayline[1])
            else:
                iov = line[:line.find(" - ")]
                print("Data base IOV: %s" % line)
    
    return mydict, iov


def ReadNewCalib(name):
    mydict = dict()
    report_dict = { "Blayer":0 , "L1":0 , "L2":0 , "Disk":0, "unexp.":0}
    with open(name) as fp:
        lines = fp.readlines()
        mod = ""
        for line in lines:
            if line.startswith("L") or line.startswith("D"):
                
                if line.startswith("L0"): report_dict["Blayer"] += 1 
                elif line.startswith("L1"): report_dict["L1"] += 1 
                elif line.startswith("L2"): report_dict["L2"] += 1 
                elif line.startswith("D"): report_dict["Disk"] += 1 
                else:
                    print(line)
                    report_dict["unexp."] += 1 
                
                splitline = line.rstrip("\n").split(" ")
                if len(splitline) == 2:
                    mydict[splitline[1]] = []
                    mod = splitline[1]
                else:
                    print("ReadNewCalib - ERROR Length of the module is not the correct one. Expected ['module_name', 'hash_ID']. Value:", splitline)
            elif line.startswith("I"):
                splitline = line.rstrip("\n").split(" ")
                splitline.pop(0)
                float_splitline = [float(string) for string in splitline]
                mydict[mod].append(float_splitline)
    
    return mydict, report_dict

def ReadOldCalib(name):
    mydict = dict()
    with open(name) as fp:
        lines = fp.readlines()
        mod = ""
        for line in lines:
            if line.startswith("L") or line.startswith("D"):
                
                splitline = line.rstrip("\n").split(" : ")
                splitline = list(filter(None,splitline[1].split(" ")))
                if len(splitline) == 2:
                    mydict[splitline[0]] = []
                    mod = splitline[0]
                else:
                    print("ReadNewCalib - ERROR Length of the module is not the correct one. Expected ['module_name', 'hash_ID']. Value:", splitline)
            elif line.startswith("I"):
                splitline = line.rstrip("\n").split(" ")
                splitline.pop(0)
                float_splitline = [float(string) for string in splitline]
                mydict[mod].append(float_splitline)
    return mydict
            
def avg(a, b):
  return 1/2*(a + b)    


# This function recovers the thresholds for the normal, long and ganged pixels
def recover_thr(thr1, thr2, listarray, ref_val):
    recovered_value =  avg(thr1, thr2)
    
    if recovered_value == 0:
        if not listarray:
            recovered_value = ref_val
        else:
            recovered_value = sum(listarray)/len(listarray)
    return recovered_value

# This function recovers other than the thresholds
def recover(listarray, ref_val):
    
    if ref_val == 0:
        if not listarray:
            return ref_val
        else:
            return sum(listarray)/len(listarray)
    
    return ref_val 

def recover_empties(new_calib, ref_calib):
    
    report_dict = dict()
    report_counter_dict = {"empty" : 0 , "all_zeros":0}
    
    for module, frontEnds in new_calib.items():
        
        nFE = len(frontEnds)
        # Checking FE with data loss - All empty
        for itr_fe in range(nFE):
            
            failures = frontEnds[itr_fe].count(0) + frontEnds[itr_fe].count(-28284.3)

            if failures != 0 :
                if module not in report_dict:
                    report_dict[module] = []
                    
                # Recovering O but not empty FE
                if failures != len(frontEnds[itr_fe]):
                    report_counter_dict["empty"] += 1
                    report_dict[module].append("    FE%02d with %i zeros. Positions:" % (itr_fe,failures))
                    
                    # Running over values and replace them with the previous calibration
                    for itr_par in range(len(frontEnds[itr_fe])):
                        if( frontEnds[itr_fe][itr_par] == 0 or frontEnds[itr_fe][itr_par] == -28284.3):
                            
                            report_dict[module][-1] = report_dict[module][-1] + (" %s,"%(str(itr_par)))
                            
                            # List of the same parameter from the other FE of the same module, it will be used in order to get the average of the module frontEnds
                            val_list = [new_calib[module][i][itr_par] for i in range(nFE) if new_calib[module][i][itr_par] != 0  ]
                            val_list_fit = [new_calib[module][i][itr_par] for i in range(nFE) if new_calib[module][i][itr_par] != 0 and new_calib[module][i][itr_par] != -28284.3 ]
                            
                            # Threshold for normal pixels
                            if itr_par == 0:
                                new_calib[module][itr_fe][itr_par] = recover_thr(frontEnds[itr_fe][4], frontEnds[itr_fe][8], val_list, ref_calib[module][itr_fe][itr_par] )
                                
                            # Threshold for long pixels
                            elif itr_par == 4:
                                new_calib[module][itr_fe][itr_par] = recover_thr(frontEnds[itr_fe][0], frontEnds[itr_fe][8], val_list, ref_calib[module][itr_fe][itr_par]) 
                                
                            # Threshold for ganged pixels
                            elif itr_par == 8:
                                new_calib[module][itr_fe][itr_par] = recover_thr(frontEnds[itr_fe][0], frontEnds[itr_fe][4], val_list, ref_calib[module][itr_fe][itr_par]) 
                                
                            # rest of parameters
                            else:
                                # itr_par 13 or 16 are the fit parameters for the denominator. Bad values are 0 or -28284.3 so we need to exclude them from the list
                                if itr_par == 13 or itr_par == 16:
                                    new_calib[module][itr_fe][itr_par] = recover(val_list_fit, ref_calib[module][itr_fe][itr_par])
                                else:
                                    new_calib[module][itr_fe][itr_par] = recover(val_list, ref_calib[module][itr_fe][itr_par])
                            
                # Recovering empty FE    
                else :
                    report_counter_dict["all_zeros"] += 1
                    report_dict[module].append("    FE%02d full copy" % itr_fe)
                    new_calib[module][itr_fe] = ref_calib[module][itr_fe]
    
    return report_dict, report_counter_dict

def UpdateAndSave(new_calib, ref_calib):
    
    # Making a copy of the reference calibration
    updated_calib = ref_calib.copy()
    
    # Updating the value with the new calibration
    updated_calib.update(new_calib)
    
    total_fe = 0
    for key, values in updated_calib.items():
        total_fe += len(values)
        
    
    print("%-45s: %6i" % ("Total modules in the calibration candidate",len(updated_calib)))
    print("%-45s: %6i\n" % ("Total FE in the calibration candidate",total_fe))

    
    
    # Commented out since it could be used for comparison - Experts only
    # Fprint(new_calib    , "final_new_calib.txt")
    # Fprint(ref_calib    , "final_ref_calib.txt")
    Fprint(updated_calib, "FINAL_calibration_candidate.txt")

    return updated_calib

def Fprint(dict, name):
    f = open(name, "w")
    for key, values in dict.items():
        
        for value in values:
            f.write("%s %i %i %i %i %i %i %i %i %i %i %i %i %.6g %.6g %.6g %.6g %.6g %.6g %.6g %.6g\n" % (
                key, 
                value[ 0], value[ 1], value[ 2], value[ 3],
                value[ 4], value[ 5], value[ 6], value[ 7],
                value[ 8], value[ 9], value[10], value[11],
                value[12], value[13], value[14],
                value[15], value[16], value[17],
                value[18], value[19]
            ))

def UpdateCalib(tag):
    ref_calib, iov = ReadDbFile(tag+".log")
    new_calib, read_report = ReadNewCalib("calibration_merged.txt")
    
    # modifying the new_calib dictionary in order to recover the empty FE 
    report, counter_report = recover_empties(new_calib,ref_calib)
    
    UpdateAndSave(new_calib,ref_calib)
    
    f = open("log_recovery.txt", "w")
    str = "-------- SUMMARY OF PIXEL CALIB RECOVERY --------\n"
    str += "Modules Calibrated:\n"
    
    for key,value in read_report.items():
        str += "%10s read: %4i\n" % (key, value)
    str += "\n"
    
    str += "Error counters:\n"
    str += "   %15s: %5i\n" % ("Empty values", counter_report["empty"])
    str += "   %15s: %5i\n" % ("Full FE copied", counter_report["all_zeros"])
    
    print(str)
    print("More information in: log_recovery.txt")
    print("NEW CALIBRATION file to update the DB: FINAL_calibration_candidate.txt")
    
    str += """Positions of single 0's:
    0: normal_threshold,  1: normal_RMS,  2: normal_noise,  3: normal_intime
    4: long_threshold  ,  5: long_RMS  ,  6: long_noise  ,  7: long_intime  
    8: ganged_threshold,  9: ganged_RMS, 10: ganged_noise, 11: ganged_intime
    12: 13: 14: Fitting for normal pixels
    15: 16: 17: Fitting for long and ganged pixels
    18: 19: quality and smearing (used for MC)\n\n"""
            
    str += "Modules with empties:\n"
    for key,values in report.items():
        str += "HashID: %4s\n" % (key)
        for value in values:
            str += value + "\n"
    f.write(str)




# Just used for testing - Experts only                
if __name__ == "__main__":
    
    UpdateCalib("PixelChargeCalibration-DATA-RUN2-UPD4-26")
    
    exit(0)    


