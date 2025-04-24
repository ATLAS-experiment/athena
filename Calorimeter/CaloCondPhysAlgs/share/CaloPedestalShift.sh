#!/bin/bash

if [[ $# < 3 ]];
then
    echo "Syntax: $0 <filename> <tag> <Run1> <LB1>  <Run2> <LB2>"
    echo "<tag> : can be UPD4 or BOTH"
    echo "<Run1> <LB1> : run /lumiblock for IoV start of database update"
    echo "<Run2> <LB2> : run /lumiblock for IoV end of database update (if not given use open-ended IoV)"
    exit
fi

summaryFile="pedestal.summary.txt"

inputfilename=$1
tag=$2
run1=$3 
lb1=$4 
run2=-1
lb2=-1

if [[ $# >4 ]];
then
 run2=$5
 lb2=$6
fi

echo " Run /LB for IoV start update for pedestal     " ${run1} ${lb1}
if [[ $run2 > 0 ]];
then
 echo " Run /LB for IoV end update for pedestal      " ${run2} ${lb2}
fi

echo "    be careful: you need ~60 Mbytes of free disk space in the running directory... "

if [ -f caloSqlite.db ];
    then
    echo "ERROR: Output file caloSqlite exists already. Remove it before running this script !"
    exit
fi

if [ -f $summaryFile ]
    then 
    /bin/rm  $summaryFile
fi

if [ -f calopedestal.txt ]
    then
    /bin/rm calopedestal.txt
fi

if [ -f pedestalshift_data.root ]
    then
    /bin/rm pedestalshift_data.root
fi

touch $summaryFile

if ! which AtlCoolCopy 1>/dev/null 2>&1
then
    echo "No offline setup found!"
    exit
fi

upd1TagName="UPD1"
upd4TagName="UPD4"

if [ $tag == "UPD4" ]
    then
    echo "Working on UPD4 list"
    tags="${upd4TagName}"
elif [ $tag == "BOTH" ]
    then
    echo "Working on UPD1 and UPD4 lists"
    tags="${upd1TagName} ${upd4TagName}"
else
    echo "ERROR, expected  'UPD4' or 'BOTH' as type, got: $tag"
    exit 2
fi

echo "tags" ${tags}

for t in $tags
do
  echo $t
  echo "use tag" $t
  echo "Running athena to add pedestal shift values to existing ${t} tag"
  python -m CaloCondPhysAlgs.CaloAddPedShiftConfig  --input ${inputfilename}  --run ${run1} --lb ${lb1}  > pedestal.log 2>&1
  if [ $? -ne 0 ];  then
        echo "Athena reported an error ! Please check pedestal.log!"
        exit
  fi

  if grep -q ERROR pedestal.log
        then
        echo "An error occured ! Please check pedestal.log!"
        exit
  fi

  nLines=`wc -l calopedestal.txt | cut -f 1 -d " "`
  echo " Ncells*Ngains for which pedestal is computed  " ${nLines}
  if [ $nLines -ne 547404 ]; then
       echo "Number of lines in pedstal ascii file is not correct, check log file to investigate possible problem \(or disk space\)"
       exit
  fi

  grep "Pedestal shift found for cell" pedestal.log > ${summaryFile}
  nTotal=`wc -l ${summaryFile} | cut -f 1 -d " "`
  echo " ===> Number of cells*gains for which pedestal changed " ${nTotal}
  echo "      Check file "  ${summaryFile} " for the full list "
  echo "      You can also look at pedestalshift_data.root for ntuple information "

  echo "Produce sqlite file for LAr cell pedestalshift values" 
  CaloPedestal_fillDB.py ${run1} ${lb1} ${run2} ${lb2} ${t} > makedb.log 2>&1
  if [ $? -ne 0 ];  then
      echo "An error is reported, check makedb.log"
      exit
  fi

  if [ -f dumpDB_$t.py ]
    then
      /bin/rm dumpDB_$t.py
  fi

  if [[ "$t" == *"UPD1"* ]]; then 
      tagname="'CALOOflPedestalCellPedestal-UPD1-00'"
  elif [[ "$t" == *"UPD4"* ]]; then
      tagname="'CALOOflPedestalCellPedestal-UPD4-00'"
  fi

  cat > dumpDB_$t.py << _EOF_
from CoolConvUtilities import AtlCoolTool
toolNew = AtlCoolTool.AtlCoolTool("sqlite://;schema=caloSqlite.db;dbname=CONDBR2")
toolNew.usetag($tagname)
a=toolNew.more('/CALO/Ofl/Pedestal/CellPedestal')
f = open('dumpDB_$t.txt','w')
f.write(str(a))
f.close()
exit()
_EOF_

  if [ -f dumpDB_$t.txt ]
      then
       /bin/rm dumpDB_$t.txt
  fi

  python dumpDB_$t.py

  echo " "
  echo "   Content of new sqlite file : "
  cat dumpDB_$t.txt

done

echo "  "
echo "  After checking that everything is OK, upload the new sqlite file with:"
echo " export COOL_FLASK=https://cool-proxy-app.cern.ch"
echo "  /afs/cern.ch/user/a/atlcond/utilsproxy/AtlCoolMerge.py --flask caloSqlite.db  CONDBR2 ATONR_COOLOFL_GPN ATLAS_COOLOFL_CALO_W <password>"
exit
