#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Tool for the parsing of event generation log files
# This keeps all the event generation specific pieces in Generators packages
# Gets imported and used in the job transform infrastructure

# For logging (obv)
from AthenaCommon import Logging
# For log line matching
import re

class evgenParserTool:

    def __init__ ( self, name = 'evgenLogParser', **kw ):
        self.msg = Logging.logging.getLogger(name)
        # For FixHepMC, keep a list of reasons for rejection, counts, and lines for logging
        self.FixHepMCDict = {'reasons':{},'denominator':0,'lines':{}}
        # For TestHepMC, we need summary statistics as well as all the individual numbers
        # Keep number passing and failing, p/f line (for printing), all the rates with the lines, which ones are(n't) included in the efficiency
        # the line for the rates, the line for the final efficiency, and the number of events in this log passing/failing (for converting % to count)
        self.TestHepMCDict = {'p':0,'f':0,'pfline':None,'rate':{},'notinc':[],'rline':None,'effline':None,'lastpf':0}
        # For the filter sequence, keep the weighted and unweighted numbers and the line
        self.FilterSeqDict = {'num':0,'den':0,'wnum':0,'wden':0,'line':None,'wline':None}
        # Process the Metadata as well
        self.MetadataDict = {'sumOfPosWeights':0.,'sumOfNegWeights':0.,'sumOfSqrWeights':0.,
                             'sumOfPosWeightsNoFilter':0.,'sumOfNegWeightsNoFilter':0.,'sumOfSqrWeightsNoFilter':0.,
                             'xsec_holder':0.,'xsec_weight':0.,'xsec_sum':0.}
        self.isMP = -1

    def processLine( self, line ):
        ''' Function to process a log line and keep what's needed for final reporting'''

        # Skip PerfMonMTSvc report
        if "PerfMonMTSvc" in line:
            return

        # First check for lines from FixHepMC
        if 'FixHepMC' in line and 'INFO Removed' in line:
            # Use the loops line to count the denominator
            if 'because of loops' in line:
                self.FixHepMCDict['denominator'] += int( line.split(' of ')[1].split()[0].strip() )
            # Grab the reason for failure from the line; they all follow the same formula
            reason = line.split('particles')[1]
            # Add to the count for that reason
            if reason not in self.FixHepMCDict['reasons']:
                self.FixHepMCDict['reasons'][reason] = 0
            self.FixHepMCDict['reasons'][reason] += int( line.split('Removed')[1].split()[0].strip() )
            # Make sure that we also have a log line for printing later if we need it
            if reason not in self.FixHepMCDict['lines']:
                self.FixHepMCDict['lines'][reason] = [ line.split('Removed')[0] , line.split('particles')[1] ]
        # Second up: Filter statistics
        elif 'Py:EvgenFilterSeq' in line:
            # First check weighted, then unweighted (one is a substring of the other...)
            if 'Weighted Filter Efficiency' in line:
                # Grab all the numbers from the log. The line prints the numerator and denominator explicitly at the end
                numbers = re.findall(r'[\d.]+',line)
                self.FilterSeqDict['wnum'] += float( numbers[-2] )
                self.FilterSeqDict['wden'] += float( numbers[-1] )
                # Grab the log line to print
                self.FilterSeqDict['wline'] = line.split('=')[0]
            elif 'Filter Efficiency' in line:
                # Grab all the numbers from the log. The line prints the numerator and denominator explicitly at the end
                numbers = re.findall(r'[\d.]+',line)
                self.FilterSeqDict['num'] += int( numbers[-2] )
                self.FilterSeqDict['den'] += int( numbers[-1] )
                # Grab the log line to print
                if self.FilterSeqDict['line'] is None:
                    self.FilterSeqDict['line'] = line.split('=')[0]
        # Third and finally: TestHepMC
        elif 'TestHepMC' in line and 'Event' in line:
            if 'Events passed' in line:
                # Simplest line, just shows the numbers of events passing and failing
                numbers = re.findall(r'[\d.]+',line)
                self.TestHepMCDict['p'] += int(numbers[-2])
                self.TestHepMCDict['f'] += int(numbers[-1])
                # Save the log line so that we can reproduce it later
                self.TestHepMCDict['pfline'] = line.split('=')[0]
                # Keep track of the denominators to ensure we get the percentages right
                self.TestHepMCDict['lastpf'] = int(numbers[-2]) + int(numbers[-1])
            else:
                # Otherwise this is an event rate line. First figure out what the reason in the line is, and index count on that
                reason = line.split('Event rate')[1].split('=')[0]
                if reason not in self.TestHepMCDict['rate']:
                    self.TestHepMCDict['rate'][reason] = 0
                    # Some rates are not included in the test efficiency; we should be ready to point that out as well
                    if 'not included in test efficiency' in line:
                        self.TestHepMCDict['notinc'] += [reason]
                # We have to convert the logged percentage back to counts, and then back to percentage in the report
                my_perc = float( line.split('=')[1].split('%')[0] )
                self.TestHepMCDict['rate'][reason] += my_perc * self.TestHepMCDict['lastpf']/100.
                # And last we just have to get the log format for an event rate line right
                self.TestHepMCDict['rline'] = line.split('Event rate')[0]
        # Extra catch for Efficiency line for TestHepMC
        elif 'TestHepMC' in line and 'Efficiency' in line:
            # If this is just the efficiency line, all we need is the log line format
            self.TestHepMCDict['effline'] = line.split('=')[0]
            # Because this is a unique line, we will also use it to check if we are running MP
            self.isMP += 1
        elif 'MetaData:' in line:
            # If it's a metadata line, we just sum them up, with one exception
            field = line.split('MetaData:')[1].split('=')[0].strip()
            if field in self.MetadataDict:
                self.MetadataDict[field] += float( line.split('=')[1] )
            # Cross section requires special attention
            # The cross section field itself comes first, so we have to just stash it
            if field == 'cross-section (nb)':
                self.MetadataDict['xsec_holder'] = float( line.split('=')[1] )
            # The weights fields are also there; we need the 'no filter' versions
            # Use = here to reset each round
            elif field == 'sumOfPosWeightsNoFilter':
                self.MetadataDict['xsec_weight'] = float( line.split('=')[1] )
            # Sum of negative weights is last, and now we have all the info we need
            elif field == 'sumOfNegWeightsNoFilter':
                my_negw = float( line.split('=')[1] )
                self.MetadataDict['xsec_sum'] += self.MetadataDict['xsec_holder']*(self.MetadataDict['xsec_weight']-my_negw)
                # We don't need to keep the sum of weights here, because we have it elsewhere

    def report(self):
        ''' Function to print final statistics grabbed from the logs'''
        # If we aren't running MP, then forget it
        if self.isMP<1:
            self.msg.debug('Not running MP, nothing to do')
            return
        # Now we are going to print updated statistics for all the handlers
        # Because in an MP job we get a primary worker report as well, the number of workers is just self.isMP
        self.msg.info(f'Printing final summary statistics from {self.isMP} MP workers')
        # First print all the FixHepMC stuff - just rates for each reason we remove particles
        for reason in self.FixHepMCDict['reasons']:
            print(f"{self.FixHepMCDict['lines'][reason][0]}Removed {self.FixHepMCDict['reasons'][reason]} of {self.FixHepMCDict['denominator']} particles {self.FixHepMCDict['lines'][reason][1]}")
        # Next print all the information from TestHepMC, starting with the pass/fail summary
        print(f"{self.TestHepMCDict['pfline']}= {self.TestHepMCDict['p']}, Events Failed = {self.TestHepMCDict['f']}")
        # We will have the same denominator for all the ratios that we print next
        denom = self.TestHepMCDict['p'] + self.TestHepMCDict['f']
        # Now go through all the TestHepMC checks, and for each one recreate the log line
        for rate in self.TestHepMCDict['rate']:
            # Again, have to handle zero denominators correctly
            pct = 0.
            if denom>0:
                pct = (self.TestHepMCDict['rate'][rate] / denom)*100.
            print(f"{self.TestHepMCDict['rline']}Event rate {rate} = {pct}%{' (not included in test efficiency)' if rate in self.TestHepMCDict['notinc'] else ''}")
        # Now print the final efficiency; make sure we handle zeroes correctly
        final_eff = 1.
        if denom:
            final_eff = self.TestHepMCDict['p']/denom
        print(f"{self.TestHepMCDict['effline']}= {final_eff*100.}%")
        # Now print our filter sequence summary - again, make sure we handle zeroes correctly
        eff = (self.FilterSeqDict['num']/self.FilterSeqDict['den']) if self.FilterSeqDict['den'] > 0 else 1.
        print(f"{self.FilterSeqDict['line']} = {eff} [{self.FilterSeqDict['num']} / {self.FilterSeqDict['den']}]")
        eff = (self.FilterSeqDict['wnum']/self.FilterSeqDict['wden']) if self.FilterSeqDict['wden'] >0 else 1.
        print(f"{self.FilterSeqDict['wline']} = {eff} [{self.FilterSeqDict['wnum']} / {self.FilterSeqDict['wden']}]")
        # Done!

    def updateMetadata(self, metadata):
        ''' Update the metadata based on the logfile information'''
        # If we aren't running MP, then forget it
        if self.isMP<1:
            self.msg.debug('Not running MP, nothing to do')
            return metadata
        # Print the updated metadata as we go, as well as updating the dictionary
        # First by convention is the cross-section, which we have to calculate
        denom = self.MetadataDict['sumOfPosWeightsNoFilter']-self.MetadataDict['sumOfNegWeightsNoFilter']
        my_xsec = 0.
        if denom > 0.:
            my_xsec = self.MetadataDict['xsec_sum'] / denom
        self.msg.info(f'cross-section (nb)= {my_xsec:e}')
        metadata['cross-section (nb)'] = f'{my_xsec:e}'
        # Now come all the fields that we had saved
        self.msg.info('Updated metadata:')
        for field in self.MetadataDict:
            # Need to skip the cross section related fields
            if field in metadata:
                self.msg.info(f'{field} = {self.MetadataDict[field]:e}')
                metadata[field] = f'{self.MetadataDict[field]:e}'
        # Generator filter efficiency needs some special handling
        geneff = 1.
        if denom>0:
            geneff = (self.MetadataDict['sumOfPosWeights']-self.MetadataDict['sumOfNegWeights'])/denom
        self.msg.info(f'GenFiltEff = {geneff:e}')
        if 'GenFiltEff' in metadata:
            metadata['GenFiltEff'] = f'{geneff:e}'
        return metadata

