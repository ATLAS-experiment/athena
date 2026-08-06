#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

folderPath = "/TRIGGER/L1Calo/V1/Calibration/"

import argparse
import os
import coral
import json
import pandas as pd
from statistics import median
from coldpie import cool
import hashlib
from itertools import chain

from AthenaConfiguration import TestDefaults


parser = argparse.ArgumentParser(description='Check what conditions are defined for MC',formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("--db",nargs='+',default=["COOLOFL_TRIGGER/OFLP200"],help="specify sqlite files to check local")
parser.add_argument("--globalTag",nargs='+',default=[TestDefaults.defaultConditionsTags.RUN4_MC],help="what globaltag to use. if use sqlite file will become blank")

args = parser.parse_args()



from AthenaConfiguration.RunToTimestampData import RunToTimestampDict

# ignore all runs <= 313000 ... these are MC16 and earlier ... we don't care for these
RunToTimestampDict = {k: v for k, v in RunToTimestampDict.items() if k > 313000}

# build a commentsDict
def load_comments(filename):
    comments = {}
    import re
    from pathlib import Path
    pattern = re.compile(
        r'^\s*(\d+)\s*:\s*\d+\s*,\s*#\s*(.*)$'
    )

    for line in Path(filename).read_text().splitlines():
        m = pattern.match(line)
        if m:
            run_number = int(m.group(1))
            comment = m.group(2).strip()
            comments[run_number] = comment

    return comments

from AthenaConfiguration import RunToTimestampData
comments = load_comments(RunToTimestampData.__file__)


dbSvc = cool.DatabaseSvcFactory.databaseService()


summaries = {} # will be a dict of dicts .. first key is folder name, second is hash, value is: iov-list,summary

allGlobalTags = set()

for dbName in args.db:
    globalTagList = list(args.globalTag)

    if os.path.exists(dbName):
        # we assume the DB is OFLP200 in the sqlite file. Use coolDBDiscovery script to check??
        dbName = f"sqlite://;schema={dbName};dbname=OFLP200"
        globalTagList = ["sqlite"]
    print(dbName)

    db = dbSvc.openDatabase(dbName)

    globalTags = db.getFolderSet("/").listTags()
    for globalTag in globalTagList:
        #print(globalTags)
        if globalTag != "sqlite" and globalTag not in globalTags:
            exit(f"GlobalTag {args.globalTag} missing")

        allGlobalTags.add(globalTag)

        for folder in db.listAllNodes():
            if not folder.startswith(folderPath): continue
            folder = db.getFolder(folder)
            folderName = folder.fullPath().split("/")[-1]
            if folderName not in summaries: summaries[folderName] = {}
            try:
                ftag = folder.resolveTag(globalTag) if globalTag!="sqlite" else ""
            except BaseException:
                ftag = None
            if ftag is not None:
                print(folder.fullPath(),":",ftag)

                objs = list(folder.browseObjects(0,9223372036854775807,cool.ChannelSelection.all(),ftag))
                iovs = {}
                for o in objs:
                    timePair = (globalTag,int(o.since()*1e-9),int(o.until()*1e-9)) # include globalTag in key
                    if timePair not in iovs: iovs[timePair] = {}
                    channelId = o.channelId()
                    record = {}
                    for s in o.payload().specification(): # loop over fields
                        # also s.storageType() returns an int (enum) of type, but we just use type(..)
                        b = o.payload().field(s.name()).data()
                        if type(b)== coral.Blob:
                            # interpret as json
                            record.update(json.loads(b.read(b.size())))
                        else:
                            record[s.name()] = b
                    iovs[timePair][channelId] = record

                # go through IoVs, creating hash of payload and store in summaries
                # use hash values to identify equivalent calibrations
                for k1, d1 in iovs.items():
                    hashVal = hashlib.sha256(json.dumps(d1, sort_keys=True).encode()).hexdigest()
                    if hashVal in summaries[folderName]:
                        # this payload already exists ... add iov to list
                        summaries[folderName][hashVal][0] += [k1]
                    else:
                        # create a summary
                        df = pd.DataFrame.from_dict(d1, orient="index")
                        summary = {}
                        for col in df.columns:
                            s = df[col].dropna()
                            if len(s) == 0:
                                continue
                            if isinstance(s.iat[0], list):
                                values = list(chain.from_iterable(s))
                            else:
                                values = s.tolist()
                            sumStr = f"{min(values)}|{median(values)}|{max(values)}" if min(values)!= max(values) else f"{min(values)}"
                            summary[col] = {"summary":sumStr}

                        summary = pd.DataFrame(summary).T

                        summaries[folderName][hashVal] = [[k1], summary]




for folder,fSummaries in summaries.items():
    print(folder,":")
    allSummaries = {}
    for i,s in fSummaries.items():
        allSummaries[i[:5]] = s[1]
        #print("",s[2],":")
        #print(s[1])
    result = pd.concat(
        {name: df["summary"] for name, df in allSummaries.items()},
        axis=1,
    )
    print(result)
    print("********")

# go through run numbers, determine which "calibration" it has for each of the folders
records = []
for k,v in RunToTimestampDict.items():
    record = {}
    record["Campaign"] = str(k) + ":" + comments[k]
    for folder,fSummaries in summaries.items():
        setNum = []
        for globalTag in sorted(list(allGlobalTags)):
            for h,s in fSummaries.items():
            #print(folder,s[0])
                for tag,since,until in s[0]:
                    if tag==globalTag and since<=v and until>v:
                        setNum += [h[:5]]
        record[folder] = "|".join(setNum)
    records += [record]

df = pd.DataFrame(records).set_index("Campaign")
df = df.sort_values(by=df.columns.tolist())

print("\n Summary Table:")
print(sorted(list(allGlobalTags)))

text = df.to_string()
lines = text.splitlines()

# Header is the first two lines if the index has a name, otherwise the first line.
header = lines[:2]
rows = lines[2:]

print("\n".join(header))

last = None

for i, (_, row) in enumerate(df.iterrows()):
    if i > 0:
        prev = df.iloc[i - 1]
        if not (row == prev).all():
            print("-" * len(lines[0]))
    print(rows[i])


