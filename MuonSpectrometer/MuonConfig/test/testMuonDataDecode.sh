#!/bin/sh

python -c 'from MuonConfig.MuonRdoDecodeConfig import muonRdoDecodeTestData; cfg=muonRdoDecodeTestData(); cfg.run(maxEvents=20)'
