#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from glob import glob
import os, shutil
import argparse
import json
import pandas as pd
import re

# ------------------------------------------------------------------------------------------------------
# SET UP
# ------------------------------------------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Compare the rates of different streams')
parser.add_argument('--HLTMenu', type=str, help='HLT Menu File as input')
parser.add_argument('--workdir', type=str, default='stream_analysis', help='Working directory')
parser.add_argument('--targetlumi', type=float, default=2.0e34, help='Target luminosity')
parser.add_argument('--maxEvents', type=int, default=-1, help='Max events')
parser.add_argument('aod_dir', type=str, help='Directory containing AOD files from a rate reprocessing')
parser.add_argument('--l1_ps', type=str, help='L1 PSK JSON file from the Rulebook')
parser.add_argument('--hlt_in', type=str, help='JSON file from the Rulebook with the PSK values')
parser.add_argument('--hlt_ps_stream0', '-psS0', type=str, default='HLTPrescale_stream0.json', 
                                    help='JSON file with the HLT PS for a single stream selected')
parser.add_argument('--hlt_ps_stream', '-psS', type=str, default='HLTPrescale_stream.json', 
                                    help='JSON with the HLT PS for both stream selected')
parser.add_argument('--hlt_ps_chain', '-psC',type=str, default='HLTPrescale_chain.json', 
                                    help='JSON with the HLT PS when the chosen chain is disabled')
parser.add_argument('--hlt_ps_chain_set', '-psCS', type=str, default='HLTPrescale_chainSet.json', 
                                    help='JSON with the HLT PS for the chain set chosen')
parser.add_argument('--hlt_ps_chain_ref', '-psCR', type=str, default='HLTPrescale_chain_ref.json', 
                                    help='JSON with the HLT PS for the reference chain in chain comparison')
parser.add_argument('--hlt_ps_chain_comp', '-psCC', type=str, default='HLTPrescale_chain_comp.json', 
                                    help='JSON with the HLT PS for the compared chain in chain comparison')
parser.add_argument('--stream0', type=str, default='Main', 
                                    help='Reference stream for stream and chain analysis, default is Main')
parser.add_argument('--stream', nargs='+', type=str, default=' ', 
                                    help='List of streams to compare to stream0')
parser.add_argument('--chain', nargs='+', type=str, default=' ', 
                                    help='List of chains to compare to stream0')
parser.add_argument('--chain_set', nargs='+', type=str, default=' ', 
                                    help='Set of chains get the unique rate of')
parser.add_argument('--chain_comp', nargs='+', type=str, default=' ', 
                                    help='List of chains to compare to the first chain in the list')
parser.add_argument('--stream_out', type=str, default='streamRates.txt', 
                                    help='output stream analysis file as a txt file')
parser.add_argument('--chain_out', type=str, default='chainRates.txt', 
                                    help='output chain analysis file as a txt file')
parser.add_argument('--chain_comp_out', type=str, default='chainCompRates.csv', 
                                    help='output chain comparison file as a csv file')
parser.add_argument("--force", "-f", default=False, action="store_true", help="Overwrite existing output directory")


args = parser.parse_args()


# Make working dir if needed
try:
    os.mkdir(args.workdir)
except OSError:
    if args.force:
        shutil.rmtree(args.workdir)
        os.mkdir(args.workdir)
    if not args.force:
        print("\nthe output directory already exists, please clean up or be more creative :)")
        print("or use --force if you want to overwirte the existing output directory.\n")
        exit()
    
    

# Glob input files with abs path
filesIn = glob(f'{os.path.abspath(args.aod_dir)}/*')
print(f"Found {len(filesIn)} input files in '{args.aod_dir}'")

HLTMenu_filen = args.HLTMenu.split('/', 1)[1] if '/' in args.HLTMenu else args.HLTMenu
hlt_in_filen = args.hlt_in.split('/', 1)[1] if '/' in args.hlt_in else args.hlt_in
l1_ps_filen = args.l1_ps.split('/', 1)[1] if '/' in args.l1_ps else args.l1_ps

