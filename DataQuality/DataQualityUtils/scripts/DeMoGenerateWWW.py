#! /usr/bin/env python
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
# Author : Benjamin Trocme (CNRS/IN2P3 - LPSC Grenoble) - 2022 - 2023
#
# Script used to generate to https://atlasdqm.web.cern.ch/atlasdqm/DeMo/ webpage
#
# Documentation: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataQualityDemo
#############################################################################################

import sys, socket, pathlib, errno, re, subprocess, os
import argparse

from DeMoLib import retrieveYearTagProperties,returnPeriod

maindir = "/eos/project/a/atlas-dataquality/www/DeMo/"
libdir = os.path.dirname(__file__)

weeklyYear = "2024"
weeklyTag = "AtlasReady"

global run3_yt
run3_yt = {"2022":["AtlasReady_BestLumi_HEAD","AtlasReady_BestLumi","GRL_M"],
           "2023":["AtlasReady","GRL_summer"],
           "2024":["AtlasReady"]}

# run3_yt_description is retrieved from the DeMoConfig.dat files. See below
global run3_yt_description
run3_yt_description = {}

global subsystems
subsystems = {"Inner detector":["Pixel","SCT","TRT"],
              "Calorimeters":["LAr","Tile"],
              "Muon":["RPC","MDT","TGC","CSC"],
              "Trigger/Lumi/Global":["Trig_L1","Trig_HLT","Lumi","Global"],
              "Forward": ["ALFA","AFP","LUCID","ZDC"],
              "Combined perf.":["IDGlobal","BTag","CaloCP","MuonCP"]} 
global subsystemName
subsystemName = {"Pixel":"Pixel",
                 "SCT":"SCT",
                 "TRT":"TRT",
                 "LAr":"LAr",
                 "Tile":"Tile",
                 "RPC":"RPC",
                 "MDT":"MDT",
                 "TGC":"TGC",
                 "CSC":"CSC",
                 "Trig_L1":"L1 trigger",
                 "Trig_HLT":"HLT trigger",
                 "Lumi":"Luminosity",
                 "Global":"Global",
                 "ALFA":"ALFA",
                 "AFP":"AFP",
                 "LUCID":"LUCID",
                 "ZDC":"ZDC",
                 "IDGlobal":"ID global",
                 "BTag":"Btagging",
                 "CaloCP":"Calorimeter combined",
                 "MuonCP":"Muon combined"}

subsystemNotInRun2 = {}
subsystemNotInRun3 = ["CSC"]

runListDir = "./YearStats-common"

################################################################
# When https is found in a character line, add a link in html
# If none, simply return the line unchanged.
def addLink(l):
    htmlSplitted = l.split("https")

    if len(htmlSplitted)>1: # Url found -> Direct link added
        urlAlreadyReplaced = []
        for iUrl in range(1,len(htmlSplitted)):
            url = "https%s"%(htmlSplitted[iUrl].split(" ")[0])
            if (url not in urlAlreadyReplaced):
                l = l.replace(url,'''<a href="%s" target="_blank"> %s </a>'''%(url,url))
            urlAlreadyReplaced.append(url)

    return l


