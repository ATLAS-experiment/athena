#!/bin/bash

COMP=comparitor.json
MENUDIR=$TestArea/Trigger/TriggerCommon/TriggerMenuMT/python/HLT/Menu


[ -e $COMP ] && rm $COMP

LAST=

# get all the specific sets and write json file ...

echo "{" >> $COMP

for JOB in $(grep HLT comparitor.txt | awk '{print $1}' ) ; do

    # make sure each set is only porocessed once ...
    
    [ "x$LAST" == "x$JOB" ] && continue

    echo "JOB: $JOB"
    
    # generate the key for the set ...
    
    JOBT=$( echo $JOB | sed 's|:||g' )
    
    # generate the chain list for this set ...
    
    [ -e tchains.log ] && rm tchains.log
    
    grep "$JOB" comparitor.txt | awk '{print $2}' > tchains.log 

    CHAINS=
    
    for CHAIN in $(cat tchains.log) ; do 

	# if the menu directory is avilable, check that the chains are in the menu first
	if [ -d $MENUDIR ]; then
	    SCHAIN=$(echo $CHAIN | sed 's|:.*||g' | sed 's|_HLT.*||g')
	    !( grep -q $SCHAIN $MENUDIR/*run3_v1.py ) && echo "chain: $SCHAIN for $JOB not in menu" && continue 
	fi
	    
	CHAINS="$CHAINS $CHAIN "
    done

    # write to the file ...

    [ "x$LAST" == "x" ] ||  printf ",\n" >> $COMP
    
    printf "   \"$JOBT\": {\n"                  >> $COMP
    printf "       \"chains\" : \"$CHAINS\" \n" >> $COMP 
    printf "   }"                               >> $COMP 

    LAST=$JOB

done

printf "\n}\n" >> $COMP

cp $COMP $TestArea/Trigger/TrigValidation/TrigInDetValidation/share/$COMP
