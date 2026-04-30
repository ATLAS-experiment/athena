include("Sherpa_i/Base_Fragment.py")
include("Sherpa_i/PDF4LHC21.py")

evgenConfig.description = "Sherpa 3 + SMEFTsim_topU3l_MwScheme_UFO example JO"
evgenConfig.keywords = []
evgenConfig.contact  = []

genSeq.Sherpa_i.RunCard="""

# ME generator setup
ME_GENERATORS:
        - Comix

SCALES: METS
MODEL: SMEFTsim_topU3l_MwScheme_UFO
WIDTH_SCHEME: Fixed
PARTICLE_DATA:
        24: 
             Width: 0
        23:
             Width: 0



PROCESSES:

- 93 93 -> 24 93 93{2}:
    Order: {QCD: Any, QED: Any, SMHLOOP: Any, NP: 1, NPshifts: Any, NPprop: Any, NPcpv: Any, NPcbb: Any, NPcbB: Any, NPcbBB: Any, NPcbd1: Any, NPcbd8: Any, NPcbe: Any, NPcbG: Any, NPcbH: Any, NPcbj1: Any, NPcbj8: Any, NPcbl: Any, NPcbu1: Any, NPcbu8: Any, NPcbW: Any, NPcdB: Any, NPcdd1: Any, NPcdd8: Any, NPcdG: Any, NPcdH: Any, NPcdW: Any, NPceB: Any, NPced: Any, NPcee: Any, NPceH: Any, NPceu: Any, NPceW: Any, NPcG: Any, NPcGtil: Any, NPcH: Any, NPcHB: Any, NPcHbox: Any, NPcHbq: Any, NPcHBtil: Any, NPcHd: Any, NPcHDD: Any, NPcHe: Any, NPcHG: Any, NPcHGtil: Any, NPcHj1: Any, NPcHj3: Any, NPcHl1: Any, NPcHl3: Any, NPcHQ1: Any, NPcHQ3: Any, NPcHt: Any, NPcHtb: Any, NPcHu: Any, NPcHud: Any, NPcHW: Any, NPcHWB: Any, NPcHWBtil: Any, NPcHWtil: Any, NPcjd1: Any, NPcjd8: Any, NPcje: Any, NPcjj11: Any, NPcjj18: Any, NPcjj31: Any, NPcjj38: Any, NPcjQbd1: Any, NPcjQbd8: Any, NPcjQtu1: Any, NPcjQtu8: Any, NPcjtQd1: Any, NPcjtQd8: Any, NPcju1: Any, NPcju8: Any, NPcjujd1: Any, NPcjujd11: Any, NPcjujd8: Any, NPcjujd81: Any, NPcjuQb1: Any, NPcjuQb8: Any, NPcld: Any, NPcle: Any, NPclebQ: Any, NPcledj: Any, NPcleju1: Any, NPcleju3: Any, NPcleQt1: Any, NPcleQt3: Any, NPclj1: Any, NPclj3: Any, NPcll: Any, NPcll1: Any, NPclu: Any, NPcQb1: Any, NPcQb8: Any, NPcQd1: Any, NPcQd8: Any, NPcQe: Any, NPcQj11: Any, NPcQj18: Any, NPcQj31: Any, NPcQj38: Any, NPcQl1: Any, NPcQl3: Any, NPcQQ1: Any, NPcQQ8: Any, NPcQt1: Any, NPcQt8: Any, NPcQtjd1: Any, NPcQtjd8: Any, NPcQtQb1: Any, NPcQtQb8: Any, NPcQu1: Any, NPcQu8: Any, NPcQujb1: Any, NPcQujb8: Any, NPctB: Any, NPctb1: Any, NPctb8: Any, NPctd1: Any, NPctd8: Any, NPcte: Any, NPctG: Any, NPctH: Any, NPctj1: Any, NPctj8: Any, NPctl: Any, NPctt: Any, NPctu1: Any, NPctu8: Any, NPctW: Any, NPcuB: Any, NPcud1: Any, NPcud8: Any, NPcuG: Any, NPcuH: Any, NPcutbd1: Any, NPcutbd8: Any, NPcuu1: Any, NPcuu8: Any, NPcuW: Any, NPcW: Any, NPcWtil: Any, NPQjujb8: Any}
    CKKW: 20.0

HARD_DECAYS:
   Enabled: true
   Channels:
      24,2,-1:  { Status: 2 }
      24,4,-3:  { Status: 2 }

SELECTORS:

     - [PT, 24, 20, E_CMS]

UFO_PARAM_CARD: param_SMEFTsim_topU3l_MwScheme_UFO.dat
"""

#genSeq.Sherpa_i.Parameters += []
genSeq.Sherpa_i.OpenLoopsLibs = []
genSeq.Sherpa_i.ExtraFiles = ['libSherpaSMEFTsim_topU3l_MwScheme_UFO.so', 'param_SMEFTsim_topU3l_MwScheme_UFO.dat']
genSeq.Sherpa_i.NCores = 1