hlt_ps_stream0_filen = args.hlt_ps_stream0.split('/', 1)[1] if '/' in args.hlt_ps_stream0 else args.hlt_ps_stream0
hlt_ps_stream_filen = args.hlt_ps_stream.split('/', 1)[1] if '/' in args.hlt_ps_stream else args.hlt_ps_stream
hlt_ps_chain_filen = args.hlt_ps_chain.split('/', 1)[1] if '/' in args.hlt_ps_chain else args.hlt_ps_chain
hlt_ps_chain_set_filen = args.hlt_ps_chain_set.split('/', 1)[1] if '/' in args.hlt_ps_chain_set else args.hlt_ps_chain_set
hlt_ps_chain_ref_filen = args.hlt_ps_chain_ref.split('/', 1)[1] if '/' in args.hlt_ps_chain_ref else args.hlt_ps_chain_ref
hlt_ps_chain_comp_filen = args.hlt_ps_chain_comp.split('/', 1)[1] if '/' in args.hlt_ps_chain_comp else args.hlt_ps_chain_comp

stream0_filen = args.stream0.split('/', 1)[1] if '/' in args.stream0 else args.stream0
stream_filen = args.stream.split('/', 1)[1] if '/' in args.stream else args.stream
chain_filen = args.chain.split('/', 1)[1] if '/' in args.chain else args.chain
chain_set_filen = args.chain_set.split('/', 1)[1] if '/' in args.chain_set else args.chain_set
chain_comp_filen = args.chain_comp.split('/', 1)[1] if '/' in args.chain_comp else args.chain_comp

output_stream_filen = args.stream_out.split('/', 1)[1] if '/' in args.stream_out else args.stream_out
output_chain_filen = args.chain_out.split('/', 1)[1] if '/' in args.chain_out else args.chain_out
output_chain_comp_filen = args.chain_comp_out.split('/', 1)[1] if '/' in args.chain_comp_out else args.chain_comp_out

shutil.copy(args.HLTMenu, f"{args.workdir}/{HLTMenu_filen}")
shutil.copy(args.l1_ps, f"{args.workdir}/{l1_ps_filen}")
shutil.copy(args.hlt_in, f"{args.workdir}/{hlt_in_filen}")

origdir = os.getcwd()
os.chdir(args.workdir)

cmd_total = [
    '--inputPrescalesHLTJSON', hlt_in_filen,
    '--outputPrescalesHLTJSON1', hlt_ps_stream0_filen,
    '--outputPrescalesHLTJSON2', hlt_ps_stream_filen,
    '--stream0', stream0_filen,
    '--stream', stream_filen,
    '--chain', chain_filen,
    '--chainSet', chain_set_filen,
    '--chainComp', chain_comp_filen,
    ]

# ------------------------------------------------------------------------------------------------------
# STREAM AND CHAIN SELECTION PART OF THE CODE
# ------------------------------------------------------------------------------------------------------

def stream_json(menu_in, psks_in, stream1, stream2):
    f = open(menu_in)
    menu = json.load(f)

    h = open(psks_in)
    psks = json.load(h)

    dict_stream1 = {}
    dict_stream2 = {}
    for i in menu['chains']: # go through all the chains
        for k in psks['prescales']: # go through all the chains in the psk_in (k)
            if i == k: # make sure you work with the same chains
                for j in menu['chains'][i]['streams']:
                    if j == stream1: 
                        dict_stream1[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                         "enabled": True}
                        dict_stream2[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                         "enabled": True}
                    elif j == stream2:
                        dict_stream2[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                           "enabled": True}
                        dict_stream1[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}
                    else:
                        dict_stream1[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}
                        dict_stream2[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}
    return dict_stream1, dict_stream2

