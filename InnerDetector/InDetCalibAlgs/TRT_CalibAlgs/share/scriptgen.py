"""Module to configure .sub files for a particular script, creating the script itself in the process.
The functions in this module are largely the same, and all return the name of the created .sub file.
These names are accumulated in a list (called subnames) in the main script, and are used in dag.py.

Authors: Various (adapted by Nathan Simpson)
Filename: scriptgen.py
Last modified: 07/10/2019
"""

import os, sys, random
from util import it_statusmail
from CosmicTemplate import cosmic
from CollisionTemplate import collision


def parallel(
    config,
    inputfiles,
    calibconstants,
    outputfile,
    iscalib,
    calfile,
    caltag,
    oiter,
    calsettings2,
    ijob,
):

    ostring = "export HOME=" + config["Calibdir"].split("testarea")[0] + "\n"
    # ostring += "cd $WORKDIR\n"

    # copy some precision constants for bhadd to work
    ostring += "cp -fv " + config["Batchdir"] + "/dbconst.txt precision_constants.txt\n"
    # copy some oldt0s for the first iteration
    ostring += "cp -fv  /afs/cern.ch/user/a/attrtcal/TRT_Calibration/Tier0/manual/Collisions2022/oldt0s.txt " + config["Batchdir"] + "/temp/trtcalib_-1_oldt0s.txt\n"

    ostring += "echo $WORKDIR\n"
    ostring += "shopt -s expand_aliases\n"
    ostring += "pwd\n"
    ostring += "export STAGE_SVCCLASS=atlcal\n"
    ostring += "uname -a\n"

    cpcmd = "cp -fv "
    if outputfile.find("/eos/") >= 0:
        cpcmd = "xrdcp -f "

    if iscalib:
        ostring += config["SetupCmdCal"] + "\n"
    else:
        ostring += config["SetupCmdRec"] + "\n"

    if config["NoRec"] and not iscalib:
        if float(config["DoShift"]) > 0:
            mint0 = -float(config["DoShift"])
            maxt0 = float(config["DoShift"])
            stept0 = (maxt0 - mint0) / int(config["MaxParallel"])
            t0shift = mint0 + ijob * stept0
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + outputfile
                + "_basic_shifted_%.1f.root.bz2 basic.root\n" % t0shift
            )
            ostring += "root -b -q  basic.root '%s/graph_points.C(%f,%d)'\n" % (
                config["Calibdir"],
                t0shift,
                oiter + 1,
            )
            ostring += "cp -fv gp_* " + config["Batchdir"] + "/temp/.\n"
            if t0shift == 0:
                ostring += "cp -fv shiftres_* " + config["Batchdir"] + "/temp/.\n"
        else:
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + outputfile
                + "_basic.root.bz2 basic.root.bz2\n"
            )
            ostring += "bunzip2 -v basic.root.bz2\n"
            ostring += "ln -sf " + config["Calibdir"] + "/bhadd.cpp\n"
            ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
            ostring += "echo I'm trying to make bhadd in:\n"
            ostring += "pwd\n"
            ostring += "make bhadd\n"
            ostring += "./bhadd dumfile basic.root\n"
            ostring += (
                cpcmd
                + "tracktuple.root "  # root://eosatlas.cern.ch/"
                + outputfile
                + "_tracktuple.root\n"
            )
        return ostring

    if iscalib:
        prevcalfile = calfile.strip("%02i" % (oiter + 1)) + "%02i" % oiter
        if caltag == "error_Barrel":
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + calfile
                + "_tracktuple.root tracktuple.root\n"
            )
            ostring += "ln -sf " + config["Calibdir"] + "/ErrorsOptimization.cpp\n"
            ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
            ostring += "make ErrorsOptimization\n"

        elif caltag == "error_Endcap":
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + calfile
                + "_tracktuple.root tracktuple.root\n"
            )
            ostring += "ln -sf " + config["Calibdir"] + "/ErrorsOptimization.cpp\n\n"
            ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
            ostring += "make ErrorsOptimization\n"
        else:
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + calfile
                + "_basic.root.bz2 merged.root.bz2\n"
            )
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + prevcalfile
                + "_oldt0s.txt calib_constants_in.txt\n"
            )
            ostring += "bunzip2 -v *.bz2\n"

    ifiles = []
    # copy all inputfiles to local area
    if iscalib:
        ostring += (
            "cp -v "
            + config["Batchdir"]
            + "/input/calibout_start.txt " + config["Batchdir"] + "/input/calibout_-1.txt\n"
        )
        ostring += (
            "cp -v "
            + config["Batchdir"]
            + "/input/calibout_%02d.txt calibconstants.txt\n" % oiter
#            + "/input/calibout_start.txt calibconstants.txt\n" # change name
        )
        ostring += "cp -v " + config["Calibdir"] + "/finedelays.txt .\n"
    for inputfile in inputfiles:
        if inputfile.find("castor:") >= 0:
            ostring += "rfcp " + inputfile.split(":")[1] + " .\n"
        elif inputfile.find("eosatlas:") >= 0:
            ostring += (
                "xrdcp root://eosatlas.cern.ch/" + inputfile.split(":")[1] + " .\n"
            )  # + " .\n"
            print("input files: ")
            print(inputfiles)
            # print(inputfile.split(":")[1])
        else:
            ostring += "cp " + inputfile + " .\n"

        # strip path from inputfile
        ifile = os.path.basename(inputfile)
        ifiles.append(ifile)

    t0shift = 0
    if not iscalib:
        ostring += "ln -sf " + config["Calibdir"] + "/bhadd.cpp\n"
        ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
        ostring += "make bhadd\n"
        if float(config["DoShift"]) > 0:
            mint0 = -float(config["DoShift"])
            maxt0 = float(config["DoShift"])
            stept0 = (maxt0 - mint0) / int(config["MaxParallel"])
            t0shift = mint0 + ijob * stept0
            ishiftfiles = 9 + int(random.random() * 10)
            print("SHIFTING T0s %f ns, USING FILES FROM JOB %d" % (t0shift, ishiftfiles))
            ostring += "#SHIFTING T0s %f ns\n" % t0shift
            if oiter >= 0:
                ostring += (
                    "python "
                    + config["Calibdir"]
                    + "/cfilter.py t0shift %f " % t0shift
                    + calibconstants
                    + "\n"
                )
            else:
                ostring += (
                    "python "
                    + config["Calibdir"]
                    + "/cfilter.py t0shift %f " % t0shift
                    + config["Batchdir"]
                    + "/input/calibout_start.txt"
                    + "\n"
                )
            calibconstants = "/dbconst.txt"
            # random.shuffle(config["shiftfiles"])
            inputfiles = config["shiftfiles"]
            # inputfiles=config["shiftfiles"][ishiftfiles]
            # print("###### %d"%len(config["shiftfiles"]))

    ostring += "ls -al\n"

    # set environment variables (detector description)
    if config["DataType"] == "ESD":
        "dum"

    # create jobOptions
    sys.path.append(config["Calibdir"])
    if iscalib:
        if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
            if (
                (not config["IsCosmicMC"])
                & (not config["IsCosmic"])
                & (not config["IsCollision"])
                & (not config["IsHeavyIons"])
            ):
                from ESDCalibTemplate import ESDCalib

                jO = ESDCalib(config, ifiles, calibconstants, caltag.split("_", 1)[1])
            elif config["IsCosmicMC"]:
                from CosmicMCCalibTemplate import cosmiccalib

                jO = cosmiccalib(
                    config, ifiles, calibconstants, caltag.split("_", 1)[1]
                )
            elif config["IsCollision"]:
                if config["DataType"] == "ESD":
                    # from CollisionCalibTemplate import collisioncalib
                    # jO=collisioncalib(config,ifiles,calibconstants,caltag.split("_",1)[1],calsettings2)
                    from RAWCalibTemplate import collisioncalib

                    jO = collisioncalib(
                        config,
                        ifiles,
                        calibconstants,
                        caltag.split("_", 1)[1],
                        calsettings2,
                    )
                if config["DataType"] == "RAW":
                    from RAWCalibTemplate import collisioncalib

                    jO = collisioncalib(
                        config,
                        ifiles,
                        calibconstants,
                        caltag.split("_", 1)[1],
                        calsettings2,
                    )
            elif config["IsHeavyIons"]:
                if config["DataType"] == "ESD":
                    from CollisionCalibTemplate import collisioncalib

                    jO = collisioncalib(
                        config,
                        ifiles,
                        calibconstants,
                        caltag.split("_", 1)[1],
                        calsettings2,
                    )
                if config["DataType"] == "RAW":
                    from RAWCalibTemplate import collisioncalib

                    jO = collisioncalib(
                        config,
                        ifiles,
                        calibconstants,
                        caltag.split("_", 1)[1],
                        calsettings2,
                    )
            else:
                from CosmicCalibTemplate import cosmiccalib

                jO = cosmiccalib(
                    config,
                    ifiles,
                    calibconstants,
                    caltag.split("_", 1)[1],
                    calsettings2,
                )
        else:
            ostring += "./ErrorsOptimization tracktuple.root " + caltag + "\n"

    elif config["IsCosmic"]:
        jO = cosmic(config, ifiles, calibconstants, config["NumEvents"][oiter + 1])
    elif config["IsCosmicMC"]:
        print("COSMIC MC!!!!")
        from CosmicMCTemplate import cosmicMC

        jO = cosmicMC(config, inputfiles, calibconstants)
    elif config["IsSingleBeam"]:
        from SingleBeamTemplate import singlebeam

        jO = singlebeam(config, inputfiles, calibconstants)
    elif config["IsCollision"]:
        if config["DataType"] == "ESD":
            from CollisionTemplate import collision

            jO = collision(
                config, ifiles, calibconstants, config["NumEvents"][oiter + 1]
            )
        if config["DataType"] == "RAW":
            #            from IDTracksTemplate import idtracks
            #            jO=idtracks(config,inputfiles,calibconstants,config["NumEvents"][oiter+1])
            from RAWTemplate import collision

            jO = collision(
                config, ifiles, calibconstants, config["NumEvents"][oiter + 1]
            )
    elif config["IsHeavyIons"]:
        if config["DataType"] == "ESD":
            from HITemplate import collision

            jO = collision(
                config, ifiles, calibconstants, config["NumEvents"][oiter + 1]
            )
        if config["DataType"] == "RAW":
            from RAWHITemplate import collision

            jO = collision(
                config, ifiles, calibconstants, config["NumEvents"][oiter + 1]
            )

    else:
        if config["DataType"] == "ESD":
            from ESDTemplate import ESD

            jO = ESD(inputfiles, calibconstants)
        if config["DataType"] == "RAW":
            from RAWTemplate import RAW

            jO = RAW(inputfiles, calibconstants, config)

    # add them to the script
    if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
        ostring += "cat > joboptions.py  <<EOF\n"
        ostring += jO + "\nEOF\n"

    # run athena
    if iscalib:
        if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
            # ostring+="athena joboptions.py | tee log\n"
            #  ostring+="athena joboptions.py 2>&1\n"
            if config["IsCosmic"]:

                ostring += (
                    "Reco_tf.py --inputBSFile=%s --beamType='cosmics' --autoConfiguration='everything' --postExec='from IOVDbSvc.CondDB import conddb;conddb.addOverride(\"/ Indet/ TrkErrorScaling\",\"TrkErrorScaling_R2_M6\");InDetFlags.useBroadClusterErrors.set_Value_and_Lock(False);' --preExec='rec.doTrigger.set_Value_and_Lock(False);rec.doTile.set_Value_and_Lock(False);rec.doEgamma.set_Value_and_Lock(False);rec.doTau.set_Value_and_Lock(False);rec.doZdc.set_Value_and_Lock(False);rec.doLucid.set_Value_and_Lock(False);rec.doCalo.set_Value_and_Lock(False);rec.doLArg.set_Value_and_Lock(False);rec.doJetMissingETTag.set_Value_and_Lock(False);rec.doMuon.set_Value_and_Lock(False);rec.doMuonCombined.set_Value_and_Lock(False);from CaloRec.CaloCellFlags import jobproperties;jobproperties.CaloCellFlags.doLArHVCorr=False;jobproperties.CaloCellFlags.doPileupOffsetBCIDCorr.set_Value_and_Lock(False);from InDetRecExample.InDetJobProperties import InDetFlags;from TrigHLTMonitoring.HLTMonFlags import HLTMonFlags;HLTMonFlags.doBphys=False;DQMonFlags.enableLumiAccess=False;InDetFlags.doInnerDetectorCommissioning.set_Value_and_Lock(True);DQMonFlags.doStreamAwareMon=False;from JetRec.JetRecFlags import jetFlags;jetFlags.useTracks=False;larCondFlags.OFCShapeFolder.set_Value_and_Lock(\"\");InDetFlags.ForceCoraCool=True;from MuonDQAMonFlags.MuonDQAProperFlags import MuonDQADetFlags;MuonDQADetFlags.doMDTTGCL1Mon.set_Value_and_Lock(False);' --conditionsTag=%s --maxEvents=%s --geometryVersion=%s --outputESDFile=ESD.test.root --postInclude='joboptions.py'\n"
                    % (ifiles[0], config["GLtag"], "1", config["DetDescVer"])
                )

            elif config["DataType"] == "RAW":

                ostring += (
                    "Reco_tf.py --inputBSFile=%s --conditionsTag=%s --geometryVersion=%s --maxEvents %s --beamType='collisions' --ignoreErrors=True --autoConfiguration='everything' --preExec 'rec.doExpressProcessing.set_Value_and_Lock(True);from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags;TrkDetFlags.TRT_BuildStrawLayers=True;rec.doTau=False;rec.doMuonCombined=False;rec.doJetMissingETTag=False;rec.doForwardDet=False;rec.doCalo=False;rec.doMuon = False;rec.doTrigger =False;rec.doEgamma=False;jetFlags.Enabled=False;InDetFlags.doPerfMon=False;InDetFlags.doMonitoringAlignment=False;InDetFlags.doMonitoringGlobal=False;InDetFlags.doMonitoringSCT=False;InDetFlags.doMonitoringPixel=False;InDetFlags.doCaloSeededAmbi.set_Value_and_Lock(False);rec.doCaloRinger.set_Value_and_Lock(False);'   --outputESDFile=ESD.test.root --postInclude='default:HIRecExample/trig_outputPostExec.py,RecJobTransforms/UseFrontier.py,joboptions.py' \n"
                    % (ifiles[0], config["GLtag"], config["DetDescVer"], 1)
                )

            else:
                ostring += "athena joboptions.py 2>&1\n"

    else:
        if config["SingleFile"]:
            ostring += "athena joboptions.py 2>&1\n"
        else:
            if config["DataType"] == "RAW":
                filetest = ""
                count = 0
                for ii in ifiles:
                    filetest += ii
                    count += 1
                    if count < len(ifiles):
                        filetest += ","
                if config["IsCosmic"]:

                    ostring += "Reco_tf.py --inputBSFile=%s --beamType='cosmics' --ignoreErrors=True  --autoConfiguration='everything' --postExec='from IOVDbSvc.CondDB import conddb;conddb.addOverride(\"/ Indet/ TrkErrorScaling\",\"TrkErrorScaling_R2_M6\");InDetFlags.useBroadClusterErrors.set_Value_and_Lock(False);' --preExec='rec.doTrigger.set_Value_and_Lock(False);rec.doTile.set_Value_and_Lock(False);rec.doEgamma.set_Value_and_Lock(False);rec.doTau.set_Value_and_Lock(False);rec.doZdc.set_Value_and_Lock(False);rec.doLucid.set_Value_and_Lock(False);rec.doCalo.set_Value_and_Lock(False);rec.doLArg.set_Value_and_Lock(False);rec.doJetMissingETTag.set_Value_and_Lock(False);rec.doMuon.set_Value_and_Lock(False);rec.doMuonCombined.set_Value_and_Lock(False);from CaloRec.CaloCellFlags import jobproperties;jobproperties.CaloCellFlags.doLArHVCorr=False;jobproperties.CaloCellFlags.doPileupOffsetBCIDCorr.set_Value_and_Lock(False);from InDetRecExample.InDetJobProperties import InDetFlags;from TrigHLTMonitoring.HLTMonFlags import HLTMonFlags;HLTMonFlags.doBphys=False;DQMonFlags.enableLumiAccess=False;InDetFlags.doInnerDetectorCommissioning.set_Value_and_Lock(True);DQMonFlags.doStreamAwareMon=False;from JetRec.JetRecFlags import jetFlags;jetFlags.useTracks=False;larCondFlags.OFCShapeFolder.set_Value_and_Lock(\"\");InDetFlags.ForceCoraCool=True;from MuonDQAMonFlags.MuonDQAProperFlags import MuonDQADetFlags;MuonDQADetFlags.doMDTTGCL1Mon.set_Value_and_Lock(False);' --conditionsTag=%s --maxEvents=%s --geometryVersion=%s --outputESDFile=ESD.test.root --postInclude='joboptions.py'\n" % (
                        filetest,
                        config["GLtag"],
                        config["NumEvents"][oiter + 1],
                        config["DetDescVer"],
                    )

                else:
                    ostring += "Reco_tf.py --inputBSFile=%s --conditionsTag=%s --geometryVersion=%s --maxEvents %s --beamType='collisions' --ignoreErrors=True --autoConfiguration='everything' --preExec 'rec.doExpressProcessing.set_Value_and_Lock(True);from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags;TrkDetFlags.TRT_BuildStrawLayers=True;rec.doTau=False;rec.doMuonCombined=False;rec.doJetMissingETTag=False;rec.doForwardDet=False;rec.doCalo=False;rec.doMuon = False;rec.doTrigger =False;rec.doEgamma=False;jetFlags.Enabled=False;InDetFlags.doPerfMon=False;InDetFlags.doMonitoringAlignment=False;InDetFlags.doMonitoringGlobal=False;InDetFlags.doMonitoringSCT=False;InDetFlags.doMonitoringPixel=False;InDetFlags.doCaloSeededAmbi.set_Value_and_Lock(False);rec.doCaloRinger.set_Value_and_Lock(False);'   --outputESDFile=ESD.test.root --athenaopts='--threads=8' --postInclude='default:HIRecExample/trig_outputPostExec.py,RecJobTransforms/UseFrontier.py,joboptions.py' \n" % (
                        filetest,
                        config["GLtag"],
                        config["DetDescVer"],
                        config["NumEvents"][oiter + 1],
                    )

            else:
                ostring += "athena joboptions.py 2>&1\n"

        # ostring+="grep 'Number of events' log\n"

    # copy the output
    if config["GetConstants"]:
        ostring += (
            "cp caliboutput.txt "
            + config["Batchdir"]
            + "/input/calibout_start.txt"
            + "\n"
        )
    else:
        if iscalib:
            if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
                ostring += (
                    cpcmd
                    + " calibout.root "
                    # + "root://eosatlas.cern.ch/"
                    + outputfile
                    + ".root\n"
                )
                ostring += (
                    cpcmd
                    + " calibout_rt.txt "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_rt.txt\n"
                )
                ostring += (
                    cpcmd
                    + " calibout_binrt.txt "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_binrt.txt\n"
                )
                ostring += (
                    cpcmd
                    + " calibout_t0.txt "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_t0.txt\n"
                )
