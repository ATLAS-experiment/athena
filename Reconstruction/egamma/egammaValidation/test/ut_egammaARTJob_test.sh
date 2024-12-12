#!/bin/sh

# Reco stage
Reco_tf.py --CA --inputRDOFile=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/RDO/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/100events.RDO.pool.root --outputAODFile=Nightly_AOD_reco.pool.root --maxEvents=1 --autoConfiguration="everything" --preInclude egammaConfig.ConfigurationHelpers.egammaOnlyFromRaw --postInclude egammaValidation.egammaArtSpecialContent.egammaArtSpecialContent >>/dev/null 2>&1

stat=$?
if [ $stat -eq 0 ] 
then
	echo "=== RECO TF SUCCESS === "
else
	echo "=== RECO TF FAILURE ==="
	echo " RAW TO ALL step log ===> "
	cat log.RAWtoALL
	exit $stat
fi
# rm files not needed anymore
rm log.RAWtoALL >> /dev/null 2>&1

# Merge stage
AODMerge_tf.py --CA --inputAODFile=Nightly_AOD_reco.pool.root --outputAOD_MRGFile=Nightly_AOD.pool.root >>/dev/null 2>&1

stat=$?
if [ $stat -eq 0 ] 
then
	echo "=== MERGE SUCCESS ==== "
else
	echo "==== MERGE FAILURE"
	echo " AOD MERGE  log ===> "
	cat log.AODMerge
	exit $stat
fi
# rm files not needed anymore
rm log.AODMerge >> /dev/null 2>&1

# Histo stage
runegammaMonitoring.py -p 'electron' >> histo.log 2>&1
state=$?
mv Nightly-monitoring.hist.root Nightly-monitoring_electron.hist.root
runegammaMonitoring.py -p 'gamma' >> histo.log 2>&1
statg=$?
mv Nightly-monitoring.hist.root Nightly-monitoring_gamma.hist.root

if [ $state -eq 0 -a $statg -eq 0 ]
then
	echo "=== HISTO MAKER SUCCESS === "
else
	echo "==== HISTO MAKER FAILURE" $state $statg
	echo " HISTO MAKER  log ===> "
	cat  histo.log
	exit 1
fi

# rm files not needed anymore
rm -f Nightly_AOD*.pool.root >> /dev/null 2>&1
rm histo.log  >> /dev/null 2>&1

# Final plot stage 
EgammaARTmonitoring_plotsMaker.py Nightly-monitoring_electron.hist.root Nightly-monitoring_electron.hist.root electron >> plot.log 2>&1
state=$?
EgammaARTmonitoring_plotsMaker.py Nightly-monitoring_gamma.hist.root Nightly-monitoring_gamma.hist.root gamma >> plot.log 2>&1
statg=$?

if [ $state -eq 0 -a $statg -eq 0 ]
then
	echo "=== PLOT MAKER SUCCESS === "
else
	echo "=== PLOT MAKER FAILURE === " $state $statg
	echo " PLOT MAKER  log ===> "
	cat plot.log
	exit 1
fi

# rm files not needed anymore
rm *.png >> /dev/null 2>&1
rm Nightly-monitoring_*.hist.root >> /dev/null 2>&1
rm BN_ComparisonPlots_*.hist.root >>  /dev/null 2>&1
rm plot.log >>  /dev/null 2>&1

