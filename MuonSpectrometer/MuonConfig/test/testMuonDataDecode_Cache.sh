#!/bin/sh

python -c 'from MuonConfig.MuonRdoDecodeConfig import muonRdoDecodeTestData; cfg=muonRdoDecodeTestData(True); cfg.run(maxEvents=20)'