def chain_json(menu_in, psks_in, chain, stream):
    dict_stream = {}
    dict_chain = {}

    f = open(menu_in)
    menu = json.load(f)

    h = open(psks_in)
    psks = json.load(h)

    for i in menu['chains']: # go through all the chains
        for k in psks['prescales']: # go through all the chains in the psk_in (k)
            if i == k: # make sure you work with the same chains
                for j in menu['chains'][i]['streams']: # go through all the streams
                    if j == stream: 
                        dict_stream[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                     "enabled": True}
                        dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                      "enabled": True}
                if k == chain:
                    dict_stream[i] = {"hash": menu['chains'][i]['nameHash'], 
                                         "prescale": psks['prescales'][k]['prescale'], "enabled": True}
                    dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False} # here chain is the one when it is DISABLED
    return dict_stream, dict_chain

def chain_comp_json(menu_in, ref_chain, comp_chain):
    # PSKs are set to 1! no need for input prescales
    f = open(menu_in)
    menu = json.load(f)

    dict_chain = {}

    for i in menu['chains']:
        if i == ref_chain:
            dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": 1, "enabled": True}
        elif i == comp_chain:
            dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": 1, "enabled": True}
        else:
            dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}
    
    return dict_chain

def chain_ref_json(menu_in, chain):
    # PSKs are set to 1! no need for input prescales
    f = open(menu_in)
    menu = json.load(f)

    dict_chain = {}

    for i in menu['chains']:
        if i == chain:
            dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": 1, "enabled": True}
        else:
            dict_chain[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}

    return dict_chain

def psk_file(dict, psk_file): 
    psk_dict = {
        "filetype": "hltprescale",
        "name": "Physics_pp_2.0e+34_2340b",
        "prescales":dict
    }
    
    with open(psk_file, 'w') as file:
        json.dump(psk_dict, file, indent=4)

def chain_set_json(menu_in, psks_in, chains):
    f = open(menu_in)
    menu = json.load(f)

    h = open(psks_in)
    psks = json.load(h)

    dict_chain_set = {}

    for i in menu['chains']: # go through all the chains
        for k in psks['prescales']: # go through all the chains in the psk_in (k)
            if i == k:
                dict_chain_set[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": psks['prescales'][k]['prescale'], 
                                      "enabled": True}
            for j in range(len(chains)):
                if i == chains[j]:
                    dict_chain_set[i] = {"hash": menu['chains'][i]['nameHash'], "prescale": -1, "enabled": False}
    
    return dict_chain_set


# ------------------------------------------------------------------------------------------------------------
# GETTING REGEX INPUTS
# ------------------------------------------------------------------------------------------------------------

def chain_regex(menu_in):

    f = open(menu_in)
    menu = json.load(f)

    while True:
        regex_input = input("RegEx expression: ")
        
        list_of_chains = []
        chains_to_analyse = []
        
        for i in menu['chains']:
            list_of_chains.append(i)
            
        m = 0
        for chain in list_of_chains:
            if re.match(regex_input, chain):
                m += 1
                chains_to_analyse.append(chain)
            else:
                continue
            
        if m != 0:
            print('These are the chains you are trying to compare: ', "\n".join(chains_to_analyse))
            user_conf = input('Are you sure you want to continue? [y/n]')
            if user_conf.lower() in ["yes", "y"]:
                print("sure, let's go")
                return chains_to_analyse
            if user_conf.lower() in ["no", 'n']:
                print("alright, then try again")            
        else:       
            print('Are you sure you are RegEx entering the expression correctly? Try again :)')

# -------------------------------------------------------------------------------------------------------------------
# RATES ANALYSIS
# -------------------------------------------------------------------------------------------------------------------

