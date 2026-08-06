# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


def _ds2ts(datestamp, format):
    from time import strptime
    from calendar import timegm
    try:
        ts = strptime(datestamp, format)
        return int(timegm(ts)) * 10**9
    except ValueError:
        raise ValueError(f"ERROR in time specification for TimeStampToRunLumi,"
                         f"it should be in format {format}")


def TimeStampToRunLumi(tmstmp, guard=1, dbInstance="CONDBR2", format="%Y-%m-%d:%H:%M:%S/%Z", gettime=False):
    """returns a (run, lumiblock) pair corresponding to the indicated timestamp;
       it consists in the run started at most two days before the timestamp,
       and still ongoing at that time. 
    
        Arguments:
        tmstmp -- timestamp (numerical or text)
                if numerical, interpreted as a unix timestamp (in nanoseconds)
                if text, interpreted either as a date/time in the 'format' format
        guard -- scales the default two-days window to search for runs in the DB
        dbInstance -- DB instance used for the translation
        format -- anything understood by time.strptime()
        gettime -- if True, returns a triplet (run, LB, inferred timestamp in ns) instead of the default pair
    """
    from PyCool import cool
    from time import asctime,localtime
    if isinstance(tmstmp, str): tmstmp = _ds2ts(tmstmp, format)
    dbSvc = cool.DatabaseSvcFactory.databaseService()
    db=dbSvc.openDatabase("COOLONL_TRIGGER/"+dbInstance)
    folder=db.getFolder("/TRIGGER/LUMI/LBTIME")
    range=guard*24*60*60*1e9 # 2 days in ns
    t1=int(tmstmp-range)
    t2=int(tmstmp+range)
    itr=folder.browseObjects(t1,t2,cool.ChannelSelection.all())
    while itr.goToNext():
        obj=itr.currentRef()
        if obj.until()>tmstmp:
            pl=obj.payload()
            run=pl["Run"]
            lb=pl["LumiBlock"]
            print ("Found Run/Lumi [%i/%i] lasting from %s to %s" %\
                   (run,lb,asctime(localtime(obj.since()/1e9)),asctime(localtime(obj.until()/1e9))))
            itr.close()
            db.closeDatabase()
            return ((run,lb), tmstmp) if gettime else (run,lb)
    print ("WARNING: No run/lumi block found for time",asctime(localtime(tmstmp/1e9)),"in folder /TRIGGER/LUMI/LBTIME of DB COOLONL_TRIGGER/CONDBR2")
    itr.close()
    db.closeDatabase()
    return (None, tmstmp) if gettime else None


def fillInputFlags(flags, datestamp: str, infiniteRun=999999):
    rlb, t = TimeStampToRunLumi(datestamp, gettime=True)
    if rlb is None:
        rlb = (infiniteRun, 0)
        print(f"WARNING: failed to convert specified date/time into a run/lumiblock number. "
              f"Using 'infinite' run-number {rlb[0]}")
    flags.Input.RunNumbers = [rlb[0]]
    flags.Input.LumiBlockNumbers = [rlb[1]] 
    flags.Input.TimeStamps = [int(t / 10**9)]
