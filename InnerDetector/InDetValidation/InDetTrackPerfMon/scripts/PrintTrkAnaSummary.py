#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys, os, argparse, ROOT
import pandas as pd
import math
from uncertainties import ufloat as rd

# Parsing arguments
commandName = os.path.basename( sys.argv[0] )
summaryDirDefault="InDetTrackPerfMonPlots/&TrkAnaName&/Offline/Tracks/"
parser = argparse.ArgumentParser( description = commandName+" options:" )
parser.add_argument( "-t", "--testFile", help="Path to input TEST file" )
parser.add_argument( "-r", "--refFile", default="", help="Path to input REFERENCE file" )
parser.add_argument( "-T", "--testLabel", default="TEST", help="Label for TEST" )
parser.add_argument( "-R", "--refLabel", default="REF", help="Label for REFERENCE" )
parser.add_argument( "-d", "--dirName", default=summaryDirDefault, help="Name of the TDirectory path with plots" )
parser.add_argument( "-a", "--analyses", default="TrkAnaEF", help="Comma-separeted list of track analyses to process" )
parser.add_argument( "-o", "--outName", default="TrkAnaSummary_&TrkAnaName&.html", help="Name of the output html files" )
MyArgs = parser.parse_args()
anaList = MyArgs.analyses.strip().split(',')

if not MyArgs.testFile:
    print( "ERROR: input test file not provided" )
    sys.exit(1)

def fround( c, e ):
    e = math.fabs( e )
    res = str( rd( c, e ) )
    return res.replace( '+/-', ' \u00b1 ' ).replace( '(', '( ' ).replace( ')', ' )' ) 

def processFile( inFileName, dirName, label, data, dataDict, index, updateIndex=True, printMultiplicity=True ):
    if updateIndex: index.clear()
    sList = []
    sDict = {}
    inFile = ROOT.TFile.Open( inFileName, "READ" )

    ## multiplicities
    if printMultiplicity:
        hs = inFile.Get( dirName+"Multiplicities/summary" )
        for i in range( 1, 8 ) :
            if hs:
                if updateIndex : index.append( hs.GetXaxis().GetBinLabel(i) )
                c = hs.GetBinContent(i)
                e = hs.GetBinError(i)
                sList.append( f"{c:.0f} \u00b1 {e:.0f}" )
                sDict.update( { "mult_"+str(i) : [ c, e ] } )
            else:
                if updateIndex : index.append(" ")
                sList.append( "-" )

    # efficiencies
    he = inFile.Get( dirName+"Efficiencies/eff_vs_truth_inclusive" )
    if updateIndex : index.append( " " )
    sList.append( " " )
    if updateIndex : index.append( "Eff_vs_truth" )
    if he:
        c = 100*he.GetEfficiency(1)
        eu = 100*he.GetEfficiencyErrorUp(1)
        el = 100*he.GetEfficiencyErrorLow(1)
        e = max( [ eu, el ] )
        sList.append( f"{fround( c, e )} %" )
        sDict.update( { "eff" : [ c, e ] } )
    else : sList.append( "-" )

    ## technical efficiency
    het = inFile.Get( dirName+"Efficiencies/Technical/eff_vs_truth_inclusive" )
    if updateIndex : index.append( "Tech_eff_vs_truth" )
    if het:
        c = 100*het.GetEfficiency(1)
        eu = 100*het.GetEfficiencyErrorUp(1)
        el = 100*het.GetEfficiencyErrorLow(1)
        e = max( [ eu, el ] )
        sList.append( f"{fround( c, e )} %" )
        sDict.update( { "tech_eff" : [ c, e ] } )
    else : sList.append( "-" )


    ## pT resolution
    hrpt = inFile.Get( dirName+"Resolutions/resolution_pt_vs_truth_inclusive" )
    if updateIndex : index.append( " " )
    sList.append( " " )
    if updateIndex : index.append( "Resolution_pT_vs_truth" )
    if hrpt:
        c = hrpt.GetBinContent(1)
        e = hrpt.GetBinError(1)
        sList.append( f"{fround( c, e )} GeV" )
        sDict.update( { "res_pt" : [ c, e ] } )
    else : sList.append( "-" )


    ## d0 resolution
    hrd0 = inFile.Get( dirName+"Resolutions/resolution_d0_vs_truth_inclusive" )
    if updateIndex : index.append( "Resolution_d0_vs_truth" )
    if hrd0:
        c = hrd0.GetBinContent(1)
        e = hrd0.GetBinError(1)
        sList.append( f"{fround( c, e )} \u03BCm" )
        sDict.update( { "res_d0" : [ c, e ] } )
    else : sList.append( "-" )


    ## z0 resolution
    hrz0 = inFile.Get( dirName+"Resolutions/resolution_z0_vs_truth_inclusive" )
    if updateIndex : index.append( "Resolution_z0_vs_truth" )
    if hrz0:
        c = hrz0.GetBinContent(1)
        e = hrz0.GetBinError(1)
        sList.append( f"{fround( c, e )} \u03BCm" )
        sDict.update( { "res_z0" : [ c, e ] } )
    else : sList.append( "-" )


    ## fake rate
    hf = inFile.Get( dirName+"FakeRates/fakerate_vs_offl_inclusive" )
    if updateIndex : index.append( " " )
    sList.append( " " )
    if updateIndex : index.append( "FakeRate_vs_reco" )
    if hf:
        c = 100*hf.GetEfficiency(1)
        eu = 100*hf.GetEfficiencyErrorUp(1)
        el = 100*hf.GetEfficiencyErrorLow(1)
        e = max( [ eu, el ] )
        sList.append( f"{fround( c, e )} %" )
        sDict.update( { "fake" : [ c, e ] } )
    else : sList.append( "-" )


    ## duplicate rate
    hd = inFile.Get( dirName+"Duplicates/duplrate_vs_truth_inclusive" )
    if updateIndex : index.append( "DuplicateRate_vs_truth" )
    if hd:
        c = 100*hd.GetEfficiency(1)
        eu = 100*hd.GetEfficiencyErrorUp(1)
        el = 100*hd.GetEfficiencyErrorLow(1)
        e = max( [ eu, el ] )
        sList.append( f"{fround( c, e )} %" )
        sDict.update( { "dupl" : [ c, e ] } )
    else : sList.append( "-" )


    ## updating data
    data.update( { label : sList } )
    dataDict.update( { label : sDict } )
    inFile.Close()

