# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Helper functions for working with parameter settings in MadGraph

from AthenaCommon import Logging
mgparlog = Logging.logging.getLogger('MadGraphParamHelpers')    

def set_SM_params(class_instance,FourFS=False):
    """ Set default SM parameters
    Recommended parameter page from PMG:
    https://twiki.cern.ch/twiki/bin/view/AtlasProtected/McProductionCommonParametersMC15
    """
    
    param_card_settings = {
    'mass' : {
'5':  "0.000000",
'15': "1.777000e+00",
'23': "9.118760e+01",
'24': "8.039900e+01",
'25': "1.250000e+02",
        },
    'yukawa' : { '15': "1.777000e+00 # ymtau" },
    'DECAY' : {
'5'  : """DECAY  5   0.000000e+00""",
'15' : """DECAY  15   0.000000e+00""",
'23' : """DECAY  23   2.495200e+00""",
'24': """DECAY  24   2.085000e+00
    3.377000e-01   2   -1   2
    3.377000e-01   2   -3   4
    1.082000e-01   2  -11  12
    1.082000e-01   2  -13  14
    1.082000e-01   2  -15  16""",
       }
    }
    if FourFS:
        param_card_settings['mass']['5']="4.950000e+00"

    # do not set particle mass or yukawa if parameter does not exist in card
    existing_particle_info= class_instance.paramCardDict #parse_particle_info(process_dir)
    for block in param_card_settings:
        for pdgid in list(param_card_settings[block].keys()):
            if block in existing_particle_info and pdgid not in existing_particle_info[block]:
                mgparlog.warning('Not setting {} for {} as parameter does not exist in param card'.format(block,pdgid))
                param_card_settings[block].pop(pdgid)
                
    class_instance.modify_paramCardDict(params=param_card_settings)


def set_top_params(class_instance,mTop=172.5,FourFS=False):
    """ Set default parameters requested by the top group
    This is a convenience helper function
    Recommended parameter page from PMG:
    https://twiki.cern.ch/twiki/bin/view/AtlasProtected/McProductionCommonParametersMC15
    """
    # Set SM parameters
    set_SM_params(class_instance,FourFS)
    # Set Higgs parameters
    set_higgs_params(class_instance)
    # Calculate the top width based on the mass
    # From https://gitlab.cern.ch/atlasphys-top/reco/MC/blob/master/MCinfo/get_t_width.py
    import math
    # ATLAS MC11 conventions == PDG2010 
    # Vtb=0.999152 
    # using Vtb=1.0 since part of the inputs was already produced using this approximation 
    Vtb=1.0
    M_W=80.399
    # PDG2010 
    G_F=1.16637*(math.pow(10,-5))
    # MSbar alpha_s(mt)
    alpha_s=0.108
    # Born gamma coeff. 
    C1=G_F/(8*math.pi*math.sqrt(2))
    # Born approximation (taking intermediate W-boson to be on-shell) [1]
    wTop_B=C1*math.pow(float(mTop),3)*math.pow(Vtb,2)*pow((1-math.pow((M_W/float(mTop)),2)),2)*(1+2*pow((M_W/float(mTop)),2))
    # EW and QCD corrections to Born: QCD dominates, EW can be neglected [1],[2],[3]
    wTop=wTop_B*(1-0.81*alpha_s-1.81*pow(alpha_s,2))

    param_card_settings = {
    'mass' : { '6':  str(mTop) },
    'yukawa' : { '6': "1.725000e+02 # ymt" },

    'DECAY' : { '6'  : """DECAY  6   """+str(wTop)+"""
    1.000000e+00   2    5  24 # 1.32""",
        },
    }
    class_instance.modify_paramCardDict(params=param_card_settings)


def set_higgs_params(class_instance):
    """ Set Higgs mass and decays
    BR from the Higgs XSec working group for a 125.0 GeV Higgs
    https://twiki.cern.ch/twiki/pub/LHCPhysics/LHCHXSWGBRs/BR.central.dat
    """
    param_card_settings = {
     'MASS' : { '25': "1.250000e+02" },
     'DECAY': { '25': """DECAY  25   6.382339e-03
     5.767E-01 2   5  -5 # H->bb
     6.319E-02 2  15 -15 # H->tautau
     2.192E-04 2  13 -13 # H->mumu
     2.462E-04 2   3  -3 # H->ss
     2.911E-02 2   4  -4 # H->cc
     8.569E-02 2  21  21 # H->gg
     2.277E-03 2  22  22 # H->gammagamma
     1.539E-03 2  22  23 # H->Zgamma
     2.146E-01 2  24 -24 # H->WW
     2.641E-02 2  23  23 # H->ZZ""" }
     }

    class_instance.modify_paramCardDict(params=param_card_settings)


