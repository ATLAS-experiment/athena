#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''
@file RatesScanTrigger.py
@brief Accumulator class to buffer data for a single scan trigger and export this to ROOT
'''

from AthenaCommon.Logging import logging
log = logging.getLogger('RatesScanTrigger')


def apply_pileup_correction(hist_cumulative, metadata):
    """Apply pileup correction to cumulative histogram"""

    BCR = metadata['bunchCrossingRate']
    pileup = metadata['targetMu']
    
    # Create corrected histogram
    hist_pileup_corrected = hist_cumulative.Clone(f"{hist_cumulative.GetName()}_pileup_corrected")
    
    # Apply pileup correction formula to each bin
    for bin in range(1, hist_pileup_corrected.GetNbinsX() + 1):
        bin_value = hist_cumulative.GetBinContent(bin)
        P_scatter = bin_value / BCR / pileup
        corrected_value = BCR * (1.0 - pow(1.0 - P_scatter, pileup))
        hist_pileup_corrected.SetBinContent(bin, corrected_value)
        
        # Propagate errors
        bin_error = hist_cumulative.GetBinError(bin)
        if bin_value > 0:
            error_scale = corrected_value / bin_value 
            hist_pileup_corrected.SetBinError(bin, bin_error * error_scale)
        else:
            hist_pileup_corrected.SetBinError(bin, 0)
    
    return hist_pileup_corrected


class RatesScanTrigger:
    def __init__(self, name, metadata, numerators):
        self.name = name

        self.sumHisto = None

        for key, histo in numerators.items():
            denominator = 1.0
            try:
                if metadata['multiSliceDiJet']:
                    denominator = metadata['n_evts_weighted'+key]
                else:
                    denominator = metadata['normalisation'+key]
            except KeyError:
                log.error(f"Key {key} not found in slice denominator dictionary")
            histo.Scale(1/denominator)
            if self.sumHisto is None: 
                self.sumHisto = histo
            else:
                self.sumHisto.Add(histo)  
        if metadata['doBinomialCorrection']: self.sumHisto = apply_pileup_correction(self.sumHisto, metadata)


    def export(self, exportdict):
        myDict = {}
        myDict['rate'] = self.sumHisto
        exportdict[self.name] = myDict