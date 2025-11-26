#!/usr/bin/env bash

TOPDIR=$(dirname -- "$( readlink -f -- "$0"; )")
KRB5CCNAME=${KRB5CCNAME:-FILE:/run/user/$UID/krb5cc}
KRBPATH=$(echo $KRB5CCNAME | awk -F : '{ print $2 }')

( cd /eos/user/a/atlasdqm ; cd /eos/project/o/oracle/public/admin )
( cd /cvmfs/atlas.cern.ch ; cd /cvmfs/sft.cern.ch ; cd /cvmfs/atlas-nightlies.cern.ch )
( cd /cvmfs/atlas-condb.cern.ch )

cd $TOPDIR
apptainer --quiet exec --env KRB5CCNAME=$KRB5CCNAME --env-file envvars \
--bind /eos/user/a/atlasdqm,/eos/project/o/oracle/public/admin,$KRBPATH,/cvmfs \
--bind /afs/cern.ch/user/a/atlasdqm/private:/config \
--pwd /RunDCSCalc docker://registry.cern.ch/atlas-dqm-core/dcscalculator:latest /bin/bash
