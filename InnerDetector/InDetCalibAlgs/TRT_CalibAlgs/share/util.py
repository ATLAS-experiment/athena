###############################################
############## SOME FUNCTIONS #################
###############################################
import json, glob, fnmatch, os, random, sys, scriptgen
from time import gmtime, strftime, mktime, time


import json

# Log of DAGMan debugging messages                 : dag.dag.dagman.out
# Log of HTCondor library output                     : dag.dag.lib.out
# Log of HTCondor library error messages             : dag.dag.lib.err
# Log of the life of condor_dagman itself          : dag.dag.dagman.log
# dag status:
# 0: OK
# 1: error; an error condition different than those listed here
# 2: one or more nodes in the DAG have failed
# 3: the DAG has been aborted by an ABORT-DAG-ON specification
# 4: removed; the DAG has been removed by condor_rm
# 5: a cycle was found in the DAG
# 6: the DAG has been halted
def make_status_mail(oiter):
    with open("iteration" + str(int(oiter)) + ".dag.metrics", "r") as metrics:
        dic = json.load(metrics)

    duration = dic["duration"]

    hours = duration // (60 * 60)
    minutes = (duration - hours * 60 * 60) // 60
    seconds = duration - (hours * 60 * 60) - (minutes * 60)
    time = str(int(hours)) + "h " + str(int(minutes)) + "m " + str(int(seconds)) + "s"

    msg = (
        "This is an automated email from running manual TRT data calibration.\n\nStatus of iteration {}:\n".format(
            oiter
        )
        + "Job graph with id {} terminated after {} with exit code {}.\n".format(
            dic["dagman_id"], time, dic["exitcode"]
        )
        + "Out of {} jobs submitted, {} were ran, of which {} failed and {} succeeded.".format(
            dic["jobs"],
            dic["total_jobs_run"],
            dic["jobs_failed"],
            dic["jobs_succeeded"],
        )
    )
    with open("mail.txt", "w+") as file:
        file.write(msg)