def getRatio( a, ae, b, be ):
    if b==0 :
        return ( 0., 0. )

    c = a / b
    ce = math.sqrt( ( ae / b )**2 + ( a * be / b**2 )**2 )
    return ( c, ce )

def computeRatios( ltest, lref, data, dataDict, printMultiplicity=True ):
    sList = []

    ## multiplicities
    if printMultiplicity:
        for i in range( 1, 8 ) :
            if "mult_"+str(i) in dataDict[ltest] and "mult_"+str(i) in dataDict[lref] :
                c, e = getRatio( dataDict[ltest]["mult_"+str(i)][0],
                                 dataDict[ltest]["mult_"+str(i)][1],
                                 dataDict[lref]["mult_"+str(i)][0],
                                 dataDict[lref]["mult_"+str(i)][1] )
                sList.append( f"{fround( c, e )}" )
            else:
                sList.append( "-" )

    # efficiencies
    sList.append( " " )
    if "eff" in dataDict[ltest] and "eff" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["eff"][0],
                         dataDict[ltest]["eff"][1],
                         dataDict[lref]["eff"][0],
                         dataDict[lref]["eff"][1] )
        c = 100*c
        e = 100*e
        sList.append( f"{fround( c, e )} %" )
    else : sList.append( "-" )

    ## technical efficiency
    if "tech_eff" in dataDict[ltest] and "tech_eff" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["tech_eff"][0],
                         dataDict[ltest]["tech_eff"][1],
                         dataDict[lref]["tech_eff"][0],
                         dataDict[lref]["tech_eff"][1] )
        c = 100*c
        e = 100*e
        sList.append( f"{fround( c, e )} %" )
    else : sList.append( "-" )


    ## pT resolution
    sList.append( " " )
    if "res_pt" in dataDict[ltest] and "res_pt" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["res_pt"][0],
                         dataDict[ltest]["res_pt"][1],
                         dataDict[lref]["res_pt"][0],
                         dataDict[lref]["res_pt"][1] )
        sList.append( f"{fround( c, e )}" )
    else : sList.append( "-" )


    ## d0 resolution
    if "res_d0" in dataDict[ltest] and "res_d0" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["res_d0"][0],
                         dataDict[ltest]["res_d0"][1],
                         dataDict[lref]["res_d0"][0],
                         dataDict[lref]["res_d0"][1] )
        sList.append( f"{fround( c, e )}" )
    else : sList.append( "-" )


    ## z0 resolution
    if "res_z0" in dataDict[ltest] and "res_z0" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["res_z0"][0],
                         dataDict[ltest]["res_z0"][1],
                         dataDict[lref]["res_z0"][0],
                         dataDict[lref]["res_z0"][1] )
        sList.append( f"{fround( c, e )}" )
    else : sList.append( "-" )


    ## fake rate
    sList.append( " " )
    if "fake" in dataDict[ltest] and "fake" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["fake"][0],
                         dataDict[ltest]["fake"][1],
                         dataDict[lref]["fake"][0],
                         dataDict[lref]["fake"][1] )
        sList.append( f"{fround( c, e )}" )
    else : sList.append( "-" )


    ## duplicate rate
    if "dupl" in dataDict[ltest] and "dupl" in dataDict[lref] :
        c, e = getRatio( dataDict[ltest]["dupl"][0],
                         dataDict[ltest]["dupl"][1],
                         dataDict[lref]["dupl"][0],
                         dataDict[lref]["dupl"][1] )
        sList.append( f"{fround( c, e )}" )
    else : sList.append( "-" )


    ## updating data
    data.update( { ltest + " / " + lref  : sList } )