# Functions for PMG default parameters from S Moebius
def do_PMG_updates(class_instance):
    """ Update the parameters according to PMG defaults
    Takes a process directory
    No return value -- updates the param card in place
    """
    param_card_settings = get_PMG_updates(class_instance)
    class_instance.modify_paramCardDict(params=param_card_settings)


def check_PMG_updates(class_instance):
    """ Check if the param card is consistent with the PMG values
    Takes a process directory
    Prints info in case there is an inconsistency
    Return value is an error code (0 for all ok)
    """
    param_card_settings = get_PMG_updates(class_instance)
    code = 0
    for block in param_card_settings:
        if len(param_card_settings[block].keys())>0:
            mgparlog.info('Block '+block+' needs updates: '+str(param_card_settings[block]))
            code = 1
    return code


def get_PMG_updates(class_instance):
    """ Get the required PMG parameter updates
    Takes the location of a process directory
    Returns the dictionary of changes needed for updating to the default params
    """
    # Load the list of masses and widths from the param card
    masses = get_masses(class_instance.paramCardDict)
    widths = get_widths(class_instance.paramCardDict)
    yukawas = get_block(class_instance.paramCardDict, 'yukawa')

    # Read in parameters from dictionary
    from EvgenProdTools.offline_dict import parameters
                        
    # Create translation dictionary to be able to produce dictionary that can be read by build_param_card function
    # for now only the quarks, leptons and bosons are included
    particles_for_update = [  '6',  '5',  '4', '3', '2', '1', # Quarks
                             '25', '24', '23', # Bosons
                             '11', '13', '15' ] # Leptons
    newparamdict = {
        'mass' : {},
        'decay' : {},
        'yukawa' : {},
      }

    # Loop through parameter dictionary and fill newparamdict, which will then be used to update the param_card
    for pid, settings in parameters['particles'].items():
        # dictionary key is the PID of a particle
        # settings is a dictionary with mass, width and name as keys
        # In case we need a name for debugging or other things, it's here
        #particle_name = settings['name']
        if pid in particles_for_update:
            width = settings['width']
            mass = settings['mass']
            # Mass: Always set it, even if it wasn't in the dictionary
            if pid not in masses or masses[pid]!=mass:
                newparamdict['mass'][pid] = mass
            # Width: Always set it, even if it wasn't in the dictionary
            if pid not in widths or widths[pid]!=width:
                newparamdict['decay'][pid] = 'DECAY '+pid+' '+width
            # Yukawa: Set it only if it was in the dictionary
            # This is to protect against models that don't use Yukawa blocks as normal
            if pid in yukawas and yukawas[pid]!=mass:
                newparamdict['yukawa'][pid] = mass

    # Remove empty items from the dictionary for cleanliness
    finalparamdict = { block : newparamdict[block] for block in newparamdict if len(newparamdict[block].keys())>0 }

    mgparlog.info('The parameters that will be updated are:')
    mgparlog.info(finalparamdict)
    
    return finalparamdict


# Functions to get values from a param card
def get_masses(paramDict):
    """ Function to get the masses from a param card
    Takes the location of a param card
    Returns a dictionary of PID key, mass (string) values
    """
    for block in paramDict:
        if 'mass' in block.lower():
            return paramDict[block]
    # Just in case there is no 'mass' block
    raise RuntimeError('Could not find the MASS block in the parameter card dictionary')
    



def get_widths(paramDict):
    """ Function to get the widths from a param card
    Takes the location of a param card
    Returns a dictionary of PID key, width (string) values
    """
    for block in paramDict:
        if block.lower() == 'decay':
            return paramDict[block]
    # Just in case there is no 'decay' block
    raise RuntimeError('Could not find the DECAY block in the parameter card dictionary')


def get_block(paramDict, block_name):
    """ Function to get values from a block in the param card
    Takes the location of a param card and block name
    Returns a dictionary of key--value for that block
    """
    for block in paramDict:
        if block.lower() == block_name.lower():
            return paramDict[block]
    # Just in case there is no block with that name block
    mgparlog.warning('Could not find the %s block in the parameter card dictionary',str(block_name))
    return {}