def extractDDV2(config, sflist):
    print("")
    print("#####################################################################################################################################")
    print("##           ")
    print("##           Extracting detector description version.....")
    print("##           Extracting conditions tags.....")
    print("##           Extracting field setup.....")
    print("##           This can take a while, please wait..................")
    print("##           ")
    user = os.popen("echo $USER").readlines()
    testing = "/tmp/" + user[0].split("\n")[0] + "/DumpSetting.sh"
    outf = open(testing, "w")
    outf.write("#!/bin/sh\n")
    ostring = "# Script to dump some parameters\n"
    ostring += "cd /tmp/$USER/ \n"
    ostring += "export STAGE_SVCCLASS=atlcal\n"
    ostring += config["SetupCmdRec"] + "\n"
    dumfile = (sflist[0])[1].split("castor:")[1]
    print("##           %s" % dumfile)
    jO = (
        """
    inFile = '%s'"""
        % dumfile
    )
    jO += """
    inputFileSummary = {}
    import PyUtils.AthFile as athFile
    fi = athFile.fopen(inFile)

    inputFileSummary = fi.fileinfos
    runNumber=inputFileSummary['run_number'][0]
    geometry=inputFileSummary['geometry']
    condtag=inputFileSummary['conditions_tag']
    taginfo=inputFileSummary["tag_info"]
    release=taginfo['AtlasRelease']
    # --- Autosetup of magnetic field
    from AthenaCommon.BFieldFlags import jobproperties
    from CoolConvUtilities.MagFieldUtils import getFieldForRun
    cool=getFieldForRun(runNumber)
    jobproperties.BField.barrelToroidOn=False
    jobproperties.BField.endcapToroidOn=False
    jobproperties.BField.solenoidOn=False
    if cool.toroidCurrent()>100.:
            jobproperties.BField.barrelToroidOn.set_Value_and_Lock(True)
            jobproperties.BField.endcapToroidOn.set_Value_and_Lock(True)
    if cool.solenoidCurrent()>100.:
            jobproperties.BField.solenoidOn.set_Value_and_Lock(True)



    print("=================== AUTO CONFIGURED B FIELD AND DETECTOR GEOMETRY: =======================")
    print("===     Release                  : ", release)
    print("===     RunNumber                : ", runNumber)
    print("===     DetGeometry              : ", geometry)
    print("===     Conditionstags           : ", condtag)
    print("===     BField.barrelToroidOn    : ",jobproperties.BField.barrelToroidOn())
    print("===     BField.endcapToroidOn    : ",jobproperties.BField.endcapToroidOn())
    print("===     BField.solenoidOn        : ",jobproperties.BField.solenoidOn())
    print("==========================================================================================")

    theApp.EvtMax = 1
    """
    ostring += "cat > joboptions.py  <<EOF\n"
    ostring += jO + "\nEOF\n"
    ostring += "athena joboptions.py > /tmp/$USER/logDUMP\n"
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 /tmp/$USER/DumpSetting.sh")
    os.system("/tmp/$USER/DumpSetting.sh")

    release = os.popen(
        " less /tmp/$USER/logDUMP | grep Release | awk '{print $4}'"
    ).readlines()
    runnumber = os.popen(
        " less /tmp/$USER/logDUMP | grep RunNumber | awk '{print $4}'"
    ).readlines()
    detgeometry = os.popen(
        " less /tmp/$USER/logDUMP | grep DetGeometry | awk '{print $4}'"
    ).readlines()
    conditionstag1 = os.popen(
        " less /tmp/$USER/logDUMP | grep Conditionstags | awk '{print $4}'"
    ).readlines()
    conditionstag = conditionstag1[0].split(",")[0]
    barreltoroidon = os.popen(
        " less /tmp/$USER/logDUMP | grep BField.barrelToroidOn | awk '{print $4}'"
    ).readlines()
    endcaptoroidon = os.popen(
        " less /tmp/$USER/logDUMP | grep BField.endcapToroidOn | awk '{print $4}'"
    ).readlines()
    solenoidon = os.popen(
        " less /tmp/$USER/logDUMP | grep BField.endcapToroidOn | awk '{print $4}'"
    ).readlines()

    release1 = release[0].split("-")
    release2 = release1[1].split("\n")[0] + "," + release1[0] + ",32,runtime"

    print("## ")
    print("##		The run number seen is:         %s" % runnumber[0].split("\n")[0])
    print("## ")
    print("##           The Release Used in the reco was: %s" % release2)
    print("##                   the one in the configuration file is: %s" % config[ "ReleaseRec" ])
    print("## ")
    print("##		The Detector geometry  seen is: %s" % detgeometry[0].split("\n")[0])
    print("##			the one in the configuration file is: %s" % config["DetDescVer"])
    print("## ")
    print("##		The Conditions Tag seen is:     %s" % conditionstag.split("\n")[0])
    print("##			the one in the configuration file is: %s" % config["GLtag"])
    print("## ")
    print("##		The toroid for barrel is set:   %s" % barreltoroidon[0].split("\n")[0])
    print("##			the one in the configuration file is: %s" % config["barrelToroid"])
    print("## ")
    print("##		The toroid for endcap is set:   %s" % endcaptoroidon[0].split("\n")[0])
    print("##			the one in the configuration file is: %s" % config["endcapToroid"])
    print("## ")
    print("##		The solenoid is set:            %s" % solenoidon[0].split("\n")[0])
    print("##			the one in the configuration file is: %s" % config["Solenoid"])
    print("##      ")
    print("##		Would you like to use settings read from file (recommended), say [y]")
    print("##           	in case you want to use the settings in config file say [n]")
    print("##      ")
    inp = "empty"
    while (inp != "y") & (inp != "n") & (inp != "yes") & (inp != "no"):
        inp = raw_input("##           Enter yes [y] or no [n] ")
    print("##      ")
    print("##      ")
    print("##      ")
    print("#####################################################################################################################################")
    if (inp == "y") | (inp == "yes"):
        print("##           Contants will be changed")
        config["ReleaseRec"] = release2
        config["DetDescVer"] = detgeometry[0].split("\n")[0]
        config["GLtag"] = conditionstag.split("\n")[0]
        config["barrelToroid"] = barreltoroidon[0].split("\n")[0]
        config["endcapToroid"] = endcaptoroidon[0].split("\n")[0]
        config["Solenoid"] = solenoidon[0].split("\n")[0]

    return 0


def extractDDV(config, sflist):
    print()
    print("Extracting detector description version (can take a few seconds) ...\n")
    dumfile = (sflist[0])[1].split("castor:")[1]
    os.system("rfcp %s dumfile" % dumfile)
    vertag = os.popen(
        "%s; python %s/dumpVersion.py -f dumfile | grep GeoAtlas | grep ATLAS | awk '{print $2}'"
        % (config["SetupCmdCal"], config["Calibdir"])
    ).readlines()
    glotag = os.popen(
        "%s; python %s/dumpVersion.py -f dumfile | grep IOVDbGlobalTag | awk '{print $2}'"
        % (config["SetupCmdCal"], config["Calibdir"])
    ).readlines()
    atlrel = os.popen(
        "%s; python %s/dumpVersion.py -f dumfile | grep AtlasRelease | awk '{print $2}'"
        % (config["SetupCmdCal"], config["Calibdir"])
    ).readlines()
    print("version is " + vertag[0].split("\n")[0])
    print("global tag is " + glotag[0].split("\n")[0])
    print("ATLAS release is " + atlrel[0].split("\n")[0])
    os.system("rm -f dumfile")

    return vertag[0].split("\n")[0]


