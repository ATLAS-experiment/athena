"""Module to configure .sub files for a particular script, creating the script itself in the process.
The functions in this module are largely the same, and all return the name of the created .sub file.
These names are accumulated in a list (called subnames) in the main script, and are used in dag.py.

Authors: Various (adapted by Nathan Simpson)
Filename: scriptsub.py
Last modified: 13/08/2019
"""

import scriptgen, os, subprocess
from util import extract_const

logpath = ""  # configure if you want logs in a different directory! create said dir before running.


def mail(config, oiter):
    oiter = "0" + str(oiter)
    sname = "mail" + oiter + ".sh"

    with open(sname, "w+") as script:
        script.write("#!/bin/bash\n")
        script.write(scriptgen.mail(config, oiter))


#    with open(sname + ".sub", "w+") as sub:
#        sub.write(
#            "executable  = "
#            + sname
#            + "\n"
#            + "arguments   = $(ProcId)\n"
#            + "output      = "
#            + logpath
#            + "TRTcal_output/"
#            + sname
#            + ".$(ProcId).out\n"
#            + "error       = "
#            + logpath
#            + "TRTcal_error/"
#            + sname
#            + ".$(ProcId).err\n"
#            + "log         = "
#            + logpath
#            + "TRTcal_log/"
#            + "."
#            + ".$(ProcId).log\n"
#            + '+JobFlavour = "tomorrow"\n'
#            + 'transfer_output_files = ""\n'
#            + "queue"
#        )
#


def parallel(
    config,
    inputfiles,
    calibconstants,
    outputfile,
    sname,
    jname,
    waitlist,
    calsettings2,
    oiter,
    ijob,
):
    """Create and configure ntuple creation scripts & .sub files, to be run in parallel."""
    # Set up script
    outf = open(sname, "w")
    outf.write("#!/bin/bash\n")
    ostring = scriptgen.parallel(
        config,
        inputfiles,
        calibconstants,
        outputfile.split(".root")[0],
        False,
        "",
        "",
        oiter,
        calsettings2,
        ijob,
    )
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 " + sname)

    # Write submisssion file
    subFile = open(sname + ".sub", "w+")

    subFile.write(
        "executable  = "
        + sname
        + "\n"
        + "arguments   = $(ProcId)\n"
        + "output      = "
        + logpath
        + "TRTcal_output/"
        + sname
        + ".$(ProcId).out\n"
        + "error       = "
        + logpath
        + "TRTcal_error/"
        + sname
        + ".$(ProcId).err\n"
        + "log         = "
        + logpath
        + "TRTcal_log/"
        + "."
        + ".$(ProcId).log\n"
        + '+JobFlavour = "tomorrow"\n'
        + 'transfer_output_files = ""\n'
        + "queue"
    )

    subFile.close()

    subname = sname + ".sub"

    # would be sub 1111
    if config["SubmitRecJobs"]:
        return subname


def merge(
    config, inputfiles, outputfile, mergecmd, batchq, sname, jname, waitlist, oiter
):
    """Create and configure ntuple merging script & .sub file."""
    outf = open(sname, "w")
    outf.write("#!/bin/sh\n")
    ostring = scriptgen.merge(
        config, inputfiles, outputfile.split(".root")[0], mergecmd, oiter
    )
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 " + sname)

    # Write submisssion file
    subFile = open(sname + ".sub", "w+")

    subFile.write(
        "executable  = "
        + sname
        + "\n"
        + "arguments   = $(ProcId)\n"
        + "output      = "
        + logpath
        + "TRTcal_output/"
        + sname
        + ".$(ProcId).out\n"
        + "error       = "
        + logpath
        + "TRTcal_error/"
        + sname
        + ".$(ProcId).err\n"
        + "log         = "
        + logpath
        + "TRTcal_log/"
        + "."
        + ".$(ProcId).log\n"
        + '+JobFlavour = "tomorrow"\n'
        + 'transfer_output_files = ""\n'
        + "queue"
    )

    subFile.close()

    subname = sname + ".sub"

    if config["SubmitMergeJobs"]:
        return subname