################################################################
def addHeader(page,year):

    run3 = (year != "Run2")

    page.write('''<!doctype html>\n''')
    page.write('''<html lang=''>\n''')
    page.write('''<head>\n''')
    page.write('''   <meta charset='utf-8'>\n''')
    page.write('''   <meta http-equiv="X-UA-Compatible" content="IE=edge">\n''')
    page.write('''   <meta name="viewport" content="width=device-width, initial-scale=1">\n''')
    page.write('''   <link rel="stylesheet" href="styles.css">\n''')
    page.write('''   <script src="http://code.jquery.com/jquery-latest.min.js" type="text/javascript"></script>\n''')
    page.write('''   <script src="script.js"></script>\n''')
    page.write('''   <title>DeMo outputs</title>\n''')
    page.write('''</head> \n''')
    page.write('''<body>\n''')


    page.write('''<div id='cssmenu'>\n''')
    page.write('''<ul>\n''')
    if (year == "Run2"):
        page.write('''   <li><a href='index-Run2.html'><span><b>Run 2</b></span></a></li>\n''')
    else:
        page.write('''   <li><a href='index-Run2.html'><span>Run 2</span></a></li>\n''')
    for irun3_year in run3_yt.keys():
        if (irun3_year == year):
            page.write('''   <li><a href='index-%s.html'><span><b>%s</b></span></a></li>\n'''%(irun3_year,irun3_year))
        else:
            page.write('''   <li><a href='index-%s.html'><span>%s</span></a></li>\n'''%(irun3_year,irun3_year))
    page.write('''</ul>\n''')
    page.write('''</div>\n''')

    
    page.write('''<div id='cssmenu'>\n''')
    page.write('''<ul>\n''')

    ### Year stats for all systems
    page.write('''   <li class='active has-sub'><a><span>Overview</span></a>\n''')
    page.write('''     <ul>\n''')
    if (run3):
        if (year == weeklyYear):
            page.write('''   <li><a href='Weekly.html'><span>Weekly</span></a></li>\n''')
                
        for iTag in run3_yt[year]:
            page.write('''       <li><hr><a href='YearStats-%s-%s.html'><span>Year stats - %s </span></a></li>\n'''%(year,iTag,run3_yt_description["%s_%s"%(year,iTag)]))
            page.write('''       <li><a href='recapDefectsHighlights-%s-%s.html'><span>Defect highlights (beta) - %s </span></a></li>\n'''%(year,iTag,run3_yt_description["%s_%s"%(year,iTag)]))
    else:
        page.write('''       <li><a href='YearStats-2018-DQPaper_2018.html'><span>2018 overview - DQ Paper</span></a></li>\n''')
        page.write('''       <li><a href='YearStats-2017-DQPaper_2017.html'><span>2017 overview - DQ Paper</span></a></li>\n''')
        page.write('''       <li><a href='YearStats-2016-DQPaper_2016.html'><span>2016 overview - DQ Paper</span></a></li>\n''')
        page.write('''       <li><a href='YearStats-2015-DQPaper_2015.html'><span>2015 overview - DQ Paper</span></a></li>\n''')

    page.write('''     </ul>\n''')
    page.write('''   </li>\n''')
    
    for iSystem in subsystems.keys():
        page.write('''   <li class='active has-sub'><a><span>%s</span></a>'''%iSystem)
        page.write('''      <ul>\n''')

        for iSystem in subsystems[iSystem]:
            if ((run3) and (iSystem not in subsystemNotInRun3)) or ((not run3) and (iSystem not in subsystemNotInRun2)):
                page.write('''<li class='has-sub'><a ><span>%s</span></a>'''%subsystemName[iSystem])
                page.write('''   <ul>\n''')
            else:
                continue

            if (run3) and (iSystem not in subsystemNotInRun3):
                if (year == weeklyYear):
                    page.write('''      <li><a href='%s-Weekly.html'><span>Weekly</span></a></li>\n'''%(iSystem))
                for iTag in run3_yt[year]:
                    page.write('''      <li><hr><a href='%s-YearStats-%s-%s.html'><span>Year stats - %s </span></a></li>\n'''%(iSystem,year,iTag,run3_yt_description["%s_%s"%(year,iTag)]))
                    page.write('''      <li><a href='%s-recapDefects-%s-%s.html'><span>Defect recap - %s </span></a></li>\n'''%(iSystem,year,iTag,run3_yt_description["%s_%s"%(year,iTag)]))
            elif (iSystem not in subsystemNotInRun2):
                page.write('''      <li><hr><a href='%s-YearStats-DQPaper_.html'><span>Year stats - Run2</span></a></li>\n'''%(iSystem))
                page.write('''      <li class='last'><a href='%s-recapDefects-DQPaper_.html'><span>Defect recap - Run2</span></a></li>\n'''%(iSystem))
            page.write('''   </ul>\n''')
            page.write('''</li>\n''')
        page.write('''</ul>\n''')
        page.write('''</li>\n''')

    page.write('''</ul>\n''')
    page.write('''</div>\n''')

    page.write('''<br>\n''')

    return

