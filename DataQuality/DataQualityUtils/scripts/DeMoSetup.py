#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble)- 2017 - 2023
# Python 3 migration by Miaoran Lu (University of Iowa)- 2022
#
# For each year/tag/system, an output directory YearStats-[system]/[year]/[tag] is required.
# This directory also contains some subdirectories
# This script creates all needed directories for a new set of system/year/tag.
# If the set already exists, the script simply exits
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

import os,sys
import argparse

runListDir = "./YearStats-common"

parser = argparse.ArgumentParser(description='')
parser.add_argument('-y','--year',dest='parser_year',default = "2024",help='Year [Default: 2024].',action='store')
parser.add_argument('-t','--tag',dest='parser_tag',help='DeMo tag',action='store')
parser.add_argument('-s','--system',dest='parser_system',default="",help='System: LAr, CaloCP... [Default : all systems : LAr,Pixel,SCT,TRT,Tile,MDT,TGC,RPC,Trig_L1,Trig_HLT,Lumi,Global,ALFA,AFP,LUCID,ZDC,IDGlobal,BTag,CaloCP,MuonCP]',action='store')
parser.add_argument('--description',dest='parser_description',default = "",help='Short description for DeMo plots',action='store')
parser.add_argument('--defectTag',dest='parser_defectTag',default = "",help='Defect tag',action='store')
parser.add_argument('--vetoTag',dest='parser_vetoTag',default = "",help='Veto tag (relevant only for LAr)',action='store')
parser.add_argument('--oflLumiTag',dest='parser_oflLumiTag',default = "",help='Veto tag (relevant only for LAr)',action='store')
parser.add_argument('--oflLumiAcctTag',dest='parser_oflLumiAcctTag',default = "",help='Veto tag (relevant only for LAr)',action='store')
parser.add_argument('--xmlGRL',dest='parser_xmlGRL',default = "",help='xml good run list used to create a run list in YearStats-common. Files stored on the web (starting with https) are allowed',action='store')

args = parser.parse_args()
parser.print_help()

systems = []
if args.parser_system == "":
  systems = ["LAr","Pixel","SCT","TRT","Tile","MDT","TGC","RPC","Trig_L1","Trig_HLT","Lumi","Global","ALFA","AFP","LUCID","ZDC","IDGlobal","BTag","CaloCP","MuonCP"]
else:
  systems.append(args.parser_system)

b_createDeMoConfig = True

configFilename = "%s/%s/DeMoConfig-%s-%s.dat"%(runListDir,args.parser_year,args.parser_year,args.parser_tag)
if os.path.exists(configFilename):
    print("Warning: this year/tag apparently already exists. As the database tags must be uniform among systems, I will keep the already defined tags:")
    os.system("cat %s"%configFilename)
    confirm = input("Please type OK to agree and proceed with the creation of missing system directories: ")
    b_createDeMoConfig = False
    if ("OK" not in confirm):
      sys.exit()

