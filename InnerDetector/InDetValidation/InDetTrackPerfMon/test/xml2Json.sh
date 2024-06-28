#!/bin/bash
XslPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 3 -name hdefXmlToJson.xsl -print -quit 2>/dev/null)
XmlPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 3 -name hdef.xml -print -quit 2>/dev/null)
#for debugging path
echo $XmlPath
echo $XslPath
#produce json file from xml
xsltproc -o hdef.json $XslPath $XmlPath
#
#is the output valid json?
jq . hdef.json
JsonRefPath=$(find -H ${DATAPATH//:/ } -mindepth 1 -maxdepth 3 -name ref.json -print -quit 2>/dev/null)
echo $JsonRefPath
#is the output identical to the reference (ignoring whitespace)?
diff -b hdef.json $JsonRefPath