################################################################
def addFooter(page):
    page.write('''<div style="text-align:left" class="rectangle">\n''')
    page.write('''Link to the <a href="logs/DeMoCron.log"> cron job output </a> - Documentation available <a href="https://twiki.cern.ch/twiki/bin/view/Atlas/DataQualityDemo"> here </a> <br>\n''')
    page.write('''Original developer: Benjamin Trocm&eacute (LPSC Grenoble) - Maintenance: Benjamin Trocm&eacute (LPSC Grenoble)\n''')
    page.write('''</div></body>\n''')
    page.write('''<html>\n''')
    return

################################################################
def addHeaderWeekly(page):
    page.write('''<div style="text-align:left" class="rectangle">\n''')
    page.write('''Considered dataset: <br>\n''')
    page.write('''- all ATLAS-ready runs acquired in the last 7 days <br>\n''')
    page.write('''- all ATLAS-ready runs fully signed off in the last 7 days <br>\n''')
    page.write('''- all runs considered for the upcoming wednesday formal signoff (list available <a href="%s/%s/runlist-weekly.dat" target="_blank"> here </a>)<br>'''%(runListDir,weeklyYear))
    page.write('''Plots updated daily at midnight<br>\n''')
    page.write('''Status meaning (based on UNCHECKED defect status):<br>\n''')
    page.write('''-EXPR.   : no assessment yet<br>\n''')
    page.write('''-BULK    : express assessment completed<br>\n''')
    page.write('''-DONE    : express and bulk assessment completed (but no final signoff - relevant only for LAr)<br>\n''')
    page.write('''-FINAL OK: assessment completed<br>\n''')
    page.write('''The spade symbol after the status indicates that this run is supposed to be signed off at the upcoming DQ wednesday meeting <br>\n''')
    page.write('''</div>\n''')
    return

