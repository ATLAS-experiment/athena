#!/bin/bash
python -m PixelCalibAlgs.HitMapBuilderConfig --filesInput /eos/atlas/atlascerngroupdisk/det-pixdq/raw_data/486877/data24_13p6TeV.00486877.express_express.merge.RAW._lb0654._SFO-ALL._0001.1
makeInactiveModuleList 486877 HitMap.root

