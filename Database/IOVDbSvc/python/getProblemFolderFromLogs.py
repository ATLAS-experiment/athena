# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @file getProblemFolderFromLogs.py
## @brief The script for parse Athena debug logs and finding folders with different data (CREST and COOL)
## @author Evgeny Alexandrov <Evgeny.Alexandrov@cern.ch>
##
## Every IOV key a folder was retrieved at is compared, in one cool_crest_compare call per folder.
## The exit code is 0 if all compared folders are the same, 1 if any folder is different or could
## not be compared, 2 if the input file is missing.
##
## Limitation: cool_crest_compare compares JSON dumps in which CORAL writes float and double values
## with the default stream precision (6 significant digits), so values which differ only beyond
## that are reported as the same.

import sys
import os.path
import argparse

from os import path


if __name__ == "__main__":
    # Parse arguments
    parser = argparse.ArgumentParser(description='Coolr browser.', add_help=False)
    parser.add_argument('--input', required=True,help='Input file in text folrat. It is output of athena job. It require has lines with info about load data: INFO Retrieved object: folder ')
    parser.add_argument('--gtag', required=True,
                        help='Global tag for CREST')
    parser.add_argument('--gcooltag', required=True,
                        help='Global tag for COOL')
    parser.add_argument('--host', default='https://crest.cern.ch',
                        help='Host of the CREST service (default: https://crest.cern.ch)')
    parser.add_argument('--port', default='443',
                        help='Port of the CREST service (default: 443)')
    parser.add_argument('--out', default='log.out',
                        help='Out file for logs (default: log.out)')
    parser.add_argument('--path', default='cool_crest_compare',
                        help='Path to compare (default: cool_crest_compare)')
    parser.add_argument('--debug', default=False, action=argparse.BooleanOptionalAction, help='Set debug output')

    args = parser.parse_args()
    server = "{0}:{1}".format(args.host, args.port)
    fOut="log.out"
    appPath=args.path
    if args.out:
      fOut=args.out
    if not path.exists(args.input) or not path.isfile(args.input):
      print("Error: input file is not exists")
      sys.exit(2)
    data = {}
    st=""
    noUsedData=[]
    # Parce athena log and found all folders and other parameters for compare data
    with open(args.input) as inp_file:
        for line in inp_file:
            if line.rstrip().find('/TagInfo<metaOnly/>')>-1:
                lst=line.rstrip().split(',')
                for e in lst:
                    if e.find('<db>')<0:
                        continue
                    el ={}
                    if  e.find('<tag>')>0:
                        sTag=e[e.find('<tag>')+5:]
                        if sTag.find('</tag>')>0:
                                sTag=sTag[:sTag.find('</tag>')]
                                el['tag']=sTag
                    db=e[e.find('<db>')+4:]
                    db=db[:db.find('</db>')]
                    if e.find('<db>')<4:
                        e=e[e.find('</db>')+5:]
                    folder = e[e.find('/'):]
                    if folder.find('<')>-1:
                        folder = folder[:folder.find('<')]
                    elif folder.find(' ')>-1:
                        folder = folder[:folder.find(' ')]
                    else:
                        folder = folder[:folder.find('\'')]
                    el['db']=db.strip()
                    data[folder.strip()]=el
            if line.rstrip().find('Using tag ')>0:
                st = line.rstrip()[line.rstrip().find('Using tag ')+10:]
                sTag = st[:st.find(' ')]
                if st.rstrip().find('for folder ')>0:
                    st = st.rstrip()[st.rstrip().find('for folder '):]
                    st = st[st.find('/'):]
                    sFolder = st.strip()
                    el = data.setdefault(sFolder.strip(), {})
                    el['tag']=sTag
                    data[sFolder.strip()]=el
            if line.rstrip().find(' is requested but no data retrieved')>0:
                st = line.rstrip()[:line.rstrip().find(' is requested but no data retrieved')]
                st = st[st.find('WARNING Folder')+14:]
                noUsedData.append(st.strip())
            if line.rstrip().find('Retrieved object: folder ')>0:
                st = line.rstrip()[line.rstrip().find('Retrieved object: folder '):]
                st = st[st.find('/'):]
                sFolder = st[:st.find(' ')]
                st= st[st.find('IOV ')+4:]
                st=st[:st.find(' ')]
                el = data.setdefault(sFolder.strip(), {})
                # Keep every key the folder was retrieved at, each one once
                if st not in el.setdefault('timestamps', []):
                    el['timestamps'].append(st)
    if args.debug:
        print(data)
    print("List of commands for compare:")
    # The out file keeps the output of earlier runs, only the part written from here on is parsed
    with open(fOut, 'a') as f:
        logStart=f.tell()
    noDbFolder=[]
    # run compare
    for elem in data.keys():
        js=data[elem]
        if 'timestamps' not in js:
            continue
        if 'db' not in js:
            noDbFolder.append(elem)
            continue
        # cool_crest_compare takes all keys of a folder in one -t and prints one result line per key
        command = appPath + ' -g '+ args.gtag+' -G '+args.gcooltag+ ' -f '+elem + ' -c "'+ js['db']+'"'
        command +=' -t ' + ' '.join(js['timestamps'])+' -C '+server
        if 'tag' in js:
            if js['tag'].find('HEAD')>=0:
                command +=' --head'
            else:
                command +=' -T '+js['tag']
        js['command']=command
        command +=' >>'+fOut+' 2>&1'
        print(command)
        with open(fOut, 'a') as f:
          print('Compare folder name:', elem, file=f)
          print(command,file=f)
        os.system(command)
    #parse output of compare folders (CREST and COOL)    
    print("Problem folders:")
    curFolder=""
    coolProblem={}
    crestProblem={}
    coolCrash={}
    crestCrash={}
    difFolder={}
    noCrestData={}
    correctFolder={}
    correctData={}
    wrongTag={}
    allFolder=[]
    #status: 0 - the same, 1 - the same data, 2 - different data, 3 - no CREST data, 4 - CREST problem, 5 - COOL problem, 6 - CREST crash, 7 - COOL crash
    results={}
    with open(fOut) as log_file:
        log_file.seek(logStart)
        for line in log_file:

            if line.find('Compare folder name:')>-1:
                curFolder=line.rstrip()[line.rstrip().find('Compare folder name:')+20:]
                curFolder=curFolder.lstrip()
                if curFolder not in allFolder:
                  allFolder.append(curFolder)
                results.setdefault(curFolder, {})  
            elif curFolder == "":
                continue
            if line.find('Start CREST dump:')>-1:
                results[curFolder]["-1"]=6
                continue
            elif line.find('Start COOL dump:')>-1:
                results[curFolder]["-1"]=7
                continue
            elif line.find('ERROR in Crest. Global tag for folder')>-1:
                results[curFolder]["-1"]=8
                continue
            if line.find('timestamp:')>-1:
                ts = line[line.find('timestamp:')+10:].strip()
            else:
                continue
            if line.rstrip().find('is the same in COOL and CREST')>-1:
                results[curFolder][ts]=0
            elif line.rstrip().find('has the same only data in COOL and CREST')>-1:
                results[curFolder][ts]=1    
            elif line.rstrip().find('is different in COOL and CREST')>-1:
                results[curFolder][ts]=2
            elif line.find('ERROR in Crest. CREST output has no data.')>-1:
                results[curFolder][ts]=3
            elif line.find('CREST output file problem')>-1:
                results[curFolder][ts]=4
            elif line.find('COOL output file problem')>-1:
                results[curFolder][ts]=5
    for curFolder in allFolder:
        js=data[curFolder]
        if curFolder not in results:
            continue
        res=results[curFolder]
        if len(res)>1:
            res.pop("-1", None)
        isPrint=False
        for key,value in res.items():
            if value==0:
                el = correctFolder.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==1:
                el = correctData.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==2:
                isPrint=True
                el = difFolder.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==3:
                isPrint=True
                el = noCrestData.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==4:
                isPrint=True
                el = crestProblem.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==5:
                isPrint=True
                el = coolProblem.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==6:
                isPrint=True
                el = crestCrash.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==7:
                isPrint=True
                el = coolCrash.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
            elif value==8:
                isPrint=True
                el = wrongTag.setdefault(curFolder, [])
                if key not in el:
                    el.append(key)
        if isPrint:            
            print(js['command'])
    unCompared=data.keys()-allFolder
    compList=set(unCompared)-set(noUsedData)
    print("Total number of folders in logs:",len(data),"; Total number of folders which compare (used):",len(allFolder), "; Total number of unread folders:", len(noUsedData),"; Correct folders:",len(correctFolder),"; Correct data:",len(correctData),"; Different folders:",len(difFolder),"; No CREST data:",len(noCrestData),"; COOL problems:",len(coolProblem),"; CREST problems:",len(crestProblem),"; COOL crash:",len(coolCrash),"; CREST crash:",len(crestCrash),"; Wrong CREST tag:",len(wrongTag))
    if len(correctData)>0:
        print("Only data:",correctData)
    if len(difFolder)>0:
        print("Different data:",difFolder)
    if len(noCrestData)>0:
        print("No data in CREST:",noCrestData)    
    if len(coolProblem)>0:
        print("COOL problem folders:",coolProblem)
    if len(crestProblem)>0:
        print("CREST problem folders:",crestProblem)
    if len(coolCrash)>0:
        print("COOL crash in folders:",coolCrash)
    if len(crestCrash)>0:
        print("CREST crash in folders:",crestCrash)
    if len(wrongTag)>0:
        print("CREST globaltagmap has wrong tag:",wrongTag)
    if len(noDbFolder)>0:
        print("Folders without <db> in the Folders property (not compared):",noDbFolder)
    if len(compList)>0:
        if len(unCompared):
            print("Incomparable folders:",unCompared)
        if len(noUsedData)>0:
            print("Unread (Unused) folders:",noUsedData)
    elif len(noUsedData)>0:
        print("Unread (Unused) folders:",noUsedData)
    if len(correctFolder)>0 and args.debug:
        print("Correct folders:",correctFolder)
    if len(allFolder)>0 and args.debug:
        print("Full folder list:",allFolder)
    if args.debug:
        print("Total list folders in log:",list(data.keys()))
    if len(difFolder)+len(coolProblem)+len(crestProblem)+len(crestCrash)+len(coolCrash)+len(noDbFolder)>0:
        sys.exit(1)


