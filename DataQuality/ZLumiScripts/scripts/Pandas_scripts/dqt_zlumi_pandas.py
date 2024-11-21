#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import numpy as np
import csv
from array import array
import ROOT
import sys, os
import logging
import argparse
import math
from DQUtils import fetch_iovs
from DQUtils.iov_arrangement import inverse_lblb
import ZLumiScripts.tools.zlumi_alleff as dq_eff
import ZLumiScripts.tools.zlumi_mc_cf as dq_cf


from DataQualityUtils import doZLumi

# testing toy sampling
import ZLumiScripts.tools.toys as toys

ROOT.gROOT.SetBatch(ROOT.kTRUE)
ROOT.gStyle.SetOptStat(0)

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument('--infile', type=str, help='input HIST file')
parser.add_argument('--grl', type=str, help='Specify an input GRL')
parser.add_argument('--tag', type=str, help='Lumi tag', default='OflLumiAcct-Run3-003')
parser.add_argument('--useofficial', action='store_true', help='Use official lumi folder (otherwise, use OflLumiAcct')
parser.add_argument('--lumifolder', type=str, help='Lumi folder', default='/TRIGGER/OFLLUMI/OflPrefLumi')
parser.add_argument('--lumitag', type=str, help='Lumi tag', default='OflLumi-Run3-003')
parser.add_argument('--outdir', type=str, help='Directory to dump plots', default='plots')
parser.add_argument('--dblivetime', action='store_true', help='Look up livetime from DB')
parser.add_argument('--campaign', type=str, help='mc16a/d/e, mc21, mc23a')

args     = parser.parse_args()
campaign = args.campaign

ZPURITYFACTOR = 0.9935
if args.campaign in ["mc21", "mc23a"]:
    ZXSEC = 2.0675
else:
    ZXSEC = 1.929

zee_missing_lbs = []
zmumu_missing_lbs = []

ntoys = 10000000
do_toys = False

fin = ROOT.TFile.Open(args.infile)
runname = None
for key in fin.GetListOfKeys():
    if key.GetName().startswith('run_'):
        runname = key.GetName()
        runnumber = int(runname.replace('run_',''))
        break

if not runname:
    logging.critical("Can't find run_* directory in input file %s", args.infile)
    sys.exit(1)

print("Starting HIST to CSV conversion for Run ", runnumber)

if args.outdir: 
    out_dir = args.outdir
    os.system("mkdir -p " + out_dir)
    out_dir += "/" + runname + ".csv"
else: 
    out_dir = runname + ".csv"

if args.campaign in ["mc21", "mc23a"]:
    lb_length_name = '%s/GLOBAL/DQTGlobalWZFinder/duration_vs_LB' % runname
    livetime_name = '%s/GLOBAL/DQTGlobalWZFinder/avgLiveFrac_vs_LB' % runname
else:
    lb_length_name = '%s/GLOBAL/DQTGlobalWZFinder/m_lblength_lb' % runname
    livetime_name = '%s/GLOBAL/DQTGlobalWZFinder/m_livetime_lb' % runname


if args.grl:
    grlReader = ROOT.Root.TGoodRunsListReader(args.grl)
    grlReader.Interpret()
    grl = grlReader.GetMergedGRLCollection()
else:
    grlname = 'grl_'+str(runnumber)+'.xml'
    grl_file = doZLumi.makeGRL(runnumber, 'PHYS_StandardGRL_All_Good', grlname)

lb_length_old = fin.Get(lb_length_name)
lbmin, lbmax = lb_length_old.FindFirstBinAbove(0, 1, 0, -1), lb_length_old.FindLastBinAbove(0, 1, 0, -1)
lb_length = ROOT.TProfile('lb_length', 'LB length', int(lbmax-lbmin), lbmin, lbmax)

for i in range(lbmin, lbmax):
    lb_length.Fill(i, lb_length_old.GetBinContent(i))

logging.info('low, high LBs: %s, %s', lbmin, lbmax)

if args.dblivetime:
    logging.info('Starting livetime lookup ... (remove when we have a proper in-file implementation ...)')
    livetime = ROOT.TProfile('livetime', 'Livetime', int(lbmax-lbmin), lbmin, lbmax)
else:
    livetime = fin.Get(livetime_name)

official_lum_zero = ROOT.TProfile('official_lum_zero', 'official inst luminosity', int(lbmax-lbmin), lbmin, lbmax)
official_mu = ROOT.TProfile('official_mu', 'official mu', int(lbmax-lbmin), lbmin, lbmax)