def it_statusmail(config, oiter):

    ostring = "python mail.py {}\n".format(oiter)
    ostring += "  cat mail.txt | mail -s 'Manual TRT Calibration -- iteration {} exit status' {} >> mail.txt\n".format(
        oiter, config["UserMail"]
    )

    return ostring


def extract_const(type, file, tempfile):
    fi = open(file, "r")
    fo = open(tempfile, "w")
    if type == "t0":
        ndat = 3
    if type == "rt":
        ndat = 6
    ostring = ""
    lines = fi.readlines()
    i = 0
    for line in lines:
        values = line.split(":")
        if len(values) > 1:
            # print line
            # print values[1].split(' ')
            if len(values[1].split(" ")) == ndat:
                i = i + 1
                ostring += line
    fo.write(ostring)
    print("extracted %i %s-constants to file: %s" % (i, type, tempfile))
    fi.close()
    fo.close()


def analyzeJob(iteration, stepid):
    files = os.popen("ls -tr trtcal_%02i_%s*.sh.o" % (iteration, stepid)).readlines()
    rfiles = os.popen("ls -tr trtcal_%02i_%s*.sh" % (iteration, stepid)).readlines()

    calctime = True
    njobstot = len(rfiles)

    njobs = 0
    for i, file in enumerate(files):
        status = "UNKNOWN"
        fbase = file.split(".sh")[0]
        finfo = (
            os.popen("ls -al --time-style=long-iso %s.sh.o" % fbase).readline()
        ).split()
        rfinfo = (
            os.popen("ls -al --time-style=long-iso %s.sh" % fbase).readline()
        ).split()
        contents = os.popen("cat %s.sh.o" % fbase).readlines()
        for content in contents:
            if content.find("CPU time") >= 0:
                cputime = (content.split("\n")[0]).split()[3]
            if content.find("Exited with exit code") >= 0:
                status = "ERROR"
            if content.find("Successfully completed") >= 0:
                status = "OK"

        if i == 0:
            year0 = int(finfo[5].split("-")[0])
            month0 = int(finfo[5].split("-")[1])
            day0 = int(finfo[5].split("-")[2])
            hour0 = int(finfo[6].split(":")[0])
            min0 = int(finfo[6].split(":")[1])
            ttup0 = (year0, month0, day0, hour0, min0, 0, 0, 0, 0)
            tm0 = mktime(ttup0)
        year = int(finfo[5].split("-")[0])
        month = int(finfo[5].split("-")[1])
        day = int(finfo[5].split("-")[2])
        hour = int(finfo[6].split(":")[0])
        min = int(finfo[6].split(":")[1])
        ttup = (year, month, day, hour, min, 0, 0, 0, 0)
        tm = mktime(ttup)

        ryear = int(rfinfo[5].split("-")[0])
        rmonth = int(rfinfo[5].split("-")[1])
        rday = int(rfinfo[5].split("-")[2])
        rhour = int(rfinfo[6].split(":")[0])
        rmin = int(rfinfo[6].split(":")[1])
        rttup = (ryear, rmonth, rday, rhour, rmin, 0, 0, 0, 0)
        rtm = mktime(rttup)

        statline = ""

        njobs = njobs + 1
        statline = (
            "%3i %s %s  %-20s:  status = %-8s, CPU time = %-12s, realtime = %s"
            % (
                i + 1,
                finfo[5],
                finfo[6],
                fbase,
                status,
                "%3.1f min" % (float(cputime) / 60),
                "%3.0f min" % ((tm - rtm) / 60),
            )
        )
        if tm < rtm:
            statline += "   ... old"
        print(statline)

    return njobs, njobstot, statline


