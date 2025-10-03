#!/bin/bash

if [[ $# < 4 ]] || [[ $# > 9 ]];
then
    echo "Syntax: $0 [-append] [-mc] [-openiov] [-supercell] <tag> <Run1> <LB1> <File> [Run2] [LB2]"
    echo "    or: $0 -mc [-append] [-newtag] [-openiov] [-supercell] <tag> <Run1> <Run0> <File> [Run2]"
    echo "optional -mc switches to OFLP200, arguments are then interpreted differently"
    echo "optional -append is adding the content of File to a DB"
    echo "optional -newtag is creating a new tag, named by incrementing the tag suffix, and filling all IOVs (only with -mc)"
    echo "optional -openiov is updating UPD4 with open end IOV, if Run2/LB2 is not given" 
    echo "optional -supercell is working for SC folders/tags"
    echo "<tag> can be 'UPD1', 'UPD4', 'UPD3' or 'BOTH' or 'All' or 'Bulk'.  'BOTH' means UPD1 and UPD4, UPD4 update is automatically updating also Bulk. All means UPD1,UPD3 and UPD4 with Bulk. Bulk is updating only Bulk. When running with -mc the actual tag must be specified instead of those keywords."
    echo "<Run1> <LB1> are start IOV (for UPD4/Bulk)"
    echo "<Run0> specifies the IOV from which bad channels are read (only with -mc). If set to 0, Run1 is used."
    echo "<File> is text file with changed channels, each line should have: B/E pos_neg FT Slot Channel CalibLine BadBitDescription"
    echo "optional Run2 LB2 are end IOV (for UPD4/Bulk) - first LB after the end of problem, if not given, the IOV lenght is exactly one 1 (unless -openiov is set)"
    exit
fi


outputSqlite="BadChannels.db"
outputSqliteOnl="BadChannelsOnl.db"
summaryFile="LArBuildBadChannelDB.summary.txt"

if [ $1 == "-mc" ]
then
    mc="--MC"
    dbname=OFLP200
    shift
else
    mc=""
    dbname=CONDBR2
fi
if [ $1 == "-append" ]
then
    echo "Appending to previous bad-channel list"
    append=1
    shift
else
    append=0
fi
if [ $1 == "-newtag" ]
then
    if [[ $mc != "" ]]; then
        echo "Create and fill a new tag"
        newtag=1
    else
        echo "ERROR: -newtag is not supported without -mc, ignored."
        newtag=0
    fi
    shift
else
    newtag=0
fi
if [ $1 == "-openiov" ]
then
    echo "Open ended IOV"
    openiov=1
    shift
else
    openiov=0
fi
if [ $1 == "-supercell" ]
then
    echo "Working on SC"
    issc=1
    shift
else
    issc=0
fi

if [[ $mc == "" ]]
then
    if [ $issc == 0 ]
    then
        echo "Resolving current folder-level tag suffix for /LAR/BadChannelsOfl/BadChannels...."
        # should be simply: fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" /LAR/BadChannelsOfl/BadChannels`
        # but for the moment, the configuration of acron jobs using this script apparently requires explicitly including frontier in the connection string:
        fulltag=`getCurrentFolderTag.py "frontier://ATLF/();schema=ATLAS_COOLOFL_LAR;dbname=CONDBR2" /LAR/BadChannelsOfl/BadChannels` 
        if [ $? -ne 0 ]
        then
            exit 1
        fi
        upd4TagName=`echo $fulltag | grep -o "RUN2-UPD4-[0-9][0-9]"` 
        echo "Found UPD4 $upd4TagName"
        upd1TagName="RUN2-UPD1-00"
        BulkTagName="RUN2-Bulk-00"
        upd3TagName="RUN2-UPD3-00"
    else   
        echo "Resolving current folder-level tag suffix for /LAR/BadChannelsOfl/BadChannelsSC...."
        #fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" /LAR/BadChannelsOfl/BadChannelsSC` 
        # same as above
	fulltag=`getCurrentFolderTag.py "frontier://ATLF/();schema=ATLAS_COOLOFL_LAR;dbname=CONDBR2" /LAR/BadChannelsOfl/BadChannelsSC` 
        if [ $? -ne 0 ]
        then
            exit 1
        fi
        upd4TagName=`echo $fulltag | grep -o "RUN3-UPD4-[0-9][0-9]"` 
        echo "Found UPD4 $upd4TagName"
        upd1TagName="RUN3-UPD1-00"
        BulkTagName="RUN3-Bulk-00"
        upd3TagName="RUN3-UPD3-00"
    fi
else
        upd1TagName=""
        BulkTagName=""
        upd3TagName=""
fi

tag=$1
shift
if [[ $mc != "" ]]
then
   echo "Working on OFLP200"
   tags="${tag}"
elif [ $tag == "UPD1" ]
    then
    echo "Working on UPD1 list"
    tags="${upd1TagName}"
elif [ $tag == "UPD4" ]
    then
    echo "Working on UPD4 list"
    tags="${upd4TagName}"
elif [ $tag == "UPD3" ]
    then
    echo "Working on UPD3 list"
    tags="${upd3TagName}"
elif [ $tag == "Bulk" ]
    then
    echo "Working on Bulk list"
    tags="${BulkTagName}"
elif [ $tag == "BOTH" ]
    then
    echo "Working on UPD1 and UPD4 lists"
    tags="${upd1TagName} ${upd4TagName}"
elif [ $tag == "All" ]
    then
    echo "Working on UPD1, UPD3, Bulk and UPD4 lists"
    tags="${upd1TagName} ${upd3TagName} ${BulkTagName} ${upd4TagName}"
else
    echo "ERROR, for operations on CONDBR2 expected 'UPD1', 'UPD4' or 'BOTH' or 'All' or 'Bulk' or 'UPD3' as type, got: $tag"
    exit 2
fi

echo "tags" ${tags}

if echo $1 | grep -q "^[0-9]*$";
then
    runnumber=$1
    shift
else
    echo "ERROR: Expected a run-number, got $1"
    exit 3
fi

if echo $1 | grep -q "^[0-9]*$";
then
    if [[ $mc == "" ]]; then
        lbnumber=$1
        runnumber0=$runnumber
    else
        runnumber0=$1
	lbnumber=0
    fi
    shift
else
    echo "ERROR: Expected a lumi-block-number (or a run-number with -mc), got $1"
    exit 4
fi

if [[  $# == 0 ]]
    then
    echo "ERROR: No input files found!"
    exit 5
fi

if [ ! -f $1 ];
      then
      echo "ERROR File $1 not found!"
      exit 6
fi
echo "Adding $1"
catfiles=" $1"
shift

if [[ $# > 0 ]]
then
   if echo $1 | grep -q "^[0-9]*$";
   then
     runnumber2=$1
     shift
   else
     echo "ERROR: Expected a run-number, got $1, not using this parameter !!!"
     runnumber2=-1
     shift
   fi
else   
   runnumber2=-1
fi

if [[  $# > 0 ]]
then
  if echo $1 | grep -q "^[0-9]*$";
  then
    if [[ $mc == "" ]]; then
        lbnumber2=$1
     else
        lbnumber2=$0
        echo "WARNING: LB2 argument incompatible with -mc, ignored."
    fi
    shift
  else
    echo "ERROR: Expected a lumi-block-number, got $1, not using Run2/lb2 parameter !!!"
    runnumber2=-1
    lbnumber2=-1
  fi
#  if [[ $runnumber2 > 0 ]] && [[ $lbnumber2 == 0 ]]
#  then
#    runnumber2=$[ $runnumber2 - 1 ]
#    lbnumber2=4294967295 
#  fi  
else
  lbnumber2=0
fi

if [[ $openiov == 1 ]] && [[ $runnumber2 > 0 ]]
then
   echo "Could not handle -openiov and RunEnd at the same time"
   exit 7
fi

if [[ $mc != "" ]] && [[ $runnumber2 < $runnumber ]] && [[ $openiov == 0 ]]
then
    echo "For operations on OFLP200, please either use -openiov or provide an explicit IOV upper bound!"
    exit 107
fi

if [ -f $outputSqlite ];
    then
    echo "WARNING: Output file $outputSqlite exists already. Will be overwritten or modified!"
fi

if [ -f $outputSqliteOnl ];
    then
    if echo $tags | grep -q UPD1
    then
    echo "WARNING: Output file $outputSqliteOnl exists already. Will be overwritten!"
    fi
fi

if [ -f $summaryFile ]
    then 
    rm -rf $summaryFile
fi

touch $summaryFile



if ! which AtlCoolCopy 1>/dev/null 2>&1
then
    echo "No offline setup found!"
    exit 8
fi

for t in $tags
do

  echo Working on tag $t 
  inputTextFile="bc_input_$t.txt"
  outputTextFile="bc_output_$t.txt"
  oldTextFile="bc_previous_$t.txt"
  diffTextFile="bc_diff_$t.txt"


  if [ -f $inputTextFile ];
      then
      echo "Temporary file $inputTextFile exists already. Please remove!"
      exit 9
  fi

  if [ -f $outputTextFile ];
      then
      echo "Temporary file $outputTextFile exists already. Please remove!"
      exit 9
  fi

  if [ -f $oldTextFile ];
      then
      echo "Output file $oldTextFile exists already. Please remove!"
      exit 9
  fi

  if [ $issc == 1 ];
      then
      SCParam="--SC"
  else
      SCParam=""
  fi

  if [[ $mc != "" ]]; then
      database="LAR_OFL"
      folder="/LAR/BadChannels/BadChannels"
      outputSql=${outputSqlite}
  elif [[ "$t" == *"UPD1"* ]]; then 
      database="LAR_ONL"
      folder="/LAR/BadChannels/BadChannels"
      folderofl="/LAR/BadChannelsOfl/BadChannels"
      outputSql=${outputSqliteOnl}
  else
      database="LAR_OFL"
      folder="/LAR/BadChannelsOfl/BadChannels"
      outputSql=${outputSqlite}
  fi
  if [ $issc == 1 ];
      then
      folder=${folder}"SC"
      if [[ "$t" == *"UPD1"* ]]; then 
         folderofl=${folderofl}"SC"
      fi   
  fi
  echo "Running athena to read current database content...with run number " $runnumber0

  o2alog=oracle2ascii_$t.log
  python -m LArBadChannelTool.LArBadChannel2Ascii -r $runnumber0 -o $oldTextFile -d ${database} -t ${t} $SCParam $mc > $o2alog 2>&1 
  # in case reading status from some online sqlite file: 
  #python -m LArBadChannelTool.LArBadChannel2Ascii -r $runnumber0 -o $oldTextFile -d "old/BadChannelsOnl.db" -f /LAR/BadChannels/BadChannelsSC -t "LARBadChannelsBadChannelsSC-RUN3-UPD1-00" $SCParam > oracle2ascii_$t.log 2>&1 

  if [ $? -ne 0 ];  then
      echo "Athena reported an error reading back sqlite file ! Please check ${o2alog}!"
      exit 10
  fi

  if [ $append == 1 ]
      then 
      catfiles1="$oldTextFile $catfiles"
  else
      catfiles1=$catfiles
  fi

  cat $catfiles1 > $inputTextFile
  if [ $? -ne 0 ];  then
      echo "Failed to concatenate input files!"
      exit 11
  fi

  iovEnd=""
  echo "$t and  $openiov" 
  if [[ $t == ${upd1TagName} || $openiov == 1 ]]
      then
      iovEnd=""
  else
      if  [[ $runnumber2 > 0 ]]
      then
          iovEnd="--runnumber2  $runnumber2 --lbnumber2 $lbnumber2"
      else  
          iovEnd="--runnumber2  $[ $runnumber + 1] --lbnumber2 0"
      fi  
  fi
  echo "Running athena to build sqlite database file ..."
  echo "Parameters..."
  echo "Parameters: -o ${outputSql} -t $t -r $runnumber -l $lbnumber -f ${folder} $SCParam $mc ${inputTextFile} ${inputTextFile} $iovEnd"
  a2slog=ascii2sqlite_$t.log
  python -m LArBadChannelTool.LArBadChannelDBAlg -o ${outputSql} -t $t -r $runnumber -l $lbnumber -f ${folder} $SCParam $mc ${inputTextFile} $iovEnd > $a2slog 2>&1
  if [ $? -ne 0 ];  then
    echo "Athena reported an error! Please check $a2slog!"
    exit 12
  fi

  if grep -q ERROR $a2slog
      then
      echo "An error occured during ascii2sqlite job! Please check $a2slog!"
      exit 13
  fi

  if grep -q "REJECTED" $a2slog
      then
      echo "ERROR: At least one line in the input text file could not be read. Syntax Error? See $a2slog"
  fi

  echo "Running athena to test readback of sqlite database file"
  s2alog=sqlite2ascii_$t.log
  python -m LArBadChannelTool.LArBadChannel2Ascii -o $outputTextFile -d $outputSql -t $t -f ${folder} -r $runnumber -l $lbnumber  $SCParam $mc > $s2alog 2>&1
  if [ $? -ne 0 ];  then
      echo "Athena reported an error reading back sqlite file ! Please check $s2alog!"
      exit 14
  fi


  if grep  ERROR $s2alog
  then
      echo "An error occured during reading back sqlite file ! Please check $s2alog!"
      exit 15
  fi

  
  if [[ $t == ${upd4TagName} ]]
  then
     echo "Copying UPD4 to Bulk as well..."
     AtlCoolCopy "sqlite://;schema=${outputSql};dbname=$dbname"  "sqlite://;schema=${outputSql};dbname=$dbname"  -f ${folder} -t ${folder//\//}-${upd4TagName} -of ${folder} -ot ${folder//\//}-${BulkTagName}  -c > AtlCoolCopy.ofl.log 2>&1
  fi   

  if [[ $t == ${upd1TagName} ]]
      then
      echo "Copying UPD1 to offline database..."
      AtlCoolCopy "sqlite://;schema=${outputSql};dbname=$dbname" "sqlite://;schema=${outputSqlite};dbname=$dbname" -f ${folder} -t ${folder//\//}-${upd1TagName} -of  ${folderofl} -ot ${folderofl//\//}-${upd1TagName} -c > AtlCoolCopy.onl.log 2>&1
      
      if [ $? -ne 0 ];  then
	  echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.onl.log!"
	  exit 16
      fi
  fi
  if [[ $newtag == 1 ]]
  then
      t1=$(echo $t | gawk 'match($0, "(.*)-([0-9]+)", x) {printf("%s-%02d", x[1], x[2]+1)}')
      echo "Copying existing and new IOV(s) to new tag $t1..."
      dbstr="sqlite://;schema=$outputSqlite;dbname=$dbname"
      AtlCoolCopy COOLOFL_LAR/$dbname $dbstr -f $folder -t $t -ot $t1
      AtlCoolCopy $dbstr $dbstr -f $folder -t $t -ot $t1
  fi

  if [ -f $diffTextFile ]
      then 
      rm -rf $diffTextFile
  fi

  diff $oldTextFile $outputTextFile > $diffTextFile

  nNew=`grep -c "^>" $diffTextFile`
  nGone=`grep -c "^<" $diffTextFile`
  nTotal=`wc -l $outputTextFile | cut -f 1 -d " "`
  echo "Summary info for $t tag" >> $summaryFile
  echo "  Added $nNew bad channels and removed $nGone from $t list" >> $summaryFile
  echo "  Total number of channel in the new $t list: $nTotal"  >> $summaryFile
  echo " Output text files:" >> $summaryFile
  echo "  $outputTextFile: Text version of the new bad channel list (read back from sqlite)" >> $summaryFile
  echo "  $oldTextFile: Text version of the previous database content" >> $summaryFile
  echo "  $diffTextFile: Diff between the two lists" >> $summaryFile

  if [[ $t == $upd4TagName ]]
      then
      if ! grep -q ${runnumber} /afs/cern.ch/user/a/atlcond/scratch0/nemo/prod/web/calibruns.txt
      then
          echo " *** WARNING *** Run ${runnumber} is not on the NEMO watchlist! Outside of CalibLoop?" >> $summaryFile
      fi
  fi

  echo "Done with $t"
  echo ""
done


cat $summaryFile

echo "Output sqlite files:"
if [[ $mc == "" ]]
then
    echo "$outputSqlite: Containing UPD1 and/or UPD4 and/or Bulk version of bad-channel list for OFFLINE DB. UPD4 valid as of run $runnumber"
else
    echo "$outputSqlite: Containing bad-channel list for OFFLINE MC DB."
fi
echo "Upload to OFFLINE oracle server:"
echo "export COOL_FLASK=https://cool-proxy-app.cern.ch"
if [[ $newtag == 0 ]]
then
    echo "/afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --flask ${outputSqlite} $dbname ATONR_COOLOFL_GPN ATLAS_COOLOFL_LAR_W password"
else
    echo "/afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --flask --folder $folder --tag $t1 $outputSqlite $dbname ATONR_COOLOFL_GPN ATLAS_COOLOFL_LAR_W password"
fi
if [ -f $outputSqliteOnl ];
then
    echo "$outputSqliteOnl: Containing UPD1 version of bad-channel list for ONLINE DB."
    echo "Upload to ONLINE oracle server using"
    echo "export COOL_FLASK=https://cool-proxy-app.cern.ch"
    echo "/afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --online $outputSqliteOnl CONDBR2 ATONR_COOL ATLAS_COOLONL_LAR_W <password>"
fi 