lblb = fetch_iovs("LBLB", runs=runnumber)
lbtime = inverse_lblb(lblb)
iovs_acct = fetch_iovs('COOLOFL_TRIGGER::/TRIGGER/OFLLUMI/LumiAccounting', lbtime.first.since, lbtime.last.until, tag=args.tag)

if args.useofficial:
    print("Using official lumitag", args.lumitag)
    iovs_lum = fetch_iovs('COOLOFL_TRIGGER::%s' % args.lumifolder, lblb.first.since, lblb.last.until, tag=args.lumitag, channels=[0])

lb_start_end = {}
lb_lhcfill = {}
for iov in lblb:
    lb_start_end[iov.since & 0xffffffff] = (iov.StartTime/1e9, iov.EndTime/1e9)

for iov in iovs_acct:
    if not lbmin-1 < iov.LumiBlock < lbmax:
        continue
    lb_lhcfill[iov.LumiBlock] = iov.FillNumber
    if args.dblivetime:
        livetime.Fill(iov.LumiBlock, iov.LiveFraction)

    if not args.useofficial:
        official_lum_zero.Fill(iov.LumiBlock, iov.InstLumi/1e3)
        official_mu.Fill(iov.LumiBlock, iov.AvEvtsPerBX)
    else:
        offlumiov = [_ for _ in iovs_lum if _.since.lumi==iov.LumiBlock]
        if len(offlumiov) != 1: 
            print('MAJOR PROBLEM, LUMI IOV MISMATCH', iov.LumiBlock)
            continue

        offlumiov = offlumiov[0]
        official_lum_zero.Fill(iov.LumiBlock, offlumiov.LBAvInstLumi/1e3)
        official_mu.Fill(iov.LumiBlock, offlumiov.LBAvEvtsPerBX)

lb_full = lb_length.Clone('lb_full').ProjectionX()
divisor = lb_length.Clone('divisor').ProjectionX()
px = livetime.ProjectionX()
divisor.Multiply(px)

# Get run-wise electron channel histos outside of loop
# Also do the container efficiency bkg fit here as it is done
# once per run, then normalised to the ratio of LB/run in the function
# container_efficiency
hto = fin.Get('%s/GLOBAL/DQTGlobalWZFinder/m_ele_template_os' % (runname))
hts = fin.Get('%s/GLOBAL/DQTGlobalWZFinder/m_ele_template_ss' % (runname))

# ==== Set signal region == 0, then fit ====
# hphotontotal = fin.Get("%s/GLOBAL/DQTGlobalWZFinder/m_elContainertp_nomatch" % (runname))
# hphotontotal.GetXaxis().SetRangeUser(66000, 250000)
# h_fit = hphotontotal.Clone()
# for xbin in range(1, h_fit.GetNbinsX()+1): 
#     mass = h_fit.GetBinLowEdge(xbin)
#     if mass > 75000 and mass < 100000: 
#         h_fit.SetBinContent(xbin, 0)
#         h_fit.SetBinError(xbin, 0)
# h_fit.Fit("pol2", "q")


o_recoeff_fit = {}
o_recoerr_fit = {}
# Do electron channel reco eff calculation once, 
# then apply iterative correction to bkg subtraction in the main loop
lb_minus_one_reco_eff = [1.0, 1.0, 1.0]
for ibin in range(1, int(lbmax-lbmin)+1):
    this_lb = int(lb_full.GetBinCenter(ibin))
    lb = "lb_" + str(this_lb)
    pileup = round(official_mu[ibin])

    hmo = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_good_os' % (runname, lb))
    hms = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_good_ss' % (runname, lb))
    hno = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_bad_os' % (runname, lb))
    hns = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_bad_ss' % (runname, lb))

    try: 
        eff, err = dq_eff.template_method(hmo, hms, hno, hns, hto, hts, False, lb_minus_one_reco_eff) # do_scale set to false here as we are calculating it, when applying set True (next loop)
        lb_minus_one_reco_eff = [eff, err, this_lb]
    except AttributeError: 
        eff = 0
        err = 0
    
    if err != 0 and eff != 0: 
        weight = 1/pow(err, 2)
        if pileup not in o_recoeff_fit: 
            o_recoeff_fit[pileup] = weight * eff
            o_recoerr_fit[pileup] = weight
        else:
            o_recoeff_fit[pileup] += weight * eff
            o_recoerr_fit[pileup] += weight

