##########################################################
############# SECONDARY JOB SCEDULERS ####################
##########################################################

import scriptsub, os


def parallel(config, inputlist, calibconstants, iter, waitlist, calsettings2):
    # run only maxjobs jobs (or less in case there are less than maxjobs inputfiles
    subnames = []
    # try to make equal sized jobs
    # if type(inputlist[0])==TupleType:
    if 1:

        totsizeall = 0
        for file in inputlist:
            totsizeall += file[0]

        if config["SingleFile"]:
            inputlist2 = []
            inputlist2.append(inputlist[-1])
            # inputlist2.append(inputlist[int(len(inputlist)/2)])
            singlesize = inputlist2[0][0]
            inputlist = inputlist2
            print("USING FILE       : %s" % inputlist[0][1].split("/")[-1])

        totsize = 0
        for file in inputlist:
            totsize += file[0]
        maxsize = totsize / config["MaxParallel"]
        print("TOTAL DATA SIZE  : %i (%f GB)" % (
            totsizeall,
            float(totsizeall) / (1024 * 1024 * 1024),
        ))
        if config["SingleFile"]:
            print("SIZE OF FILE     : %i (%f GB)" % (
                singlesize,
                float(singlesize) / (1024 * 1024 * 1024),
            ))
            print("FRACTION OF DATA : %f" % (float(singlesize) / float(totsizeall)))
        else:
            print("MAX JOB SIZE     : %i" % maxsize)
            print()

        jobsize = 0
        ijob = 0
        i = []
        dummyshiftfiles = []
        for file in inputlist:
            jobsize += file[0]
            i.append(file[1])
            if jobsize > maxsize:
                jobsize = 0
                ijob = ijob + 1
                # if ijob==5: config["shiftfiles"]=i
                dummyshiftfiles += i
                i = []
        config["shiftfiles"] = []
        if float(config["DoShift"]) > 0:
            print("SELECTING FILES!")
            for shf in dummyshiftfiles:
                if config["NoRec"]:
                    config["shiftfiles"].append(shf)
                else:
                    if (
                        os.popen(
                            "export STAGE_SVCCLASS=atlcal; stager_qry -M "
                            + shf.split("castor:")[-1]
                        )
                        .read()
                        .find("STAGED")
                        >= 0
                    ):
                        config["shiftfiles"].append(shf)

        jobsize = 0
        ijob = 0
        ifile = 0
        i = []
        outlist = []
        jlist = []
        for file in inputlist:
            ifile = ifile + 1
            jobsize += file[0]
            i.append(file[1])
            if jobsize > maxsize:
                print("JOB %i CONTAINS %i BYTES (%i FILE(S))" % (
                    ijob + 1,
                    jobsize,
                    ifile,
                ))
                ifile = 0
                jobsize = 0
                ijob = ijob + 1
                if float(config["DoShift"]) > 0:
                    sname = "trtcal_%02d_%04d_t0shift.sh" % (iter, ijob)
                else:
                    sname = "trtcal_%02d_%04d.sh" % (iter, ijob)
                jname = config["JobPrefix"] + "%02d_%04d" % (iter, ijob)
                outname = config["TempReadDir"]
                outname += "/trtcalib_%02d_%04d.root" % (iter, ijob)
                if not ijob in config["ExcludeRuns"]:
                    outlist.append(outname)
                    jlist.append(jname)
                    subname = scriptsub.parallel(
                        config,
                        i,
                        calibconstants,
                        outname,
                        sname,
                        jname,
                        waitlist,
                        calsettings2,
                        iter - 1,
                        ijob,
                    )
                    subnames.append(subname)
                else:
                    print("EXCLUDED!!")
                i = []
        return jlist, outlist, subnames

    if len(inputlist) > config["MaxParallel"]:
        if len(inputlist) % config["MaxParallel"] == 0:
            files_per_job = len(inputlist) / config["MaxParallel"]
        else:
            files_per_job = len(inputlist) / config["MaxParallel"] + 1
        iflist = []
        start = -1
        end = -1
        for i in range(len(inputlist) / files_per_job):
            start = i * files_per_job
            end = (i + 1) * files_per_job
            if end > (len(inputlist)):
                end = len(inputlist)
            if start >= len(inputlist) - 1:
                print("More Jobs than inputfiles!!!")
            else:
                iflist.append(inputlist[start:end])
        if end < len(inputlist):
            iflist.append(inputlist[end:])

        count = 1
        jlist = []
        outlist = []
        for i in iflist:
            outname = config["TempReadDir"]
            outname += "/trtcalib_%02d_%04d.root" % (iter, count)
            outlist.append(outname)
            if float(config["DoShift"]) > 0:
                sname = "trtcal_%02d_%04d_t0shift.sh" % (iter, count)
            else:
                sname = "trtcal_%02d_%04d.sh" % (iter, count)
            jname = config["JobPrefix"] + "%02d_%04d" % (iter, count)
            subname = scriptsub.parallel(
                config,
                i,
                calibconstants,
                outname,
                sname,
                jname,
                waitlist,
                calsettings2,
                iter - 1,
                0,
            )
            subnames.append(subname)
            jlist.append(jname)
            count = count + 1
    else:
        files_per_job = 1
        iflist = []
        start = -1
        end = -1
        for i in range(len(inputlist)):
            start = i
            end = i + 1
            iflist.append(inputlist[start:end])

        count = 1
        jlist = []
        outlist = []
        for i in iflist:
            print(i)
            outname = config["TempReadDir"]
            outname += "/trtcalib_%02d_%04d.root" % (iter, count)
            outlist.append(outname)
            if float(config["DoShift"]) > 0:
                sname = "trtcal_%02d_%04d_t0shift.sh" % (iter, count)
            else:
                sname = "trtcal_%02d_%04d.sh" % (iter, count)
            jname = config["JobPrefix"] + "%02d_%04d" % (iter, count)
            subname = scriptsub.parallel(
                config,
                i,
                calibconstants,
                outname,
                sname,
                jname,
                waitlist,
                calsettings2,
                iter - 1,
                0,
            )
            subnames.append(subname)
            jlist.append(jname)
            count = count + 1

    return jlist, outlist, subnames


