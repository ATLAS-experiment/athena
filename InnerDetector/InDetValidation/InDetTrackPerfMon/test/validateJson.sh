#!/bin/bash
JsonPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 3 -name EffPlotsDef.json -printf %h -quit 2>/dev/null)
echo $JsonPath
#validate all json files in /data
jq . $JsonPath/*.json

