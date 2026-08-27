# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


import ROOT
import pyAMI.client
from pyAMI.atlas.api import get_dataset_info

def SampleHandler_QueryAmi(samples):
    # set up an AMI client
    # This is the basic minimum - and it will look for an encrypted file
    # with your user credentials.  If it does not find that it will try
    # for a VOMS proxy.  Make the encrypted file by running "ami auth"
    # first.  See https://atlas-ami.cern.ch/AMI/pyAMI/examples/api.html

    amiClient = pyAMI.client.Client('atlas')

    # The quantities in the class MetaDataSample are pretty much all I need:
    #    * whether it is data or MC
    #    * the luminosity of the sample
    #    * the k-factor of the sample (only for MC)
    #    * the number of events in the sample
    #    * the cross section of the sample (only for MC)
    #    * the filter efficiency

    data = ROOT.SH.MetaDataQuery()
    data.messages = 'done by ami query'
    # I am assuming that "samples" is a list of dataset names, and that
    # the user already checked that they exist and are valid
    for sample in samples:
        sample_noscope = sample.split(':')[-1]

        mydata = ROOT.SH.MetaDataSample(sample)
        # The first question you ask is whether it is data or MC.
        # description: 1 for data, 0 for MC, or -1 if this is not known.
        mydata.source = 'https://atlas-ami.cern.ch/AMI/pyAMI/'
        mydata.unknown = 0

        if sample.startswith("mc"):
            mydata.isData = 0
        elif sample.startswith("data"):
            mydata.isData = 1
        else:
            mydata.isData = -1

        # AMI does not specifically catalogue TID datasets, so strip the
        # suffix off the name that is actually queried (sample_noscope).
        if "_tid" in sample_noscope:
            print("Stripping tid suffix from " + sample_noscope)
            sample_noscope = sample_noscope.split("_tid")[0]

        # All datasets should have the number of events.  If the dataset
        # is not known to AMI, flag it and move on rather than aborting
        # the whole query.
        try:
            amiinfo = get_dataset_info(amiClient, sample_noscope)[0]
        except Exception as e:
            print("failed to get AMI info for " + sample_noscope + ": " + str(e))
            mydata.unknown = 1
            data.addSample(mydata)
            continue

        mydata.nevents = int(amiinfo['totalEvents'])

        # AMI does not yet have a function for getting luminosity, and we
        # have no k-factor information, so those are placeholders.
        if mydata.isData == 1:
            mydata.crossSection = -1
            mydata.filterEfficiency = -1
        else:
            mydata.luminosity = -1
            # MC - can get cross-section and filter efficiency
            try:
                xsec = float(amiinfo['approx_crossSection'])
                effic = float(amiinfo['approx_GenFiltEff'])
            except KeyError as e:
                print("AMI info missing cross-section/filter efficiency for " + sample_noscope + ": " + str(e))
                mydata.unknown = 1
                data.addSample(mydata)
                continue
            mydata.crossSection = xsec
            mydata.filterEfficiency = effic
            if mydata.crossSection > 0 and mydata.filterEfficiency > 0:
                mydata.luminosity = float(float(mydata.nevents) / (mydata.crossSection * mydata.filterEfficiency))

        data.addSample(mydata)
    return data
