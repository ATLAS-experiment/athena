# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

## @file getProblemFolderFromLogs.py
## @brief The script for parse Athena debug logs and finding folders with different data (CREST and COOL)
## @author Evgeny Alexandrov <Evgeny.Alexandrov@cern.ch>

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
    parser.add_argument('--host', default='http://crest-j23.cern.ch',
                        help='Host of the CREST service (default: http://crest-j23.cern.ch)')
    parser.add_argument('--port', default='8080',
                        help='Port of the CREST service (default: 8080)')
    parser.add_argument('--out', default='log.out',
                        help='Out file for logs (default: log.out)')
    parser.add_argument('--path', default='cool_crest_compare',
                        help='Path to compare (default: cool_crest_compare)')
    parser.add_argument('--debug', default='',
                        help='Set debug output')

    args = parser.parse_args()
    server = "{0}:{1}".format(args.host, args.port)
    fOut="log.out"
    isDebug=False
    if args.debug:
        isDebug=bool(args.debug)
    appPath=args.path
    if args.out:
      fOut=args.out
    if not path.exists(args.input) or not path.isfile(args.input):
      print("Error: input file is not exists")
      sys.exit()
    data = {}
    st=""
    # Parce athena log and found all folders and other parameters for compare data
    with open(args.input) as inp_file:
        while line := inp_file.readline():
            if line.rstrip().find('/TagInfo<metaOnly/>')>0:
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
            if line.rstrip().find('Retrieved object: folder ')>0:
                st = line.rstrip()[line.rstrip().find('Retrieved object: folder '):]
                st = st[st.find('/'):]
                sFolder = st[:st.find(' ')]
                st= st[st.find('IOV ')+4:]
                st=st[:st.find(' ')]
                el = data[sFolder.strip()]
                el['timestamp']=st
                data[sFolder.strip()]=el
    if isDebug:
        print(data)
    print("List of commands for compare:")
    # run compare
    for elem in data.keys():
        js=data[elem]
        if 'timestamp' not in js:
            continue    
        command = appPath + ' -g '+ args.gtag+' -G '+args.gcooltag+ ' -f '+elem + ' -c "'+ js['db']+'" -t ' + js['timestamp']+' -C '+server
        if 'tag' in js:
            if js['tag'].find('HEAD')>=0:
                command +=' --head'
        command +=' >>'+fOut+' 2>&1'
        print(command)
        with open(fOut, 'a') as f:
          print('Compare folder name:', elem, file=f)
          print(command,file=f)
        os.system(command)
    #parse output of compare folders (CREST and COOL)    
    print("Problem folders:")
    curFolder=""
    coolProblem=[]
    crestProblem=[]
    difFolder=[]
    correctFolder=[]
    allFolder=[]
    with open(fOut) as log_file:
        while line := log_file.readline():
            if line.find('Compare folder name:')>-1:
                if curFolder != "":
                    js=data[curFolder]
                    command = appPath + ' -g '+ args.gtag+' -G '+args.gcooltag+ ' -f '+ curFolder + ' -c '+ js['db']+' -t ' + js['timestamp']+' -C '+server
                    print(command)
                    curFolder=""
                curFolder=line.rstrip()[line.rstrip().find('Compare folder name:')+20:]
                curFolder=curFolder.lstrip()
                allFolder.append(curFolder)
            if line.rstrip().find('is the same in COOL and CREST')>0:
                correctFolder.append(curFolder)
                curFolder=""
            if line.find('CREST output file problem')>-1:
                if curFolder != "":
                    crestProblem.append(curFolder)
            if line.find('COOL output file problem')>-1:
                if curFolder != "":
                    coolProblem.append(curFolder)
            if line.rstrip().find('is different in COOL and CREST')>0:
                fld = line.rstrip()[line.rstrip().find('\"')+1:]
                fld = fld[:fld.find('\"')]
                difFolder.append(fld)
                js=data[fld]
                command = appPath + ' -g '+ args.gtag+' -G '+args.gcooltag+ ' -f '+ fld + ' -c '+ js['db']+' -t ' + js['timestamp']+' -C '+server
                print(command)
                curFolder=""
    print("Total number of folders in logs:",len(data)," Total number of folders which compare (used):",len(allFolder)," Correct folders:",len(correctFolder)," Different folders:",len(difFolder)," COOL problems:",len(coolProblem)," CREST problems:",len(crestProblem))
    if len(difFolder)>0:
        print("Different data:",difFolder)
    if len(coolProblem)>0:
        print("COOL problem folders:",coolProblem)
    if len(crestProblem)>0:
        print("CREST problem folders:",crestProblem)
    if len(correctFolder)>0 and isDebug:
        print("Correct folders:",correctFolder)
    if len(allFolder)>0 and isDebug:
        print("Full folder list:",allFolder)
    if isDebug:
        print("Total list folders in log:",list(data.keys()))