#                ostring += (
#                    cpcmd
#                    + " calibout_dict.txt "
#                    # +"root://eosatlas.cern.ch/"
#                    + outputfile
#                    + "_dict.txt\n"
#                )
# The above file is not found. Try instead:
#                ostring += (
#                    cpcmd
#                    + " dictconst.txt "
#                    # +"root://eosatlas.cern.ch/"
#                    + outputfile
#                    + "_dict.txt\n"
# The above file is not found either.
#                )
                ostring += (
                    cpcmd
                    + " calib_constants_out.txt "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_const.txt\n"
                )
                # ostring+=cpcmd + " log root://eosatlas.cern.ch/"+outputfile+"_log.txt\n"
                if oiter == -1:
                    ostring += (
                        "cp caliboutput.txt "
                        + config["Batchdir"]
                        + "/input/calibout_start.txt\n"
                    )
                # ostring+="cp ntuple.pmon.gz "+outputfile+"_pmon.gz\n"
            else:
                ostring += (
                    cpcmd
                    + " errorsout.root       "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + ".root\n"
                )
                ostring += (
                    cpcmd
                    + " errors_rt.txt        "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_rt.txt\n"
                )
                ostring += (
                    cpcmd
                    + " errors_rt.ps         "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + ".ps\n"
                )
        else:
            if float(config["DoShift"]) > 0:
                ostring += (
                    cpcmd
                    + " basic.root "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_basic_shifted_%.1f.root.bz2\n" % t0shift
                )
                ostring += "root -b -q  basic.root '%s/graph_points.C(%f,%d)'\n" % (
                    config["Calibdir"],
                    t0shift,
                    oiter + 1,
                )
                ostring += "cp -fv gp_* " + config["Batchdir"] + "/temp/.\n"
                if t0shift == 0:
                    ostring += "cp -fv shiftres_* " + config["Batchdir"] + "/temp/.\n"
                ostring += (
                    "cp -fv basic.root "
                    + config["Batchdir"]
                    + "/temp/basic_shifted_%02d_%.2f.root\n" % (oiter + 1, t0shift)
                )
            else:
                ostring += "./bhadd dumfile basic.root\n"
                ostring += "bzip2 -v basic.root\n"
                ostring += (
                    cpcmd
                    + " basic.root.bz2 "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_basic.root.bz2\n"
                )
                ostring += (
                    cpcmd
                    + " dumfile.stat "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_basic.stat\n"
                )
                ostring += "cat TRT_StrawStatusOutput*ewFormat.txt  >> TRT_StrawStatusOutput.txt\n"
                #we skip DCS analysis for now (PH)
                #ostring += "cat TRT_StrawStatusOutput*Voltage_trips.txt  >> TRT_StrawLVStatusOutput.txt\n"
                ostring += (
                    cpcmd
                    + " TRT_StrawStatusOutput.txt "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_TRT_StrawStatusOutput.txt\n"
                )
                #we skip DCS analysis for now (PH)
                #ostring += (
                    #cpcmd
                    #+ " TRT_StrawLVStatusOutput.txt "
                    # +"root://eosatlas.cern.ch/"
                    #+ outputfile
                    #+ "_TRT_StrawLVStatusOutput.txt\n"
                #)
                ostring += (
                    cpcmd
                    + " tracktuple.root "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_tracktuple.root\n"
                )
                # ostring+=cpcmd + " log root://eosatlas.cern.ch/"+outputfile+"_atlog.txt\n"

        # ostring+="cp monitoring.root "+outputfile+"_monitoring.root\n"

        # ostring+="cp log "+outputfile+"_athenalog.txt\n"

    ostring += "ls -al\n"

    return ostring


