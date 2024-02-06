######################################################
############### MAIN JOB SCHEDULER ###################
######################################################

import runbatch, sys, os
from util import create_calsettings


def run_iteration(config, inputlist, iter, waitlist, startconstants):
    # 'Old' iteration number
    oiter = iter - 1

    # List of condor .sub files to submit
    subnames = []

    # Display settings
    print("DoRt ", config["DoRt"][oiter + 1])
    print("DoT0 ", config["DoT0"][oiter + 1])
    print("Do Xenon/Argon straws??  ", config["DoArXe"])

    # Check Rt settings
    # TODO understand these
    if config["DoRt"][oiter + 1] == "none":
        calsettings2 = create_calsettings(config, "detector", config["DoT0"][oiter + 1])
        print("DoRT at detector level, but later will be corrected")
    else:
        calsettings2 = create_calsettings(
            config, config["DoRt"][oiter + 1], config["DoT0"][oiter + 1]
        )

    if oiter < 0:  # iter == 1
        # 1st iteration
        if startconstants == "db":
            #
            calibconstants = ""
        else:
            if not os.path.isfile(startconstants):
                print("File with start calib constants does not exist: %s" % startconstants)
                print("Check configuration file!")
                sys.exit(-1)
            calibconstants = startconstants
    else:
        calibconstants = "%s_%02d.txt" % (config["CalPrefix"], oiter)

    calconst = calibconstants

    jlist, outlist, stemp = runbatch.parallel(
        config, inputlist, calibconstants, iter, waitlist, calsettings2
    )
    subnames += stemp
    if not config["SubmitRecJobs"]:
        jlist = []
        subnames = []

    calibconstants = "%s_%02d.txt" % (config["CalPrefix"], iter)
    calibconstantspool = "%s_%02d.pool.root" % (config["CalPrefix"], iter)
    calibconstantsdb = "%s_%02d.db" % (config["CalPrefix"], iter)

    counter = 0
    max_merge = 4

    # setupcmd=config["SetupCmd"]
    calibcmd = config["CalibCmd"]
    mergecmd = config["MergeCmd"]
    mergetxtcmd = config["MergeTxtCmd"]
    constdir = config["ConstDir"]
    batchq = config["Batchqueue"].split(",")[0]
    batchdir = config["Batchdir"]

    while len(outlist) > max_merge:
        noutlist = []
        wlist = []
        intermediate = len(outlist) / max_merge
        for i in range(intermediate):
            start = i * max_merge
            end = (i + 1) * max_merge
            plist, oname, stemp = runbatch.merge_partial(
                config,
                outlist[start:end],
                mergecmd,
                batchq,
                iter,
                jlist[start:end],
                counter,
            )
            subnames += stemp
            counter = counter + 1
            for i in plist:
                wlist.append(i)
            noutlist.append(oname)

        if intermediate * max_merge < len(outlist):
            start = intermediate * max_merge
            end = len(outlist)
            plist, oname, stemp = runbatch.merge_partial(
                config,
                outlist[start:end],
                mergecmd,
                batchq,
                iter,
                jlist[start:end],
                counter,
            )
            subnames += stemp
            counter = counter + 1
            for i in plist:
                wlist.append(i)
            noutlist.append(oname)

        # now copy over noutlist and wlist
        outlist = noutlist
        jlist = wlist

    # now run final merge procedure
    # jlist,outname=runbatch.merge_final(config,outlist,outdir,mergecmd,batchq,iter,jlist)
    jlist, outname, stemp = runbatch.merge_final(
        config, outlist, config["TempDir"], mergecmd, batchq, iter, jlist
    )
    subnames += stemp

    # run the parallel calibration
    inputlist.sort()
    mlist, caltags, couttags, stemp = runbatch.parallel_calib(
        config,
        outname,
        calibcmd,
        batchq,
        iter,
        jlist,
        inputlist[0][1],
        calconst,
        oiter,
        calsettings2,
    )
    subnames += stemp

    # make calibconst text files
    nlist, stemp = runbatch.mkcalin(
        config, calibconstants, iter, mlist, caltags, couttags, oiter, batchdir
    )
    subnames += stemp

    # merge root files and generate db files
    olist, stemp = runbatch.convert(
        config, calibconstants, iter, nlist, caltags, couttags, oiter, batchdir
    )
    subnames += stemp

    print(olist)
    print(subnames)
    return nlist, subnames