################################################################
def addHeaderYearStatsRecap(page,year="",tag="",system="",type="YearStats"):
    page.write('''<div style="text-align:left" class="rectangle">\n''')

    if (system != ""):
        if (type == "YearStats"):
            page.write('''<b>%s - %s</b> - Switch to the <a href="%s-recapDefects-%s-%s.html"> defect recap page</a><br>\n'''%(year,run3_yt_description["%s_%s"%(year,tag)],system,year,tag))
        else:
            page.write('''<b>%s - %s</b> - Switch to the <a href="%s-YearStats-%s-%s.html"> year stats page</a><br>\n'''%(year,run3_yt_description["%s_%s"%(year,tag)],system,year,tag))
    else:
        page.write('''<b>%s - %s</b><br>\n'''%(year,run3_yt_description["%s_%s"%(year,tag)]))

    if "AtlasReady" in tag: # Year tag named AtlasReady are usually devoted to monitor all runs
        page.write('''All runs with ATLAS ready considered.''')
    if "GRL" in tag:
        page.write('''List of runs derived from the <a href="https://twiki.cern.ch/twiki/bin/view/AtlasProtected/GoodRunListsForAnalysisRun3" target="_blank">GRL twiki </a>.''')
    page.write('''<br> Direct link to the <a href="%s/%s/runlist-%s-%s.dat" target="_blank"> list. </a> <br>\n'''%(runListDir,year,year,tag))

    # Scan the DeMoLib.py file to extract the monitored defects and the database tags
    DeMoLibFile = open("%s/DeMoLib.py"%libdir,"r")
    offLumiTag = ""
    offLumiAcctTag = ""
    defectTag = ""
    if (system != ""):
        listOfDefects = {"partIntol":"","partTol":"","globIntol":"","globTol":""}
        
        for line in DeMoLibFile:
            if (system != ""):
                defectSearch = "%s system"%system
                for iType in listOfDefects.keys():
                    if re.search(defectSearch, line) and iType in line:
                        listOfDefects[iType] = (((line.split(" = ")[1]).split("# %s system"%system)[0]).replace(" ","")).replace(",",", ")

        DeMoLibFile.close()
            
        if (system != ""):
            page.write("<hr>")
            page.write('''<b>Monitored defects </b><br>\n''')
            if len(listOfDefects["globIntol"])>2:
                page.write('''Global    intolerable defects: %s  <br>'''%listOfDefects["globIntol"])
            if len(listOfDefects["partIntol"])>2:
                page.write('''Partition intolerable defects: %s  <br>'''%listOfDefects["partIntol"])
            if len(listOfDefects["globTol"])>2:
                page.write('''Global    tolerable defects  : %s  <br>'''%listOfDefects["globTol"])
            if len(listOfDefects["partTol"])>2:
                page.write('''Partition tolerable defects  : %s  <br>'''%listOfDefects["partTol"])

    # Add the database tags
    page.write('''<b>Database tags </b><br>\n''')
    yearTagProperties = retrieveYearTagProperties(year,tag)
    for iyt in yearTagProperties.keys():
        if (iyt == "Defect tag") or (iyt == "OflLumi tag") or (iyt == "OflLumiAcct tag"):
            page.write('''%s: %s <br>\n'''%(iyt,yearTagProperties[iyt]))
        if (iyt == "Veto tag") and (system == "LAr"):
            page.write('''%s: %s <br>\n'''%(iyt,yearTagProperties[iyt]))

    if (system != ""):
        page.write('''<hr><a href="YearStats-%s/%s/%s/runs-notYetSignedOff.dat" target="_blank"> Runs not yet signed off </a> - \n'''%(system,year,tag))
        if (type == "YearStats"):
            page.write('''<a href="YearStats-%s/%s/%s/daemon-grl.out" target="_blank"> daemon output </a> - \n'''%(system,year,tag))
        else:
            page.write('''<a href="YearStats-%s/%s/%s/daemon-grl.out" target="_blank"> daemon output </a> - \n'''%(system,year,tag))
        # Add the time of the last reset
        if (os.path.exists("YearStats-%s/%s/%s/lastResetYS.dat"%(system,year,tag))):
            lastResetFile = open("YearStats-%s/%s/%s/lastResetYS.dat"%(system,year,tag),"r")
            for iline in lastResetFile.readlines():
                page.write("Last year-stats reset: %s"%iline)

    # Look for remaining tokens
    p = subprocess.Popen("ls",stdout=subprocess.PIPE)
    (output, err) = p.communicate()
    remainingToken = ""
    for iLine in (output.decode('ascii').split("\n")):
        if (system != ""):
            if "%s-%s-%s.token"%(system,year,tag) in iLine:
                remainingToken = "%s<br>"%iLine
        else:
            if "%s-%s.token"%(year,tag) in iLine:
                remainingToken = remainingToken + "%s<br>"%iLine
    if remainingToken != "":
        page.write('''<hr><b>Remaining tokens</b>\n''')
        page.write('''<br>There are some remaining tokens in ~atlasdqm/w1/DeMo: %s <br> This may indicate a problem in processing. Please check the daemon ouputs...'''%remainingToken)

    if (system == ""):
        page.write('''</div>\n''')
    else:
        page.write("<hr>\n")

    return

################################################################
def addHeaderRun2RecapDefects(page):
    page.write('''<div style="text-align:left" class="rectangle">\n''')
    page.write('''Considered dataset: all "stable beam" runs signed off.\n''')
    page.write('''<br><a href='#2018' target='_self'> 2018 dataset </a>\n''')
    page.write('''<br><a href='#2017' target='_self'> 2017 dataset </a>\n''')
    page.write('''<br><a href='#2016' target='_self'> 2016 dataset </a>\n''')
    page.write('''<br><a href='#2015' target='_self'> 2015 dataset </a>\n''')
    page.write('''</div>\n''')
    return

