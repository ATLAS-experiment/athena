# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
evgenConfig.description = "Geneva4 MC pp -> Z production for tutorial example at 13000 GeV to 2 muons"
evgenConfig.keywords = ["2muon","Z"]
evgenConfig.contact = ["andri.verbytskyi@mpp.mpg.de"]

from Geneva_i.GenevaUtils import GenevaConfig, GenevaRun

#class with the superchic initialization parameters.  Please see GenevaUtils for a complete list of tunable parameters.
scConfig = GenevaConfig(runArgs)

scConfig.card = """
#-------------------------------------------------------------------------------
# Author(s):
#    Simone Alioli, Christian Bauer, Frank Tackmann
#
# Copyright:
#    Copyright (C) 2017, 2018 Geneva Collaboration
#
#    This file is part of the Geneva MC framework. Geneva is distributed under
#    the terms of the GNU General Public License version 3 (GPLv3), see the
#    COPYING file that comes with this distribution for details.
#    Please respect the academic usage guidelines in the GUIDELINES file.
#
# Description:
#    User options for inclusive pp -> V production for tutorial example
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Global run options
#-------------------------------------------------------------------------------

global:
   process: pp_V
   run_name: "tutorial"
   num_events: 100000

input_output:
   verbosity: info
   overwrite_output: true

event_generation:
   unweighting:
      partial_relative: 30
   random:
      seed: 1

process:
   pp_V:
      initial_state:
         beams: pp
         Ecm: 13000
         pdf_provider:
            LHAPDF:
               set: "PDF4LHC15_nnlo_100"
      final_state:
         boson_type: Z
         boson_mass: 91.1876
         boson_width: 2.4952
         decay: mu+mu-
      calculation: SCETppV012
      phase_space: PP2BosonJets
      matrix_element_provider: OpenLoops

#-------------------------------------------------------------------------------
# Calculation
#-------------------------------------------------------------------------------

calculation:
   SCETppV012:
      precision: NNLO+NNLL0+NLL1
      scale_settings:
         fixed_order:
            dynamic: TransverseMass
      couplings:
         alpha_s: fromPDF
         alpha_em:
            scale: 91.1876
            value: 7.55638E-03
         GF: 1.16639E-05
         sin2W: 0.2226459

#-------------------------------------------------------------------------------
# Phase space generator
#-------------------------------------------------------------------------------

phase_space:
   PP2BosonJets:
      generation_cuts:
         boson_mass:
            min: 50
            max: 150
        """
scConfig.sigmabelow='35.96 55.97 14.27'

scConfig.parseCard()
scConfig.yaml['phase_space']['PP2BosonJets']['generation_cuts']['boson_mass']['min']=55
scConfig.points = 100000

GenevaRun(scConfig, genSeq)
include('Geneva_i/Pythia8_Base_Common.py')