# Fill the various tags/description in a dat file to be read by DeMoLib
# DeMoConfig.dat is common to all systems
if b_createDeMoConfig:
  print("=========== Treatment of tags")
  config = open(configFilename, 'w')

  if (args.parser_description) == "":
    tmp_d = args.parser_tag
    print("No description provided. Tag name used -> %s"%tmp_d)
  else:
    tmp_d = args.parser_description
  config.write("Description: %s\n"%tmp_d)

  if (args.parser_defectTag) == "":
    if (args.parser_year == "2015"): tmp_d = "DetStatus-v105-pro22-13"
    elif (args.parser_year == "2016"): tmp_d = "DetStatus-v105-pro22-13"
    elif (args.parser_year == "2017"): tmp_d = "DetStatus-v105-pro22-13"
    elif (args.parser_year == "2018"): tmp_d = "DetStatus-v105-pro22-13"
    elif (args.parser_year == "2022"): tmp_d = "HEAD"
    elif (args.parser_year == "2023"): tmp_d = "HEAD"
    elif (args.parser_year == "2024"): tmp_d = "HEAD"
    print("No defect tag provided. Default year tag used -> %s"%tmp_d)
  else:
    tmp_d = args.parser_defectTag    
  config.write("Defect tag: %s\n"%tmp_d)

  if (args.parser_vetoTag) == "":
    if (args.parser_year == "2015" or args.parser_year == "2016" or args.parser_year == "2017" or args.parser_year == "2018"): tmp_d = "LARBadChannelsOflEventVeto-RUN2-UPD4-11"
    elif (args.parser_year == "2022"): tmp_d = "LARBadChannelsOflEventVeto-RUN2-UPD4-11"
    elif (args.parser_year == "2023"): tmp_d = "LARBadChannelsOflEventVeto-RUN2-UPD4-11"
    elif (args.parser_year == "2024"): tmp_d = "LARBadChannelsOflEventVeto-RUN2-UPD4-13"
    print("No veto tag provided. Default year tag used -> %s"%tmp_d)
  else:
    tmp_d = args.parser_vetoTag    
  config.write("Veto tag: %s\n"%tmp_d)

  if (args.parser_oflLumiTag) == "":
    if (args.parser_year == "2015"): tmp_d = "OflLumi-13TeV-004"
    elif (args.parser_year == "2016"): tmp_d = "OflLumi-13TeV-009"
    elif (args.parser_year == "2017"): tmp_d = "OflLumi-13TeV-010"
    elif (args.parser_year == "2018"): tmp_d = "OflLumi-13TeV-010"
    elif (args.parser_year == "2022"): tmp_d = "OflLumi-Run3-002"
    elif (args.parser_year == "2023"): tmp_d = "OflLumi-Run3-002"
    elif (args.parser_year == "2024"): tmp_d = "OflPrefLumi-RUN3-UPD4-02"
    print("No oflLumi tag provided. Default year tag used -> %s"%tmp_d)
  else:
    tmp_d = args.parser_oflLumiTag    
  config.write("OflLumi tag: %s\n"%tmp_d)

  if (args.parser_oflLumiAcctTag) == "":
    if (args.parser_year == "2015" or args.parser_year == "2016" or args.parser_year == "2017" or args.parser_year == "2018"): tmp_d = "Unknown"
    elif (args.parser_year == "2022"): tmp_d = "OflLumiAcct-Run3-002"
    elif (args.parser_year == "2023"): tmp_d = "OflLumiAcct-Run3-002"
    elif (args.parser_year == "2024"): tmp_d = "OflLumiAcct-Run3-005"
    print("No OflLumiAcct tag provided. Default year tag used -> %s"%tmp_d)
  else:
    tmp_d = args.parser_oflLumiAcctTag    
  config.write("OflLumiAcct tag: %s\n"%tmp_d)

  config.close()

# Create the missing directories
for iSystem in systems:    
  print("== System: %s"%iSystem)
  direct = "YearStats-%s"%iSystem
  if not os.path.exists(direct):
    print("%s system directory does not exists. Creating it"%direct)
    os.system("mkdir %s"%direct)

  direct = "YearStats-%s/%s"%(iSystem,args.parser_year)
  if not os.path.exists(direct):
    print("%s year directory does not exists. Creating it"%direct)
    os.system("mkdir %s"%direct)

  direct = "YearStats-%s/%s/%s"%(iSystem,args.parser_year,args.parser_tag)
  if not os.path.exists(direct):
    print("%s tag directory does not exists. Creating it"%direct)
    os.system("mkdir %s"%direct)
    os.system("mkdir %s/Run"%direct)
    os.system("mkdir %s/Weekly"%direct)


# When requested, generation of run list
print("=========== Treatment of run list")
newRunListName = "%s/%s/runlist-%s-%s.dat"%(runListDir,args.parser_year,args.parser_year,args.parser_tag)

if args.parser_xmlGRL != "":
  if "https" in args.parser_xmlGRL:
    os.system("wget %s -P %s"%(args.parser_xmlGRL,runListDir))
    file_xmlGRL = open("%s/%s"%(runListDir,args.parser_xmlGRL.split("/")[-1]))
  else:
    file_xmlGRL = open(args.parser_xmlGRL)

  newRunList = open(newRunListName, 'w')

  for iline in file_xmlGRL.readlines():
    if ("<Run>" in iline and "</Run>" in iline):
      runNumber = (iline.split("<Run>")[1]).split("</Run>")[0]
      newRunList.write("%s\n"%runNumber)

  newRunList.close()

if not os.path.exists(newRunListName):
  print("Warning: the %s file does not exist yet. Please create it by hand or with the --xmlGRL option."%newRunListName)