################################################################
def createRun2LegacyPages():
    ## Run 2 Year stats for all systems
    for iYear in ["2015","2016","2017","2018"]:
        page = open("%s/YearStats-%s-DQPaper_%s.html"%(maindir,iYear,iYear),"w")
        addHeader(page,"Run2")
        for iSystem in subsystemName.keys():
            if (iSystem not in subsystemNotInRun2):
                page.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%iSystem)
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(iSystem,iYear,iYear))
                if iSystem == "LAr":
                    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(iSystem,iYear,iYear))
                page.write('''</div>\n''')
        addFooter(page)
        page.close()

    ## Run 2 YearStats page per system
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            page = open("%s/%s-YearStats-DQPaper_.html"%(maindir,iSystem),"w")
            addHeader(page,"Run2")
            for iYear in ["2015","2016","2017","2018"]:
                page.write('''<div style="text-align:left" class="rectangle"><b>%s</b>\n'''%(iYear))
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-defects.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(iYear,iYear))
                if iSystem == "LAr":
                    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-veto.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(iYear,iYear))
                page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-LAr/%s/DQPaper_%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(iYear,iYear))
                page.write('''</div>\n''')
            addFooter(page)
            page.close()

    ## Run 2 RecapDefect page per system
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun2):
            page = open("%s/%s-recapDefects-DQPaper_.html"%(maindir,iSystem),"w")
            addHeader(page,"Run2")
            addHeaderRun2RecapDefects(page)
            for iYear in ["2015","2016","2017","2018"]:
                page.write('''<hr>\n''')
                page.write('''<a id='%s'> %s dataset </a>\n'''%(iYear,iYear))
                readFile = open("YearStats-%s/%s/DQPaper_%s/recapDefects.html"%(iSystem,iYear,iYear))
                for line in readFile:
                    page.write(line)                    
            addFooter(page)
            page.close()

################################################################
def createWeeklyOverview(weeklyYear,weeklyTag):
    weekly = open("%s/Weekly.html"%maindir,"w")
    addHeader(weekly,weeklyYear)
    addHeaderWeekly(weekly)
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            weekly.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSystem])
            weekly.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache"/></figure>\n'''%(iSystem,weeklyYear,weeklyTag))
            weekly.write('''<a> Link to the <a href="%s-Weekly.html" > weekly plots.</a>\n'''%(iSystem))
            weekly.write('''</div>\n''')
    addFooter(weekly)
    weekly.close()

################################################################
def createWeeklySystem(weeklyYear,weeklyTag,system):
    page = open("%s/%s-Weekly.html"%(maindir,system),"w")
    addHeader(page,weeklyYear)
    addHeaderWeekly(page)
    page.write('''<div style="text-align:left" class="rectangle"><b>%s</b> - <a href="YearStats-%s/%s/%s/daemon-weekly.out" target="_blank"> Daemon output </a>\n'''%(subsystemName[system],system,weeklyYear,weeklyTag))
    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/defects--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(system,weeklyYear,weeklyTag,weeklyTag))
    if system == "LAr" :
        page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/veto--Run--%s.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(system,weeklyYear,weeklyTag,weeklyTag))
        page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/noiseBurst_veto_evol.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(system,weeklyYear,weeklyTag))
    page.write('''<figure> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/Weekly/summary-0.png" alt=""  width="750" content="no-cache" /></figure>\n'''%(system,weeklyYear,weeklyTag))
    addFooter(page)
    page.close()

################################################################
def createYearStatsSystem(year,tag,system):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/%s-YearStats-%s-%s.html"%(maindir,system,year,tag),"w")
    addHeader(page,year)
    addHeaderYearStatsRecap(page,year,tag,system,"YearStats")
    page.write('''<div style="text-align:left" class="rectangle"><figure> <a href="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" target="_blank"> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /> </a></figure>\n'''%(system,year,tag,system,year,tag))
    if system == "LAr" :
        page.write('''<figure> <a href="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-veto.png" target="_blank"><img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-veto.png" alt=""  width="750" content="no-cache" /></a></figure>\n'''%(system,year,tag,system,year,tag))
    page.write('''<figure> <a href="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-lumi.png" target="_blank"> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-lumi.png" alt=""  width="750" content="no-cache" /></a></figure>\n'''%(system,year,tag,system,year,tag))
    page.write('''</div>\n''')
    page.write('''</div>\n''')
    addFooter(page)
    page.close()

