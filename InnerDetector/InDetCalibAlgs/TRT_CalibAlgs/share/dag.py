import re


def dag_append(dagnames, reg, subfilenames):
    """Append dagnames with correctly formatted job names.
    Uses filenames from subfilenames that match regex string reg.

    Keyword arguments:
    dagnames -- the list of dag job names you want to append to
    reg -- (regex) string used as search criteria
    subfilenames -- the list of filenames to search

    Returns: list
    (original list appended with names matching reg, list of j-indexed job names (J0,J1,etc.))
    """
    jnames = [
        "JOB " + "J" + str(i + len(dagnames)) + " " + name + "\n"
        for i, name in enumerate(list(filter(reg.match, subfilenames)))
    ]
    jlist = [jb.split()[1] for jb in jnames]
    return dagnames + jnames, jlist


def grab_merge_names(filelist):
    """Quick utility function to grab names of merge scripts.

    Keyword arguments:
    filelist -- list of .sub filenames (generated within the RunTRTCalib.py script)

    Returns: list
    """
    reg = re.compile("trtcal_\d\d_merge_")
    names = [
        "merge" + x.split("merge")[1].split(".")[0]
        for x in list(filter(reg.match, filelist))
    ]
    # get unique names only
    unq = []
    for x in names:
        if x in names and x not in unq:
            unq.append(x)

    return unq


def write_dag(filelist, iterlist, mail, area):
    """Creates the file dag.dag.

    File format of dag.dag:
    JOB (job name, e.g. J1) (.sub file to submit, e.g. myscript.sh.sub)
    ...

    PARENT J1 J2 (etc.) CHILD J3 (etc.)
    ...

    Keyword arguments:
    filelist -- list of .sub filenames (generated within the RunTRTCalib.py script)
    iterlist -- list of TWO-DIGIT STRINGS corresponding to iteration numbers, e.g ['00','01',...]

    Returns: void
    """

    # make life easier by adding each sub-merge script as a seperate condition:
    # start by searching for matching names

    # order in which scripts are to be submitted
    order = (
        ["\d"]  # n-tuple creation
        + grab_merge_names(filelist)  # batch merging of ntuples
        + [
            "merge.sh",  # final merge step
            "calib_[\d|-]",  # sub-calibrations
            "calib_barrel",  # barrel calibration
            "calib_all",  # final calibration
            "mci",  # shift constants
            "cnv",  # make plots
        ]
    )
    fnames = []
    for iternum in iterlist:

        daglist = []  # list of job names in DAG
        family = ""  # string of parent/child relations of jobs
        # prefix of every script
        trtstr = "trtcal_" + iternum + "_"

        # iterate over regex-formated filenames
        for k, name in enumerate(order):

            reg = re.compile(trtstr + name)

            # if blank, move on
            if any([reg.match(file) for file in filelist]):

                # append matching filenames to daglist
                daglist, joblist = dag_append(daglist, reg, filelist)

                # add children
                # control flow == first job is not a child
                if k != 0:
                    children = "CHILD "
                    for jnum in joblist:
                        children += jnum + " "
                    family += children + "\n"

                # add parents
                # control flow == last job is not a parent
                if k != len(order) - 1:
                    parents = "PARENT "
                    for jnum in joblist:
                        parents += jnum + " "
                    family += parents
        # write out dag file
        fname = "iteration" + str(iternum) + ".dag"
        fnames.append(fname)
        with open(fname, "w") as dag:
            for job in daglist:
                dag.write(job)
            dag.write("\n" + family)

    jobs = ""
    family = "\n"
    for i, stuff in enumerate(zip(iterlist, fnames)):
        it, fname = stuff
        jname = fname.replace(".dag", "")
        jobs += "SUBDAG EXTERNAL " + jname + " " + fname + "\n"
        # mail =  "I" + str(int(it))
        # jobs += "JOB " + mail + " mail"+str(int(it))+".sh.sub\n"
        jobs += "SCRIPT POST " + jname + " mail.py 0" + str(int(it)) + " " + mail + " " + area + "\n"
        # family += "PARENT "+fname.replace(".dag", "")+" CHILD " + mail + '\n'
        if i is not len(fnames) - 1:
            # family += "PARENT "+ mail + " CHILD "+fnames[i+1].replace(".dag", "") + '\n'
            family += (
                "PARENT " + jname + " CHILD " + fnames[i + 1].replace(".dag", "") + "\n"
            )

    with open("dag.dag", "w") as dag:
        dag.write(jobs + family)


if __name__ == "__main__":
    # Create dummy filelist, iterlist
    filelist = [
        "trtcal_01_0001.sh.sub",
        "trtcal_01_0002.sh.sub",
        "trtcal_01_merge.sh.sub",
        "trtcal_01_calib_-1_1.sh.sub",
        "trtcal_01_calib_-1_0.sh.sub",
        "trtcal_01_calib_-1_2.sh.sub",
        "trtcal_01_calib_barrel.sh.sub",
        "trtcal_01_calib_2_b.sh.sub",
        "trtcal_01_calib_2_c.sh.sub",
        "trtcal_01_calib_2_a.sh.sub",
        "trtcal_01_calib_2_d.sh.sub",
        "trtcal_01_calib_1_1.sh.sub",
        "trtcal_01_calib_1_0.sh.sub",
        "trtcal_01_calib_1_2.sh.sub",
        "trtcal_01_calib_-2_d.sh.sub",
        "trtcal_01_calib_-2_b.sh.sub",
        "trtcal_01_calib_-2_c.sh.sub",
        "trtcal_01_calib_-2_a.sh.sub",
        "trtcal_01_calib_all.sh.sub",
        "trtcal_01_calib_-2.sh.sub",
        "trtcal_01_calib_-1.sh.sub",
        "trtcal_01_calib_2.sh.sub",
        "trtcal_01_calib_1.sh.sub",
        "trtcal_01_mci.sh.sub",
        "trtcal_01_cnv.sh.sub",
        "trtcal_02_0001.sh.sub",
        "trtcal_02_0002.sh.sub",
        "trtcal_02_merge.sh.sub",
        "trtcal_02_merge_001.sh.sub",
        "trtcal_02_merge_002.sh.sub",
        "trtcal_02_calib_-1_1.sh.sub",
        "trtcal_02_calib_-1_0.sh.sub",
        "trtcal_02_calib_-1_2.sh.sub",
        "trtcal_02_calib_barrel.sh.sub",
        "trtcal_02_calib_2_b.sh.sub",
        "trtcal_02_calib_2_c.sh.sub",
        "trtcal_02_calib_2_a.sh.sub",
        "trtcal_02_calib_2_d.sh.sub",
        "trtcal_02_calib_1_1.sh.sub",
        "trtcal_02_calib_1_0.sh.sub",
        "trtcal_02_calib_1_2.sh.sub",
        "trtcal_02_calib_-2_d.sh.sub",
        "trtcal_02_calib_-2_b.sh.sub",
        "trtcal_02_calib_-2_c.sh.sub",
        "trtcal_02_calib_-2_a.sh.sub",
        "trtcal_02_calib_all.sh.sub",
        "trtcal_02_calib_-2.sh.sub",
        "trtcal_02_calib_-1.sh.sub",
        "trtcal_02_calib_2.sh.sub",
        "trtcal_02_calib_1.sh.sub",
        "trtcal_02_mci.sh.sub",
        "trtcal_02_cnv.sh.sub",
    ]
    iterlist = ["01", "02"]

    # Create dag based on above -- outputted to dag.dag
    write_dag(filelist=filelist, iterlist=iterlist, mail="test")