# Run full rates analysis using RatesAnalysisProcessingWrapper.py
def exec_rates(hlt_ps, dir):

    #filesIn_str = "','".join(filesIn)

    cmd_rates = [
        'RatesAnalysisProcessingWrapper.py',
        '--hlt_ps', hlt_ps, 
        '--l1_ps', l1_ps_filen,
        '--targetlumi', str(args.targetlumi),
        args.aod_dir, 
        '--workdir', dir
    ]

    if args.maxEvents>0:
        cmd_rates += ['--maxEvents',str(args.maxEvents)]
    
    command = ' '.join(cmd_rates)

    status = os.WEXITSTATUS(os.system(command))
    if status != 0:
        exit('fail')

# ------------------------------------------------------------------------------------------------------------
# GETTING THE STREAM AND CHAIN RATE - CSVs
# ------------------------------------------------------------------------------------------------------------

def stream_rate(stream1, stream2, dir):
    
    df_stream0 = pd.read_csv(f"{dir}/output/csv/Table_Rate_Group_HLT_All.csv") 
    df_stream0.set_index('Name', inplace=True)
    rate0 = float(df_stream0.loc['RATE_GLOBAL_HLT']['Weighted PS Rate [Hz]'])
    rate0err = float(df_stream0.loc['RATE_GLOBAL_HLT']['Weighted PS Rate Err [Hz]'])

    df_global = pd.read_csv(f"{stream2}/output/csv/Table_Rate_Group_HLT_All.csv") 
    df_global.set_index('Name', inplace=True)
    rateglobal = float(df_global.loc['RATE_GLOBAL_HLT']['Weighted PS Rate [Hz]'])
    rateglobalerr = float(df_global.loc['RATE_GLOBAL_HLT']['Weighted PS Rate Err [Hz]'])

    df_stream2 = pd.read_csv(f"{stream2}/output/csv/Table_Rate_ChainHLT_HLT_All.csv") 
    df_stream2.set_index('Name', inplace=True)
    rate2 = float(df_global.loc[f"STREAM:{stream2}"]['Weighted PS Rate [Hz]'])
    rate2err = float(df_global.loc[f"STREAM:{stream2}"]['Weighted PS Rate Err [Hz]'])

    unique_rate = rateglobal - rate0

    try:
        percentage = (rate2 - unique_rate)/rate2 * 100
    except ZeroDivisionError:
        print("Cannot compute streams' overlap percentage (ZeroDivisionError): percentage set to None")
        percentage = None
  

    with open(output_stream_filen, 'w') as f:
        print('Computing the unique rate of a stream w.r.t. a different stream', file=f)
        print('Reference Stream: ', stream1, file=f)
        print('Compared Stream: ', stream2, file=f)
        table = [[rate0, rate0err],
                 [rateglobal, rateglobalerr],
                 [rate2, rate2err],
                 [unique_rate, ' ']]
        df_out = pd.DataFrame(table, index = [f'Total rate of {stream1}' ,'Rate of the two streams including overlap', f'Total rate of {stream2}', f"Unique rate of {stream2}"], 
                                    columns = ['Weighted PS Rate [Hz]', "Weighted PS Rate Err [Hz]"])
        print(df_out, file=f)
        print(f"The overlap between {stream1} and {stream2} is {percentage}", file=f)
        
