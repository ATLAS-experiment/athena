#!/usr/bin/env bash

year=$1
batch=$2
stream=$3
update=$4

CVMFS="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunsLists"
#indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/HighMu/data${year}_13p6TeV"
#outdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data${year}_13p6TeV/"

#indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/HighMu/data22_13p6TeV/"
indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/MergedOutputs/HighMu/data23_13p6TeV/All_Outputs/"

#outdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data22_13p6TeV/temp/physics_Main_MC23a/"
outdir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/data23_13p6TeV/temp/physics_Main_customgrl3/"


for dir in $indir
do    

    
    for infilename in $(ls $dir)
    do
	infile=${dir}${infilename}
    
	[ $year == 22 ] && grl="${CVMFS}/data22_13p6TeV/20230207/data22_13p6TeV.periodAllYear_DetStatus-v109-pro28-04_MERGED_PHYS_StandardGRL_All_Good_25ns.xml" && campaign=mc23a
	
	[ $year == 23 ] && grl="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/data23_grl/data23_13p6TeV_grl.xml" && campaign=mc23a
	 #&& grl="${CVMFS}/data23_13p6TeV/20230712/physics_25ns.xml"

	if [[ $batch = "local" ]]
	then
	    python -u dqt_zlumi_pandas.py --dblivetime --useofficial --grl $grl --infile $infile --campaign $campaign --outdir ${outdir} --update ${update}
	elif [[ $batch = "batch" ]]
	then
	    run_dir=$(pwd) 
	    run_num=$(echo $infile | sed 's/.*tree_//;s/.root//')
	    mkdir -p batch_jobs/20${year}/${run_num}
	    cd batch_jobs/20${year}/${run_num}

	    echo "#!/usr/bin/env bash"                                                           >  batcher.sh 
	    echo "export ATLAS_LOCAL_ROOT_BASE=\"/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase\"" >> batcher.sh 
	    echo "source \${ATLAS_LOCAL_ROOT_BASE}/user/atlasLocalSetup.sh -q"                   >> batcher.sh 
	    echo "asetup 23.0.18,Athena,here"                                                    >> batcher.sh 
	    echo "python -u ${run_dir}/dqt_zlumi_pandas.py --dblivetime --useofficial --infile $infile --grl $grl --campaign $campaign --outdir ${out_dir}" >> batcher.sh 
	    chmod +x batcher.sh

	    echo "executable            = batcher.sh"       >  batcher.sub
	    echo "arguments             = "                 >> batcher.sub
	    echo "output                = batcher.out"      >> batcher.sub
	    echo "error                 = batcher.err"      >> batcher.sub
	    echo "log                   = batcher.log"      >> batcher.sub
	    echo "+JobFlavour           = \"microcentury\"" >> batcher.sub
	    echo "queue"                                    >> batcher.sub

	    condor_submit batcher.sub
	    cd ../../../
	else
	    echo "Unrecognised job option!!!"
	    echo "Please resubmit with option either 'local' or 'batch'"
	    exit
	fi
    done
done