def merge(config, inputfiles, outputfile, mergecmd, oiter):
    ostring = "export HOME=" + config["Calibdir"].split("testarea")[0] + "\n"

    # ostring += "cd $WORKDIR\n"
    ostring += "echo $WORKDIR\n"
    ostring += "shopt -s expand_aliases\n"
    ostring += "export STAGE_SVCCLASS=atlcal\n"
    ostring += config["SetupCmdRec"] + "\n"
    ostring += "echo\n"

    # copy precision constants for bhadd to work
    ostring += "cp -fv " + config["Batchdir"] + "/dbconst.txt precision_constants.txt\n"

    cpcmd = "cp -fv "
    if config["TempOnCastor"]:
        cpcmd = "xrdcp -f "

    # copy inputfile to local area
    for i in inputfiles:
        if config["CleanRRoot"] and not config["TempOnCastor"]:
            ostring += "mv -v " + i.split(".root")[0] + "*.root.bz2 .\n"
            ostring += "mv -v " + i.split(".root")[0] + "_tracktuple.root .\n"
            if oiter == 0:
                ostring += (
                    "mv -v " + i.split(".root")[0] + "_TRT_StrawStatusOutput.txt .\n"
                )
        else:
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + i.split(".root")[0]
                + "_basic.root.bz2 .\n"
            )
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + i.split(".root")[0]
                + "_tracktuple.root .\n"
            )
            if oiter == 0:
                ostring += (
                    cpcmd
                    # + " root://eosatlas.cern.ch/"
                    + i.split(".root")[0]
                    + "_TRT_StrawStatusOutput.txt .\n"
                )

    if config["UseHist"]:
        # if (oiter>0): ostring+="ln -sf " + config["Batchdir"] + "/input/calibout_%02d.txt precision_constants.txt\n"%(oiter-1)
        ostring += "ln -sf " + config["Calibdir"] + "/bhadd.cpp\n"
        ostring += "ln -sf " + config["Calibdir"] + "/TRT_StrawStatus_merge.cc\n"
        ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
        ostring += "make bhadd\n"
        ostring += "make TRT_StrawStatus_merge\n"
    else:  # TEST
        ostring += (
            "ln -sf "
            + config["Batchdir"].split("manual")[0]
            + "manual/hadd/getentries.cpp\n"
        )  # "/afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/getentries.cpp\n"
        ostring += (
            "ln -sf " + config["Batchdir"].split("manual")[0] + "manual/hadd/Makefile\n"
        )  # "/afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/Makefile\n"
        ostring += "make getentries\n"
    #ostring += (
    #    "cp -fv "
    #    + config["Batchdir"].split("manual")[0]
    #    + "manual/hadd/hadd2 .\n"  # "/afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/hadd2 .\n"
    #)
    ostring += ( "cp -fv /afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/hadd2 .\n")

    # ENDTEST
    ostring += "bunzip2 -v *.bz2\n"
    ostring += "ls -al\n"

    if config["UseHist"]:
        ostring += "./bhadd merged_basic.root trtcalib*basic*.root\n"
        ostring += "hadd  merged_tracktuple.root trtcalib*tracktuple.root\n"
        if oiter == 0:
            ostring += "./TRT_StrawStatus_merge merged_strawstatus trtcalib*TRT_StrawStatusOutput.txt\n"
    else:
        ostring += "hadd merged_basic.root trtcalib*basic*.root\n"
        ostring += "./getentries merged_basic.root > merged_basic.root.stat\n"
        if oiter == 0:
            ostring += "./TRT_StrawStatus_merge merged_strawstatus trtcalib*TRT_StrawStatusOutput.txt\n"

    ostring += "bzip2 -v merged_basic.root.part*\n"
    ostring += """ls -1 trtcalib*.root | awk '{printf "SOURCEFILE %s\\n",$1}' >>  merged_basic.root.stat\n"""

    if outputfile.find("output") >= 0:
        for calarg in ["_all"]:
            ostring += (
                cpcmd
                + " merged_basic.root.part0.bz2 "
                # +"root://eosatlas.cern.ch/"
                + outputfile
                + "_basic"
                + calarg
                + ".root.bz2\n"
            )
            ostring += (
                cpcmd
                + " merged_basic.root.stat "
                # +"root://eosatlas.cern.ch/"
                + outputfile
                + "_basic"
                + calarg
                + ".stat\n"
            )
            ostring += (
                cpcmd
                + " merged_tracktuple.root "
                # +"root://eosatlas.cern.ch/"
                + outputfile
                + "_tracktuple.root\n"
            )
            if oiter == 0:
                ostring += (
                    cpcmd
                    + " merged_strawstatus "
                    # +"root://eosatlas.cern.ch/"
                    + outputfile
                    + "_TRT_StrawStatusOutput.txt\n"
                )
    else:
        ostring += (
            cpcmd
            + " merged_basic.root.part0.bz2 "
            # +"root://eosatlas.cern.ch/"
            + outputfile
            + "_basic.root.bz2\n"
        )
        ostring += (
            cpcmd
            + " merged_basic.root.stat "
            # +"root://eosatlas.cern.ch/"
            + outputfile
            + "_basic.stat\n"
        )
        ostring += (
            cpcmd
            + " merged_tracktuple.root "
            # +"root://eosatlas.cern.ch/"
            + outputfile
            + "_tracktuple.root\n"
        )
        if oiter == 0:
            ostring += (
                cpcmd
                + " merged_strawstatus "
                # + "root://eosatlas.cern.ch/"
                + outputfile
                + "_TRT_StrawStatusOutput.txt\n"
            )
        #stat.stat does not exist
        #ostring += (
        #    cpcmd
        #    + " stat.stat "
        #    +"root://eosatlas.cern.ch/"
        #    + outputfile
        #    + "_basic.stat\n"
        #)

    # copy back outputfile
    if outputfile.find("castor:") >= 0:
        ostring += (
            "xrdcp -f trtcalib.root root://eosatlas.cern.ch/"
            + outputfile.split(":")[1]
            + "\n"
        )

    ostring += "ls -al\n"

    return ostring