def chain_rate(chain, stream, dir):
    # df where the chain is disabled - get the menu rate when this is alone
    df = pd.read_csv(f"{chain}/output/csv/Table_Rate_Group_HLT_All.csv")
    df.set_index('Name', inplace=True)

    df_chain = pd.read_csv(f"{dir}/output/csv/Table_Rate_ChainHLT_HLT_All.csv")
    df_stream = pd.read_csv(f"{dir}/output/csv/Table_Rate_Group_HLT_All.csv")
    df_chain.set_index('Name', inplace=True)
    df_stream.set_index('Name', inplace=True)
    
    chain_rate = float(df_chain.loc[chain]['Unique Rate [Hz]'])
    chain_rate_err = float(df_chain.loc[chain]['Unique Rate Err [Hz]'])

    global_rate = float(df_stream.loc[f"STREAM:{stream}"]['Weighted PS Rate [Hz]'])
    global_rate_err = float(df_stream.loc[f"STREAM:{stream}"]['Weighted PS Rate Err [Hz]'])

    global_dis_rate = float(df.loc['RATE_GLOBAL_HLT']['Weighted PS Rate [Hz]']) # chain is disabled
    global_dis_rate_err = float(df.loc['RATE_GLOBAL_HLT']['Weighted PS Rate Err [Hz]']) # chain is disabled

    unique_contri = global_rate - global_dis_rate
    percentage = unique_contri/global_rate * 100

    with open(output_chain_filen, 'w') as f:
        print('Computing the unique rate of a chain w.r.t. a stream', file=f)
        print('Reference Stream: ', stream, file=f)
        print('Compared Chain: ', chain, file=f)
        table = [[chain_rate, chain_rate_err],
                 [global_rate, global_rate_err],
                 [global_dis_rate, global_dis_rate_err],
                 [unique_contri, '---']]
        df_out = pd.DataFrame(table, index = ['Chain Unique Rate', 'Stream and Chain Rate',
                                                'Stream without Chain Rate', 'Unique Contribution'],
                                    columns = ['Rate [Hz]', 'Rate Error [Hz]'])
        print(df_out, file=f)
        print(f"{chain} has a unique contribution of", percentage, '% to ' f"{stream}", file=f)

def chain_comp_rate(chain1, chain2):
    
    df1 = pd.read_csv(f"{chain1}/output/csv/Table_Rate_ChainHLT_HLT_All.csv")
    df1.set_index('Name', inplace=True)
    
    df2 = pd.read_csv(f"{chain2}/output/csv/Table_Rate_ChainHLT_HLT_All.csv")
    df2.set_index('Name', inplace=True)

    # rate where only one chain is enabled
    wrate1 = float(df1.loc[chain1]['Weighted PS Rate [Hz]'])
    urate1 = float(df1.loc[chain1]['Unique Rate [Hz]'])
    wrate1err = float(df1.loc[chain1]['Weighted PS Rate Err [Hz]'])
    urate1err = float(df1.loc[chain1]['Unique Rate Err [Hz]'])

    # rate when both chains are enabled
    wrate1_comb = float(df2.loc[chain1]['Weighted PS Rate [Hz]'])
    urate1_comb = float(df2.loc[chain1]['Unique Rate [Hz]'])
    wrate2 = float(df2.loc[chain2]['Weighted PS Rate [Hz]'])
    urate2 = float(df2.loc[chain2]['Unique Rate [Hz]'])
    wrate1_comb_err = float(df2.loc[chain1]['Weighted PS Rate Err [Hz]'])
    urate1_comb_err = float(df2.loc[chain1]['Unique Rate Err [Hz]'])
    wrate2err = float(df2.loc[chain2]['Weighted PS Rate Err [Hz]'])
    urate2err = float(df2.loc[chain2]['Unique Rate Err [Hz]'])

    try:
        percentage = wrate2/wrate1_comb * 100
    except ZeroDivisionError:
        print("Cannot compute chains' overlap percentage (ZeroDivisionError): percentage set to None")
        percentage = None

    with open(f"ChainComp_{i}", 'w') as f:
        table = [[wrate1, wrate1err, urate1, urate1err],
                 [wrate1_comb, wrate1_comb_err, urate1_comb, urate1_comb_err],
                 [wrate2, wrate2err, urate2, urate2err]]
        pd.set_option('display.max_colwidth', None)
        df_out = pd.DataFrame(table, columns = ['Weighted PS Rate', 'Weighted PS Rate Error', 'Unique Rate', 'Unique Rate Error'],
                        index = ['Reference Chain Only', 'Reference Chain', 'Comparison Chain'])
        print(df_out, file=f)
        print('The overlap between ' f"{chain1} and " f"{chain2} is", percentage, '%', file=f)