arr_rec_eff = array('d')
arr_rec_err = array('d')
arr_mu      = array('d')

for pileup in o_recoeff_fit: 
    arr_mu.append(float(pileup))
    arr_rec_eff.append(o_recoeff_fit[pileup]/o_recoerr_fit[pileup])
    arr_rec_err.append(1/pow(o_recoerr_fit[pileup], 0.5))

# If no pileup data is available, add dummy entry with all parameters set to 1

if len(arr_mu) == 0:
    arr_mu.append(1)
    arr_rec_eff.append(1)
    arr_rec_err.append(1)

tg_fit = ROOT.TGraphErrors(len(arr_mu), arr_mu, arr_rec_eff, ROOT.nullptr, arr_rec_err)
if len(o_recoeff_fit) == 0 or len(o_recoeff_fit) == 1:
    fit_type = "pol0"
elif len(o_recoeff_fit) == 2:
    fit_type = "pol1"
elif len(o_recoeff_fit) > 2:
    fit_type = "pol2"

tg_fit.Fit(fit_type, "q")

csvfile = open(out_dir, 'w')
csvwriter = csv.writer(csvfile, delimiter=',')
csvwriter.writerow(['FillNum','RunNum','LBNum','LBStart','LBEnd','LBLive','LBFull','OffLumi','OffMu', 'PassGRL', 
                    'ZeeRaw','ZeeRawErr','ZeeN1','ZeeN2','ZeeEffTrig','ZeeErrTrig','ZeeEffReco','ZeeErrReco','ZeeEffComb','ZeeErrComb','ZeeEffAComb','ZeeErrAComb','ZeeDefTrig','ZeeDefReco','ZeeLumi','ZeeLumiErr','ZeeRate',
                    'ZmumuRaw','ZmumuRawErr','ZmumuN1','ZmumuN2','ZmumuEffTrig','ZmumuErrTrig','ZmumuEffReco','ZmumuErrReco','ZmumuEffComb','ZmumuErrComb','ZmumuEffAComb','ZmumuErrAComb','ZmumuDefTrig','ZmumuDefReco','ZmumuLumi','ZmumuLumiErr','ZmumuRate',
                    'ZllLumi','ZllLumiErr'])

lb_minus_one_reco_eff = {}
lb_minus_one_reco_eff["Zee"]   = [1.0, 1.0, 1]
lb_minus_one_reco_eff["Zmumu"] = [1.0, 1.0, 1]

lb_minus_one_trig_eff = {}
lb_minus_one_trig_eff["Zee"]   = [1.0, 1.0, 1]
lb_minus_one_trig_eff["Zmumu"] = [1.0, 1.0, 1]

bad_database = False

