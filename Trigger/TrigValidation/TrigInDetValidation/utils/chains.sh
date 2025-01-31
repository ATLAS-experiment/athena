#!/bin/bash

# grep -A 5 "if (i"  Trigger/TrigValidation/TrigInDetValidation/python/TrigInDetArtSteps.py

CompChains=
VertexCompChains=
LRTCompChains=

if [ $# -eq 0 -o "x$1" == "x-a" ]; then

  cp TrigInDetArtSteps.py.pre   pre.py
  cp pre.py pre.py.bak

    
  for git in $(grep "if ( *i=" pre.py | sed "s|or.*||" | sed "s|.*==.||" | sed "s|'.*||" | sed "s|.).*||" ) ; do

    echo $git

#    grep "$git:" comparitor.txt | awk '{print $2}' 
#    echo
#    grep "$git:" comparitor.txt | awk '{print $2}' | sed ' s|:.*||' | sed 's|_HLT.*||'
#    echo
#    grep "$git:" comparitor.txt | awk '{print $2}' | sed ' s|:.*||' | sed 's|_HLT.*||' | sort -u  

    CHAINS=
    
    for CHAIN in $(grep "$git:" comparitor.txt | awk '{print $2}' | sed ' s|:.*||' | sed 's|_HLT.*||' | sort -u ) ; do 

#	echo "  chains += \"'$CHAIN',\"\n"
	CHAINS="$CHAINS                chains += \"'$CHAIN',\"\n"
    
    done	

    printf  "$CHAINS"

    cat pre.py | sed "s|\($git\)Chains|$CHAINS|" > pre2.py
    mv pre2.py pre.py

    echo " --------------------------------------------------"
    echo

  done

  cp pre.py $TestArea/Trigger/TrigValidation/TrigInDetValidation/python/TrigInDetArtSteps.py 

fi




if [ $# -eq 0 -o "x$1" == "x-d" ]; then

  # create the dat file

  cp chains.save chains.dat

  for git in $(grep "if ( *i="  $TestArea/Trigger/TrigValidation/TrigInDetValidation/python/TrigInDetArtSteps.py | sed "s|or.*||" | sed "s|.*==.||" | sed "s|'.*||" | sed "s|.).*||" ) ; do
    
    gitrun=$git

    ( grep -q ${git}vtx comparitor.txt ) && gitrun="$git ${git}vtx"
    
    for sigs in $gitrun ; do

	echo "$sigs"
	
	for CHAIN in $(grep "$sigs:" comparitor.txt | awk '{print $2}' | sort -u ) ; do 

	    if ( echo $sigs | grep -q LRT ); then
		LRTCompChains="$LRTCompChains    \"$CHAIN\",\n"
	    elif ( echo $sigs | grep -q vtx ); then
		VertexCompChains="$VertexCompChains    \"$CHAIN\",\n"
	    else
		CompChains="$CompChains    \"$CHAIN\",\n"
	    fi
	    
	done	

	if ( echo $sigs | grep -q LRT ); then
	    LRTCompChains="$LRTCompChains\n"
	elif ( echo $sigs | grep -q vtx ); then
	    VertexCompChains="$VertexCompChains\n"
	else
	    CompChains="$CompChains\n"
	fi

    done
	
    
  done

  cp chains.dat chains.dat-pre
  cat chains.dat-pre | sed "s|VertexCompChains|$VertexCompChains|" | sed "s|LRTCompChains|$LRTCompChains|" | sed "s|CompChains|$CompChains|" > chains.dat

  cp chains.dat $TestArea/Trigger/TrigAnalysis/TrigInDetAnalysisUser/share/TIDAdata-chains-run3.dat

fi


if [ $# -eq 0 -o "x$1" == "x-j" ]; then
    generate_json.sh
fi 