def merge_partial(config, inputlist, mergecmd, batchq, iter, waitlist, counter):
    jlist = []
    subnames = []

    outname = config["TempDir"]
    outname += "/trtcalib_%02d_merge_%03d.root" % (iter, counter)
    sname = "trtcal_%02d_merge_%03d.sh" % (iter, counter)
    jname = config["JobPrefix"] + "%02dm%03d" % (iter, counter)

    subname = scriptsub.merge(
        config, inputlist, outname, mergecmd, batchq, sname, jname, waitlist, iter
    )
    jlist.append(jname)
    subnames.append(subname)

    return jlist, outname, subnames


def merge_final(config, inputlist, outdir, mergecmd, batchq, iter, waitlist):
    jlist = []
    subnames = []

    outname = outdir
    outname += "/trtcalib_%02d.root" % iter
    sname = "trtcal_%02d_merge.sh" % iter
    jname = config["JobPrefix"] + "%02dmerge" % iter

    subname = scriptsub.merge(
        config, inputlist, outname, mergecmd, batchq, sname, jname, waitlist, iter
    )
    jlist.append(jname)
    subnames.append(subname)

    return jlist, outname, subnames


def parallel_calib(
    config,
    inputfile,
    calibcmd,
    batchq,
    iter,
    waitlist,
    datafile,
    calibconst,
    oiter,
    calsettings2,
):
    jlist = []
    subnames = []
    caltags = []
    couttags = []

    icalib = 0
    for calarg in calsettings2:
        print(calarg)
        sname = "trtcal_%02d_calib" % iter + calarg + ".sh"
        jname = config["JobPrefix"] + "%02dc%02d" % (iter, icalib)

        if calarg == "error_Barrel":
            if config["DoErrorOptimization"][iter]:
                subname = scriptsub.pcalib(
                    config,
                    inputfile,
                    calibcmd,
                    batchq,
                    calarg,
                    sname,
                    jname,
                    waitlist,
                    calsettings2[calarg],
                    calsettings2[calarg],
                    datafile,
                    calibconst,
                    oiter,
                    calsettings2,
                )
                jlist.append(jname)
                subnames.append(subname)
                caltags.append(calarg)
                couttags.append(calarg)
                icalib = icalib + 1
        elif calarg == "error_Endcap":
            if config["DoErrorOptimization"][iter]:
                subname = scriptsub.pcalib(
                    config,
                    inputfile,
                    calibcmd,
                    batchq,
                    calarg,
                    sname,
                    jname,
                    waitlist,
                    calsettings2[calarg],
                    calsettings2[calarg],
                    datafile,
                    calibconst,
                    oiter,
                    calsettings2,
                )
                jlist.append(jname)
                subnames.append(subname)
                caltags.append(calarg)
                couttags.append(calarg)
                icalib = icalib + 1
        else:
            subname = scriptsub.pcalib(
                config,
                inputfile,
                calibcmd,
                batchq,
                calarg,
                sname,
                jname,
                waitlist,
                calsettings2[calarg]["sel"],
                calsettings2[calarg]["opt"],
                datafile,
                calibconst,
                oiter,
                calsettings2,
            )
            jlist.append(jname)
            subnames.append(subname)
            caltags.append(calarg)
            couttags.append(calarg)
            icalib = icalib + 1

    return jlist, caltags, couttags, subnames


def convert(config, calibconstants, iter, waitlist, caltags, couttags, oiter, batchdir):
    jlist = []
    subnames = []

    sname = "trtcal_%02d_cnv.sh" % iter
    jname = config["JobPrefix"] + "%02dcnv" % iter

    pooloutput = "%s_%02d.pool.root" % (config["CalPrefix"], iter)
    dboutput = "%s_%02d.db" % (config["CalPrefix"], iter)

    subname = scriptsub.convert(
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
    )
    jlist.append(jname)
    subnames.append(subname)

    return jlist, subnames


def mkcalin(config, calibconstants, iter, waitlist, caltags, couttags, oiter, batchdir):
    jlist = []
    subnames = []

    sname = "trtcal_%02d_mci.sh" % iter
    jname = config["JobPrefix"] + "%02dmci" % iter

    pooloutput = "%s_%02d.pool.root" % (config["CalPrefix"], iter)
    dboutput = "%s_%02d.db" % (config["CalPrefix"], iter)

    subname = scriptsub.mkcalin(
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
    )
    jlist.append(jname)
    subnames.append(subname)

    scriptsub.mail(config, iter)
    return jlist, subnames
