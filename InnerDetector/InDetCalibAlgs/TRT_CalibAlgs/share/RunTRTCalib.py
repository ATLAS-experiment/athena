import sys, os, subprocess, glob
from time import gmtime, strftime
from runiter import run_iteration
from util import read_settings, build_filelist, extractDDV2
from RAWTemplate import collision
from dag import write_dag

###############################################
############## MAIN ROUTINE ###################
###############################################

config = read_settings(sys.argv)

batchdir = config["Batchdir"]
batchq = config["Batchqueue"]
inputdir = config["MultiInputdir"]
inputpattern = config["MultiInputpattern"]
rttag = config["RTtag"]
t0tag = config["T0tag"]
IOV = [config["IOVstart"], 0, config["IOVend"], 4294967295]
maxjobs = config["MaxParallel"]
numiterations = config["NumIter"]
firstiteration = config["StartIter"]
startconstants = config["StartConst"]

minlumi = config["MultiMinLumi"]
maxlumi = config["MultiMaxLumi"]

calibcmd = config["Calibdir"] + "/CalibrateTRT"
oldcalibcmd = config["Calibdir"] + "/TRTCalibration.py"
mergecmd = config["Calibdir"] + "/Merge _-_-_-_-_-_*"
mergetxtcmd = config["Calibdir"] + "/MergeCalibOutput"
outdir = batchdir + "/output"
constdir = batchdir + "/input"
calibprefix = batchdir + "/input/calibout"

jobprefix = config["JobPrefix"]

# create a uniqe calibid (timestamp)
calibid = ""
if not os.path.isdir(batchdir):
    calibid = strftime("%Y%m%d%H%M%S", gmtime())
else:
    calibid = os.popen("ls -1 %s/id_*" % batchdir).read().strip().split("id_")[-1]

# add some configuration flags
config["TempDir"] = batchdir + "/temp"
# if config["TempOnCastor"]:
#    config["TempDir"] = (
#        "/eos/atlas/user/"
#        + os.environ["USER"][0]
#        + "/"
#        + os.environ["USER"]
#        + "/tempstore/temp_"
#        + calibid
#    )
config["SetupCmdRec"] = (
    "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n" + "alias setupATLAS=\'source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh\'\n" + "setupATLAS\n" + "export AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup\n" + "alias asetup='source $AtlasSetup/scripts/asetup.sh'\n" + "asetup "
    + config["ReleaseRec"]
    + " ; source "
    + config["WorkdirRec"]
)
config["SetupCmdCal"] = (
    "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n" + "alias setupATLAS=\'source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh\'\n" + "setupATLAS\n" + "export AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup\n" + "alias asetup='source $AtlasSetup/scripts/asetup.sh'\n" + "asetup "
    + config["Release"]
    + " ; source "
    + config["Workdir"]
)
#config["SetupCmdCal"] = (
#    "source /afs/cern.ch/project/gd/apps/atlas/slc3/local/setup.sh\nexport AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup\nalias asetup='source $AtlasSetup/scripts/asetup.sh'\nasetup "
#    + config["Release"]
#    + "  ; source  "
#    + config["Workdir"]
#)
config["CalibCmd"] = calibcmd
config["MergeCmd"] = mergecmd
config["MergeTxtCmd"] = mergetxtcmd
config["ConstDir"] = constdir
config["OutDir"] = outdir
config["CalPrefix"] = calibprefix
config["Localdir"] = os.path.abspath(".")

# just print the cmt setup command
if config["GetCMTSetup"]:
    print( config["SetupCmdRec"] )
    sys.exit(0)

# create a new batch directory if it doesn't already exist
if not os.path.isdir(batchdir):
    print("Batch directory = %s ...\n" % config["Batchdir"])
    os.system("mkdir -v " + batchdir)
    os.system("mkdir -v " + batchdir + "/input")
    os.system("mkdir -v " + batchdir + "/output")
    os.system("mkdir -v " + batchdir + "/temp")
    idfile = open(batchdir + "/id_" + calibid, "w")
    idfile.write(config["TempDir"])
    idfile.close()
    if config["TempOnCastor"]:
        os.system("export STAGE_SVCCLASS=atlcal; rfmkdir %s" % config["TempDir"])
    if config["SubPat"] not in ["0000"]:
        print("\nCopying files for reference ...\n")
        os.system(
            "cp -fv " + sys.argv[0] + " " + batchdir
        )  # copy files for using as reference if any jobs are submitted
        os.system("cp -fv %s %s/configfile" % ("lastconfigfile", batchdir))