def build_filelist(inputdir, pattern, minlumi, maxlumi):
    flist = []
    sflist = []
    fdict = {}
    lumidict = {}

    print(inputdir)
    index = -1
    for idir, dir in enumerate(inputdir):
        if dir.find("castor:") >= 0:
            index = index + 1
            min = int(minlumi[index])
            max = int(maxlumi[index])
            if min > max:
                print("#################                   ERROR on LUMIBLOCK DEF!!!           ##############################")
                sys.exit(0)
            if min == max:
                min = -10
                max = 100000000000000000

            lines = os.popen("rfdir " + dir.split(":")[1]).readlines()
            for i in lines:
                tokens = i.split()
                fsize = tokens[4]
                if int(tokens[4]) >= 502573:
                    if fnmatch.fnmatch(tokens[8], pattern[idir]):
                        fname = dir + "/" + tokens[8]
                        tokens2 = tokens[8].split("_")
                        tokens3 = tokens2[3].split("lb")
                        tokens3 = tokens3[1].split(".")
                        lumi = int(tokens3[0])
                        if (lumi >= min) & (lumi <= max):
                            lumidict[lumi] = lumi
                            # print(lumi)
                            # stagestat=os.popen('stager_qry -M %s'%fname.strip('castor:')).readlines()[1]
                            # if stagestat.find('STAGED')>=0:
                            # print('USING STAGED FILE ' + fname.strip('castor:'))
                            flist.append(fname)
                            sflist.append((int(fsize), fname))
                            fdict[int(fsize)] = fname
        if dir.find("eosatlas:") >= 0:
            index = index + 1
            min = int(minlumi[index])
            max = int(maxlumi[index])
            if min > max:
                print("#################                   ERROR on LUMIBLOCK DEF!!!           ##############################")
                sys.exit(0)
            if min == max:
                min = -10
                max = 100000000000000000
            print(dir)
            print(dir.split(":")[1])
            print("eos ", dir.split(":")[1])
            lines = os.popen(
                # "/afs/cern.ch/project/eos/installation/0.3.15/bin/eos.select " ## causing errors -- has been phased out
                "eos ls -l "
                + dir.split(":")[1]
            ).readlines()
            for i in lines:
                tokens = i.split()
                print(tokens)
                fsize = tokens[4]
                if int(tokens[4]) >= 502573:
                    if fnmatch.fnmatch(tokens[8], pattern[idir]):
                        fname = dir
                        if tokens[8] not in fname:
                            fname += tokens[
                                8
                            ]  # -- This seems to doubly produce filename
                        tokens2 = tokens[8].split("_")
                        tokens3 = tokens2[3].split("lb")
                        tokens3 = tokens3[1].split(".")
                        lumi = int(tokens3[0])
                        if (lumi >= min) & (lumi <= max):
                            lumidict[lumi] = lumi
                            # print(lumi)
                            # stagestat=os.popen('stager_qry -M %s'%fname.strip('castor:')).readlines()[1]
                            # if stagestat.find('STAGED')>=0:
                            # print('USING STAGED FILE ' + fname.strip('castor:'))
                            print("fname:" + str(fname))
                            flist.append(fname)
                            sflist.append((int(fsize), fname))
                            fdict[int(fsize)] = fname
        else:
            if pattern[idir].find("[") >= 0:
                print("Run range selected!!!")
                pattern2 = pattern[idir].split(",")
                initial = pattern2[0].split("[")
                final = pattern2[1].split("]")
                ii = int(initial[1])
                while ii <= int(final[0]):
                    flist += glob.glob(dir + "/" + initial[0] + str(ii) + final[1])
                    ii += 1
            else:
                flist = glob.glob(dir + "/" + pattern[idir])
            sflist = []

    # for ilumi,lumi in enumerate(lumidict):
    #    print(ilumi,lumi)

    # sflist=fdict.items()
    # sflist.sort()
    # print(sflist)
    # sys.exit(0)

    return flist, sflist


