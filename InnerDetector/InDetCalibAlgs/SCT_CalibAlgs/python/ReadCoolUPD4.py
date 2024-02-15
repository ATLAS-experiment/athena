#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

############################################################################################
# @file: Script to extract information for the uploaded in the COOL DB runs with UPD4 Tag
#       
# @Author: Peter Vankov <peter.vankov@cern.ch>
###########################################################################################

#!/bin/env python
import sys
from PyCool import cool

#############################################################################################
def formDbString(technology,server,schema,dbname):
  theString=technology+'://'+server+';schema='+schema+';dbname='+dbname
  return theString
  
#############################################################################################
def openDatabase(dbstring):
  dbSvc=cool.DatabaseSvcFactory.databaseService()
  technology='oracle'
  server='ATLAS_COOLPROD'
  [schema,dbname]=dbstring.split('/')
  myDbString=formDbString(technology, server, schema, dbname)
  try:
      db=dbSvc.openDatabase(myDbString)
  except Exception as e:
      print('Problem opening %s in GetRunList'%myDbString)
      print(e)
      sys.exit(-1)
  return db

#############################################################################################
def formIov(runNumber):
  s,u=((runNumber) << 32), ((runNumber + 1) << 32) - 1
  print ("formIOV: ", s, u)
  return s,u

#############################################################################################
def formIovFromTo(runNumberStart, runNumber):
  s,u=((runNumberStart ) << 32), ((runNumber + 1) << 32) - 1
  print ("formIOV: ", s, u)
  return s,u
  
#############################################################################################
# this modification checks which was the k-last run before the processed one
def getRunNumberStart(runNumber, k):
    runlistfile = open("/afs/cern.ch/work/s/sctcalib/lastRuns.txt", mode = 'r')
    content = runlistfile.read()
    runlistfile.close()
    content_list = content.split("\n")
    content_list = [line for line in content_list if line.strip()]
    integer_int_list = list(map(int, content_list))
    if (runNumber >= integer_int_list[-1]):
        RNS = integer_int_list[len(integer_int_list)-(k+5)]
    else:
        RNS = integer_int_list[integer_int_list.index(runNumber)-(k+5)]
    return RNS
  
#############################################################################################
def GetRunList(dbstring, folder, tag, runNumber, k):
    db=openDatabase(dbstring)
    print('TAG   ',tag,'   TAG')
    folderlist=db.listAllNodes()
    print('    folderlist=db.listAllNodes()')

    for i in folderlist: print(i)

    myfolder=db.getFolder(folder)
    taglist=myfolder.listTags()
    print('    taglist=myfolder.listTags()')
    for i in taglist: print(i)
    
    runNumberStart = getRunNumberStart(runNumber, k)
    
    iovSince, iovUntil = formIovFromTo(runNumberStart,runNumber)
 
    temp=[]
    objs=myfolder.browseObjects(iovSince,iovUntil,cool.ChannelSelection.all(),tag)
    for obj in objs:
        #mypayload=obj.payload()
        #dfl=mypayload['DefectList']
        temp.append(obj.since()>>32)
    objs.close()
    db.closeDatabase()
    
    Temp=sorted(temp)
    #print('GetRunList: array is', Temp)
    ls=[]
    runtmp = -999
    if ( Temp[0] != runNumber ):
        ls.append(Temp[0])
        runtmp = Temp[0]
    for i in range(len(Temp)-1):
        if (Temp[i] != runtmp):
             if ( Temp[i] != runNumber ):
                 ls.append(Temp[i])
                 runtmp = Temp[i]
   
    print(ls)

    mylist=[]
    for i in range(k): mylist.append( ls[len(ls)-i-1] )
        
    return mylist

#############################################################################################
def GetNumNoisyMods(dbstring, folder, tag, runNumber):
    db=openDatabase(dbstring)

    myfolder=db.getFolder(folder)
    iovSince, iovUntil = formIov(runNumber)

    NumNoisyMods=0
    objs=myfolder.browseObjects(iovSince,iovUntil,cool.ChannelSelection.all(),tag)
    for obj in objs:
        mypayload=obj.payload()
        dfl=mypayload['DefectList']
        if not dfl:
            continue
        NumNoisyMods=NumNoisyMods+1
    objs.close()
    db.closeDatabase()
    
    return NumNoisyMods

#############################################################################################
def GetNumNoisyStrips(dbstring, folder, tag, runNumber):
    db=openDatabase(dbstring)
    myfolder=db.getFolder(folder)
    iovSince, iovUntil = formIov(runNumber) 

    #Array_numNoisyStrips, numNoisyStrips = [], 0
    numNoisyStrips = 0
    objs=myfolder.browseObjects(iovSince,iovUntil,cool.ChannelSelection.all(),tag)
    for obj in objs:
        mypayload=obj.payload()
        dfl=mypayload['DefectList']
        if not dfl:
            continue
        # line = str(obj.since()>>32)+"  "+str(dfl)
        line = str(dfl)
        words = line.split()
        numNoisyStripsAdds=0
        for j in range(len(words)):
            jk=words[j]
            if jk.find("-")>=0:
                sep=jk.replace ( '-', ' ' )
                sep1=sep.split()
                numNoisyStripsAdds=numNoisyStripsAdds+int(sep1[1])-int(sep1[0])
        numNoisyStrips=numNoisyStrips+len(words)+numNoisyStripsAdds
    objs.close()
    db.closeDatabase()

    return numNoisyStrips
