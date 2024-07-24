#!/bin/python3

import os, sys, errno, glob, subprocess, pathlib, getpass, datetime
import argparse
import xmlrpc.client
import subprocess

# Dictionary with folder info used for LArCompleteToFlat
folderInfo = {}
folderInfo["/LAR/ElecCalibOflSC/OFC/PhysWave/RTM/4samples1phase"] = {"key":"LArOFC", "classtype":"LArOFCComplete"}
folderInfo["/LAR/ElecCalibOflSC/Shape/RTM/4samples1phase"] = {"key":"LArShape", "classtype": "LArShapeComplete"}
folderInfo["/LAR/ElecCalibOflSC/Pedestals/Pedestal"] = {"key":"LArPedestal", "classtype": "LArPedestalComplete"}
folderInfo["/LAR/ElecCalibOflSC/Ramps/RampLinea"] = {"key":"LArRamp", "classtype": "LArRampComplete"}
folderInfo["/LAR/ElecCalibOflSC/MphysOverMcal/RTM"] = {"key":"LArMphysOverMcal", "classtype": "LArMphysOverMcalComplete"}



if "Athena_DIR" not in os.environ:
    print("It looks like Athena is not set up... this script will not work without it.")
    print("Please do : ")
    print("setupATLAS; asetup Athena,24.0.54; source /eos/project-a/atlas-larcalib/public/build/x86_64-el9-gcc13-opt/setup.sh")
    sys.exit(1)


from PyCool import cool,coral

def getFolderList(dbstring, retdb=False, cleanlist=False):
    # get database service and open database
    dbSvc=cool.DatabaseSvcFactory.databaseService()
    # database accessed via logical name, can also use physical connections
    try:
        db=dbSvc.openDatabase(dbstring)
    except Exception as e:
        print(f"Problem opening database {e}")
        sys.exit(-1)
    print(f"Opened database {dbstring}")
    # now list the folders
    folderlist= [ str(s) for s in db.listAllNodes()]
    if cleanlist:
        # Careful... some folders have similar names e.g. OFC and OFCCali
        # folderlist = [j for j in folderlist if all(j not in k if j!=k else True for k in folderlist)]
        folderlist = [f for f in folderlist if f.count("/") >= 3]

    if retdb:
        return folderlist, db
    else:
        return folderlist


#from mp import parallel_exec 
#sys.path.append("/afs/cern.ch/user/l/larmon/public/prod/LArPage1/makeJIRA")
#import jiraComment as jc

class cfile():
    #subclass file to have a more convienient use of writeline
    def __init__(self, filename, mode = 'w'):
        self.file = open(filename, mode, buffering=1)
    def wl(self, string):
        self.file.writelines(string + '\n')
        self.file.flush() 
        os.fsync(self.file) 
        return None
    def close(self):
        self.file.close()

def chmkDir( path ):
    os.umask(0)
    try:
        print( "mkdir "+path )
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise
        pass

def get_latest_run():
    dqmsite = "atlasdqm.cern.ch"
    dqmpassfile = "/afs/cern.ch/user/l/larmon/public/atlasdqmpass.txt"
    if not os.path.isfile(dqmpassfile):
        return 555555
    dqmpass = None
    dqmapi = None
    with open(dqmpassfile, "r") as f:
        dqmpass = f.readlines()[0].strip()
        if ":" not in dqmpass:
            print("Problem reading dqmpass")
            sys.exit()
        try:
            dqmapi = None #xmlrpc.client.ServerProxy("https://"+dqmpass+"@"+dqmsite)
        except:
            print("Failed to connect to atlasdqm, therefore giving a fake run number 555555")
            return 555555
    if dqmapi is not None:
        return dqmapi.get_latest_run()
    else:
        return 555555

    