def chain_set_rate(chains):
    df1 = pd.read_csv("fullMenu/output/csv/Table_Rate_Group_HLT_All.csv")
    df1.set_index('Name', inplace=True)
    
    df2 = pd.read_csv("setMenu/output/csv/Table_Rate_Group_HLT_All.csv")
    df2.set_index('Name', inplace=True)

    # Global HLT Rate 
    fullrate = float(df1.loc['RATE_GLOBAL_HLT']['Weighted PS Rate [Hz]'])
    setrate = float(df2.loc['RATE_GLOBAL_HLT']['Weighted PS Rate [Hz]'])
    fullrate_err = float(df1.loc['RATE_GLOBAL_HLT']['Weighted PS Rate Err [Hz]'])
    setrate_err = float(df2.loc['RATE_GLOBAL_HLT']['Weighted PS Rate Err [Hz]'])

    unique_rate = fullrate - setrate
    percentage = unique_rate/setrate * 100

    with open("SetMonitoring.txt", 'w') as f:
        table = [[fullrate, fullrate_err],
                 [setrate, setrate_err]]
        pd.set_option('display.max_colwidth', None)
        df_out = pd.DataFrame(table, columns = ['Global HLT Rate', 'Global HLT Rate Error'],
                        index = ['Full Menu', 'Chains disabled'])
        print("The chains that have been disabled are: ", file=f)
        for i in range(len(chains)):
            print(f"{chains[i]}", file=f)
        print("-------------------------------------------------------", file=f)
        print(df_out, file=f)
        print("-------------------------------------------------------", file=f)
        print('Unique rate of the set: ', unique_rate, file=f)
        print('The set has a contribution of', percentage, '% to the Global HLT Rate', file=f)

def pretty_csvs(chains_list):
    
    # csv part goes here now
    with open('ChainAnalysisSummary.txt', 'w') as t:
        
        pd.set_option('max_colwidth', 400)
        
        print('The reference chain is ',chains_list[0] , file=t)
        print('--------------------------------------------------------------------------------', file=t)
        print('--------------------------------------------------------------------------------', file=t)
        
        for p in range(1, len(chains_list)):
            
            print('For chain', chains_list[p], ':' , file=t)
            df = pd.read_csv(f"ChainComp_{p}")
            print(df.drop([0]), file=t)
            print('--------------------------------------------------------------------------------', file=t)
        
# ------------------------------------------------------------------------------------------------------------
# CALLING THE FUNCTIONS
# ------------------------------------------------------------------------------------------------------------

if stream_filen != ' ':

    s = 0
    streamdir = os.getcwd()

    for j in range(len(stream_filen)):
        s += 1
        os.makedirs(f"stream_{s}")

        shutil.copy(args.HLTMenu, f"stream_{s}/{HLTMenu_filen}")
        shutil.copy(args.l1_ps, f"stream_{s}/{l1_ps_filen}")
        shutil.copy(args.hlt_in, f"stream_{s}/{hlt_in_filen}")

        os.chdir(f"stream_{s}")

        #choosing the streams
        dict_stream0 = stream_json(HLTMenu_filen, hlt_in_filen, stream0_filen, stream_filen[j])[0]
        dict_stream = stream_json(HLTMenu_filen, hlt_in_filen, stream0_filen, stream_filen[j])[1]
        # dumping the json
        psk_file(dict_stream0, hlt_ps_stream0_filen)
        psk_file(dict_stream, hlt_ps_stream_filen)
        # rates analysis
        exec_rates(hlt_ps_stream0_filen, f"{stream0_filen}")
        exec_rates(hlt_ps_stream_filen, stream_filen[j])
        #csv part
        stream_rate(stream0_filen, stream_filen[j], f"{stream0_filen}")

        os.chdir(streamdir)

