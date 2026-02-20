#!/bin/bash

if [[ $# < 3 ]];
then
  echo "Syntax: $0 [-append] [-offline] [-onerun] <Run> <LBb> [<LBe>] File1 [Folder] ..."
  exit 1
fi

inputTextFile="mf_input.txt"
outputTextFile="mf_output.txt"
outputSqlite="MissingFEBs.db"
outputSqliteOnl="MissingFEBsOnl.db"
oldTextFile="mf_previous.txt"
diffTextFile="mf_diff.txt"
upd3TagName="RUN2-UPD3-01"

if [ $1 == "-append" ]
then
    echo "Appending to previous bad-FEB list"
    catfiles=$oldTextFile
    shift
else
    catfiles=""
fi

if [ $1 == "-offline" ]
then
    echo "Creating only offline tags"
    online=0
    shift
else
    online=1 
fi

if [ $1 == "-onerun" ]
then
    echo "Creating closed IOV, for one run only"
    onerun=1
    shift
else
    onerun=0 
fi

if echo $1 | grep -q "^[0-9]\{5,\}$";
then
    runnumber=$1
    shift
else
    echo "ERROR: Expected a run-number, got $1"
    exit 1
fi


if echo $1 | grep -q "^[0-9]*$";
then
    lbnumber=$1
    shift
else
    echo "ERROR: Expected a lumi-block-number, got $1"
    exit 1
fi

if echo $1 | grep -q "^[0-9]*$";
then
  lbnumbere=$1
  runnumbere=$runnumber
  shift
else  
  lbnumbere=-1
fi  

if ! which AtlCoolCopy 1>/dev/null 2>&1
then
    echo "No offline setup found!"
    exit 2
fi

if [[ $online == 1 ]] && [[ $lbnumbere -ge 0 ]]
then
   echo "Make no sense closed IOV for online.... "
   exit 3
fi

if [[ $onerun == 1 ]] && [[ $lbnumbere -ge 0 ]]
then
   echo "WARNING: both -onerun and LB2 given.... LB2 taking precedence !!!!"
fi

if [ -f $inputTextFile ];
then
  echo "Temporary file $inputTextFile exists already. Please remove!"
  exit 4
fi

if [ -f $outputTextFile ];
then
  echo "Temporary file $outputTextFile exists already. Please remove!"
  exit 4
fi

if [ -f $outputSqlite ];
then
  echo "Output file $outputSqlite exists already. Please remove!"
  exit 4
fi

if [ -f $outputSqliteOnl ];
then
  echo "Output file $outputSqliteOnl exists already. Please remove!"
  exit 4
fi

if [ -f $oldTextFile ];
then
  echo "Output file $oldTextFile exists already. Please remove!"
  exit 4
fi


if [ -f ${outputSqlite}.tmp ];
then
    rm -f ${outputSqlite}.tmp
fi

echo "Left parameters: " $# $1

if [ ! -f $1 ];
    then
    echo "ERROR File $1 not found!"
    exit 4
fi
echo "Adding $1"
catfiles="${catfiles} ${1%%:}"
shift

echo "Left parameters: " $# $1
if [[ $# > 0 ]]
then
   Folder=$1
else   
   Folder="/LAR/BadChannelsOfl/MissingFEBs"
fi

#Get UPD4-nn tag connected to 'current':
echo "Resolving current folder-level tag suffix for ${Folder} ...."
fulltag=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" $Folder` 
upd4TagName=`echo $fulltag | grep -o "RUN2-UPD4-[0-9][0-9]"` 
gtag=`echo $fulltag | grep BLKPA | awk '{print $2}'`
echo "Found $gtag $upd4TagName"
fulltages=`getCurrentFolderTag.py "COOLOFL_LAR/CONDBR2" $Folder True` 
upd1TagName=`echo $fulltages | grep -o "RUN2-UPD1-[0-9][0-9]"` 
echo "Found $upd1TagName"

#create a tag string from folder
IFS='/' 
read -r -a array <<< "$Folder"
fldtag=""
for i in ${array[@]}
do
  fldtag+=$i
done
IFS=' ' 

if [[ $online == 1 ]] 
then
   mytag=${fldtag}-${upd1TagName}
   database="LAR_ONL"
else   
   mytag=${fldtag}-${upd4TagName}
   database="LAR_OFL"
fi


echo "Running athena to read current database content...",${database}
python -m LArBadChannelTool.LArMissingFeb2Ascii -r $runnumber -l $lbnumber -o $oldTextFile -d ${database} -t ${mytag} -f ${Folder} > oracle2ascii_$mytag.log 2>&1 
if [ $? -ne 0 ];  then
    echo "Athena reported an error reading back sqlite file ! Please check oracle2ascii_${mytag}.log!"
    exit 5
fi

echo "cat the files:"$catfiles":to " $inputTextFile
if [ ! -f ${oldTextFile} ];
    then
    echo "ERROR File mf_previous.txt not found!"
    exit 6
fi
cat $catfiles > $inputTextFile
#cat mf_previous.txt new.txt > $inputTextFile
#cp mf_previous.txt $inputTextFile
#cat new.txt >> $inputTextFile
if [ $? -ne 0 ];  then
    echo "Failed to concatinate input files!"
    exit 7
fi

if [ $lbnumbere -ge 0 ]; then
   endlb=$[ $lbnumbere + 1]
   prefix="IOVEndRun=${runnumber};IOVEndLB=$endlb;"
elif [ $onerun -eq 1 ]; then
   prefix=$[ $runnumber + 1]
   prefix="IOVEndRun=${prefix};IOVEndLB=0;"
fi  
if [[ $onerun == 0 ]]
then
    iovEnd=""
else
    if  [[ $runnumbere > 0 ]]
    then
        iovEnd="--runnumber2  $runnumbere"
        if [[ $lbnumbere > 0 ]]
        then
           iovEnd="${iovEnd} --lbnumber2 $lbnumbere"
        else
           iovEnd="${iovEnd} --lbnumber2 0"
        fi
    else   
        iovEnd="--runnumber2  $[ $runnumber + 1] --lbnumber2 0"
    fi
fi

echo "Tag: " $mytag
echo "Running athena to build sqlite database file ..."
#prefix="${prefix}IOVBeginRun=${runnumber};IOVBeginLB=${lbnumber};sqlite=\"${outputSqlite}.tmp\";Folder=\"${Folder}\";GlobalTag=\"${gtag}\";TagPostfix=\"-${upd4TagName}\";"
#echo "prefix: ${prefix}"
#athena.py -c $prefix LArBadChannelTool/LArMissingFebDbAlg.py > ascii2sqlite.log 2>&1
echo "Parameters: -o ${outputSql} -f $Folder -t $mytag -r $runnumber -l $lbnumber ${inputTextFile} $iovEnd"
python -m LArBadChannelTool.LArMissingFebsDBAlg -o ${outputSqlite}.tmp -t $mytag -r $runnumber -l $lbnumber -f ${Folder} ${inputTextFile} $iovEnd > ascii2sqlite_$mytag.log 2>&1

if [ $? -ne 0 ];  then
    echo "Athena reported an error! Please check ascii2sqlite_${mytag}.log!"
    exit 8
fi

 
if grep -q ERROR ascii2sqlite_${mytag}.log
then
    echo "An error occured during ascii2sqlite job! Please check ascii2sqlite_${mytag}.log!"
    exit 8
fi

if grep -q "REJECTED" ascii2sqlite_${mytag}.log
then
    echo "ERROR: At least one line in the input text file could not be read. Syntax Error?"
fi


cp ${outputSqlite}.tmp ${outputSqlite}

#if [ $onerun -eq 1 ] || [ $lbnumbere -ge 0 ]; then
#   pref="RunNumber=${runnumber};LBNumber=${lbnumber};"
#else   
#   pref=""
#fi
#pref="${pref}sqlite=\"${outputSqlite}\";OutputFile=\"${outputTextFile}\";Folder=\"${Folder}\";GlobalTag=\"${gtag}\";tag=\"${fldtag}-${upd4TagName}\";"
echo "Running athena to test readback of sqlite database file"
#athena.py  -c ${pref} LArBadChannelTool/LArMissingFebs2Ascii.py > sqlite2ascii.log 2>&1
echo "Parameters....  -o $outputTextFile -d $outputSqlite -t $mytag -f ${Folder} -r $runnumber -l $lbnumber "
python -m LArBadChannelTool.LArMissingFeb2Ascii -o $outputTextFile -d $outputSqlite -t $mytag -f ${Folder} -r $runnumber -l $lbnumber  > sqlite2ascii_$mytag.log 2>&1

if [ $? -ne 0 ];  then
    echo "Athena reported an error reading back sqlite file ! Please check sqlite2ascii_${mytag}.log!"
    exit 9
fi


if grep -q ERROR sqlite2ascii_${mytag}.log
then
    echo "An error occured during reading back sqlite file ! Please check sqlite2ascii_${mytag}.log!"
    exit 9
fi

if [ $online -eq 1 ]; then
   echo "Copying UPD1 to UPD4 tag..."
   AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${Folder} -t $mytag -ot ${fldtag}-${upd4TagName}  > AtlCoolCopy.upd4.log 2>&1

   if [ $? -ne 0 ];  then
       echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.upd4.log!"
       exit 10
   fi
else   
   echo "Copying UPD4 to UPD1 tag..."
   AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${Folder} -t $mytag -ot ${fldtag}-${upd1TagName}  > AtlCoolCopy.upd1.log 2>&1

   if [ $? -ne 0 ];  then
       echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.upd1.log!"
       exit 10
   fi
fi

echo "Copying UPD4 to UPD3 tag..."
AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqlite};dbname=CONDBR2" -f ${Folder} -t $mytag -ot ${fldtag}-${upd3TagName} > AtlCoolCopy.upd3.log 2>&1

if [ $? -ne 0 ];  then
    echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.upd3.log!"
    exit 10
fi

if ! grep -q ${runnumber} /afs/cern.ch/user/a/atlcond/scratch0/nemo/prod/web/calibruns.txt
    then
    echo " *** WARNING *** Run ${runnumber} is not on the NEMO watchlist! Outside of CalibLoop? Not a stable-beam run?" 
fi


if [ $online -eq 1 ]; then
   echo "Copying to for online database..."
   #create a online folder name
   IFS='/' 
   read -r -a array1 <<< "$Folder"
   onlfld=""
   for i in ${array1[@]}
   do
     if [[ $i == "BadChannelsOfl" ]] 
     then 
         onlfld+="/BadChannels"
         continue
     fi    
     onlfld+="/"$i
   done
   onlfld=${onlfld##/}
   #create the online tag
   read -r -a array2 <<< "$onlfld"
   onlfldtag=""
   for i in ${array2[@]}
   do
     onlfldtag+=$i
   done
   IFS=' ' 

   echo "Copying to the: "${onlfld} " with tag " ${onlfldtag}-${upd1TagName}
   if [ $onerun -eq 1 ]; then
      AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqliteOnl};dbname=CONDBR2" -f ${Folder} -t $mytag -of ${onlfld} -ot ${onlfldtag}-${upd1TagName} -c > AtlCoolCopy.onl.log 2>&1
   else   
      AtlCoolCopy "sqlite://;schema=${outputSqlite}.tmp;dbname=CONDBR2" "sqlite://;schema=${outputSqliteOnl};dbname=CONDBR2" -f ${Folder} -t $mytag -of ${onlfld} -ot ${onlfldtag}-${upd1TagName} -r 2147483647 -a -c > AtlCoolCopy.onl.log 2>&1
   fi   


   if [ $? -ne 0 ];  then
      echo "AtlCoolCopy reported an error! Please check AtlCoolCopy.onl.log!"
      exit 10
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

echo "Added $nNew missing FEBs and removed $nGone"
echo "Total number of FEBs in the new list: $nTotal" 

if [ -f ${outputSqlite}.tmp ];
then
    rm -f ${outputSqlite}.tmp
fi


echo "Output files:"
echo "$outputSqlite: Containing UPD3 version of the missing FEB list for the OFFLINE DB. UPD3 valid as of run $runnumber"
if [ $online -eq 1 ]; then
   echo "            and UPD1 for the OFFLINE DB"
   echo "$outputSqliteOnl: Containing UPD1 version of the missing FEB list for the ONLINE DB"
fi   
echo "$outputTextFile: Text version of the new bad channel list (read back from sqlite)"

echo "" 
echo "Upload to OFFLINE oracle server using"
echo "export COOL_FLASK=https://cool-proxy-app.cern.ch"
echo "/afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --flask ${outputSqlite} CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_LAR_W <password>"
echo ""
if [ $online -eq 1 ]; then
  echo "Upload to ONLINE oracle server using"
  echo "/afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --online ${outputSqliteOnl} CONDBR2 ATONR_COOL ATLAS_COOLONL_LAR_W <password>"
fi