def pcalib(config, inputfile, calibcmd, caltag, calsetting, optstring):
    ostring = "export HOME=" + config["Calibdir"].split("testarea")[0] + "\n"

    # ostring += "cd $WORKDIR\n"
    ostring += "echo $WORKDIR\n"
    ostring += "shopt -s expand_aliases\n"
    ostring += "pwd \n"
    ostring += config["SetupCmdCal"] + "\n"

    cpcmd = "cp -fv "
    if config["TempOnCastor"]:
        cpcmd = "xrdcp -f"

        filename = (
            inputfile.split(".root")[0].split("/")[
                len(inputfile.split(".root")[0].split("/")) - 1
            ]
            + "_calib"
        )

    if caltag.find("all") >= 0:
        ostring += (
            cpcmd
            + " -fv "
            + inputfile
            + "_basic"
            + caltag
            + ".root.bz2 merged.root.bz2\n"
        )
    else:
        ostring += cpcmd + " -fv " + inputfile + "_basic_all.root.bz2 merged.root.bz2\n"

    ostring += "bunzip2 -v *.bz2\n"
    ostring += "ln -sf " + config["Calibdir"] + "/finedelays.txt\n"
    ostring += "ln -sf " + config["Calibdir"] + "/CalibrateTRT.cpp\n"
    ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
    ostring += "make CalibrateTRT\n"
    ostring += (
        "./CalibrateTRT "
        + calsetting
        + " "
        + optstring
        + " "
        + config["MinRT"]
        + " "
        + config["MinT0"]
        + " "
        + config["RtRelation"]
        + " merged.root -1 | tee log.txt\n"
    )
    ostring += "grep -Ev '1065353216' calibout_t0.txt > calibout_t0.txt.tmp; diff calibout_t0.txt calibout_t0.txt.tmp >> log.txt; echo T0 >> log.txt\n"
    ostring += "grep -Ev '0 0 0 0 0' calibout_t0.txt.tmp > calibout_t0.txt.cleaned; diff calibout_t0.txt.tmp calibout_t0.txt.cleaned >> log.txt; echo T0 >> log.txt\n"
    ostring += "grep -Ev '1065353216' calibout_rt.txt > calibout_rt.txt.tmp; diff calibout_rt.txt calibout_rt.txt.tmp >> log.txt; echo RT >> log.txt\n"
    ostring += "grep -Ev '0 0 0 0 0' calibout_rt.txt.tmp > calibout_rt.txt.cleaned; diff calibout_rt.txt.tmp calibout_rt.txt.cleaned >> log.txt; echo RT >> log.txt\n"
    ostring += (
        "cp calibout.root " + config["TempDir"] + "/" + filename + caltag + ".root\n"
    )
    ostring += (
        "cp calibout_t0.txt.cleaned "
        + config["TempDir"]
        + "/"
        + filename
        + caltag
        + "_t0.txt\n"
    )
    ostring += (
        "cp calibout_rt.txt.cleaned "
        + config["TempDir"]
        + "/"
        + filename
        + caltag
        + "_rt.txt\n"
    )
    ostring += "cp log.txt " + config["TempDir"] + "/" + filename + caltag + ".log\n"
    #ostring += "source /cvmfs/atlas.cern.ch/repo/sw/software/21.0/sw/lcg/releases/LCG_88/ROOT/6.08.06/x86_64-centos7-gcc62-opt/bin/thisroot.sh; \n"
    ostring += "cp -v $ROOTSYS/bin/thisroot.sh .;chmod u+x thisroot.sh;source thisroot.sh; \n"

    return ostring


