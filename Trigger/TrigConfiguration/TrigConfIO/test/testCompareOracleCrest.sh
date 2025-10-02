#!/usr/bin/bash

# this is a first version of a test comparing direct and crest access to the trigggerdb
# it can later be turned into a unit test

# keys from run 504392
smk=3491
l1psk=20025
hltpsk=14161
bgsk=2966

oracle_conn="TRIGGERDB_RUN3"
crest_conn="CONF_DATA_RUN3"

echo "removing old .json files"
rm -f ./*.json

echo "Downloading menu, prescales, bunchgroup, and monitoring from oracle"
echo "TriggerMenuRW --db ${oracle_conn} --smk ${smk} --l1psk ${l1psk} --hltpsk ${hltpsk} --bgsk ${bgsk} -w Oracle"
TriggerMenuRW --db ${oracle_conn} --smk ${smk} --l1psk ${l1psk} --hltpsk ${hltpsk} --bgsk ${bgsk} -w Oracle >& /dev/null 

echo "Downloading menu, prescales, bunchgroup, and monitoring via crest"
echo "TriggerMenuRW --crest-db ${crest_conn} --smk ${smk} --l1psk ${l1psk} --hltpsk ${hltpsk} --bgsk ${bgsk} -w Crest"
TriggerMenuRW --crest-db ${crest_conn} --smk ${smk} --l1psk ${l1psk} --hltpsk ${hltpsk} --bgsk ${bgsk} -w Crest >& /dev/null

all_match=true

# Compare two files and return:
# 0 = match, 1 = differ
compare_files() {
    local f1="$1"
    local f2="$2"
   
    if diff "$f1" "$f2" >/dev/null; then
        echo "Match: $f1 and $f2"
        return 0
    else
        echo "Mismatch: $f1 and $f2"
        all_match=false
        return 1
    fi
}

compare_files L1Menu_Oracle.json L1Menu_Crest.json

compare_files HLTMenu_Oracle.json HLTMenu_Crest.json

compare_files HLTMonitoring_Oracle.json HLTMonitoring_Crest.json

compare_files HLTJobOptions_Oracle.json HLTJobOptions_Crest.json

compare_files L1PrescalesSet_Oracle.json L1PrescalesSet_Crest.json

compare_files HLTPrescalesSet_Oracle.json HLTPrescalesSet_Crest.json

compare_files BunchGroupSet_Oracle.json BunchGroupSet_Crest.json

if $all_match; then
    echo "All pairs match."
    exit 0
else
    echo "One or more pairs differ."
    exit 1
fi