class logFile():
    def __init__(self, logpath):
        self.logfile = open(logpath, "r", newline="\n")
        self.lines = self.logfile.readlines()    
        self.runs = self.runsFromLog()
        self.name = pathlib.Path(logpath).stem
        
    def runsFromLog(self):
        runs = []
        for line in self.lines:
            if not line.startswith("Run "): continue
            runs.append(int(line.split("Run")[1].split(":")[0]))
        return runs
    def __str__(self):
        toprint = []
        for line in self.lines:
            if not line.startswith("Run "): continue
            line = line.strip("\n")
            toprint.append(line)
        return "\n".join(toprint)
    def getTopDirs(self, topdir):
        self.topdirs = []
        for run in self.runs:
            self.infiles = glob.glob(f"{topdir}/*{run}*/root_files/*{run}*") 
            self.topdirs.extend( list(set([ str(pathlib.Path(i).parents[1]) for i in self.infiles ])) )
        self.topdirs = list(set(self.topdirs))
        self.sqldbs = self.getDBfiles()
        self.bcsnapshots = self.getBCSnapshots()
        return self.topdirs
    
    def getDBfiles(self):        
        indbs = [ g for pd in self.topdirs for g in glob.glob(f"{pd}/*/mysql.db") ]
        indbs = list(set(indbs))
        self.pooldirs = list(set([ str(pathlib.Path(i).parents[0]) for i in indbs ]) )
        return indbs

    def getBCSnapshots(self):
        indbs = [ g for pd in self.topdirs for g in glob.glob(f"{pd}/*/SnapshotBadChannel.db") ]
        indbs = list(set(indbs))
        return indbs
        # /eos/project/a/atlas-larcalib/AP/00473386_00473391_00473394_FMhighEtaBack_HIGH_2/pool_files/SnapshotBadChannel.db


def athListFormat(vals):
    if isinstance(vals, list):
        vals = ",".join(vals)
    return vals


def printAndRun(cmd, outlogpath=None, runNow=True):
    print("**** RUNNING THE FOLLOWING COMMAND ****")
    print(cmd)
    print("***************************************")
    if runNow:
        process = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out,err = process.communicate()        
        retcode = process.returncode
        log = None
        if outlogpath is not None:
            print(f"See log: {outlogpath}")
            log = cfile(outlogpath)
            log.wl("~~~~~~~~ OUTPUT ~~~~~~~~")
            log.wl(out.decode('ascii'))
            log.wl("~~~~~~~~~~~~~~~~~~~~~~~~")
            log.wl("~~~~~~~~ ERROR ~~~~~~~~")
            log.wl(err.decode('ascii'))
            log.wl("~~~~~~~~~~~~~~~~~~~~~~~~")
        else:
            print("~~~~~~~~ OUTPUT ~~~~~~~~")
            print(out.decode('ascii'))
            print("~~~~~~~~~~~~~~~~~~~~~~~~")
            print("~~~~~~~~ ERROR ~~~~~~~~")
            print(err.decode('ascii'))
            print("~~~~~~~~~~~~~~~~~~~~~~~~")
        if retcode != 0:
            print("!!!!!!!! DID NOT TERMINATE SUCCESSFULLY... exiting")
            sys.exit()

def run_merge(insqlite, inkeys, outsqlite, outpool, Ncoll=0, poolcat="mergedPoolCat.xml", outlogpath=None, runNow=True):
    ''' Step 1 of the merging. '''
    insqlite = athListFormat(insqlite)
    inkeys = athListFormat(inkeys)
    cmd = f"python -m LArCalibProcessing.LArNewCalib_MergeDB --insqlite {insqlite} --inkeys {inkeys} --outsqlite {outsqlite} --poolfile {outpool} --isSC --poolcat {poolcat} --Ncoll {Ncoll}"
    printAndRun(cmd,outlogpath, runNow=runNow)

def run_fillofcphase(phases_txt, outkey="LArSCOFCPhase", default_phase=22, folder="/LAR/ElecCalibOflSC/OFCBin/PhysShift", tag="LARElecCalibOflSCOFCBinPhysShift-10", outsql="SCOFCPhase.db", outpool="SC_OFC_Phase_10.pool.root",outlogpath=None, runNow=True):
    ''' Step 2 *if needed* - make an sql file from picked OFC phase txt file '''
    cmd = f"LArNewCalib_FillOFCPhase.py --infile {phases_txt} --outkey {outkey} --isSC --hasid --default {default_phase} --folder {folder} --tag {tag} --outsql {outsql} --outp {outpool}"
    printAndRun(cmd,outlogpath, runNow=runNow)
    # OIOIOI run command