else:
    # else run on files in the the provided one
    if config["SubPat"] not in ["0000"]:
        print("Using old batch directory %s" % batchdir + ". Copying files for reference ...\n")
        os.system(
            "cp -fv " + sys.argv[0] + " " + batchdir
        )  # copy files for using as reference
        os.system("cp -fv %s %s/configfile" % ("lastconfigfile", batchdir))

if config["TempReadDir"] == "":
    config["TempReadDir"] = config["TempDir"]
else:
    config["TempReadDir"] = (
        "/eos/atlas/user/"
        + os.environ["USER"][0]
        + "/"
        + os.environ["USER"]
        + "/tempstore/temp_"
        + config["TempReadDir"]
    )

# Copy start constants into the batch directory for condor access
# TODO: neded? What if we have specified db as startconstants? Choose here run 451794 for both "precision constants" and "start constants" as well as "calibout_-1". This is just to avoid missing files. We need to understand better what to do.
subprocess.call(["cp", "-f", "-v","/afs/cern.ch/user/a/attrtcal/TRT_Calibration/Tier0/manual/Collisions2022/dbconst_data23_start.txt", batchdir + "/dbconst.txt"])
subprocess.call(["cp", "-f", "-v","/afs/cern.ch/user/a/attrtcal/TRT_Calibration/Tier0/manual/Collisions2022/dbconst_data23_start.txt", batchdir + "/input/calibout_start.txt"])

if (config["IsCollision"] == 1) & (config["IsCosmic"] == 1):
    print("Collisions and IsCosmic both set to true!!!!!!")
    sys.exit(0)

if (config["IsCollision"] == 1) & (config["IsHeavyIons"] == 1):
    print("Collisions and HEAVI IONS can not be selected at same time!!!!!!!!!")
    sys.exit(0)

if (config["IsHeavyIons"] == 1) & (config["IsSingleBeam"] == 1):
    print("Collisions and IsSingleBeam both set to true!!!!!!")
    sys.exit(0)

if (config["IsSingleBeam"] == 1) & (config["IsCosmic"] == 1):
    print("IsCosmic and IsSingleBeam both set to true!!!!!!")
    sys.exit(0)

if (config["IsCosmicMC"] == 1) & (config["IsCosmic"] == 1):
    print("IsCosmic and IsCosmicMC both set to true!!!!!!")
    sys.exit(0)

if (config["IsCosmicMC"] == 1) & (config["IsSingleBeam"] == 1):
    print("IsCosmicMC and IsSingleBeam both set to true!!!!!!")
    sys.exit(0)

# dump joboptions to a file
jofile = open("%s/joboptions" % batchdir, "w")
if config["IsHeavyIons"]:
    if config["DataType"] == "ESD":
        from HITemplate import collision

        jofile.write(collision(config, ["file1", "file2"], "calibconstants", -1))
    if config["DataType"] == "RAW":
        from RAWHITemplate import collision

        print("using RAW config")
        jofile.write(collision(config, ["file1", "file2"], "calibconstants", -1))
else:
    from RAWTemplate import collision
    jofile.write(collision(config, ["file1", "file2"], "calibconstants", -1))

jofile.close()

flist, sflist = build_filelist(inputdir, inputpattern, minlumi, maxlumi)

if not os.path.isdir("TRTcal_error"):
    os.system("mkdir -v TRTcal_{error,log,output}")

if len(flist) == 0:
    print("#####################################################################################")
    print("## ")
    print("##       error!")
    print("##       FILES NOT FOUND UNDER: ", inputdir)
    print("##       error!")
    print("## ")
    print("#####################################################################################")
    sys.exit()

if not config["ForceGeoTag"]:
    if (config["IsCollision"] == 1) | (config["IsCosmic"] == 1):
        extractDDV2(config, sflist)

waitlist = []
iterlist = []
subnames = []

print("Making scripts...\n")
for i in range(firstiteration, firstiteration + numiterations):
    config["Iiter"] = "%i" % i
    waitlist, stemp = run_iteration(config, sflist, i, waitlist, startconstants)
    subnames += stemp
    iterlist.append("0" + str(i))

import os

cwd = os.getcwd()

if (
    config["SubmitCalJobs"]
    or config["SubmitCnvJobs"]
    or config["SubmitMergeJobs"]
    or config["SubmitRecJobs"]
):
    print("Submitting jobs...\n")
    print('0')
    write_dag(filelist=subnames, iterlist=iterlist, mail=config["UserMail"], area=cwd)
    print('1')
    files = glob.glob("mail*")
    print('2')
    subprocess.call(["chmod", "a+x"] + files)
    print('3')
    subprocess.call(["condor_submit_dag", "-AlwaysRunPost=True", '-f', "dag.dag"])

print()
print("CALIBRATION ID: ", calibid)
print()
print(config["DoRt"])
print(config["DoT0"])
print(config["NumEvents"])
print(config["UsePol0"])
