# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This filter select events with a certain number of particles stored in the record, regardless of their status, pdgID, pT, etc.
# To use the NParticles filter in a joboption one would need to include the following lines
    #include('MCJobOptionUtils/LHEFilter_NParticles.py')
    #lheFilter = LHEFilter_NParticles()
    #lheFilter.name = "MyCustomNLeptonsFilter" # usefull when combining several filters but not mandatory
    #lheFilter.NumParticles = 1
    #lheFilter()

# NLeptons filter
class LHEFilter_NParticles(BaseLHEFilter):
    def __init__(self):
        # name of the filter, usefull for logfiles
        self.name = "NParticles"
        # set to True to invert the filter
        self.inverted = False
        # parameters for this filter
        self.NumParticles = -1 # Negative integers for inclusive (-1 means >=1), positive integers for exclusive

    def pass_filter(self,Evt):
        # first retrieves the number of particles
        NParticles = Evt.info.nparticles
        # then decide if we keep the event
        if self.NumParticles<0: # negative means inclusive
            return (NParticles >= -self.NumParticles)
        else:
            return (NParticles == self.NumParticles)