def run_ofcpick(insqlite, outsqlite, phase_sql, run, BCsnapshotDB, outpdir="./", outrdir="./", outname="Picked_phase24052024", tag="LARElecCalibOflSCOFCBinPhysShift-09", poolcat="mergedPoolCat.xml", outlogpath=None, runNow=True):  # insqlite  mergeSC.db, outssqlite mergeSCOnl_1.5_phase24052024.db , subdet Picked_phase24052024, phase_sql SCOFCPhase.d)
    ''' Step 3, picking the phase in the DB '''
    # OIOIOI here the Ncoll needs to be added in the athena part as well
    cmd=f"LArNewCalib_PhysOFCPhasePicker.py --run {run} -b {BCsnapshotDB} --insqlite {insqlite} --poolcat {poolcat} --outsqlite {outsqlite} --isSC --outpdir {outpdir} --outrdir {outrdir} --subdet {outname} --ofcphasetag {tag} --ofcphasesqlite {phase_sql}"
    printAndRun(cmd,outlogpath, runNow=runNow)
    # OIOIOI run command

def run_toCoolInline(mergedDB, outDB, infolders="ConvertToInlineSC", globalTag="LARCALIB-RUN2-00", poolcat="mergedPoolCat.xml", outlogpath=None, runNow=True):# outDB freshConstantsOnl_1.5.db
    ''' Step 4: flattening the DB '''
    #  connecting all folder level tags to some new global tag..... In calibration processing it is used to be able to define some calib. global tag in sqlite file, and then job do not need to know all individual folder level tags, athena automaticaly uses the one connected to defined global tag...
    cmd=f"/afs/cern.ch/user/l/larcalib/LArDBTools/python/BuildTagHierarchy.py {mergedDB} {globalTag}"
    printAndRun(cmd,outlogpath, runNow=runNow)
    # OIOIO run command
    cmd=f"LArCalib_ToCoolInlineConfig.py --insqlite {mergedDB} --infolders {infolders} --poolcat {poolcat} --outsqlite {outDB} --isSC"
    printAndRun(cmd,outlogpath, runNow=runNow)
    #OIOIOI run command