################################################################
def createYearStatsOverview(year,tag):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/YearStats-%s-%s.html"%(maindir,year,tag),"w")
    addHeader(page,year)
    addHeaderYearStatsRecap(page,year,tag)

    page.write('''<div style="text-align:left" class="rectangle"> <b>ATLAS </b><br>\n''')
    page.write('''Within each system, the overlaps between defects are treated.<br>\n''')
    page.write('''For the ATLAS inefficiency, the overlaps between systems are treated.<br>\n''')
    page.write('''<figure> <a href="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-Global/%s/%s/DataLoss.png" target="_blank"> <img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-Global/%s/%s/DataLoss.png" alt=""  width="750" content="no-cache" /></figure> </a>\n'''%(year,tag,year,tag))
    page.write('''</div>\n''')
    
    for iSystem in subsystemName.keys():
        if (iSystem not in subsystemNotInRun3):
            page.write('''<div style="text-align:left" class="rectangle"> <b>%s</b>'''%subsystemName[iSystem])
            page.write('''<figure> <a href="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" target="_blank"><img src="https://atlasdqm.web.cern.ch/atlasdqm/DeMo/YearStats-%s/%s/%s/grl-defects.png" alt=""  width="750" content="no-cache" /></a></figure>\n'''%(iSystem,year,tag,iSystem,year,tag))
            page.write('''</div>\n''')

    addFooter(page)
    page.close()

################################################################
def createDefectRecapSystem(year,tag,system):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/%s-recapDefects-%s-%s.html"%(maindir,system,year,tag),"w")
    addHeader(page,year)
    addHeaderYearStatsRecap(page,year,tag,system,"Recap")
    #page.write('''<div style="text-align:left" class="rectangle"><b>%s - %s</b> - <a href="YearStats-%s/%s/%s/daemon-recapDefects.out" target="_blank"> daemon output </a> - <a href="YearStats-%s/%s/%s/runs-notYetSignedOff.dat" target="_blank"> Runs not yet signed off </a> - Switch to the <a href="%s-YearStats-%s-%s.html"> year stats page</a> </div>\n'''%(year,run3_yt_description[tag],system,year,tag,system,year,tag,system,year,tag))

    if (os.path.exists("YearStats-%s/%s/%s/recapDefects.html"%(system,year,tag))):        
        readFile = open("YearStats-%s/%s/%s/recapDefects.html"%(system,year,tag))
        for line in readFile: # NB: the recapDefects.html files contain a single line for the whole table. 
            page.write(addLink(line))

    page.write('''</div>\n''')
    addFooter(page)
    page.close()