for ibin in range(1, int(lbmax-lbmin)+1):
    out_dict = {}
    out_dict["Zee"] = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    out_dict["Zmumu"] = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    
    this_lb = int(lb_full.GetBinCenter(ibin))
    loclivetime = divisor[ibin]
    # some ad-hoc Run 2 corrections
    if runnumber == 302831 and this_lb < 11:
        loclivetime = 0
    elif runnumber == 329835 and this_lb < 554:
        loclivetime = 0
    elif runnumber == 310247 and (this_lb == 442 or this_lb == 462):
        loclivetime = 0
    elif runnumber == 281385 and this_lb < 197:
        loclivetime *= 4.0/6.0
    elif runnumber == 281385 and this_lb < 375:
        loclivetime *= 5.0/6.0
    elif runnumber == 286367:
        loclivetime *= 5.0/6.0
    
    try:
        this_fill = lb_lhcfill[this_lb]
    except KeyError: 
        bad_database = True
        this_fill = "NA"

    passgrl = 1
    for channel in ["Zee", "Zmumu"]:
        if grl and not grl.HasRunLumiBlock(runnumber, this_lb):
            passgrl = 0
            continue
        # at less than 9s LB livetime: all Z counting zeroed
        if loclivetime < 9: 
            continue

        lb = "lb_" + str(this_lb)
        if channel == 'Zee':
            # Nominal signal histogram
            hname = runname + '/lb_'+str(int(ibin+lbmin-0.5))+'/GLOBAL/DQTGlobalWZFinder/m_Z_mass_opsele'

            # Tag-and-probe histos
            h = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_eltrigtp_matches_os' % (runname, lb)) 
            hmo = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_good_os' % (runname, lb))
            hms = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_good_ss' % (runname, lb))
            hno = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_bad_os' % (runname, lb))
            hns = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_bad_ss' % (runname, lb))
            #hphoton = fin.Get("%s/%s/GLOBAL/DQTGlobalWZFinder/m_elContainertp_nomatch" % (runname, lb))
            #hpass   = fin.Get("%s/%s/GLOBAL/DQTGlobalWZFinder/m_ele_tight_passkine" % (runname, lb))
            #hphoton.GetXaxis().SetRangeUser(66000, 250000)
            #hpass.GetXaxis().SetRangeUser(66000, 250000)
            if args.campaign in ["mc21", "mc23a"]:
                ACCEPTANCE = 0.2971
            else:
                ACCEPTANCE = 0.2996
        elif channel == 'Zmumu':
            # Nominal signal histogram
            hname = runname + '/lb_'+str(int(ibin+lbmin-0.5))+'/GLOBAL/DQTGlobalWZFinder/m_Z_mass_opsmu'
            
            # Tag-and-probe histos
            h = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_mutrigtp_matches' % (runname, lb))
            hmo = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_match_os' % (runname, lb))
            hms = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_match_ss' % (runname, lb))
            hno = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_nomatch_os' % (runname, lb))
            hns = fin.Get('%s/%s/GLOBAL/DQTGlobalWZFinder/m_muloosetp_nomatch_ss' % (runname, lb))
            if args.campaign in ["mc21", "mc23a"]:
                ACCEPTANCE = 0.3292
            else:
                ACCEPTANCE = 0.3323224

        try:
            z_m    = fin.Get(hname).Integral()
            z_merr = math.sqrt(z_m)
        except AttributeError:
            if channel == "Zee":
                zee_missing_lbs.append(this_lb)
            elif channel == "Zmumu":
                zmumu_missing_lbs.append(this_lb)

            continue

        
        N1 = N2 = 0
        try: 
            N1 = h.GetBinContent(2)
            N2 = h.GetBinContent(3)
            if do_toys:
                eff_trig, err_trig, arr_trig, arr_NZ = toys.toy_trigeff(N1, N2, ntoys)
            else:
                eff_trig, err_trig = dq_eff.trig_tag_and_probe(h, lb_minus_one_trig_eff[channel])
        except TypeError:
            eff_trig, err_trig = 0.0, 0.0
        except  AttributeError:
            eff_trig, err_trig = 0.0, 0.0
        if channel == "Zmumu":
            try: 
                bin1 = hmo.GetXaxis().FindBin(86000)
                bin2 = hmo.GetXaxis().FindBin(95000)
                if do_toys: 
                    eff_reco, err_reco, arr_reco = toys.muon_toy_recoeff(hmo.Integral(bin1, bin2), hms.Integral(bin1, bin2), hno.Integral(bin1, bin2), hns.Integral(bin1, bin2), ntoys)
                else: 
                    eff_reco, err_reco = dq_eff.reco_tag_and_probe(hmo, hms, hno, hns, lb_minus_one_reco_eff[channel])
            except TypeError:
                eff_reco, err_reco = 0.0, 0.0
            except  AttributeError:
                eff_reco, err_reco = 0.0, 0.0
        elif channel == "Zee": 
            try:
                if do_toys: 
                    bin1 = hmo.GetXaxis().FindBin(75000)
                    bin2 = hmo.GetXaxis().FindBin(104000)
                    bin3 = hmo.GetXaxis().FindBin(120000)
                    bin4 = hmo.GetXaxis().FindBin(250000)
                    matchos_peak = hmo.Integral(bin1, bin2)
                    matchos_tail = hmo.Integral(bin3, bin4)
                    matchss_tail = hms.Integral(bin3, bin4)
                    nomatchos_peak = hno.Integral(bin1, bin2)
                    nomatchos_tail = hno.Integral(bin3, bin4)
                    templateos_peak = hto.Integral(bin1, bin2)
                    templateos_tail = hto.Integral(bin3, bin4)
                    templatess_tail = hts.Integral(bin3, bin4)
                    eff_reco, err_reco, arr_reco = toys.electron_toy_recoeff(matchos_peak, matchos_tail, matchss_tail, nomatchos_peak, nomatchos_tail, templateos_peak, templateos_tail, templatess_tail, ntoys)
                else: 
                    eff_reco, err_reco = dq_eff.template_method(hmo, hms, hno, hns, hto, hts, True, lb_minus_one_reco_eff[channel], tg_fit.GetFunction(fit_type).Eval(official_mu[ibin]))
              
            except TypeError:
                eff_reco, err_reco = 0.0, 0.0
            except  AttributeError:
                eff_reco, err_reco = 0.0, 0.0

            #dq_eff.container_efficiency(hphoton, hphotontotal, h_fit, hpass, hto, hts)
       

        defaulted_reco_eff = 0
        defaulted_trig_eff = 0
        if eff_reco == lb_minus_one_reco_eff[channel][0]:
            defaulted_reco_eff = 1
        if eff_trig == lb_minus_one_trig_eff[channel][0]:
            defaulted_trig_eff = 1

        if eff_reco != 0.0:
            lb_minus_one_reco_eff[channel] = [eff_reco, err_reco, this_lb]
        if eff_trig != 0.0: 
            lb_minus_one_trig_eff[channel] = [eff_trig, err_trig, this_lb]
      
        if do_toys:  
            arr_comb   = (1.0 - (1.0 - arr_trig)**2) * (arr_reco)**2
            nonan_arr_comb = arr_comb[~np.isnan(arr_comb)]
            eff_comb   = np.median(nonan_arr_comb)
            err_comb   = nonan_arr_comb.std()
        else:
            eff_comb  = (1-(1-eff_trig)**2)*(eff_reco)**2
            err_comb  = ((eff_reco**2*2*(1-eff_trig)*err_trig)**2+(2*eff_reco*(1-(1-eff_trig)**2)*err_reco)**2)**.5
        
        eff_Acomb  = ACCEPTANCE * eff_comb
        err_Acomb  = ACCEPTANCE * err_comb

        if do_toys: 
            effcy     = arr_comb * dq_cf.correction(official_mu[ibin], channel, campaign, runnumber)
        else:
            effcy     = eff_comb * dq_cf.correction(official_mu[ibin], channel, campaign, runnumber)
            effcyerr  = err_comb * dq_cf.correction(official_mu[ibin], channel, campaign, runnumber)
        
        zlumi = zlumistat = zrate = 0.0
        CORRECTIONS = ZPURITYFACTOR/ACCEPTANCE/ZXSEC

        if do_toys and loclivetime != 0.0:
                arr_zlumi = np.divide(arr_NZ, effcy) * (CORRECTIONS)/loclivetime
                arr_zlumi = arr_zlumi[~np.isnan(arr_zlumi)]
                zlumi     = np.median(arr_zlumi)
                zlumistat = arr_zlumi.std()
                zrate     = zlumi / CORRECTIONS
        elif (loclivetime != 0.0 and effcy != 0.0):
                zlumi     = (z_m/effcy)*(CORRECTIONS)/loclivetime
                zlumistat = math.sqrt(pow(z_merr/effcy, 2) + pow(z_m/effcy**2*effcyerr, 2))*CORRECTIONS/loclivetime
                zrate     = zlumi / CORRECTIONS


        out_dict[channel] = [z_m, z_merr, N1, N2, eff_trig, err_trig, eff_reco, err_reco, eff_comb, err_comb, eff_Acomb, err_Acomb, defaulted_trig_eff, defaulted_reco_eff, zlumi, zlumistat, zrate]    

    lumi_index = len(out_dict['Zee'])-3
    error_index = len(out_dict['Zee'])-2
    zll_lumi = (out_dict['Zee'][lumi_index] + out_dict['Zmumu'][lumi_index])/2
    zll_lumi_err = 0.5 * math.sqrt( pow(out_dict['Zee'][error_index], 2) + pow(out_dict['Zmumu'][error_index], 2) )
    try:
        lb_start, lb_end = lb_start_end[this_lb][0], lb_start_end[this_lb][1]
    except KeyError:
        lb_start, lb_end = 0, 0
        print("WARNING: database not filled completely for LB", this_lb)
    out_write = [this_fill, runnumber, this_lb, lb_start, lb_end, loclivetime, lb_full[ibin], official_lum_zero[ibin], official_mu[ibin], passgrl] + out_dict["Zee"] + out_dict["Zmumu"] + [zll_lumi, zll_lumi_err]
    csvwriter.writerow(out_write)

if (len(zee_missing_lbs) > 0 or len(zmumu_missing_lbs) > 0):
    print("Missing LBs in Zee channel: ", zee_missing_lbs)
    print("Missing LBs in Zmumu channel: ", zmumu_missing_lbs)

if bad_database:
    print("WARNING: There was an error retrieving information from the lumi database, likely need to update the tags.")
    
csvfile.close()