def poolCatalog(poolDir, catalog=None, runNow=True):
    print(f"** Adding pool files from {poolDir} with pool_insertFileToCatalog **")
    for f in [os.path.join(poolDir, f) for f in os.listdir(poolDir) if f.endswith(".pool.root")]:
        cmd1 = "pool_extractFileIdentifier "+f
        process = subprocess.Popen(cmd1,shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out,err = process.communicate()
        retcode = process.returncode
        ident=out.decode('ascii').split(" ")[0]        
        cmd=f"pool_insertFileToCatalog.py {f}"
        if catalog is not None:
            cmd += f" --catalog='xmlcatalog_file:{catalog}'"
        print(f"---- {cmd} ({ident})")
        if runNow:
            process = subprocess.Popen(cmd,shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            out,err = process.communicate()
            retcode = process.returncode
            if retcode != 0:
                print(err.decode('ascii')) #print("ERROR: inserting file to catalogue failed")
            #print(out.decode('ascii'))
            #print(err.decode('ascii'))

def folderNamesTags(sql, mustr="mu-60", dbname="CONDBR2", verbose=False):
    dbstring=f"sqlite://;schema={sql};dbname={dbname}"
    folderlist, db = getFolderList(dbstring, retdb=True, cleanlist=True)
    ftags = {}
    for f in folderlist:
        try:
            myfolder=db.getFolder(f)
            # list tags in folder
            if verbose: print("*"*20)
            if verbose: print(f"Reading folder {f}")
            if verbose: print("*"*20)
            if verbose: print("Checking tags...")
            taglist=myfolder.listTags()
            if verbose: print( f"{len(taglist)} Tags in folder:" )
            if verbose:
                for i in taglist: print( i )
            thetag = ""
            if len(taglist) > 0:
                taglist = [ str(t) for t in taglist ]
                posstag = [ t for t in taglist if mustr in t ] 
                if len(posstag) == 1:
                    thetag = posstag[0]
                else:
                    if verbose: print(len(posstag), "Choosing first tag")
                    thetag = taglist[0]
            if verbose: print(f"Will use tag {thetag}")
            ftags[f] = thetag

        except Exception as e:
            continue
    return ftags
    
if __name__ == "__main__":  
    parser = argparse.ArgumentParser()
    #parser.add_argument( '-inputLog', '-l', type=str, required=True, dest='inputLog', help='Name of input log file which lists the runs' )
    parser.add_argument( 'inputLog', type=str,  help='Name of input log file which lists the runs' )
    parser.add_argument( '-logDir', type=str, dest='logDir', default="/afs/cern.ch/user/l/lardaq/public/hfec-calibration-logs", help="Input path for log files. Default %(default)s." )
    parser.add_argument( '-inDir', '-i', type=str, default='/eos/project-a/atlas-larcalib/AP', help="Input directory containing root files from the processing. Default %(default)s.")
    parser.add_argument('-r', '-run', dest='run', type=int, default=get_latest_run(), help='Run number - used for bad channels. Default is latest run.')
    # parser.add_argument('--online', dest="onlineFolders", nargs='+', default=["Pedestal", "MphysOverMcal", "Ramp"], help="List of folders to merge in a separate db 'mergeSCOnl.db'. Default %(default)s.")
    parser.add_argument('--folders', dest="folders", nargs='+', default=["Pedestal", "MphysOverMcal", "Ramp","AutoCorr", "OFCCali", "LArOFCPhys4samples", "LArOFCPhys4samplesMu", "PhysWave", "PhysAutoCorr", "CaliPulseParams", "DetCellParams", "CaliWave", "LArShape4samples"], help="List of folders to merge in a separate db 'mergeSC.db'. Default %(default)s.")
    parser.add_argument('-o','--outdir', dest='outdir', default=f'/tmp/{getpass.getuser()}', help="Output directory for running and producing merged files. Default %(default)s.")
    parser.add_argument('-Ncoll', dest='Ncoll', type=int, default=0, help="Pileup setting. If 0, both are made. Default %(default)s.")
    parser.add_argument('-phase_txt', dest="phase_txt", type=str, default=None, help="Full path to .txt file which contains picked phases. Provide either this or a .db version")
    parser.add_argument('-phase_db', dest="phase_db", type=str, default=None, help="Full path to .db file which contains picked phases. Provide either this or a .txt version")
    parser.add_argument('-t','--tag', dest="out_tag", type=str, default=None, help="A tag to add to the name of output files, to help keep track of what was used. e.g. could use 'Onl' or 'Ofl' to separate blobs for different uploads. Default is empty, but the current date will be added.")
    parser.add_argument('-d', '--dryRun', dest='dryRun', action='store_true', help="Perform a dry run? This will print the commands to terminal without running them")
    args   = parser.parse_args()

    runNow = not args.dryRun
    
    #insqlite=None  # comma separated list, no spaces
    #inkeys = None  # matching comma separated list
    #outsqlite=None
    #poolfile=None
    #poolcat=None
    #Ncoll=0   

    outtag = datetime.datetime.now().strftime("%y%m%d")
    if args.out_tag is not None:
        outtag = args.out_tag+"_"+outtag
        
    
    if not args.inputLog.endswith(".log"):
        args.inputLog += ".log"

    logPath = f"{args.logDir}/{args.inputLog}"

    if not os.path.isfile(logPath):
        print(f"Couldn't find file {logPath}")
        sys.exit()

    doPicking=True
    if args.phase_txt is None and args.phase_db is None:
        print("NO PICKED PHASE FILE PROVIDED!! Please supply either a .txt (-phase_txt) or .db (-phase_db) file so that the phases can be picked for the OFCs")
        print("** This means we will skip the picking part... therefore the single-phase OFC folders **")
        doPicking=False
    if args.phase_txt is not None and args.phase_db is not None:
        print("BOTH .txt AND .db PICKED PHASE FILES WERE PROVIDED. Please only provide one, so there is no ambiguity")
        sys.exit()

    for f in [ "phase_txt", "phase_db" ]:
        if getattr(args,f) is not None:
            if not os.path.isfile(getattr(args,f)):
                print(f"PROVIDED {f} DOES NOT EXIST!! Please check the path")
                sys.exit()
    

    theLog = logFile(logPath)

    # Get the list of .db sql files & BadChannels snapshots
    theLog.getTopDirs(args.inDir)
    insqlite = theLog.sqldbs
    bcsnapshots = theLog.bcsnapshots

    # Make an output directory to run in
    args.outdir = f"{args.outdir}/{theLog.name}"
    outdir_root = f"{args.outdir}/rootFiles"
    outdir_pool = f"{args.outdir}/poolFiles"
    outdir_logs = f"{args.outdir}/logs"    
    print(f"Output (including root and pool files) will go to {args.outdir}")
    chmkDir(args.outdir)
    chmkDir(outdir_root)
    chmkDir(outdir_pool)
    chmkDir(outdir_logs)

    mergedDB = f"{outdir_pool}/mergeSC_{outtag}.db"
    mergedPool = f"{outdir_pool}/merged_SC_{outtag}.pool.root"
    finalDB_flat = f"{outdir_pool}/freshConstants_1.5_{outtag}.db"
    poolcat = f"{outdir_pool}/mergedPoolCat.xml"
    phase_pool = f"{outdir_pool}/SC_OFC_Phase_10_{outtag}.pool.root"
    
    for pd in theLog.pooldirs:
        poolCatalog(pd, poolcat, runNow=runNow)


    # Step 1, make the merged sql file(s)
    run_merge(insqlite=insqlite, inkeys=args.folders, outsqlite=mergedDB, outpool=mergedPool, Ncoll=args.Ncoll, poolcat=poolcat, outlogpath=f"{outdir_logs}/run_merge_nopick.txt", runNow=runNow)

    if doPicking:
        phaseDBname = f"SCOFCPhase_{outtag}.db"
        if args.phase_txt is not None:
            # Step 2 (if needed) - make the picked phase sql file
            args.phase_db = f"{args.outdir}/{phaseDBname}"
            run_fillofcphase(phases_txt=args.phase_txt, outkey="LArSCOFCPhase", default_phase=22, folder="/LAR/ElecCalibOflSC/OFCBin/PhysShift", tag="LARElecCalibOflSCOFCBinPhysShift-10", outsql=args.phase_db, outpool=phase_pool, outlogpath=f"{outdir_logs}/run_fillofcphase.txt", runNow=runNow)

        # Step 3, apply the picking to the merged db file
        # run_ofcpick(insqlite=mergedDB_topick, outsqlite=mergedDB_picked, phase_sql=args.phase_db, run=args.run, BCsnapshotDB=bcsnapshots[0], outpdir=outdir_pool, outrdir=outdir_root, outname=f"Picked_phase_{outtag}", tag="LARElecCalibOflSCOFCBinPhysShift-09", poolcat=poolcat, outlogpath=f"{outdir_logs}/run_ofcpick.txt", runNow=runNow)
        run_ofcpick(insqlite=mergedDB, outsqlite=mergedDB, phase_sql=args.phase_db, run=args.run, BCsnapshotDB=bcsnapshots[0], outpdir=outdir_pool, outrdir=outdir_root, outname=f"Picked_phase_{outtag}", tag="LARElecCalibOflSCOFCBinPhysShift-09", poolcat=poolcat, outlogpath=f"{outdir_logs}/run_ofcpick.txt", runNow=runNow)
    else:
        print("**** NOTE: NO OFC PICKING WAS DONE, DUE TO LACK OF PICKING INPUT ****")

    # oioi get list of folders, for ones with tags add the last word to the folder list
    foldersTags = folderNamesTags(mergedDB, mustr="mu-60") # oOIOIOIOI set mu in options

    print(foldersTags)

    # folderScript = f"{args.outdir}/ConvertToInlineSC" # OIOIOI has to be local for now
    folderScript = f"ConvertToInlineSC"
    print(f"Writing folder info to {folderScript}.py")
    with open(f"{folderScript}.py", "w") as ffile:
        lines = ["inputFolders=[]"]
        for f in foldersTags.keys():
            if f in folderInfo.keys():
                theFolder = f
                theTag = foldersTags[f]
                theKey = folderInfo[f]["key"]
                theClassType = folderInfo[f]["classtype"]

                theline = f'inputFolders.append(("{theFolder}","{theTag}","{theKey}","{theClassType}"))'
                lines.append(theline)
                
            else:
                print(f"NOT SURE HOW TO FLATTEN {f}, so will skip it")
                continue
        ffile.write("\n".join(lines))
    

    # Step 4, flatten the db
    run_toCoolInline(mergedDB=mergedDB, infolders=folderScript, outDB=finalDB_flat, globalTag="LARCALIB-RUN2-00", poolcat=poolcat, outlogpath=f"{outdir_logs}/run_toCoolInline.txt", runNow=runNow)
