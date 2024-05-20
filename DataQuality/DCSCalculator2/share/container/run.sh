#!/usr/bin/env bash

( cd /eos/user/a/atlasdqm ; cd /eos/project/o/oracle/public/admin )
( cd /cvmfs/atlas.cern.ch ; cd /cvmfs/sft.cern.ch ; cd /cvmfs/atlas-nightlies.cern.ch )
( cd /cvmfs/atlas-condb.cern.ch )

docker run --env KRB5CCNAME --env-file ./envvars \
 --mount type=bind,source=/eos/user/a/atlasdqm,target=/eos/user/a/atlasdqm \
 --mount type=bind,source=/eos/project/o/oracle/public/admin,target=/eos/project/o/oracle/public/admin,readonly \
 -v /run/user/$UID/krb5cc:/run/user/$UID/krb5cc:Z \
 --mount type=bind,source=/cvmfs,target=/cvmfs,readonly \
 --mount type=bind,source=/afs/cern.ch/user/a/atlasdqm/private,target=/config,readonly \
 --pull always -q registry.cern.ch/atlas-dqm-core/dcscalculator:latest ./execute.sh once