## Remove final .html if it exists
outFile = MyArgs.outName.replace( "_&TrkAnaName&", "" )
if os.path.isfile( outFile ) :
    os.remove( outFile ) 

## Looping over all the track analyses
for anaName in anaList :
    ## Track-analysis specific quantities
    data = {}
    dataDict = {}
    index = []
    anaDirName = MyArgs.dirName.replace( "&TrkAnaName&", anaName )
    anaOutName = MyArgs.outName.replace( "&TrkAnaName&", anaName )

    testFile = ROOT.TFile.Open( MyArgs.testFile, "READ" )
    testMultiplicity = testFile.Get( anaDirName+"Multiplicities/summary" )
    testFile.Close()

    refFile = ROOT.TFile.Open( MyArgs.refFile, "READ" )
    refMultiplicity = refFile.Get( anaDirName+"Multiplicities/summary" )
    refFile.Close()

    printMultiplicity = refMultiplicity or testMultiplicity
    ## Processing test file
    processFile(
        inFileName  = MyArgs.testFile,
        dirName     = anaDirName,
        label       = MyArgs.testLabel,
        data        = data,
        dataDict    = dataDict,
        index       = index,
        printMultiplicity = printMultiplicity
    )

    ## Processing reference file
    if MyArgs.refFile :
        processFile(
            inFileName  = MyArgs.refFile,
            dirName     = anaDirName,
            label       = MyArgs.refLabel,
            data        = data,
            dataDict    = dataDict,
            index       = index,
            updateIndex = not testMultiplicity,
            printMultiplicity = printMultiplicity
        )

    ## Processing ratios
    if MyArgs.refFile :
        computeRatios(
            ltest       = MyArgs.testLabel,
            lref        = MyArgs.refLabel,
            data        = data,
            dataDict    = dataDict,
            printMultiplicity = printMultiplicity
        )

    ## printing table to screen
    df = pd.DataFrame( data, index=index )
    titleStr = f"Summary for TrackAnalysis = {anaName}:"
    print( f"\n\n---------------\n{titleStr}" )
    print( df )

    ## printing table to html output file
    with open( anaOutName, 'w' ) as f :
        print( df.to_html(), file=f )

    ## Appending html table to final .html summary file
    os.system( f"echo \"<br><b>{titleStr}</b><br>\" >> {outFile}" )
    os.system( f"cat {anaOutName} >> {outFile}" )
    os.remove( f"{anaOutName}" ) 

sys.exit(0)
