INTGRID_DIR=st3-v2-intgrid
INT_DIR=st3-v2

FILES="pwgubound-*.dat pwgfullgrid*.dat pwgseeds.dat slurmjob.sh powheg.input *0001*"

for FILE in $FILES
do
	cp ${INT_DIR}/${FILE} ${INTGRID_DIR}
done