if chain_filen == ['regex']:
    s = 0
    chaindir = os.getcwd()
    print(chaindir)

    regex_chains = chain_regex(HLTMenu_filen)

    for i in range(len(regex_chains)):
        s += 1 

        os.makedirs(f"chain_{s}")

        shutil.copy(args.HLTMenu, f"chain_{s}/{HLTMenu_filen}")
        shutil.copy(args.l1_ps, f"chain_{s}/{l1_ps_filen}")
        shutil.copy(args.hlt_in, f"chain_{s}/{hlt_in_filen}")

        os.chdir(f"chain_{s}")
        
        #choosing the chains
        dict_stream0 = chain_json(HLTMenu_filen, hlt_in_filen, regex_chains[i], stream0_filen)[0]
        dict_chain = chain_json(HLTMenu_filen, hlt_in_filen, regex_chains[i], stream0_filen)[1]
    
        # dumping the json
        psk_file(dict_stream0, hlt_ps_stream0_filen)
        psk_file(dict_chain, hlt_ps_chain_filen)
        # rates analysis
        exec_rates(hlt_ps_stream0_filen, f"{stream0_filen}")
        exec_rates(hlt_ps_chain_filen, regex_chains[i])
        # csv part
        chain_rate(regex_chains[i], stream0_filen, f"{stream0_filen}")

        # cleanup to remove the duplicate files
        os.remove(HLTMenu_filen)
        os.remove(l1_ps_filen)
        os.remove(hlt_in_filen)
        os.remove(hlt_ps_chain_filen)
        os.remove(hlt_ps_stream0_filen)

        os.chdir(chaindir)

elif chain_filen != ' ':
    c = 0
    chaindir = os.getcwd()

    for i in range(len(chain_filen)):
        c += 1 

        os.makedirs(f"chain_{c}")

        shutil.copy(args.HLTMenu, f"chain_{c}/{HLTMenu_filen}")
        shutil.copy(args.l1_ps, f"chain_{c}/{l1_ps_filen}")
        shutil.copy(args.hlt_in, f"chain_{c}/{hlt_in_filen}")

        os.chdir(f"chain_{c}")
     
        #choosing the chains
        dict_stream0 = chain_json(HLTMenu_filen, hlt_in_filen, chain_filen[i], stream0_filen)[0]
        dict_chain = chain_json(HLTMenu_filen, hlt_in_filen, chain_filen[i], stream0_filen)[1]
    
        # dumping the json
        psk_file(dict_stream0, hlt_ps_stream0_filen)
        psk_file(dict_chain, hlt_ps_chain_filen)
        # rates analysis
        exec_rates(hlt_ps_stream0_filen, f"{stream0_filen}")
        exec_rates(hlt_ps_chain_filen, chain_filen[i])

        # csv part
        chain_rate(chain_filen[i], stream0_filen, f"{stream0_filen}")

        # cleanup to remove the duplicate files
        os.remove(HLTMenu_filen)
        os.remove(l1_ps_filen)
        os.remove(hlt_in_filen)
        os.remove(hlt_ps_chain_filen)
        os.remove(hlt_ps_stream0_filen)

        os.chdir(chaindir)

if chain_comp_filen == ['regex']:
    
    regex_chainsComp = chain_regex(HLTMenu_filen)
    os.makedirs('chain_comp')
    
    shutil.copy(args.HLTMenu, f"chain_comp/{HLTMenu_filen}")
    shutil.copy(args.l1_ps, f"chain_comp/{l1_ps_filen}")
    shutil.copy(args.hlt_in, f"chain_comp/{hlt_in_filen}")
    
    os.chdir('chain_comp')
    chain_comp_dir = os.getcwd()

    # executing the commands 
    dict_chain_ref = chain_ref_json(HLTMenu_filen, regex_chainsComp[0])
    psk_file(dict_chain_ref, hlt_ps_chain_ref_filen)
    exec_rates(hlt_ps_chain_ref_filen, regex_chainsComp[0])
    os.remove(hlt_ps_chain_ref_filen)

    for i in range(1, len(regex_chainsComp)):
        
        dict_chain_comp = chain_comp_json(HLTMenu_filen, regex_chainsComp[0], regex_chainsComp[i])
        psk_file(dict_chain_comp, hlt_ps_chain_comp_filen)
        exec_rates(hlt_ps_chain_comp_filen, regex_chainsComp[i])
        chain_comp_rate(regex_chainsComp[0], regex_chainsComp[i])

        os.chdir(chain_comp_dir)

    # cleanup to remove the duplicate files
    os.remove(HLTMenu_filen)
    os.remove(l1_ps_filen)
    os.remove(hlt_in_filen)
    os.remove(hlt_ps_chain_comp_filen)
    
    # creating the csv files
    pretty_csvs(regex_chainsComp)