def read_settings(args):

    config = {}
    default = {}
    optional = {}

    # Get configfile name
    # Seems to indicate you can pass configfile as second argument
    if len(args) < 2:
        configfile = "configfile"
    else:
        if not os.path.isfile(args[1]):
            configfile = "configfile"
        else:
            configfile = args[1]

    config["Configfile"] = configfile

    # initial settings
    default["UserMail"] = "string"
    optional["UserMail"] = "string"
    default["Release"] = "string"
    default["ReleaseRec"] = "string"
    optional["ReleaseRec"] = "string"
    default["Workdir"] = "string"
    default["WorkdirRec"] = "string"
    optional["barrelToroid"] = "bool"
    default["barrelToroid"] = "bool"
    optional["endcapToroid"] = "bool"
    default["endcapToroid"] = "bool"
    optional["Solenoid"] = "bool"
    default["Solenoid"] = "bool"
    default["MinLumi"] = "string"
    optional["MinLumi"] = "string"
    default["MaxLumi"] = "string"
    optional["MaxLumi"] = "string"
    default["Calibdir"] = "string"
    default["Batchqueue"] = "string"
    default["Inputdir"] = "string"
    default["Inputpattern"] = "string"
    default["StartConst"] = "string"
    default["MinT0"] = "string"
    default["MinRT"] = "string"
    default["RTtag"] = "string"
    default["T0tag"] = "string"
    default["DataType"] = "string"
    default["DetDescVer"] = "string"
    default["GLtag"] = "string"
    default["IOVstart"] = "int"
    default["IOVend"] = "int"
    default["MaxParallel"] = "int"
    default["NumIter"] = "int"
    default["StartIter"] = "int"
    default["IsCosmic"] = "bool"
    default["IsCollision"] = "bool"
    default["IsHeavyIons"] = "bool"
    default["IsCosmicMC"] = "bool"
    default["SubmitRecJobs"] = "bool"
    default["SubmitMergeJobs"] = "bool"
    default["SubmitCalJobs"] = "bool"
    default["SubmitCnvJobs"] = "bool"
    default["SplitAC"] = "bool"
    default["MagnetOn"] = "bool"
    default["GetLog"] = "bool"
    default["CleanRRoot"] = "bool"
    default["UseHist"] = "bool"
    default["CleanCRoot"] = "bool"
    default["CleanCTxt"] = "bool"
    default["CleanCLog"] = "bool"
    default["UseChipRef"] = "bool"
    default["UsePol0"] = "boolarray"
    default["FloatPol3"] = "bool"
    default["RtRelation"] = "string"
    default["RtBinning"] = "string"
    default["DoRt"] = "stringarray"
    default["JobPrefix"] = "string"
    default["DoT0"] = "stringarray"
    default["NumEvents"] = "intarray"
    default["MeanT0"] = "int"
    default["doEc-2"] = "bool"
    default["doEc2"] = "bool"
    default["doBA-1"] = "bool"
    default["doBA1"] = "bool"
    default["OutputLevel"] = "string"
    default["T0globalValues"] = "string"
    default["DoAthenaCalib"] = "bool"
    default["RTglobalValues"] = "string"
    default["IsSingleBeam"] = "bool"
    default["Tag"] = "string"
    default["DoShift"] = "string"
    default["T0Offset"] = "string"
    optional["GetConstants"] = "bool"

    default["DoErrorOptimization"] = "boolarray"
    optional["DoErrorOptimization"] = "boolarray"

    default["DoArXe"] = "bool"
    optional["DoArXe"] = "bool"

    print("using configfile: " + configfile)
    lines = open(configfile).readlines()

    # config["NumEvents"]=-1
    config["ExcludeRuns"] = []

    # loop through lines skipping empty and comment lines
    for i in lines:
        j = i.strip()
        if len(j) < 1 or j[0] == "#":
            continue

        # line must be a setting
        tokens = j.split("=")

        # check that it is in the defaults
        type = ""
        for s, t in default.iteritems():
            if s == tokens[0].strip():
                if not type == "":
                    print("Same setting more than once: %s" % tokens[0])
                    print("Aborting ...")
                    sys.exit(-1)
                else:
                    type = t
                    if type == "intarray":
                        config[s] = []
                        dums = tokens[1].strip().split(",")
                        for dum in dums:
                            config[s].append(int(dum))
                    #                        if len(config[s])==1: config[s]=int(tokens[1].strip())
                    elif type == "boolarray":
                        config[s] = []
                        dums = tokens[1].strip().split(",")
                        for dum in dums:
                            config[s].append(dum == "True")
                    elif type == "stringarray":
                        config[s] = tokens[1].strip().split(",")
                    #                        if len(config[s])==1: config[s]=tokens[1].strip()
                    elif type == "int":
                        config[s] = int(tokens[1].strip())
                    elif type == "bool":
                        tempor = tokens[1].strip()
                        if tempor == "False":
                            config[s] = bool(0)
                        elif tempor == "True":
                            config[s] = bool(1)
                        else:
                            print("Bool variables only accepts values 'True' or 'False'")
                    else:
                        config[s] = tokens[1].strip()
        if type == "":
            print("Unknown configuration (are you using the a correct configfile?) : " + j)
            print("Aborting ...")
            sys.exit(-1)

    # make sure all settings have been found
    missing = False
    for i in default.iterkeys():
        if (not i in config) & (not i in optional):
            print("Option [%s] is missing in the configuration file! " % i)
            missing = True

    if missing:
        print("Not all settings are in the configuration File. Aborting ...")
        sys.exit(-1)
    else:
        print("All settings found. Starting ...\n")

    # read settings from commandline
    config["MakePlots"] = False
    config["MakeLocal"] = False
    config["GetCMTSetup"] = False
    config["ForceGeoTag"] = False
    config["SingleFile"] = False
    config["NoRec"] = False
    config["TempOnCastor"] = False  # True
    config["GetConstants"] = False
    config["TempReadDir"] = ""
    for arg in args:
        if arg == "getconst":
            config["GetConstants"] = True
            config["MaxParallel"] = 2
            config["SubmitRecJobs"] = True
            config["SubmitMergeJobs"] = False
            config["SubmitCalJobs"] = False
            config["SubmitCnvJobs"] = False
            config["NumIter"] = 1
            for ijob, nevt in enumerate(config["NumEvents"]):
                config["NumEvents"][ijob] = 1
        if arg == "t0shift":
            config["DoShift"] = 2
            config["MaxParallel"] = 20
            config["SubmitRecJobs"] = True
            config["SubmitMergeJobs"] = False
            config["SubmitCalJobs"] = False
            config["SubmitCnvJobs"] = False
            config["NumIter"] = 1
            for ijob, nevt in enumerate(config["NumEvents"]):
                config["NumEvents"][ijob] = 1000
        if arg == "makeplots":
            config["MakePlots"] = True
        if arg == "local":
            config["MakeLocal"] = True
        if arg == "cmt":
            config["GetCMTSetup"] = True
        if arg == "single":
            config["SingleFile"] = True
        if arg == "force":
            config["ForceGeoTag"] = True
        if arg == "norec":
            config["NoRec"] = True
        if arg == "afs":
            config["TempOnCastor"] = False
        if arg == "rt":
            config["DoT0"] = ["none"]
        if arg == "t0":
            config["DoRt"] = ["none"]
        if arg.find("=") >= 0:
            cvar = arg.split("=")[0]
            cval = arg.split("=")[1]
            if cvar in default:
                if default[cvar] == "int":
                    config[cvar] = int(cval)
                if default[cvar] == "string":
                    config[cvar] = cval
                if default[cvar] == "bool":
                    if cval == "True" or cval == "true":
                        config[cvar] = True
                    elif cval == "False" or cval == "false":
                        config[cvar] = False
                    else:
                        print("Strange logical value: " + cval)
                        sys.exit(-1)
            else:
                if cvar == "X":
                    config["ExcludeRuns"] = cval.split(",")
                if cvar == "temp":
                    config["TempReadDir"] = cval
                if cvar == "N":
                    config["NumIter"] = int(cval)
                if cvar == "Ne":
                    for ijob, nevt in enumerate(config["NumEvents"]):
                        config["NumEvents"][ijob] = int(cval)
                if cvar == "S":
                    config["StartIter"] = int(cval)
                if cvar == "P":
                    config["MaxParallel"] = int(cval)
                if cvar == "Sub":
                    if cval not in [
                        "0000",
                        "1000",
                        "0100",
                        "0010",
                        "1100",
                        "1110",
                        "1111",
                        "0111",
                        "0011",
                        "0001",
                    ]:
                        print("Wrong subimssion combination ... use one of 0000, 1000, ,0100, 0010, 1100, 1110, 1111, 0111, 0011, 0001")
                        sys.exit(0)
                    config["SubmitRecJobs"] = bool(int((cval)[0]))
                    config["SubmitMergeJobs"] = bool(int((cval)[1]))
                    config["SubmitCalJobs"] = bool(int((cval)[2]))
                    config["SubmitCnvJobs"] = bool(int((cval)[3]))

    if not ("barrelToroid" in config):
        config["barrelToroid"] = False
        if config["MagnetOn"]:
            config["barrelToroid"] = True

    if not ("endcapToroid" in config):
        config["endcapToroid"] = False
        if config["MagnetOn"]:
            config["endcapToroid"] = True

    if not ("Solenoid" in config):
        config["Solenoid"] = False
        if config["MagnetOn"]:
            config["Solenoid"] = True

    if config["SingleFile"]:
        config["StartIter"] = 0
        config["NumIter"] = 1

    for iexv, exv in enumerate(config["ExcludeRuns"]):
        config["ExcludeRuns"][iexv] = int(exv)

    config["SubPat"] = (
        str(int(config["SubmitRecJobs"]))
        + str(int(config["SubmitMergeJobs"]))
        + str(int(config["SubmitCalJobs"]))
        + str(int(config["SubmitCnvJobs"]))
    )
    if config["SubPat"] in ["1000", "1100", "1110", "0111", "0011", "0001"]:
        config["NumIter"] = 1

    if config["MakePlots"]:
        # config["NumIter"] = 1
        if not config["SubPat"] == "0000":
            config["SubPat"] = "0001"

    if config["JobPrefix"] == "":
        config["JobPrefix"] = (
            chr(int(random.random() * 25 + 65))
            + chr(int(random.random() * 25 + 65))
            + chr(int(random.random() * 25 + 65))
        )

    if config["Tag"] == "":
        config["Tag"] = strftime("%d-%b-%Y-%H.%M.%S", gmtime())

    if not "WorkdirRec" in config:
        config["WorkdirRec"] = config["Workdir"]
    else:
        if config["WorkdirRec"] == "":
            config["WorkdirRec"] = config["Workdir"]

    if not "ReleaseRec" in config:
        config["ReleaseRec"] = config["Release"]
    else:
        if config["ReleaseRec"] == "":
            config["ReleaseRec"] = config["Release"]

    if not "UserMail" in config:
        config["UserMail"] = (os.popen("echo $LOGNAME").readlines()[0]).split("\n")[
            0
        ] + "@cern.ch"
    else:
        if config["UserMail"] == "":
            config["UserMail"] = (os.popen("echo $LOGNAME").readlines()[0]).split("\n")[
                0
            ] + "@cern.ch"

    if not "MinLumi" in config:
        print("###   Lower LumiBlock not defined in ConfigFile, going to use 0 by default")
        config["MinLumi"] = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"

    if not "MaxLumi" in config:
        print("###   Upper LumiBlock not defined in ConfigFile, going to use 0 by default")
        config["MaxLumi"] = "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"

    if not "DoErrorOptimization" in config:
        print("###   DoErrorOptimization was not found in the ConfigFile. Will be set to FALSE")
        config["DoErrorOptimization"] = [
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
            False,
        ]

    if not "DoArXe" in config:
        print("###   Setup for Xe/Ar was not found in the ConfigFile. Will be set to FALSE")
        config["DoArXe"] = False

    config["MultiInputdir"] = config["Inputdir"].split(",")
    config["MultiInputpattern"] = config["Inputpattern"].split(",")

    config["MultiMinLumi"] = config["MinLumi"].split(",")
    config["MultiMaxLumi"] = config["MaxLumi"].split(",")

    config["MultiDataset"] = []
    for dir in config["MultiInputdir"]:
        config["MultiDataset"].append(dir.split("/")[-1])

    config["Batchdir"] = os.path.abspath(".") + "/batch"

    lines = []
    cfile = open("lastconfigfile", "w")
    lines.append("# Configurations for the last run\n\n")
    for k in config.iterkeys():
        if k in default:
            lines.append("%-20s = %-20s\n" % (k, config[k]))
    for k in config.iterkeys():
        if k not in default:
            lines.append("# %-20s = %-20s\n" % (k, config[k]))
    cfile.writelines(lines)
    cfile.close()

    return config