def pcalib(
    config,
    inputfile,
    calibcmd,
    batchq,
    caltag,
    sname,
    jname,
    waitlist,
    calsetting,
    optstring,
    datafile,
    calibconst,
    oiter,
    calsettings2,
):
    """Create and configure parallel calibration scripts & .sub files. (for different layers)"""
    outf = open(sname, "w")
    outf.write("#!/bin/sh\n")
    ifile = []
    if config["DoAthenaCalib"]:
        #        if type(datafile)==TupleType:
        # Was previously 'datafile[1]'
        ifile.append(datafile)
        outfile = (
            config["TempDir"]
            + "/"
            + inputfile.split(".root")[0].split("/")[-1]
            + "_calib"
            + caltag
        )
        ostring = scriptgen.parallel(
            config,
            ifile,
            calibconst,
            outfile,
            True,
            inputfile.split(".root")[0],
            caltag,
            oiter,
            calsettings2,
            0,
        )
    else:
        ostring = scriptgen.pcalib(
            config, inputfile.split(".root")[0], calibcmd, caltag, calsetting, optstring
        )
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 " + sname)

    # Write submisssion file
    subFile = open(sname + ".sub", "w+")

    subFile.write(
        "executable  = "
        + sname
        + "\n"
        + "arguments   = $(ProcId)\n"
        + "output      = "
        + logpath
        + "TRTcal_output/"
        + sname
        + ".$(ProcId).out\n"
        + "error       = "
        + logpath
        + "TRTcal_error/"
        + sname
        + ".$(ProcId).err\n"
        + "log         = "
        + logpath
        + "TRTcal_log/"
        + "."
        + ".$(ProcId).log\n"
        + '+JobFlavour = "tomorrow"\n'
        + 'transfer_output_files = ""\n'
        + "queue"
    )

    subFile.close()

    subname = sname + ".sub"

    if config["SubmitCalJobs"]:
        return subname


def mkcalin(
    config,
    calibconstants,
    pooloutput,
    dboutput,
    sname,
    jname,
    waitlist,
    caltags,
    couttags,
    oiter,
    batchdir,
):
    """Create and configure 'mci' script & .sub file."""
    outf = open(sname, "w")
    outf.write("#!/bin/sh\n")
    ostring = scriptgen.mkcalin(
        config, calibconstants, pooloutput, dboutput, caltags, couttags, oiter, batchdir
    )
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 " + sname)

    # Write submisssion file
    subFile = open(sname + ".sub", "w+")

    subFile.write(
        "executable  = "
        + sname
        + "\n"
        + "arguments   = $(ProcId)\n"
        + "output      = "
        + logpath
        + "TRTcal_output/"
        + sname
        + ".$(ProcId).out\n"
        + "error       = "
        + logpath
        + "TRTcal_error/"
        + sname
        + ".$(ProcId).err\n"
        + "log         = "
        + logpath
        + "TRTcal_log/"
        + "."
        + ".$(ProcId).log\n"
        + '+JobFlavour = "tomorrow"\n'
        + 'transfer_output_files = ""\n'
        + "queue"
    )

    subFile.close()

    subname = sname + ".sub"

    if config["SubmitCnvJobs"] and not config["MakePlots"]:
        # extract calib constants from a caliboutput text file
        print("looking for RTglobalvalues...")
        if (config["RTglobalValues"].find("file:")) >= 0:
            print("found RTglobalvalues: " + config["RTglobalValues"])
            extract_const(
                "rt",
                config["RTglobalValues"].split("file:")[1],
                "%s/tempconst_rt.txt" % config["Batchdir"],
            )
        print("looking for T0globalvalues...")
        if (config["T0globalValues"].find("file:")) >= 0:
            print("found T0globalvalues: " + config["T0globalValues"])
            extract_const(
                "t0",
                config["T0globalValues"].split("file:")[1],
                "%s/tempconst_t0.txt" % config["Batchdir"],
            )
        return subname


def convert(
    config,
    calibconstants,
    pooloutput,
    dboutput,
    sname,
    jname,
    waitlist,
    caltags,
    couttags,
    oiter,
    batchdir,
):
    """Create and configure 'cnv' plotting + converting script & .sub file. (not sure yet what is converted)"""
    outf = open(sname, "w")
    outf.write("#!/bin/sh\n")
    ostring = scriptgen.txt2db(
        config, calibconstants, pooloutput, dboutput, caltags, couttags, oiter, batchdir
    )
    outf.write(ostring)
    outf.close()
    os.system("chmod 755 " + sname)

    # Write submisssion file
    subFile = open(sname + ".sub", "w+")
    subFile.write(
        "executable  = "
        + sname
        + "\n"
        + "arguments   = $(ProcId)\n"
        + "output      = "
        + logpath
        + "TRTcal_output/"
        + sname
        + ".$(ProcId).out\n"
        + "error       = "
        + logpath
        + "TRTcal_error/"
        + sname
        + ".$(ProcId).err\n"
        + "log         = "
        + logpath
        + "TRTcal_log/"
        + "."
        + ".$(ProcId).log\n"
        + '+JobFlavour = "tomorrow"\n'
        + 'transfer_output_files = ""\n'
        + "queue"
    )

    subFile.close()

    subname = sname + ".sub"

    if config["SubmitCnvJobs"]:
        # extract calib constants from a caliboutput text file
        if (config["RTglobalValues"].find("file:")) >= 0:
            extract_const(
                "rt",
                config["RTglobalValues"].split("file:")[1],
                "%s/tempconst_rt.txt" % config["Batchdir"],
            )
        if (config["T0globalValues"].find("file:")) >= 0:
            extract_const(
                "t0",
                config["T0globalValues"].split("file:")[1],
                "%s/tempconst_t0.txt" % config["Batchdir"],
            )
        return subname
