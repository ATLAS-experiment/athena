# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os
import re
import math
import xml.dom.minidom

from AthenaPython.PyAthena import StatusCode
from AthenaPython.PyAthena import Alg

class LheConverterUpc(Alg):
    '''
    Class for modifying output LHE file from Superchic and Madgraph + ensuring compatibility with Tauola
    Intended for ultraperipheral collision (UPC) processes i.e. y y -> l+ l-
    This code also contains a hack that was present in the earlier version in the Generators/Superchic_i/python/LheConverterTauolaPhotonHack.py
    The hack was recommended by the Tauola authors, intended for the gamma gamma -> tau+ tau- process
    The hack changes the PDG ID of the initial state particles, from photons to electrons
    '''

    def __init__(self, name='LheConverterUpc', generator='Superchic', mode='Pythia8', energy=5020):
        super(LheConverterUpc, self).__init__(name=name)
        self.generator = generator # options: 'Superchic' (default), 'Madgraph5'
        self.mode = mode # options: 'Pythia8' (default), 'Tauolapp'
        self.energy = energy # energy read via JOs from runArg
    
    outFileName = 'events.lhe'
    done = False
    leptons = ['11', '-11', '13', '-13', '15', '-15']  # e, µ, τ

    def initialize(self):

        self.inFileName = self.fileName

        if self.inFileName==self.outFileName:
            import shutil
            shutil.move('./events.lhe','events.lhe_tmpconverter')
            self.inFileName = 'events.lhe_tmpconverter'
        if(os.path.isfile(self.inFileName)):
            print(self.fileName)
            return self.convert()
        else:
            return StatusCode.Failure

    def convert(self):
        '''
        Modifies `events.lhe` output file from Madgraph by doing the following:
            - Replaces init block
            - Changes photon px and py to zero and energy to pz
            - Recalculates energy scales as an average of lepton pair transverse momentum
        '''
        if not self.done:
            DOMTree = xml.dom.minidom.parse(self.inFileName)
            collection = DOMTree.documentElement

            # Replace init block
            init = collection.getElementsByTagName('init')

            # LHE requires very strict format of the <init> block
            l_init_repl1=["13","-13","2.510000e+03","2.510000e+03","0","0","0","0","3","1"]
            str_init_repl2 =" 1.000000e+00  0.000000e+00  1.000000e+00  9999"
            l_init_repl1[2]='%.6e'  %(self.energy/2.)
            l_init_repl1[3]='%.6e'  %(self.energy/2.)
            
            delimiter = "  " # Define a delimiter
            str_init_repl1 = map(str, l_init_repl1)
            jstr_init_repl1 = delimiter.join(str_init_repl1)
            jstr_init_repl1 = '\n ' + jstr_init_repl1 + '\n'
            jstr_init_repl = jstr_init_repl1 + str_init_repl2 + '\n'
            init[0].firstChild.data = jstr_init_repl

            # The comment line below indicates which part of the regex grabs the parton-level information that's to be modified. The index in list(header.groups()) is also shown
            #                                        energy scale [1]
            event_header = r'^(\s*\S*\s*\S*\s*\S*\s*)(\S*)(.*)$'

            # The comment line below indicates which part of the regex grabs the parton-level information that's to be modified. The index in list(particle) is also shown
            #                          pdgid [1]                                                       px [3]    py [5]    pz [7]    e [9]
            event_particle = r'^(\s*)([0-9-]+)(\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+)(\S+)(\s+)(\S+)(\s+)(\S+)(\s+)(\S+)(.*)$'

            events = collection.getElementsByTagName('event')
            for i, event in enumerate(events):

                # Remove the <mgrwt> block in case of Madgraph (not needed in UPC)
                nodes = event.getElementsByTagName('mgrwt')
                for node in nodes:
                    parent = node.parentNode
                    parent.removeChild(node)

                new_particles = []
                lepton_mom = []

                particles = re.findall(event_particle, event.firstChild.data, re.MULTILINE)
                for particle in particles:
                    particle = list(particle)
                    if particle[1] == '22':  # photon
                        # Zero photon transverse momentum, change energy to be equal pz
                        particle[3] = '0.'
                        particle[5] = '0.'
                        particle[9] = particle[7]
                        # If Tauolapp requested, change PID of photons to electrons (hack recommended by Tauola authors)
                        if self.mode == 'Tauolapp':
                            if i%2 == 0:
                                particle[1] = '11' if float(particle[7]) > 0.0 else '-11'
                            else:
                                particle[1] = '-11' if float(particle[7]) > 0.0 else '11'
                        new_particles.append(''.join(particle))
                    elif particle[1] in self.leptons:
                        # Read leptons px and py
                        lepton_mom.append(float(particle[3]))
                        lepton_mom.append(float(particle[5]))
                        new_particles.append(''.join(particle))
                    else:
                        new_particles.append(''.join(particle))

                # Calculate new energy scale
                l1_px, l1_py, l2_px, l2_py = lepton_mom
                l1_pt = math.sqrt(l1_px**2 + l1_py**2)
                l2_pt = math.sqrt(l2_px**2 + l2_py**2)
                energy_scale = f'{(l1_pt + l2_pt) / 2.:.9E}'

                header = re.search(event_header, event.firstChild.data, re.MULTILINE)
                header = list(header.groups())
                header[1] = energy_scale

                if self.generator == 'Superchic': 
                    event.firstChild.data = '\n'.join([''.join(header)] + new_particles) + '\n'
                if self.generator == 'Madgraph5':
                    event.firstChild.data = '\n'.join([''.join(header)] + new_particles)

            with open(self.outFileName, 'w') as output:
                output.write(DOMTree.toxml().replace('<?xml version="1.0" ?>', ' '))

            self.done = True

        return StatusCode.Success

