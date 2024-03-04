#!/bin/bash

if [[ $# < 4 ]] || [[ $# > 9 ]];
then
    echo "Syntax: $0 [-append] [-openiov] [-supercell] <tag> <Run1> <LB1>  <File> [Run2] [LB2]"
    echo "optional -append is adding the content of File to a DB"
    echo "optional -openiov is updating UPD4 with open end IOV, if Run2/LB2 is not given" 
    echo "optional -supercell is working for SC folders/tags"
    echo "<tag> can be 'UPD1', 'UPD4', 'UPD3' or 'BOTH' or 'All' or 'Bulk'.  'BOTH' means UPD1 and UPD4, UPD4 update is automatically updating also Bulk. All means UPD1,UPD3 and UPD4 with Bulk. Bulk is updating only Bulk"
    echo "<Run1> <LB1> are start IOV (for UPD4/Bulk)"
    echo "<File> is text file with changed channels, each line should have: B/E pos_neg FT Slot Channel CalibLine BadBitDescription"
    echo "optional Run2 LB2 are end IOV (for UPD4/Bulk) - first LB after the end of problem, if not given, the IOV lenght is exactly one 1 (unless -openiov is set)"
    exit
fi


outputSqlite="BadChannels.db"
outputSqliteOnl="BadChannelsOnl.db"
summaryFile="LArBuildBadChannelDB.summary.txt"

if [ $1 == "-append" ]
then
    echo "Appending to previous bad-channel list"
    append=1
    shift
else
    append=0
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

if [ $issc == 0 ]
then
   echo "Resolving current folder-level tag suffix for /LAR/BadChannelsOfl/BadChannels...."
   #fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" /LAR/BadChannelsOfl/BadChannels` 
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


tag=$1
shift
if [ $tag == "UPD1" ]
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
    echo "ERROR, expected 'UPD1', 'UPD4' or 'BOTH' or 'All' or 'Bulk' or 'UPD3' as type, got: $tag"
    exit 2
fi

echo "tags" ${tags}

#echo $1
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
    lbnumber=$1
    shift
else
    echo "ERROR: Expected a lumi-block-number, got $1"
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


if [[  $# > 0 ]]
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
    lbnumber2=$1
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
  lbnumber2=-1
fi

if [[ $openiov == 1 ]] && [[ $runnumber2 > 0 ]]
then
   echo "Could not handle -openiov and RunEnd at the same time"
   exit 7
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

  if [[ "$t" == *"UPD1"* ]]; then 
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
  echo "Running athena to read current database content...with run number " $runnumber

  python -m LArBadChannelTool.LArBadChannel2Ascii -r $runnumber -o $oldTextFile -d ${database} -t ${t} $SCParam > oracle2ascii_$t.log 2>&1 

  if [ $? -ne 0 ];  then
      echo "Athena reported an error reading back sqlite file ! Please check oracle2ascii_$t.log!"
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
  echo "Parameters: -o ${outputSql} -t $t -r $runnumber -l $lbnumber ${inputTextFile} $iovEnd"
  python -m LArBadChannelTool.LArBadChannelDBAlg -o ${outputSql} -t $t -r $runnumber -l $lbnumber -f ${folder} $SCParam ${inputTextFile} $iovEnd > ascii2sqlite_$t.log 2>&1
  if [ $? -ne 0 ];  then
    echo "Athena reported an error! Please check ascii2sqlite_$t.log!"
    exit 12
  fi

  if grep -q ERROR ascii2sqlite_$t.log
      then
      echo "An error occured during ascii2sqlite job! Please check ascii2sqlite_$t.log!"
      exit 13
  fi

  if grep -q "REJECTED" ascii2sqlite_$t.log
      then
      echo "ERROR: At least one line in the input text file could not be read. Syntax Error? See ascii2sqlite_$t.log"
  fi

  echo "Running athena to test readback of sqlite database file"
  python -m LArBadChannelTool.LArBadChannel2Ascii -o $outputTextFile -d $outputSql -t $t -f ${folder} -r $runnumber -l $lbnumber  $SCParam > sqlite2ascii_$t.log 2>&1
  if [ $? -ne 0 ];  then
      echo "Athena reported an error reading back sqlite file ! Please check sqlite2ascii_$t.log!"
      exit 14
  fi


  if grep  ERROR sqlite2ascii_$t.log
  then
      echo "An error occured during reading back sqlite file ! Please check sqlite2ascii_$t.log!"
      exit 15
  fi

  
  if [ $t == ${upd4TagName} ]
  then
     echo "Copying UPD4 to Bulk as well..."
     AtlCoolCopy "sqlite://;schema=${outputSql};dbname=CONDBR2"  "sqlite://;schema=${outputSql};dbname=CONDBR2"  -f ${folder} -t ${folder//\//}-${upd4TagName} -of ${folder} -ot ${folder//\//}-${BulkTagName}  -c > AtlCoolCopy.ofl.log 2>&1
  fi   

  if [ $t == ${upd1TagName} ]
      then
      echo "Copying UPD1 to offline database..."
      AtlCoolCopy "sqlite://;schema=${outputSql};dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${folder} -t ${folder//\//}-${upd1TagName} -of  ${folderofl} -ot ${folderofl//\//}-${upd1TagName} -c > AtlCoolCopy.onl.log 2>&1
      
      if [ $? -ne 0 ];  then
	  echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.onl.log!"
	  exit 16
      fi
  fi

  if [ -f $diffTextFile ]
      then 
      rm -rf $diffTextFile
  fi

  diff $oldTextFile $outputTextFile > $diffTextFile

  nNew=`grep -c ">" $diffTextFile`
  nGone=`grep -c "<" $diffTextFile`
  nTotal=`wc -l $outputTextFile | cut -f 1 -d " "`
  
  echo "" >> $summaryFile
  echo "Summary info for $t tag" >> $summaryFile
  echo "  Added $nNew bad channels and removed $nGone from $t list" >> $summaryFile
  echo "  Total number of channel in the new $t list: $nTotal"  >> $summaryFile
  echo "  Total number of channel in the new $t list: $nTotal"  >> $summaryFile
  echo " Output text files:" >> $summaryFile
  echo "  $outputTextFile: Text version of the new bad channel list (read back from sqlite)" >> $summaryFile
  echo "  $oldTextFile: Text version of the previous database content" >> $summaryFile
  echo "  $diffTextFile: Diff between the two lists" >> $summaryFile

  if [ $t == $upd4TagName ]
      then
      #echo "Was a UPD4 tag" 
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
echo "$outputSqlite: Containing UPD1 and/or UPD4 and/or Bulk version of bad-channel list for OFFLINE DB. UPD4 valid as of run $runnumber"
echo "Upload to OFFLINE oracle server:"
echo "export COOL_FLASK=http://aiatlas001.cern.ch:5000"
echo "/afs/cern.ch/user/a/atlcond/utilsflask/AtlCoolMerge.py  --flask ${outputSqlite} CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_LAR_W <password>"
if [ -f $outputSqliteOnl ];
then
    echo "$outputSqliteOnl: Containing UPD1 version of bad-channel list for ONLINE DB."
    echo "Upload to ONLINE oracle server using"
    echo "export COOL_FLASK=http://aiatlas001.cern.ch:5000"
    echo "/afs/cern.ch/user/a/atlcond/utilsflask/AtlCoolMerge.py BadChannels.db CONDBR2 ATONR_COOL ATLAS_COOLONL_LAR_W <password>"
fi 

