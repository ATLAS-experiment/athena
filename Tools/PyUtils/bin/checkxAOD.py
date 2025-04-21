#!/usr/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
#
# This is a modified version of PyUtils/bin/checkFile.py. It has been taught
# how to sum up the sizes of all the branches belonging to a single xAOD
# object/container.
#

__author__  = "Sebastien Binet <binet@cern.ch>, " \
    "Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>, " \
    "RD Schaffer R.D.Schaffer@cern.ch"

import sys
import os
import re
import operator

from optparse import OptionParser

if __name__ == "__main__":

    parser = OptionParser( usage = "usage: %prog [OPTION]... my.xAOD.file.pool.root" )
    p = parser.add_option
    p( "--si",
       action="store_true", dest = "siUnits",
       help = "print sizes in kB, i.e., in units of 1000 bytes"
       " (default: print sizes in KiB, i.e., in units of 1024 bytes)" )
    p( "-f",
       "--file",
       dest = "fileName",
       help = "The path to the POOL file to analyze" )
    p( "-c",
       "--csv",
       dest = "csvFileName",
       help = "Output CSV file name, to use with spreadsheets" )
    ( options, args ) = parser.parse_args()

    # Ideally, pattern lists ought to be defined such that categories do NOT overlap.
    # Should they overlap, the last, i.e., lower on this list, matched category, if any, wins.
    # The following categories currently overlap:
    # "PFO" and "Jet", "Muon" and "LRT",
    # "Trig" and "LRT", "Trig" and "PFO",
    # "Trig" and "caloringer", "InDet" and "LRT"
    # Set up categorization matching strings:
    categoryStrings = {
        "MetaData" : ["^DataHeader", "(.*)_mems$", "(.*)_timings$", "^Token$", "^RawInfoSummaryForTag$", "^index_ref$"],
        "Trig"     : ["^HLT", "^LVL1", "^L1", "^xTrig", "^Trig", "^CTP_Decision", "^TrigInDetTrackTruthMap", "^TrigNavigation", ".*TriggerTowers", "TileTTL1MBTS", "^TileL2Cnt", "RoIBResult","^_TRIGGER","^L1TopoRawData", "BunchConfKey"],
        "MET"      : ["^MET", "^METMAP", "JEMEtSums"],
        "EvtId"    : ["^ByteStreamEventInfo", "^EventInfo", "^McEventInfo", "^LumiBlockN", "^EventWeight", "^RunNumber", "^ConditionsRun", "^EventTime", "^BunchId", "^EventNumber","^IsTestBeam", "^IsSimulation", "^IsCalibration", "^AvgIntPerXing", "^ActualIntPerXing", "^RandomNumber", "^McChannel"], 
        "tau"      : ["^Tau", "^DiTauJets"],
        "PFO"      : ["(.*)EventShape$", "^AntiKt4EMPFlowJets", "^JetETMissChargedParticleFlowObjects", "^JetETMissNeutralParticleFlowObjects", "^CHS(.*)ChargedParticleFlowObjects", "^CHSNeutralParticleFlowObjects", "^JetETMissLCNeutralParticleFlowObjects", "^Global(.*)ParticleFlowObjects"],
        "egamma"   : ["^GSF", "^ForwardElectron", "^egamma", "^Electron(?!.*Ring)", "^Photon(?!.*Ring)"],
        "Muon"     : ["^Muon", "^TileMuObj", "^MS", "^SlowMuons", ".*Stau", "(.*)MuonTrackParticles$", "MUCTPI_RDO", "^RPC", "^TGC", "^MDT", "^CSC", "^sTGC", "^MM", ".*MuonMeasurements$", "^ExtrapolatedMuonTracks", "^CombinedMuonTracks", "^NCB_MuonSegments", "^UnAssocMuonSegments", "^EMEO_Muons", "^EMEO_MuonSpectrometerTrackParticles", "^xAODNSWSegments"],
        "BTag"     : ["^BTag"],
        "HGTD"     : ["^HGTD"],
        "InDet"    : ["^InDet", "^PrimaryVertices", "^ComTime_TRT", "^Pixel", "^TRT", "^SCT", "^BCM", "^CTP", "^Tracks", "^ResolvedForwardTracks", "^SplitClusterAmbiguityMap", "^SoftBVrt","^BLMHits", "^FourLeptonVertices"],
        "ITk"      : ["^ITk"],
        "ACTS"     : [".*Acts.*"],
        "Jet"      : ["^CamKt", "^AntiKt", "^Jet(?!.*ParticleFlowObjects$)","^LCOriginTopoClusters","^EMOriginTopoClusters"],
        "CaloTopo" : ["CaloCalTopoCluster", "CaloCalFwdTopoTowers"],
        "Calo"     : ["^LAr", "^AllCalo", "^AODCellContainer", "^MBTSContainer", "^CaloCompactCellContainer", "^CaloEntryLayer", "^E4prContainer", "^TileHitVec", "^TileCellVec", "^TileDigits", "^MBTSHits"],
        "Truth"    : ["^Truth", "Truth$", "TruthMap$", "TruthCollection$", "^PRD_MultiTruth", "TracksTruth$", ".*TrackTruth$", "TrackTruthCollection", "^HardScatter", "BornLeptons",".*ExitLayer$",".*EntryLayer$"],
        "AFP"      : ["^AFP"],
        "LRT"      : ["^LRT", "(.*)LRT$", "(.*)LRTTrackParticles$", "(.*)LargeD0TrackParticles$"],
        "caloringer"      : ["(.*)Ring"],
        "AnalysisElectrons" : ["^AnalysisElectrons" ],
        "AnalysisTauJets" : ["^AnalysisTauJets" ],
        "AnalysisPhotons" : ["^AnalysisPhotons" ],
        "AnalysisMuons" : ["^AnalysisMuons" ],
        "AnalysisJets" : ["^AnalysisJets" ],
        "AnalysisHLT" : ["^AnalysisHLT" ],
        "AnalysisTrigMatch" : ["^AnalysisTrigMatch" ],
        "AnalysisLargeRJets" : ["^AnalysisLargeRJets" ],
        "AnalysisSiHitElectrons" : ["^AnalysisSiHitElectrons" ],
        }
    
    fileNames = []

    if len( args ) > 0:
        fileNames = [ arg for arg in args if arg[ 0 ] != "-" ]
        pass

    if options.fileName is None and len( fileNames ) == 0:
        str( parser.print_help() or "" )
        sys.exit( 1 )

    if options.fileName is not None:
        fileName = os.path.expandvars( os.path.expanduser( options.fileName ) )
        fileNames.append( fileName )
        pass

    fileNames = set( fileNames )

    # Check the consistency with the CSV output:
    if len( fileNames ) > 1 and options.csvFileName:
        print( "WARNING  CSV output is only available when processing a single "
               "input file" )
        pass

    import PyUtils.PoolFile as PF
    if options.siUnits:
        PF.Units.kb = 1000.
        sizeUnits = "kB"
    else:
        sizeUnits = "KiB"

    # Pattern for a static/dynamic auxiliary variable identification
    auxvarptn = re.compile( r"Aux(?:Dyn)?(?:\.|:)" )
    # Loop over the specified file(s):
    for fileName in fileNames:

        # Open the file:
        poolFile = PF.PoolFile( fileName )

        # Loop over all the branches of the file, and sum up the information
        # about them in a smart way...
        summedData = {}
        categData  = {}
        for d in poolFile.data:
            # Skip metadata/TAG/etc. branches:
            # if d.dirType != "B": continue
            # The name of this branch:
            brName = d.name
            # Check if this is a static/dynamic auxiliary variable:
            m = auxvarptn.search( d.name )
            if m:
                # Yes, it is. And the name of the main object/container is:
                brName = d.name[:m.start()]
            # Check if we already know this container:
            if brName in summedData.keys():
                summedData[ brName ].memSize  += d.memSize
                summedData[ brName ].diskSize += d.diskSize
            else:
                summedData[ brName ] = \
                    PF.PoolRecord( brName,
                                   d.memSize,
                                   d.diskSize,
                                   d.memSizeNoZip,
                                   d.nEntries,
                                   d.dirType )
            # Set the C++ type name of the main object/container
            if brName == d.name:
                if summedData[ brName ].typeName and \
                   summedData[ brName ].typeName != d.typeName:
                    print(f"WARNING: Reset typeName {summedData[ brName ].typeName!r}"
                          f" -> {d.typeName!r} for {brName}", file=sys.stderr)
                summedData[ brName ].typeName = d.typeName
            pass

        # Order the records by size:
        orderedData = [rec for rec in summedData.values()]
        sorter = PF.PoolRecord.Sorter.DiskSize
        orderedData.sort( key = operator.attrgetter( sorter ) )

        # Print a header:
        print( "" )
        print( "=" * 106 )
        print( "         Event data" )
        print( "=" * 106 )
        print(f'{"Mem Size":^16} {"Disk Size":^16} {"Size/Evt":^16} {"Compression":>12}'
              f' {"Items":>8} Container Name (Type) [Category]')
        print( "-" * 106 )

        # Now, let's print the event-wise info that we gathered:
        memSize = 0.0
        diskSize = 0.0
        for d in orderedData:
            # keep branches with either the same number of entries as the number of events, or the
            # special tlp branches with extra event information
            mtlp = re.search( "_tlp.$", d.name ) or "DataHeader" in d.name
            if d.nEntries != poolFile.dataHeader.nEntries and not mtlp: continue

            colTypeName = d.typeName
            if colTypeName:
                for ptn in ("(?:_[pv]._|_tlp._|_v.>_)(.*)", "^[a-zA-Z]+_(.*_[lL]inks?)"):
                    m = re.search(ptn, d.name)
                    if m:
                        d_name   = m.group(1)
                        break
                else:
                    m = re.search("_tlp.$", d.name)
                    if m:
                        d_name = d.name[:m.start()].replace("_",":")
                    else:
                        d_name   = d.name
                nameType = "%s (%s)" % (d_name, colTypeName)
            else:
                m = re.search( "_v._", d.name )
                if m:
                    d_name   = d.name[m.end():]
                    nameType = "%s (%s)" % ( d_name, (d.name[:m.end()-1]) )
                else:
                    m = re.search("_tlp.$", d.name)
                    if m:
                        d_name = d.name[:m.start()].replace("_",":")
                        nameType = "%s (%s)" % (d_name, d_name + m.group())
                    else:
                        d_name = d.name
                        nameType = "%s (%s)" % ( d.name, "()" )

            # Find category:
            for categ in reversed(categoryStrings.keys()):
                for pattern in categoryStrings[ categ ]:
                    if re.match(pattern, d_name.replace("Bkg_","")):
                        catName = categ
                        # Stop searching since category found
                        break
                else:
                    continue
                # Stop searching since category found
                break
            else:
                catName = '*Unknown*'
            # Add on category to name/type
            nameType += ' [' + catName + ']'

            # Now sum up the sizes according to the category
            # Check if we already know this category:
            if catName in categData.keys():
                categData[ catName ].memSize  += d.memSize
                categData[ catName ].diskSize += d.diskSize
            else:
                categData[ catName ] = \
                    PF.PoolRecord( catName,
                                   d.memSize,
                                   d.diskSize,
                                   d.memSizeNoZip,
                                   d.nEntries,
                                   d.dirType )
                pass
            pass

            print(f"{d.memSize:12.3f} {sizeUnits:3} {d.diskSize:12.3f} {sizeUnits:3}"
                  f" {d.diskSize / max(poolFile.dataHeader.nEntries,1):12.3f} {sizeUnits:3}"
                  f" {d.memSize / d.diskSize:12.3f} {d.nEntries:8d}  {nameType:s}")
            memSize = memSize + d.memSize
            diskSize = diskSize + d.diskSize
            pass
        print( "-" * 106 )
        print(f"{memSize:12.3f} {sizeUnits:3} {diskSize:12.3f} {sizeUnits:3}"
              f" {diskSize / max(poolFile.dataHeader.nEntries,1):12.3f} {sizeUnits:3}"
              f" {memSize / diskSize:12.3f} {poolFile.dataHeader.nEntries:8d}  Total")
        print( "" )

        # Now print out the categorized information
        # Order the records by size:
        categorizedData = list(categData.values())
        sorter = PF.PoolRecord.Sorter.DiskSize
        categorizedData.sort( key = operator.attrgetter( sorter ) )

        print( "=" * 80 )
        print( "         Categorized data" )
        print( "=" * 80 )
        print(f'{"Disk Size/Evt":^16} {"Fraction":8} Category Name')
        print( "-" * 80 )
        totDiskSize = 0.0
        frac        = 0.0
        ds     = []
        dsFrac = []
        dsName = []
        for d in categorizedData:
            dsPerEvt     = d.diskSize / max(poolFile.dataHeader.nEntries,1)
            dsPerCatFrac = d.diskSize / diskSize
            totDiskSize += dsPerEvt
            frac        += dsPerCatFrac
            ds          += [dsPerEvt]
            dsFrac      += [dsPerCatFrac]
            dsName      += [d.name]
            print(f"{dsPerEvt:12.3f} {sizeUnits:3} {dsPerCatFrac:8.3f}  {d.name:s}")
            pass
        print(f"{totDiskSize:12.3f} {sizeUnits:3} {frac:8.3f}  Total")
        ds     += [totDiskSize]
        dsFrac += [frac]
        dsName += ["Total"]
        
        print( "" )
        print( "=" * 80 )
        print( "CSV for categories disk size/evt and fraction:" )
        # print out comment separated list in descending order
        print (",".join(reversed(dsName)))
        b = ['{:.3f}'.format(i)  for i in reversed(ds)]
        print (",".join(b))
        b = ['{:.3f}'.format(i)  for i in reversed(dsFrac)]
        print (",".join(b))
        print( "=" * 80 )
        print( "" )

        
        print( "=" * 80 )
        print( "         Meta data" )
        print( "=" * 80 )
        print(f'{"Mem Size":^16} {"Disk Size":^16} Container Name')
        print( "-" * 80 )

        # Now print the info about the metadata:
        memSize = 0.0
        diskSize = 0.0
        for d in orderedData:
            mtlp = re.search( "_tlp.$", d.name ) or "DataHeader" in d.name
            if d.nEntries == poolFile.dataHeader.nEntries or mtlp: continue
            print(f"{d.memSize:12.3f} {sizeUnits:3} {d.diskSize:12.3f} {sizeUnits:3}  {d.name:s}")
            memSize = memSize + d.memSize
            diskSize = diskSize + d.diskSize
            pass
        print( "-" * 80 )
        print(f"{memSize:12.3f} {sizeUnits:3} {diskSize:12.3f} {sizeUnits:3}  Total")
        print( "=" * 80 )

        # Write out a CSV file if one was requested:
        if options.csvFileName and ( len( fileNames ) == 1 ):
            # Open the output file:
            import csv
            args = {'newline' : ''}
            with open( options.csvFileName, "w", **args ) as f:
                writer = csv.writer( f )
                # Set up the formatting of the file:
                writer.writerow( [ "Name (Type)", "Size/Evt" ] )
                # Write all entries to it:
                for d in orderedData:
                    # Skip metadata items:
                    if d.nEntries != poolFile.dataHeader.nEntries: continue
                    # Construct the name of the entry:
                    colTypeName = d.typeName
                    if not colTypeName: continue
                    nameType = "%s (%s)" % \
                        ( d.name, colTypeName )
                    # Write the entry:
                    writer.writerow( [ nameType, d.diskSize / d.nEntries ] )
                    pass
                pass
            pass

        if len(fileNames) > 1:
            print ("")
        pass # loop over fileNames

    print ("## Bye.")
    sys.exit( 0 )
