#!/bin/bash

JsonPath=$1
echo $JsonPath

# validate all json plot definitions in data/PlotsDefFileList_default.txt
for JsonPlotDef in $( cat $JsonPath/PlotsDefFileList_default.txt ); do
  JsonPlotDefFull=$JsonPath/$( basename $JsonPlotDef )
  echo -e "\n$JsonPlotDefFull"
  jq . $JsonPlotDefFull
done

# validate json IDTPM configuration files
JsonConfigs=(
  "PlotsDefCommonValues.json" "PlotsDefCommonValues_ITk.json"
  "EFTrkAnaConfig_example.json" "offlTrkAnaConfig.json" "trkAnaConfigs_example.json" )

for JsonConfig in ${JsonConfigs[@]}; do
  JsonConfigFull=$JsonPath/$JsonConfig
  echo -e "\n$JsonConfigFull"
  if [ -e $JsonConfigFull ]; then
    jq . $JsonConfigFull
  fi
done