# __cnv.sh
def txt2db(
    config, calibconstants, pooloutput, dboutput, caltags, couttags, oiter, batchdir
):

    # merges the calibration output, and/or generates a summary pdf file
    ostring = "export HOME=" + config["Calibdir"].split("testarea")[0] + "\n"

    # ostring += "cd $WORKDIR\n"

    ostring += "echo $WORKDIR\n"
    ostring += "shopt -s expand_aliases\n"
    ostring += "pwd \n"
    ostring += "export STAGE_SVCCLASS=atlcal\n"
    ostring += config["SetupCmdRec"] + "\n"
    #ostring += "source /cvmfs/atlas.cern.ch/repo/sw/software/21.0/sw/lcg/releases/LCG_88/ROOT/6.08.06/x86_64-centos7-gcc62-opt/bin/thisroot.sh  \n"
    ostring += "cp -v $ROOTSYS/bin/thisroot.sh .;chmod u+x thisroot.sh;source thisroot.sh; \n"

    cpcmd = "cp -fv "
    if config["TempOnCastor"]:
        cpcmd = "xrdcp -f root://eosatlas.cern.ch/"

    if config["MakePlots"]:
        ostring += "#JUST REMAKE THE PLOTS\n"
        ostring += "\n#copy calibration root files & makeplots.cpp\n"
        ostring += (
            "cp -uv "
            + batchdir
            + "/output/trtcalib_%02d_histograms.root .\n" % (oiter + 1)
        )
        ostring += "ln -sf " + config["Calibdir"] + "/makeplots2.cpp\n"
        ostring += "ln -sf " + config["Calibdir"] + "/Makefile\n"
        ostring += "\n#compile makeplots\n"
        ostring += "make makeplots2\n"
        ostring += "\n#run makeplots\n"
        ostring += (
            "./makeplots2 itersum trtcalib_%02d_histograms.root " % (oiter + 1)
            + os.path.abspath(".")
            + "/lastconfigfile "
            + os.path.abspath(".")
            + "/batch/output/trtcalib_00_histograms.root\n"
        )
        ostring += "\n#exit if something went wrong\n"
        ostring += 'if [ "$?" -ne "0" ]; then\n'
        ostring += "  echo 'RETURNCODE=' $? ', EXITING!, NO FILES COPIED!'\n"
        ostring += "  exit 1\n"
        ostring += "fi\n"
        ostring += "\n#copy back the files\n"
        ostring += (
            "cp -fv trtcalib_%02d_histograms.root " % (oiter + 1)
            + batchdir
            + "/output\n"
        )
        ostring += "\n#add page numbers\n"
        ostring += """echo 'import os,sys' > addnum.py\n"""
        ostring += (
            """echo 'for line in open("itersum.ps").readlines():' >> addnum.py\n"""
        )
        ostring += """echo '    if line.find("%%Page")>=0:' >> addnum.py\n"""
        ostring += """echo '        pagenum=line.split("%%Page: ")[-1].split()[0]' >> addnum.py\n"""
        ostring += """echo '        print line.replace("%%Page: %s %s"%(pagenum,pagenum),"%%Page: %s %s \\n/Helvetica-Bold findfont 36 scalefont setfont 0 -40 moveto (Page %s) show"%(pagenum,pagenum,pagenum)).strip()' >> addnum.py\n"""
        ostring += """echo '    else:' >> addnum.py\n"""
        ostring += """echo '        print line.strip();' >> addnum.py\n"""
        ostring += "python addnum.py > itersum_new.ps\n"
        ostring += "\n#create pdf\n"
        ostring += (
            "ps2pdf itersum_new.ps "
            + batchdir
            + "/output/itersum_%02d.pdf\n" % (oiter + 1)
        )

        return ostring

    ostring += "##IF THE FILES ARE FOUND IN THE TEMP DIRECTORY DO THE MERGING\n"
    ostring += "#if [ -f %s ]; then\n" % (
        config["TempDir"] + "/trtcalib_%02d_calib" % (oiter + 1) + "_all.root"
    )

    ostring += "\n  #copy calibration root files & makeplots.cpp\n"
    if config["CleanCRoot"]:
        ostring += (
            "  mv -v "
            + config["TempDir"]
            + "/trtcalib_%02d_calib" % (oiter + 1)
            + "*.root .\n"
        )
    else:
        for caltag in couttags:
            if caltag != "_barrel":
                ostring += (
                    "  "
                    + cpcmd
                    + " "
                    + config["TempDir"]
                    + "/trtcalib_%02d_calib" % (oiter + 1)
                    + caltag
                    + ".root .\n"
                )

    ostring += (
        "  "
        + cpcmd
        + " "
        + config["TempDir"]
        + "/trtcalib_%02d_tracktuple" % (oiter + 1)
        + ".root .\n"
    )

    # Add straw status plots:

    ostring += (
        "  "
        + cpcmd
        + config["TempDir"]
        + "/trtcalib_%02d_tracktuple" % (oiter + 1)
        + ".root .\n"
    )
    ostring += "  ln -sf " + config["Calibdir"] + "/makeplots.cpp\n"
    ostring += "  ln -sf " + config["Calibdir"] + "/Makefile\n"

    #ostring += (
    #    "   cp -fv " + config["Batchdir"].split("manual")[0] + "manual/hadd/hadd2 .\n"
    #)  # "  cp -fv /afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/hadd2 .\n"
    ostring += ( "  cp -fv /afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/hadd2 .\n")

    ostring += "\n  #compile makeplots\n"
    ostring += "  make makeplots\n"
    ostring += "  ls -al\n"

    ostring += "\n  #merge calibration and the tracktuple root files\n"
    ostring += "  echo MERGING CALIBRATION OUTPUTS AND TRACKTUPLE!\n"
    ostring += (
        "  hadd -a merged_%02d_histograms.root trtcalib_%02d_calib*.root trtcalib_%02d_tracktuple.root > hadd2.txt\n"
        % (oiter + 1, oiter + 1, oiter + 1)
    )

    ostring += "\n  #run makeplots\n"
    if int(oiter) == -1:
        ostring += (
            "  ./makeplots itersum merged_%02d_histograms.root " % (oiter + 1)
            + os.path.abspath(".")
            + "/lastconfigfile merged_%02d_histograms.root\n" % (oiter + 1)
        )
    else:
        ostring += (
            "  ./makeplots itersum merged_%02d_histograms.root " % (oiter + 1)
            + os.path.abspath(".")
            + "/lastconfigfile "
            + os.path.abspath(".")
            + "/batch/output/trtcalib_00_histograms.root\n"
        )

    ostring += "\n  #exit if something went wrong\n"
    ostring += '  if [ "$?" -ne "0" ]; then\n'
    ostring += "    echo 'RETURNCODE NOT =0 EXITING!, NO FILES COPIED!'\n"
    ostring += "    exit 1\n"
    ostring += "  fi\n"

    ostring += "\n  #copy back the files\n"
    ostring += (
        "  cp -fv merged_%02d_histograms.root " % (oiter + 1)
        + batchdir
        + "/output/trtcalib_%02d_histograms.root\n" % (oiter + 1)
    )
    ostring += "\n  #add page numbers\n"
    ostring += """  echo 'import os,sys' > addnum.py\n"""
    ostring += """  echo 'for line in open("itersum.ps").readlines():' >> addnum.py\n"""
    ostring += """  echo '    if line.find("%%Page")>=0:' >> addnum.py\n"""
    ostring += """  echo '        pagenum=line.split("%%Page: ")[-1].split()[0]' >> addnum.py\n"""
    ostring += """  echo '        print line.replace("%%Page: %s %s"%(pagenum,pagenum),"%%Page: %s %s \\n/Helvetica-Bold findfont 36 scalefont setfont 0 -40 moveto (Page %s) show"%(pagenum,pagenum,pagenum)).strip()' >> addnum.py\n"""
    ostring += """  echo '    else:' >> addnum.py\n"""
    ostring += """  echo '        print line.strip();' >> addnum.py\n"""
    ostring += "  python addnum.py > itersum_new.ps\n"

    ostring += "\n  #create pdf\n"
    ostring += (
        "  ps2pdf itersum_new.ps "
        + batchdir
        + "/output/itersum_%02d.pdf\n" % (oiter + 1)
    )
    ostring += (
        "  cat finedelays_-1.txt >> "
        + batchdir
        + "/output/finedelays_%02i.txt\n" % (oiter + 1)
    )
    ostring += (
        "  cat finedelays_1.txt >> "
        + batchdir
        + "/output/finedelays_%02i.txt\n" % (oiter + 1)
    )

    ostring += "\n  #send a status mail\n"
    if oiter + 1 == 0:
        # Straw status only for 1st iter
        ostring += "\n### Do the Straw Status plotting: \n"
        ostring += (
            "  "
            + cpcmd
            + " "
            + config["TempDir"]
            + "/trtcalib_%02d_TRT_StrawStatusOutput.txt" % (oiter + 1)
            + " TRT_StrawStatusOutput.txt\n"
        )
        ostring += "  ln -sf " + config["Calibdir"] + "/TRT_StrawStatusReport.cc \n"
        ostring += "  ln -sf " + config["Calibdir"] + "/TRT_StrawMap.h \n"
        ostring += "  ln -sf " + config["Calibdir"] + "/TRT_StrawMap.txt \n"
        ostring += "  make TRT_StrawStatusReport\n"
        ostring += "  mkdir output \n"
        ostring += "  cp TRT_StrawStatusOutput.txt straws.276073.txt\n"
        ostring += "  ./TRT_StrawStatusReport 276073 \n"
        ostring += (
            "  root -l .x " + config["Calibdir"] + "/TRT_StrawStatusReport.C -q -b \n"
        )
        ostring += (
            "  cp allPlots.pdf "
            + batchdir
            + "/output/TRT_StrawStatusPlots_%02i.pdf\n" % (oiter + 1)
        )
        ostring += (
            "  cp /afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/uploadedDB/testareas/testarea_20.1.4.9/StatusTest/StrawsToMaskTmp.txt "
            + batchdir
            + "/output/athenaFormat_runDependentInactiveStraws_%02i.txt\n" % (oiter + 1)
        )

    ostring += "\n##ELSE COPY THE FILES FROM THE OUTPUT DIRECTORY AND UPDATE THEM\n"
    ostring += "#else\n"
    ostring += "\n  ##copy calibration root files & makeplots.cpp\n"
    ostring += (
        "  #cp -fv "
        + batchdir
        + "/output/trtcalib_%02d_histograms.root .\n" % (oiter + 1)
    )
    ostring += (
        "  #cp -fv "
        + batchdir
        + "/output/trtcalib_%02d_tracktuple.root .\n" % (oiter + 1)
    )
    ostring += "  #ln -sf " + config["Calibdir"] + "/makeplots.cpp\n"
    ostring += "  #ln -sf " + config["Calibdir"] + "/Makefile\n"
    ostring += "  #cp -fv /afs/cern.ch/user/i/idcalib/w0/TRT_Calibration/testarea/hadd/hadd2 .\n"

    ostring += "\n  ##compile makeplots\n"
    ostring += "  #make makeplots\n"

    ostring += "\n  ##remove the old tracktuple from the root file\n"
    ostring += "  #./makeplots restore trtcalib_%02d_histograms.root\n" % (oiter + 1)

    ostring += "\n  ##merge calibration and the new tracktuple root files\n"
    ostring += "  #echo MERGING CALIBRATION OUTPUTS AND TRACKTUPLE!\n"
    ostring += (
        "  #./hadd2 merged_%02d_histograms.root trtcalib_%02d_histograms.root trtcalib_%02d_tracktuple.root > hadd2.txt\n"
        % (oiter + 1, oiter + 1, oiter + 1)
    )

    ostring += "\n  ##run makeplots\n"
    ostring += (
        "  #./makeplots itersum merged_%02d_histograms.root " % (oiter + 1)
        + os.path.abspath(".")
        + "/lastconfigfile "
        + os.path.abspath(".")
        + "/batch/output/trtcalib_00_histograms.root\n"
    )

    ostring += "\n  ##exit if something went wrong\n"
    ostring += '  #if [ "$?" -ne "0" ]; then\n'
    ostring += "  #  echo 'RETURNCODE=' $? ', EXITING!, NO FILES COPIED!'\n"
    ostring += "  #  exit 1\n"
    ostring += "  #fi\n"

    ostring += "\n  ##copy back the files\n"
    ostring += (
        "  #cp -fv merged_%02d_histograms.root " % (oiter + 1)
        + batchdir
        + "/output/trtcalib_%02d_histograms.root\n" % (oiter + 1)
    )

    ostring += "\n  ##add page numbers\n"
    ostring += """  #echo 'import os,sys' > addnum.py\n"""
    ostring += (
        """  #echo 'for line in open("itersum.ps").readlines():' >> addnum.py\n"""
    )
    ostring += """  #echo '    if line.find("%%Page")>=0:' >> addnum.py\n"""
    ostring += """  #echo '        pagenum=line.split("%%Page: ")[-1].split()[0]' >> addnum.py\n"""
    ostring += """  #echo '        print line.replace("%%Page: %s %s"%(pagenum,pagenum),"%%Page: %s %s \\n/Helvetica-Bold findfont 36 scalefont setfont 0 -40 moveto (Page %s) show"%(pagenum,pagenum,pagenum)).strip()' >> addnum.py\n"""
    ostring += """  #echo '    else:' >> addnum.py\n"""
    ostring += """  #echo '        print line.strip();' >> addnum.py\n"""
    ostring += "  #python addnum.py > itersum_new.ps\n"

    ostring += "\n  ##create pdf\n"
    ostring += (
        "  #ps2pdf itersum_new.ps "
        + batchdir
        + "/output/itersum_%02d.pdf\n\n" % (oiter + 1)
    )

    ostring += "#fi\n"

    return ostring


