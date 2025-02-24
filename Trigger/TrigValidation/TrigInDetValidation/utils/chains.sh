#!/bin/bash

[ "x$TestArea" == "x" ] && echo "release not set up" && exit 1
    
cd $TestArea/Trigger/TrigValidation/TrigInDetValidation/utils



# disable this fragment

if [ 1 -eq 0 -o "x$1" == "x-a" ]; then

    
  cp TrigInDetArtSteps.py.pre   pre.py
  cp pre.py pre.py.bak

    
  for gitc in $(grep "^\#.*Chains" pre.py ) ; do 

    echo $gitc

    git=$(echo $gitc | sed 's|#||' | sed 's|Chains||')
    
    CHAINS=

    grep "$git:" comparitor.txt

    echo
    
    for CHAIN in $(grep "$git:" comparitor.txt | awk '{print $2}' | sed ' s|:.*||' | sed 's|_HLT.*||' | sort -u ) ; do 

	FOUND=1
	( grep -q $CHAIN $Athena_DIR/src/Trigger/TriggerCommon/TriggerMenuMT/python/HLT/Menu/* ) || FOUND=0

	[ $FOUND -eq 0 ] && echo "chain not found: $CHAIN" && continue
	
#	echo "  chains += \"'$CHAIN',\"\n"
	CHAINS="$CHAINS                chains += \"'$CHAIN',\"\n"
    
    done	

    printf  "$CHAINS"

    cat pre.py | sed "s|\($gitc\)|$CHAINS|" > pre2.py
    mv pre2.py pre.py

    echo " --------------------------------------------------"
    echo

  done

  cp pre.py $TestArea/Trigger/TrigValidation/TrigInDetValidation/python/TrigInDetArtSteps.py 
  
fi


if [ $# -eq 0 -o "x$1" == "x-d" ]; then

  CompChains=
  VertexCompChains=
  LRTCompChains=

    
  # create the dat file

  cp chains.save chains.dat

  cp TrigInDetArtSteps.py.pre   pre.py
  for gitc in $(grep "^\#.*Chains" pre.py ) ; do 

    echo $gitc

    git=$(echo $gitc | sed 's|#||' | sed 's|Chains||')

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
