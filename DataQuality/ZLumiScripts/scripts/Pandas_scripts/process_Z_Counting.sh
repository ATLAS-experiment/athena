#!/usr/bin/env bash

# usage:
# ./process_Z_Counting.sh [year] [update HISTs 0/1] [update CSVs 0/1] [update GRL]
# e.g.
# ./process_Z_Counting.sh 24 1 1 0
# where 'update=1' will only process new files not processed before (default) and '0' will overwrite everything or create new GRL

if [[ $# -ge 1 ]]; then
    year=$1
    echo "Processing Z counting for year 20${year}"
    if [[ $1 -ne 24 ]]; then
	echo "Script unvalidated for years other than 2024 - remove this block later"
	exit 1
    fi
else
    echo "Need to give year to process: e.g. 24 (25, ...)"
    exit 1
fi
if [[ $# -lt 2 ]] || [[ $2 -gt 0 ]]; then
    updateH=1
    echo "Will keep existing HISTs and only add new ones"
else
    echo "Will overwrite all HISTs on Z counting EOS"
    updateH=0
fi
if [[ $# -lt 3 ]] || [[ $3 -gt 0 ]]; then
    updateC=1
    echo "Will keep existing CSVs and only add new ones"
else
    echo "Will overwrite all CSVs on Z counting EOS"
    updateC=0
fi
if [[ $# -lt 4 ]] || [[ $4 -gt 0 ]]; then
    updateGRL=1
    echo "Will create new temporary GRL"
else
    updateGRL=0
    echo "Will use last/official GRL"
fi

dataset=data${year}_13p6TeV
indir=/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/${dataset}/
eoshistdir=/eos/atlas/atlastier0/rucio/${dataset}/physics_Main/
tmpdir=/tmp/${USER}

logdir=$indir
oldlogfile=${logdir}/process.logfile
newlogfile=${logdir}/process.logfile.new
skipruns=${logdir}/skipruns
Available_Runs=()

# this uses the following /eos dir as master source for (new) runs - not the best in long term?
# (silently) reject dirs that are not integers and that are in the manually excluded run list
# keep origin info of old log file
rm -f $newlogfile
for dir in `find $eoshistdir -mindepth 1 -maxdepth 1 -name "*" -print`; do
    run_num=${dir: -6}
    if [[ $run_num =~ ^[0-9]+$ ]] && ! grep -q $run_num $skipruns; then
	Available_Runs+=($run_num)
	if [[ ! -f $oldlogfile ]] || ! grep -q $run_num $oldlogfile; then
	    echo $run_num >> $newlogfile
	else
	    grep $run_num $oldlogfile >> $newlogfile
	fi
    fi
done

echo
echo "Available Runs for 20${year} = " ${Available_Runs[@]}
if [ -f $oldlogfile ]; then
    echo Last HIST copy was on `stat -c %y $oldlogfile`
    awk '{print $1}' $oldlogfile > ${oldlogfile}.c1
    awk '{print $1}' $newlogfile > ${newlogfile}.c1
    echo New runs since last processing: `comm -13 ${oldlogfile}.c1 ${newlogfile}.c1`
    echo Apparently lost runs since last processing: `comm -23 ${oldlogfile}.c1 ${newlogfile}.c1`
    rm ${oldlogfile}.c1 ${newlogfile}.c1
else
    echo "First run ever for this year?"
fi

echo
echo "Checking that all input histos are available on EOS and trying to refresh"

for run in ${Available_Runs[@]}; do
    if [[ ! -f ${indir}${run}.HIST.root ]] || [[ $updateH == 0 ]]; then
	# our small HIST file for this run is not in eos yet
	# first we look on Tier0 /eos for the HIST file, then we try rucio
	userucio=false
	histfile=`echo ${eoshistdir}00${run}/${dataset}.00${run}.physics_Main.merge.HIST.f*_h*/${dataset}.00${run}.physics_Main.merge.HIST.f*_h*._0001.?`
	if [[ -f $histfile ]]; then
	    echo "Using file $histfile as source of Z couting histograms for run $run"
	else
            if ! command -v rucio &> /dev/null; then
		echo "$eoshistdir does not have the needed input histogram file for run $run - if you setup rucio I will try download from there"
		sed -i "/${run}/d" $newlogfile # remove run again from log file
		continue
	    fi
            rucioset=`rucio ls --short ${dataset}.00${run}.physics_Main.merge.HIST.f*_h*|sed "s/^.*://"`
            nrucioset=`echo $rucioset | wc -w`
	    userucio=true
            if [[ $nrucioset -eq 0 ]]; then
		echo "Hist for run $run not yet saved to eos, but cannot find rucio dataset"
		sed -i "/${run}/d" $newlogfile # remove run again from log file
		continue
            elif [[ $nrucioset -gt 1 ]]; then 
		echo "HIST for run $run not yet saved to eos, but have multiple rucio datasets, will take last! " $rucioset
		rucioset=`echo ${rucioset} | awk '{print $NF;}'`
		rucio get $rucioset --dir=$tmpdir >& /dev/null
            else
		echo "HIST for run $run not found, downloading from " $rucioset
		rucio get $rucioset --dir=$tmpdir >& /dev/null
            fi
            if [[ $? -ne 0 ]]; then
		echo "Download failed for run ${run}, moving on"
		sed -i "/${run}/d" $newlogfile # remove run again from log file
		continue
            fi
            histfile=`echo ${tmpdir}/${rucioset}/${rucioset}._0001.?`
	fi

	python copySelective.py $histfile ${indir}/${run}.HIST.root >& /dev/null
        if [[ $? -ne 0 ]]; then
           echo "Something failed in extracting Z counting histos of run ${run}"
           continue
	else
	    sed -i "s#${run}#${run} ${histfile}#" $newlogfile # save origin location to logfile
            echo "Z counting histos copied to ${indir}${run}.HIST.root"
        fi
	
	if [[ $userucio == true ]]; then rm -rf ${tmpdir}/${rucioset}; fi
    fi
    if [[ ! -f ${indir}${run}.HIST.root ]]; then
        echo "Z counting histos for ${run} not (yet) available"
	sed -i "/${run}/d" $newlogfile # remove run again from log file
    fi
done
echo "HIST file copied to $indir as far as available, rotating log"
mv -f $oldlogfile ${oldlogfile}.old
mv -f $newlogfile $oldlogfile

if [[ $updateGRL == 1 ]]; then
    echo
    echo "Creating temporary PHYS_StandardGRL_All_Good GRL"
    mkdir ${tmpdir}/grls
    python grl_maker.py ${tmpdir}/grls `awk '{print $1}' $oldlogfile` >& /dev/null
    if [[ $? -ne 0 ]]; then
	echo "Something failed in GRL preparation"
	exit 1
    fi
    # check each run for empty GRL
    for run in `awk '{print $1}' $oldlogfile`; do
	if ! grep LBRange ${tmpdir}/grls/${run}_grl.xml >& /dev/null; then
	    echo "GRL for run $run is empty, removing"
	    rm ${tmpdir}/grls/${run}_grl.xml
	fi
    done
    grlname=${indir}${dataset}_PHYS_StandardGRL_All_Good_`date "+%F_%R"`.xml
    merge_goodrunslists ${tmpdir}/grls $grlname
    rm -f `readlink ${indir}latest_GRL.xml`
    rm -f ${indir}latest_GRL.xml
    ln -s $grlname ${indir}latest_GRL.xml
    echo "GRL creation complete $grlname" 
    rm -rf ${tmpdir}/grls
    ./process_HISTtoCSV.sh $year $updateC ${indir}latest_GRL.xml
else
    ./process_HISTtoCSV.sh $year $updateC
fi