# __mci.sh
def mkcalin(
    config, calibconstants, pooloutput, dboutput, caltags, couttags, oiter, batchdir
):
    ostring = "export HOME=" + config["Calibdir"].split("testarea")[0] + "\n"

    # ostring += "cd $WORKDIR\n"
    ostring += "echo $WORKDIR\n"
    ostring += "shopt -s expand_aliases\n"
    ostring += "pwd \n"
    ostring += "export STAGE_SVCCLASS=atlcal\n"
    ostring += config["SetupCmdRec"] + "\n"
    # tested numpy fix -- setupATLAS, then lsetup lcgenv numpy
    # doing setupATLAS on HTCondor:
    ostring += (
        "export ATLAS_LOCAL_ROOT_BASE=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\n"
    )
    ostring += "source ${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh\n"

    # installing numpy
    ostring += 'lsetup "lcgenv -p LCG_88 x86_64-centos7-gcc62-opt numpy" \n'

    ##
    cpcmd = "cp -fv "
    if config["TempOnCastor"]:
        cpcmd = "xrdcp -f "

    if config["SplitAC"]:
        excluded_caltags = ["_-1", "_1", "_-2", "_2", "_barrel"]
    else:
        excluded_caltags = [
            "_-1",
            "_1",
            "_-2",
            "_2",
            "_1_0",
            "_1_1",
            "_1_2",
            "_-1_0",
            "_-1_1",
            "_-1_2",
        ]
    couttags.sort()

    # copy straw status files for first iteration
    if oiter + 1 == 0:
        ostring += (
            cpcmd
            # + " root://eosatlas.cern.ch/"
            + " "
            + config["TempDir"]
            + "/trtcalib_%02d_TRT_StrawStatusOutput.txt" % (oiter + 1)
            + " %s/output/TRT_StrawStatusOutput.txt\n" % config["Batchdir"]
        )

    for caltag in couttags:
        if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + config["TempDir"]
                + "/trtcalib_%02d_calib" % (oiter + 1)
                + caltag
                + "_rt.txt .\n"
            )
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + config["TempDir"]
                + "/trtcalib_%02d_calib" % (oiter + 1)
                + caltag
                + "_t0.txt .\n"
            )
            # _const thing, unsure if needed (was commented out)
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + config["TempDir"]
                + "/trtcalib_%02d_calib" % (oiter + 1)
                + caltag
                + "_const.txt .\n"
            )
        else:
            ostring += (
                cpcmd
                # + " root://eosatlas.cern.ch/"
                + " "
                + config["TempDir"]
                + "/trtcalib_%02d_calib" % (oiter + 1)
                + caltag
                + "_rt.txt .\n"
            )

    # append the output files
    if config["DoErrorOptimization"]:
        ostring += "if [ -f %s ]; then\n" % (
            "trtcalib_%02d_calib" % (oiter + 1) + "_all_t0.txt"
        )
        ostring += "  echo '# Fileformat=2' >  calibout.txt\n"
        ostring += "  echo '# RtRelation' >>  calibout.txt\n"
        for caltag in couttags:
            if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
                if caltag not in excluded_caltags:
                    if config["RtRelation"] == "binned":
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_binrt.txt >> "
                            + "calibout.txt\n"
                        )
                    else:
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_rt.txt >> "
                            + "calibout.txt\n"
                        )
        ostring += "  echo '# errors' >>  calibout.txt\n"
        for caltag in couttags:
            if (caltag == "error_Barrel") | (caltag == "error_Endcap"):
                if caltag not in excluded_caltags:
                    if config["RtRelation"] == "binned":
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_binrt.txt >> "
                            + "calibout.txt\n"
                        )
                    else:
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_rt.txt >> "
                            + "calibout.txt\n"
                        )

        ostring += "  echo '# StrawT0' >>  calibout.txt\n"
        for caltag in couttags:
            if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
                if caltag not in excluded_caltags:
                    ostring += (
                        "  cat trtcalib_%02d_calib" % (oiter + 1)
                        + caltag
                        + "_t0.txt >> "
                        + "calibout.txt\n"
                    )
                    ostring += (
                        "  cat trtcalib_%02d_calib" % (oiter + 1)
                        + caltag
                        + "_const.txt >> "
                        + "oldt0s.txt\n"
                    )
        ostring += "  echo '#GLOBALOFFSET 0.0000' >>  calibout.txt\n"
    else:
        ostring += "if [ -f %s ]; then\n" % (
            "trtcalib_%02d_calib" % (oiter + 1) + "_all_t0.txt"
        )
        ostring += "  echo '# Fileformat=1' >  calibout.txt\n"
        ostring += "  echo '# RtRelation' >>  calibout.txt\n"
        for caltag in couttags:
            if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
                if caltag not in excluded_caltags:
                    if config["RtRelation"] == "binned":
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_binrt.txt >> "
                            + "calibout.txt\n"
                        )
                    else:
                        ostring += (
                            "  cat trtcalib_%02d_calib" % (oiter + 1)
                            + caltag
                            + "_rt.txt >> "
                            + "calibout.txt\n"
                        )
        ostring += "  echo '# StrawT0' >>  calibout.txt\n"
        for caltag in couttags:
            if not ((caltag == "error_Barrel") | (caltag == "error_Endcap")):
                if caltag not in excluded_caltags:
                    ostring += (
                        "  cat trtcalib_%02d_calib" % (oiter + 1)
                        + caltag
                        + "_t0.txt >> "
                        + "calibout.txt\n"
                    )
                    ostring += (
                        "  cat trtcalib_%02d_calib" % (oiter + 1)
                        + caltag
                        + "_const.txt >> "
                        + "oldt0s.txt\n"
                    )
        ostring += "  echo '#GLOBALOFFSET 0.0000' >>  calibout.txt\n"

    ostring += "fi\n\n"
    #    #For cosmic, normalize the t0, to keep it average constant
    if (config["MeanT0"] > 0) & ((config["IsCosmic"] | config["IsCosmicMC"])):
        filetocorrect = "%s_%02d.txt" % (config["CalPrefix"], oiter + 1)
        ostring += (
            "python "
            + config["Calibdir"]
            + "/NormalizeT0.py calibout.txt "
            + str(config["MeanT0"])
            + "\n\n"
        )

    # configure the filter for the calibration constants
    iter = ""
    filterargs = ""
    if oiter == -1:
        iter = "start"
    else:
        iter = "%02d" % oiter
    iterp1 = "%02d" % (oiter + 1)
    if config["DoT0"][oiter + 1] == "none":
        filterargs += "keept0+"  # no T0 calibration
    elif config["DoRt"][oiter + 1] == "none":
        filterargs += "keeprt+"  # no Rt calibration
    if not config["UsePol0"][oiter + 1]:
        # filterargs+='shiftt0+' #Shift T0s
        filterargs += "shiftrt+"  # Shift Rts

    # apply filter
    ostring += "ln -sf " + config["Calibdir"] + "/cfilter.py\n"
    ostring += (
        "python cfilter.py %s_%s.txt calibout.txt oldt0s.txt "
        % (config["CalPrefix"], iter)
        + filterargs.strip("+")
        + "\n\n"
    )

    # copy back files
    ostring += "cp -fv dbconst.txt %s_%02d.txt\n" % (config["CalPrefix"], oiter + 1)
# the file below is not found
#    ostring += (
#        cpcmd
#        + " dictconst.txt "  # root://eosatlas.cern.ch/"
#        + config["TempDir"]
#        + "/trtcalib_%02d_oldt0s.txt\n" % (oiter + 1)
#        + "/trtcalib_00_oldt0s.txt\n" # change name
#    )


    return ostring


def mail(config, oiter):
    return it_statusmail(config, oiter)