elif chain_comp_filen != ' ':
    
    os.makedirs('chain_comp')
    
    shutil.copy(args.HLTMenu, f"chain_comp/{HLTMenu_filen}")
    shutil.copy(args.l1_ps, f"chain_comp/{l1_ps_filen}")
    shutil.copy(args.hlt_in, f"chain_comp/{hlt_in_filen}")
    os.chdir('chain_comp')
    chain_comp_dir = os.getcwd()

    # executing the commands
    dict_chain_ref = chain_ref_json(HLTMenu_filen, chain_comp_filen[0])
    psk_file(dict_chain_ref, hlt_ps_chain_ref_filen)
    exec_rates(hlt_ps_chain_ref_filen, chain_comp_filen[0])
    os.remove(hlt_ps_chain_ref_filen)

    for i in range(1, len(chain_comp_filen)):
        dict_chain_comp = chain_comp_json(HLTMenu_filen, chain_comp_filen[0], chain_comp_filen[i])
        psk_file(dict_chain_comp, hlt_ps_chain_comp_filen)
        exec_rates(hlt_ps_chain_comp_filen, chain_comp_filen[i])
        chain_comp_rate(chain_comp_filen[0], chain_comp_filen[i])

        os.chdir(chain_comp_dir)
    
    # cleanup to remove the duplicate files
    os.remove(HLTMenu_filen)
    os.remove(l1_ps_filen)
    os.remove(hlt_in_filen)
    os.remove(hlt_ps_chain_comp_filen)
    
    # creating the csv files
    pretty_csvs(chain_comp_filen)

if chain_set_filen == ['regex']:

    regex_chainsSet = chain_regex(HLTMenu_filen)

    os.makedirs('chain_set')
    
    shutil.copy(args.HLTMenu, f"chain_set/{HLTMenu_filen}")
    shutil.copy(args.l1_ps, f"chain_set/{l1_ps_filen}")
    shutil.copy(args.hlt_in, f"chain_set/{hlt_in_filen}")
    os.chdir('chain_set')
    chain_set_dir = os.getcwd()

    #rates analysis on the full menu
    exec_rates(hlt_in_filen, 'fullMenu')
    dict_chain_set = chain_set_json(HLTMenu_filen, hlt_in_filen, regex_chainsSet)
    psk_file(dict_chain_set, hlt_ps_chain_set_filen)
    exec_rates(hlt_ps_chain_set_filen, 'setMenu')
    chain_set_rate(regex_chainsSet)

elif chain_set_filen != ' ':

    os.makedirs('chain_set')
    
    shutil.copy(args.HLTMenu, f"chain_set/{HLTMenu_filen}")
    shutil.copy(args.l1_ps, f"chain_set/{l1_ps_filen}")
    shutil.copy(args.hlt_in, f"chain_set/{hlt_in_filen}")
    os.chdir('chain_set')
    chain_set_dir = os.getcwd()

    # rates analysis on the full menu
    exec_rates(hlt_in_filen, 'fullMenu')
    dict_chain_set = chain_set_json(HLTMenu_filen, hlt_in_filen, chain_set_filen)
    psk_file(dict_chain_set, hlt_ps_chain_set_filen)
    exec_rates(hlt_ps_chain_set_filen, 'setMenu')
    chain_set_rate(chain_set_filen)