################################################################
def createDefectRecapHighlights(year,tag):
    yearTag = "%s_%s"%(year,tag)
    page = open("%s/recapDefectsHighlights-%s-%s.html"%(maindir,year,tag),"w")
    addHeader(page,year)
    addHeaderYearStatsRecap(page,year,tag)

    defectsToKeep = {"Pixel":["DISABLED"],
                     "TRT":["DISABLED"],
                     "RPC":["DISABLED"],
                     "MDT":["DISABLED"],
                     "TGC":["DISABLED"],
                     "Global":["TOROID_OFF","NOTCONSIDERED"]
                     }

    runHighlights = {}

    for iSystem in subsystemName.keys():
        if (iSystem in defectsToKeep.keys()) and (iSystem not in subsystemNotInRun3):
            if (os.path.exists("YearStats-%s/%s/%s/recapDefects.txt"%(iSystem,year,tag))):        
                readFile = open("YearStats-%s/%s/%s/recapDefects.txt"%(iSystem,year,tag))

                currentDefect = ""
                currentRun = ""

                for line in readFile:
                    if line.startswith("===== Recap for "):
                        currentDefect = ((line.split("===== Recap for ")[1]).split("===")[0]).replace(" ","")
                    elif line.startswith("Description:") or line.startswith("     Run") or (currentDefect not in defectsToKeep[iSystem]):
                        continue
                    else:
                        iovLine = line.split("|")
                        if (len(iovLine) == 8): # The defect to keep should be on a single line (i.e. not multiple iov in a given run)
                            runFound = iovLine[0]
                            if (runFound not in runHighlights.keys()):
                                runHighlights[runFound] = {}
                                runHighlights[runFound]["TotLumi"] = iovLine[2]
                            if (iovLine[3] == runHighlights[runFound]["TotLumi"]):
                                runHighlights[runFound][iSystem] = "<th>Whole run </th><th>%s : %s</th>"%(currentDefect,addLink(iovLine[7]))
                            else:
                                runHighlights[runFound][iSystem] = "<th>%s </th><th>%s : %s</th>"%(iovLine[3],currentDefect,addLink(iovLine[7]))

    page.write('''<div style="text-align:left" class="rectangle">''')

    page.write('''<table class="report"><tr class="out0"> <th width="100pix"></th><th width="100pix"></th><th width="100pix"></th><th width="150pix"></th><th width="100pix"></th></th><th width="1200pix"></th></tr>\n''')
    page.write('''<tr class="out0"><th> Run </th><th> Period </th> <th> Total lumi </th><th> Affected system </th> <th> Lost lumi </th> <th> Comment </th> </tr> \n''')

    listOfRuns = list(runHighlights.keys())
    listOfRuns.sort(reverse=True)

    for iRun in listOfRuns:
        newRun = True
        for iSystem in runHighlights[iRun].keys():
            if iSystem == "TotLumi":
                continue
            if newRun:
                page.write('''<tr class="out1"><th> %s </th> <th> %s </th> <th> %s </th> <th> %s </th> %s </tr> </tr>\n'''%(iRun,returnPeriod(int(iRun),"Pixel",year,"AtlasReady"),runHighlights[iRun]["TotLumi"],iSystem,runHighlights[iRun][iSystem]))
                newRun = False
            else:
                page.write('''<tr class="out1"><th> </th> <th> </th> <th> </th> <th> %s </th> %s </tr></tr>\n'''%(iSystem,runHighlights[iRun][iSystem]))

    page.write('''</table>\n''')


    page.write('''</div>''')
    addFooter(page)
    page.close()


###################################################################################################################
###################################################################################################################
# Main script

parser = argparse.ArgumentParser(description='')
parser.add_argument('-y','--year',dest='parser_year',default = "2024",help='Year: Run2, 2022,2023...[Default: 2024].',action='store')

args = parser.parse_args()
parser.print_help()

year = args.parser_year

# run3_yt_description is retrieved from the DeMoConfig.dat file
run3_yt_description = {}
for iYear in run3_yt.keys():
    for iTag in run3_yt[iYear]:
        DeMoConfigFile = open("%s/%s/DeMoConfig-%s-%s.dat"%(runListDir,iYear,iYear,iTag),"r")
        for iline in DeMoConfigFile:
            if "Description: " in iline:
                run3_yt_description["%s_%s"%(iYear,iTag)] = (iline.split("Description: ")[1]).replace("\n","")

homepage = open("%s/index-%s.html"%(maindir,year),"w")
addHeader(homepage,year)
addFooter(homepage)
homepage.close()

if (year == "Run2"):
## Run2 legacy pages
    createRun2LegacyPages()
else:
    ## Weekly page for all systems
    createWeeklyOverview(weeklyYear,weeklyTag)

    ## Weekly page per system
    for iSystem in subsystemName.keys():
        if iSystem not in subsystemNotInRun3:
            createWeeklySystem(weeklyYear,weeklyTag,iSystem)

    ## YearStats page for all systems
    for iTag in run3_yt[year]:
        createYearStatsOverview(year,iTag)

    ## YearStats page per system
    for iTag in run3_yt[year]:
        for iSystem in subsystemName.keys():
            if (iSystem not in subsystemNotInRun3):
                createYearStatsSystem(year,iTag,iSystem)

    ## RecapDefect page per system
    for iTag in run3_yt[year]:
        for iSystem in subsystemName.keys():
            if (iSystem not in subsystemNotInRun3):
                createDefectRecapSystem(year,iTag,iSystem)

    ## RecapDefect highlight
    for iTag in run3_yt[year]:
        createDefectRecapHighlights(year,iTag)