def create_calsettings(config, dort, dot0):
    flt = ""
    brd = ""
    # if config["UsePol0"]:
    #    flt+="0"
    if config["FloatPol3"]:
        flt += "3"
    if config["UseChipRef"]:
        brd += "B"

    trtsettings = {}  # settings for 1 job calibrating on TRT level
    trtsettings["CalibrateT0"] = "['TRT']"
    trtsettings["PrintT0Out"] = "['TRT']"
    trtsettings["CalibrateRt"] = "['TRT']"
    trtsettings["PrintRtOut"] = "['TRT']"
    trtsettings["PrintLog"] = "['TRT']"
    trtsettings["NoHistograms"] = ""
    trtsettings["UseBoardRef"] = ""
    trtsettings["SubPart"] = "'user'"
    trtsettings["SplitBarrel"] = "True"

    detsettings = {}  # settings for 4 jobs calibrating on detector level
    detsettings["CalibrateT0"] = "['TRT','Detector']"
    detsettings["PrintT0Out"] = ""
    detsettings["CalibrateRt"] = "['TRT','Detector']"
    detsettings["PrintRtOut"] = ""
    detsettings["NoHistograms"] = "['TRT']"
    detsettings["UseBoardRef"] = ""
    detsettings["PrintLog"] = "['TRT','Detector']"
    detsettings["SubPart"] = "'user'"
    detsettings["SplitBarrel"] = "True"

    laysettings = {}  # settings for 8 jobs calibrating on layer level
    laysettings["PrintT0Out"] = "['Layer','Module','Board','Chip','Straw']"
    laysettings["PrintRtOut"] = "['Layer']"
    laysettings["NoHistograms"] = "['TRT','Detector','Chip','Straw']"
    laysettings["UseBoardRef"] = ""
    laysettings["PrintLog"] = "['TRT','Detector','Layer','Module','Board','Chip']"
    laysettings["SubPart"] = "'user'"
    laysettings["SplitBarrel"] = "True"

    barsettings = {}  # settings for 1 job calibrating the whole barrel
    barsettings["CalibrateT0"] = "['TRT','Detector','Layer']"
    barsettings["PrintT0Out"] = "['Layer']"
    barsettings["PrintRtOut"] = "['Layer']"
    barsettings["NoHistograms"] = "['TRT']"
    barsettings["UseBoardRef"] = ""
    barsettings["PrintLog"] = "['TRT','Detector','Layer']"
    barsettings["SubPart"] = "'user'"
    barsettings["SplitBarrel"] = "False"

    # if no T0 calibration
    if dot0 == "none":
        trtsettings["CalibrateT0"] = ""
        detsettings["CalibrateT0"] = ""
        detsettings["PrintT0Out"] = "['Detector']"
        laysettings["CalibrateT0"] = ""
        barsettings["CalibrateT0"] = ""

    # if TO calibration on board level
    elif dot0 == "board":
        laysettings["CalibrateT0"] = "['TRT','Detector','Layer','Module','Board']"

    # if TO calibration on chip level
    elif dot0 == "chip":
        laysettings[
            "CalibrateT0"
        ] = "['TRT','Detector','Layer','Module','Board','Chip']"

    # if TO calibration on straw level
    elif dot0 == "straw":
        laysettings[
            "CalibrateT0"
        ] = "['TRT','Detector','Layer','Module','Board','Chip','Straw']"
        laysettings["NoHistograms"] = "['TRT','Detector']"

    # if TO calibration on with chip finedelays
    elif dot0 == "chipref":
        laysettings[
            "CalibrateT0"
        ] = "['TRT','Detector','Layer','Module','Board','Chip']"
        laysettings["UseBoardRef"] = "['Chip']"

    if dort == "none":
        trtsettings["CalibrateRt"] = ""
        detsettings["CalibrateRt"] = ""
        laysettings["CalibrateRt"] = ""
        barsettings["CalibrateRt"] = ""

    elif dort == "trt":
        detsettings["CalibrateRt"] = ""
        laysettings["CalibrateRt"] = ""
        barsettings["CalibrateRt"] = ""

    elif dort == "detector":
        laysettings["CalibrateRt"] = "['TRT','Detector']"
        barsettings["CalibrateRt"] = "['TRT','Detector']"

    elif dort == "layer":
        laysettings["CalibrateRt"] = "['TRT','Detector','Layer']"
        barsettings["CalibrateRt"] = "['TRT','Detector','Layer']"

    elif dort == "module":
        laysettings["CalibrateRt"] = "['TRT','Detector','Layer','Module']"
        barsettings["CalibrateRt"] = "['TRT','Detector']"
        laysettings["PrintRtOut"] = "['Layer','Module']"

    calsetting = {}
    calsetting["_all"] = {
        "sel": "_*_-_-_-_-_-_-",
        "opt": "_RTPF" + flt + "_N_N_N_N_N_N",
        "user": trtsettings,
    }
    calsetting["_barrel"] = {
        "sel": "_*_1_*_-_-_-_-",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_N"
        + flt
        + "_N"
        + flt
        + "_N"
        + brd
        + flt
        + "_N",
        "user": barsettings,
    }
    calsetting["_-1"] = {
        "sel": "_*_-1_-_-_-_-_-",
        "opt": "_RTPQ" + flt + "_RTPF" + flt + "_N_N_N_N_N",
        "user": detsettings,
    }
    calsetting["_1"] = {
        "sel": "_*_1_-_-_-_-_-",
        "opt": "_RTPQ" + flt + "_RTPF" + flt + "_N_N_N_N_N",
        "user": detsettings,
    }
    calsetting["_-1_0"] = {
        "sel": "_*_-1_0_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_-1_1"] = {
        "sel": "_*_-1_1_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_-1_2"] = {
        "sel": "_*_-1_2_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_1_0"] = {
        "sel": "_*_1_0_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_1_1"] = {
        "sel": "_*_1_1_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_1_2"] = {
        "sel": "_*_1_2_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_2"] = {
        "sel": "_*_2_-_-_-_-_-",
        "opt": "_RTPQ" + flt + "_RTPF" + flt + "_N_N_N_N_N",
        "user": detsettings,
    }
    calsetting["_-2"] = {
        "sel": "_*_-2_-_-_-_-_-",
        "opt": "_RTPQ" + flt + "_RTPF" + flt + "_N_N_N_N_N",
        "user": detsettings,
    }
    calsetting["_-2_a"] = {
        "sel": "_*_-2_0,1,2,3_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_2_a"] = {
        "sel": "_*_2_0,1,2,3_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_-2_b"] = {
        "sel": "_*_-2_4,5,6,7_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_2_b"] = {
        "sel": "_*_2_4,5,6,7_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_-2_c"] = {
        "sel": "_*_-2_8,9,10,11_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_2_c"] = {
        "sel": "_*_2_8,9,10,11_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_-2_d"] = {
        "sel": "_*_-2_12,13_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }
    calsetting["_2_d"] = {
        "sel": "_*_2_12,13_*_*_*_*",
        "opt": "_RTPQ"
        + flt
        + "_RTPQ"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + flt
        + "_RTPF"
        + brd
        + flt
        + "_FQ",
        "user": laysettings,
    }

    calsetting["error_Barrel"] = ["error_barrel"]
    calsetting["error_Endcap"] = ["error_endcap"]

    return calsetting


def make_itlog(config):
    "test"


###################################################
################ JOB OPTIONS ######################
###################################################


def create_environment_variables(config, inputfile):
    ostring = """
python %s/dumpVersion.py -f %s | tee versiontags.txt;
export DETECTOR_DESCRIPTION=`grep GeoAtlas versiontags.txt | awk '{print $2}' `
echo $DETECTOR_DESCRIPTION
""" % (
        config["Calibdir"],
        inputfile,
    )
    return ostring
