#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Authors: Calla Hinderks, Federica Piazza

import ROOT
import math

ROOT.gROOT.SetStyle("ATLAS")
ROOT.gROOT.SetBatch()

#### Dictionaries

category_dict = {
    'eff'           : 'Efficiencies',
    'tech_eff'      : 'Efficiencies/Technical',
    'purity'        : 'Efficiencies/Purities',
    'resolution'    : 'Resolutions',
    'parameter'     : 'Parameters',
    'fakerate'      : 'FakeRates',
    'duplrate'      : 'Duplicates',
    'num'           : 'Multiplicities',
    'summary'       : 'Multiplicities',
    'hits'          : 'HitsOnTracks',
    'pixels'        : 'PixelClusters',
    'strips'        : 'StripClusters',
    'tails'         : 'Parameters',
}

resolution_dict = {
    'resHelp'     : 'resHelper',
    'pullHelp'    : 'pullHelper',
    'pullwidth'   : 'pullwidth',
    'pullmean'    : 'pullmean',
    'res'         : 'resolution',
    'resmean'     : 'resmean',
    'corr'        : 'corr'  
}

def GetParserArgs() :

    from argparse import ArgumentParser
    parser = ArgumentParser( description='Parser for IDTPM plotting' )
    parser.add_argument( '--test',              help="Input test ROOT files", nargs = '+')
    parser.add_argument( '--ref',               help="Input reference ROOT files", nargs = '+')
    parser.add_argument( '-o', '--output',      help="Output directory")
    parser.add_argument( '--trkAnalysisTest',   help='IDTPM TrackAnalysis for test (e.g. TrkAnaEF)', nargs = '+')
    parser.add_argument( '--trkAnalysisRef',    help='IDTPM TrackAnalysis for reference (e.g. TrkAnaEF)', nargs = '+')
    parser.add_argument( '--chain',		        help="Trigger chain (or Offline)", default = 'Offline')
    parser.add_argument( '-t','--type',         help="type of plot (eff, tech_eff, purity, resolution, fakerate, duplrate, num, summary)")
    parser.add_argument( '-p','--param',        help="parameter (e.g pt/eta for efficiencies, d0_vs_truth_eta for resolutions, ...)")
    parser.add_argument( '--resplot',           help="which resolution plot ('resHelp','pullHelp','pullwidth','pullmean','res','resmean','corr')", default=None)
    parser.add_argument( '--log',               help="log scale on x axis, y axis or bot (x, y, xy)", default = '')
    parser.add_argument( '--ymax',              help="maximum value for y-axis ", default = 0)
    parser.add_argument( '--ymin',              help="minimum value for y-axis ", default = 0)
    parser.add_argument( '--ratioyrange',       help="range of ratio y-axis", default = None, nargs = '+', type = float)
    parser.add_argument( '--dim',               help="specify 1D or 2D ", default='1D')
    parser.add_argument( '--pipelineTest',      help="Tested pipeline", nargs = '+')
    parser.add_argument( '--pipelineRef',       help="Reference pipeline", nargs = '+')
    parser.add_argument( '--norm',              help="normalize to unit area", action='store_true')
    parser.add_argument( '--pu-comparison',     help="compare different PU samples", action='store_true')
    parser.add_argument( '--mu',                help="<mu> value (will be printed in ATLAS legend)", default = 200)
    parser.add_argument( '--sample',            help="Sample (e.g. 't#bar{t}' / 'single e, p_{T}>10 GeV', ...). Will be printed in ATLAS legend", default = '')
    parser.add_argument( '--particle',          help="Truth particle (e.g. B-hadrons / #tau, p_{T}>15 GeV). Will be printed in ATLAS legend", default = '')
    parser.add_argument( '--legend-coord',	    help="xmin, ymin, xmax, ymax TLegend coordinates", default = [0.65,0.65,0.9,0.87], type = float, nargs = '+')
    parser.add_argument( '--tag',               help="output file tag", default = '')
    parser.print_help()
    return parser.parse_args()

def isTEfficiency(obj):
    return obj.InheritsFrom("TEfficiency")

def isTProfile(obj):
    return obj.InheritsFrom("TProfile")

def getHistoName(category, args):

    par = args.param
    match category:
        case 'Parameters': return par
        case 'Resolutions': return f"{resolution_dict[args.resplot]}_{par}"
        case 'Efficiencies': return f"eff_vs_truth_{par}" if par!="truthMu" else "eff_vs_truthMu"
        case 'Efficiencies/Technical': return f"eff_vs_truth_{par}" if par!="truthMu" else "eff_vs_truthMu"
        case 'Efficiencies/Purities': return f"eff_vs_offl_{par}" if par!="truthMu" else "eff_vs_truthMu"
        case 'FakeRates': return f"fakerate_vs_offl_{par}" if par!="truthMu" else "fakerate_vs_truthMu"
        case 'Duplicates': return f"duplrate_vs_truth_{par}" if par!="truthMu" else "duplrate_vs_truthMu"
        case 'Multiplicities': return par
        case 'HitsOnTracks': return par
        case 'PixelClusters': return par
        case 'StripClusters': return par
        case _: return 'None'

def getHistoPath(cfg):

    histopath = f"InDetTrackPerfMonPlots/{cfg['trkAnalysis']}/{cfg['chain']}/Tracks/{cfg['category']}/{cfg['histo']}" if 'Clusters' not in cfg['category'] else f"InDetTrackPerfMonPlots/{cfg['trkAnalysis']}/Offline/{cfg['category']}/{cfg['histo']}"
    h = cfg['input'].Get(histopath) 
    if cfg['norm']: h.Scale(1./h.Integral())
    print(cfg['input'])

    return h

def getConfig (input, chain, pipeline, trkana, args, color, markstyle, linestyle, isRef=False):

    category = category_dict[args.type.split('_vs_')[0]] #Efficiencies, Resolutions, FakeRates, ...
    histo = getHistoName(category, args)
 
    config_dict = {
        'pipeline'      : pipeline,
        'chain'         : chain,
        'isReference'   : isRef,
        'input'         : input,
        'trkAnalysis'   : trkana,
        'category'      : category,
        'histo'         : histo,
        'legend'        : pipeline,
        'linestyle'     : linestyle,
        'linecolor'     : color,
        'markstyle'     : markstyle,
        'markcolor'     : color,
        'log'           : args.log,
        'ymax'          : float(args.ymax),
        'ymin'          : float(args.ymin),
        'ratioyrange'   : args.ratioyrange,
        'dimension'     : args.dim,
        'norm'          : args.norm,
    }

    return config_dict


def getLegend(xmin,ymin,xmax,ymax):

    leg = ROOT.TLegend(xmin,ymin,xmax,ymax)
    leg.SetFillColor(0)
    leg.SetLineColor(0)
    leg.SetBorderSize(0)
    leg.SetTextSize(0.05)
    leg.SetTextFont(42)
    leg.SetLineWidth(2)

    return leg

def getATLASLabel(args):

    sample = ''
    if args.sample == '':
        if 'ttbar' in args.ref[0]: sample = 't#bar{t}'
        if 'SingleMu' in args.ref[0]: sample = 'single #mu'
        if 'SinglePi' in args.ref[0]: sample = 'single #pi'
        if 'SingleEl' in args.ref[0]: sample = 'single #pi'
        if 'pT10_' in args.ref[0]: sample += ', p_{T} = 10 GeV'
        if 'pT100' in args.ref[0]: sample += ', p_{T} = 100 GeV'
        if 'pT1_' in args.ref[0]: sample += ', p_{T} = 1 GeV'
    else: sample = args.sample
    if args.particle != '': sample += f', {args.particle}'

    subatlas = ROOT.TLatex(.2, .84, "#splitline{#bf{#it{ATLAS}} Simulation Internal}{#splitline{#sqrt{s} = 14 TeV, HL-LHC}{#splitline{ITk Layout: 03-00-01}{<#mu> = %s, %s}}}" %(args.mu, sample))
    if args.pu_comparison:   
        subatlas = ROOT.TLatex(.2, .84, "#splitline{#bf{#it{ATLAS}} Simulation Internal}{#splitline{#sqrt{s} = 14 TeV, HL-LHC}{#splitline{ITk Layout: 03-00-01}{%s}}}" %(sample))
    subatlas.SetNDC(1)
    subatlas.SetTextFont(42)
    subatlas.SetTextSize(0.05)

    return subatlas

def getPadSizes(args, NRef):

    pad_heights = [600*0.65]
    
    for nref in range(NRef-1): pad_heights.append(600*0.35 * (1-0.05-1/3))
    pad_heights.append(600*0.35)

    return pad_heights

def getCanvas(args,cfg, NRef):

    pad_heights = getPadSizes(args, NRef)
    canv_heigh = sum(pad_heights)
    pad_bottom = [(1-sum([pad_heights[j] for j in range(i+1)])/canv_heigh) for i in range(len(pad_heights))]
    canv = ROOT.TCanvas("c","c",600,int(canv_heigh))

    canv.cd()
    pad1 = ROOT.TPad("pad1","pad1",0,pad_bottom[0],1,1)
    pad1.SetNumber(1)
    pad1.SetBottomMargin(0.05)

    ratio_pads = []

    for r in range(NRef):
        ratio_pads.append(ROOT.TPad(f"pad{r+2}",f"pad{r+2}",0,pad_bottom[r+1],1,pad_bottom[r]))
        ratio_pads[r].SetTopMargin(0.05)
        ratio_pads[r].SetBottomMargin(0.05 if r!=(NRef-1) else 1./3.)
        ratio_pads[r].SetNumber(r+2)

    if cfg['log'] == 'x':
        pad1.SetLogx()
        for r in range(NRef): ratio_pads[r].SetLogx()

    if cfg['log'] == 'y':
        pad1.SetLogy()

    if cfg['log'] == 'xy':
        pad1.SetLogx()
        pad1.SetLogy()
        for r in range(NRef): ratio_pads[r].SetLogx()

    pad1.Draw()

    for r in range(NRef): ratio_pads[r].Draw()

    return canv

def getURDRequirementLine(h, type, canv, scale, logx = False):

    scaleshift = canv.GetPad(1).GetHNDC()/canv.GetPad(2).GetHNDC()

    level = 0.01 if 'rate' in type else 2
    xmin, xmax = h.GetXaxis().GetXmin(), h.GetXaxis().GetXmax()
    ymax,ymin = h.GetMaximum(), h.GetMinimum()

    requirement_line = (ROOT.TLine(xmin,level,xmax,level))
    requirement_line.SetLineStyle(2)
    requirement_line.SetLineColor(ROOT.kRed)
    requirement_line.SetLineWidth(1)

    shiftup = 0.01*scaleshift if 'rate' not in type else 0.01
    shiftdn = 0.07*scaleshift if 'rate' not in type else 0.07
    y =  level + shiftup*(ymax-ymin) if level < ymax else ymax - shiftdn *(ymax-ymin)
    x = (.63*(xmax-xmin)+xmin) if not logx else math.exp( math.log(xmin) + 0.63*(math.log(xmax) - math.log(xmin)) )

    requirement_text = ROOT.TLatex(x,y, f"#uparrow URD requirement = {level}" if level > ymax else f"URD requirement = {level}")
    requirement_text.SetTextFont(42)
    requirement_text.SetTextSize(0.04*scale if 'rate' not in type else 0.04)
    requirement_text.SetTextColor(ROOT.kRed)
    return requirement_line, requirement_text

def setStyle(h, cfg):

    h.SetLineColor(cfg['linecolor'])
    h.SetLineStyle(cfg['linestyle'])
    h.SetMarkerStyle(cfg['markstyle'])
    h.SetMarkerColor(cfg['markcolor'])
    h.SetMarkerSize(0.8)
    h.SetLineWidth(2)

    if isTEfficiency(h):

        ROOT.gPad.Update()

        if 'Efficiencies' in cfg['category']:
            h.GetPaintedGraph().SetMaximum(1.2 if cfg['ymax']==0 else cfg['ymax'])
            h.GetPaintedGraph().SetMinimum(0.7 if cfg['ymin']==0 else cfg['ymin'])

        else:

            if 'y' in str(cfg['log']):
                h.GetPaintedGraph().SetMaximum(10*max(1,abs(math.log10(max(h.GetPaintedGraph().GetY()))))*max(h.GetPaintedGraph().GetY()) if cfg['ymax']==0 else cfg['ymax'])
                h.GetPaintedGraph().SetMinimum(max(1e-10, min(h.GetPaintedGraph().GetY())*(0.9)) if cfg['ymin']==0 else cfg['ymin'])

            else:
                h.GetPaintedGraph().SetMaximum(1.5*float(max(h.GetPaintedGraph().GetY())) if cfg['ymax']==0 else cfg['ymax'])
                h.GetPaintedGraph().SetMinimum(0 if cfg['ymin']==0 else cfg['ymin'])

        h.GetPaintedGraph().GetXaxis().SetTitleSize(0)
        h.GetPaintedGraph().GetXaxis().SetLabelSize(0)
        h.GetPaintedGraph().GetYaxis().SetLabelSize(0.05)
        h.GetPaintedGraph().GetXaxis().SetTitleSize(0.05)

    else:

        if cfg['norm']:
            h.SetMaximum(1.5 if cfg['ymax']==0 else cfg['ymax'])
            h.SetMinimum(0 if cfg['ymin']==0 else cfg['ymin'])

        else:

            if 'y' in str(cfg['log']):
                h.SetMaximum(1.5*max(1,max(1,abs(math.log10(h.GetMaximum()))))*h.GetMaximum() if cfg['ymax']==0 else cfg['ymax'])
                h.SetMinimum(max(1e-10, h.GetMinimum()*(0.9)) if cfg['ymin']==0 else cfg['ymin'])
                if 'avgNum' in cfg['histo']: h.SetMinimum(100 if cfg['ymin']==0 else cfg['ymin'])

            else:
                h.SetMaximum(1.4*h.GetMaximum() if cfg['ymax']==0 else cfg['ymax'])
                h.SetMinimum(0 if cfg['ymin']==0 else cfg['ymin'])

        h.GetXaxis().SetTitleSize(0)
        h.GetXaxis().SetLabelSize(0)
        h.GetYaxis().SetLabelSize(0.05)
        h.GetYaxis().SetTitleSize(0.05)

def setRatioStyle(ratio, cfg, color, linestyle, markstyle, ref='C000', multigraph=False, labelsizescale = 65./35., lastpad = False, splittitle = False):
    if 'vs_truthMu' in cfg['histo']: ratio.GetXaxis().SetTitle('Truth <#mu>')
    ratio.GetXaxis().SetTitleOffset(1.5)
    ratio.GetYaxis().SetTitle('#splitline{Ratio wrt}{%s}' %ref if splittitle else 'Ratio wrt %s' %ref)
    ratio.GetYaxis().SetTitleSize(0.05*labelsizescale)
    ratio.GetYaxis().SetNdivisions(505)
    ratio.GetYaxis().SetTitleOffset(1.3*1/labelsizescale)

    if cfg['ratioyrange'] is None:

        if not multigraph:
            ratio.SetMaximum()
            ratio.SetMinimum()
            ratio.GetYaxis().SetRangeUser(max(0.,min(ratio.GetMinimum()-0.1,abs(1.9-ratio.GetMaximum()))),max(ratio.GetMaximum()+0.1,abs(2-ratio.GetMinimum())))

    else:
        ratio.GetYaxis().SetRangeUser(cfg['ratioyrange'][0],cfg['ratioyrange'][1])

    ratio.GetXaxis().SetLabelSize(0.05*labelsizescale if lastpad else 0)
    ratio.GetXaxis().SetTitleSize(0.05*labelsizescale if lastpad else 0)
    ratio.GetYaxis().SetLabelSize(0.05*labelsizescale)
    ratio.GetYaxis().SetTitleSize(0.05*labelsizescale)

    if not multigraph:

        ratio.SetLineColor(color)
        ratio.SetLineStyle(linestyle)
        ratio.SetMarkerStyle(markstyle)
        ratio.SetMarkerColor(color)
        ratio.SetMarkerSize(0.8)

def getTEfficiencyRatio(histos):

    xmin, xmax = histos[0].GetPaintedGraph().GetXaxis().GetXmin(), histos[0].GetPaintedGraph().GetXaxis().GetXmax()
    g1 = ROOT.TGraphAsymmErrors(histos[0].GetPaintedGraph())
    g2 = ROOT.TGraphAsymmErrors(histos[1].GetPaintedGraph())
    title = g1.GetXaxis().GetTitle()
    ratio = ROOT.TGraphAsymmErrors()
    n, n_ref, n_test = max(g1.GetN(),g2.GetN()), g1.GetN(), g2.GetN()
    x_ref_arr, y_ref_arr = g1.GetX(), g1.GetY()
    x_test_arr, y_test_arr = g2.GetX(), g2.GetY()
    i_test, i_ref = 0, 0
    err_r_low = []
    err_r_high = []

    for i in range(n):

        if i_test < n_test and i_ref < n_ref:

            x_ref, y_ref = x_ref_arr[i_ref], y_ref_arr[i_ref]
            x_test, y_test = x_test_arr[i_test], y_test_arr[i_test]
            err_ref_low, err_ref_high = g1.GetErrorYlow(i_ref), g1.GetErrorYhigh(i_ref)
            err_test_low, err_test_high = g2.GetErrorYlow(i_test), g2.GetErrorYhigh(i_test)

            # Check points matching between test and reference
            if x_test==x_ref: 
                y_ref, y_test = y_ref_arr[i_ref], y_test_arr[i_test]
                i_test += 1
                i_ref += 1

            if x_ref>x_test:
                x_ref = x_test
                y_ref, y_test = 0,  y_test_arr[i_test]
                i_test+= 1

            if x_test>x_ref:
                x_test = x_ref
                y_test, y_ref = 0, y_ref_arr[i_ref]
                i_ref += 1

            r = y_test / y_ref if y_ref != 0 else 1000
            if y_test == 0 and y_ref == 0: r = 1
            err_r_low.append(r * math.sqrt((err_ref_low / y_ref)**2 + (err_test_low / y_test)**2) if y_ref > 0 and y_test > 0 else 0)
            err_r_high.append(r * math.sqrt((err_ref_high  / y_ref)**2 + (err_test_high / y_test)**2) if y_ref > 0 and y_test > 0 else 0)
            ratio.SetPoint(i, x_ref, r)
            ratio.GetXaxis().SetTitle(title)

    # Set errors and final x axis range
    for i in range(ratio.GetN()):

        width = ratio.GetPointX(i+1)-ratio.GetPointX(i) if i < ratio.GetN()-1 else ratio.GetPointX(i)-ratio.GetPointX(i-1)
        ratio.SetPointError(i, width/2.,width/2., err_r_low[i], err_r_high[i])

    ratio.GetXaxis().SetLimits(xmin, xmax)

    return ratio

def drawTEfficiencyOutliers(r, configs, multigraphs, markers, NRef):

    outliers_index = 0

    for j,g in enumerate(multigraphs[r].GetListOfGraphs()):
        markers.append([])
        ROOT.gPad.Update()
        ymin = ROOT.gPad.GetFrame().GetY1()
        ymax = ROOT.gPad.GetFrame().GetY2()

        for i in range(g.GetN()):

            if g.GetPointY(i) >  ymax:
                markers[r].append(ROOT.TMarker(g.GetPointX(i), ymax-(ymax-ymin)*0.05, 26))
                markers[r][outliers_index].SetMarkerColor(configs[j+NRef]['linecolor'])
                markers[r][outliers_index].SetMarkerSize(1.2)
                markers[r][outliers_index].Draw('same')
                outliers_index += 1

            if g.GetPointY(i) < ymin and g.GetPointY(i) > 0: 
                markers[r].append(ROOT.TMarker(g.GetPointX(i), ymin+(ymax-ymin)*0.05, 32))
                markers[r][outliers_index].SetMarkerColor(configs[j+NRef]['linecolor'])
                markers[r][outliers_index].SetMarkerSize(1.2)
                markers[r][outliers_index].Draw('same')
                outliers_index += 1

def drawTHOutliers(r, configs, ratios, markers, NRef):

    outliers_index = 0

    for j,ratio in enumerate(ratios):
        ROOT.gPad.Update()
        ymin = ROOT.gPad.GetFrame().GetY1()
        ymax = ROOT.gPad.GetFrame().GetY2()

        for i in range(ratio.GetNbinsX()):

            if ratio.GetBinContent(i) >  ymax: 
                markers[r].append(ROOT.TMarker(ratio.GetXaxis().GetBinCenter(i), ymax-(ymax-ymin)*0.05, 26))
                markers[r][outliers_index].SetMarkerColor(configs[j+NRef]['linecolor'])
                markers[r][outliers_index].SetMarkerSize(1.2)
                markers[r][outliers_index].Draw('same')
                outliers_index += 1

            if ratio.GetBinContent(i) < ymin and ratio.GetBinContent(i) > 0: 
                markers[r].append(ROOT.TMarker(ratio.GetXaxis().GetBinCenter(i), ymin+(ymax-ymin)*0.05, 32))
                markers[r][outliers_index].SetMarkerColor(configs[j+NRef]['linecolor'])
                markers[r][outliers_index].SetMarkerSize(1.2)
                markers[r][outliers_index].Draw('same')
                outliers_index += 1

def drawRefLine(r, ratios, reflines):

    reflines.append(ROOT.TLine(ratios[0].GetXaxis().GetXmin(),1,ratios[0].GetXaxis().GetXmax(),1))
    reflines[r].SetLineStyle(2)
    reflines[r].SetLineColor(ROOT.kBlack)
    reflines[r].SetLineWidth(1)
    reflines[r].Draw('same')

def draw(args, configs, tails=False, pu_comparison=False):

    NRef = len(args.ref) if not pu_comparison else 1
    canv = getCanvas(args,configs[0], NRef)
    ATLASLabel = getATLASLabel(args)
    legend = getLegend(args.legend_coord[0],args.legend_coord[1],args.legend_coord[2],args.legend_coord[3]) 
    doComparison = len(configs) > 1
    isTEfficiencyObj = False
    histos = [] 
    canv.cd(1)

    for i,cfg in enumerate(configs): # loop over the trkAnalysis to be compared (if single plot, there will be only 1 trkAnalysis)
        h = getHistoPath(cfg)
        isTEfficiencyObj = isTEfficiency(h)
        histos.append(h)

        if not tails: h.Draw('same' if i!=0 else '')

        else: h.Draw('sameHISTE' if i%2!=0 else 'samePE')        

        setStyle(h, cfg)

        if doComparison and cfg['dimension'] == '1D':
            draw_option = 'lp'

            if tails: 
                draw_option = 'l' if i%2!=0 else 'lp'
                jet = 'b-Jet' if i%2==0 else 'LF-Jet'
                legend.AddEntry(h, f"{cfg['legend']} {jet}",draw_option)

            else: legend.AddEntry(h, cfg['legend'],draw_option)

            legend.Draw()

    ATLASLabel.Draw()

    multigraphs = []
    reflines = []
    markers = []
    urd_lines = []
    urd_text = []

    # Loop over reference histograms to create one ratio per reference
    for r in range(NRef):

        labelsizescales = [canv.GetPad(1).GetHNDC()/canv.GetPad(i+2).GetHNDC() for i in range(NRef)]
        canv.cd(r+2)
        ratios = []
        multigraphs.append(ROOT.TMultiGraph())
        markers.append([])

        if not tails and not pu_comparison:

            for i in range(NRef, len(configs)):
                ratio_histos = [histos[r] if not isTProfile(histos[r]) else histos[r].ProjectionX(), histos[i] if not isTProfile(histos[i]) else histos[i].ProjectionX()]

                if isTEfficiencyObj: ratios.append(getTEfficiencyRatio(ratio_histos))

                else:
                    ratio = ratio_histos[1].Clone()
                    ratio.Divide(ratio_histos[0])
                    ratios.append(ratio)

            for i, ratio in enumerate(ratios):
                color = configs[i+NRef]['linecolor'] #if len(configs)>2 else ROOT.kBlack
                linestyle = configs[i+1]['linestyle']# if len(configs)>2 else 1
                markstyle = configs[i+1]['markstyle']# if len(configs)>2 else 20
                setRatioStyle(ratio, configs[i], color, linestyle, markstyle, ref=configs[r]['pipeline'], labelsizescale = labelsizescales[r], lastpad = (r==NRef-1), splittitle = (NRef>1))

                if isTEfficiencyObj:
                    multigraphs[r].Add(ratio,'p')

                else:
                    ratio.Draw('same')

            if isTEfficiencyObj:
                xmin, xmax = ratios[0].GetXaxis().GetXmin(), ratios[0].GetXaxis().GetXmax()
                setRatioStyle(multigraphs[r], configs[i], color, linestyle, markstyle, ref=configs[r]['pipeline'], multigraph=True, labelsizescale = labelsizescales[r], lastpad = (r==NRef-1), splittitle = (NRef>1))
                multigraphs[r].Draw('a')
                drawTEfficiencyOutliers(r, configs, multigraphs, markers, NRef)
                multigraphs[r].GetXaxis().SetLimits(xmin, xmax)

            else:
                drawTHOutliers(r, configs, ratios, markers, NRef)

            drawRefLine(r, ratios, reflines)

            if args.type == 'resolution' and not pu_comparison and 'C000' in args.ref[0]:
                requirement_line, requirement_text = getURDRequirementLine(ratios[0], 'resolution', canv, labelsizescales[r], logx = ('x' in args.log))
                urd_lines.append(requirement_line)
                urd_text.append(requirement_text)
                urd_lines[r].Draw('same')
                urd_text[r].Draw('same')

        else:
            for i in [0,2]:
                ratio_histos = [histos[i],histos[i+1]] 
                ratio = ratio_histos[0].Clone()
                ratio.Divide(ratio_histos[1])
                ratios.append(ratio)

            for i, ratio in enumerate(ratios):
                color = configs[i*2]['linecolor']
                linestyle = configs[i*2]['linestyle']
                markstyle = configs[i*2]['markstyle']
                setRatioStyle(ratio, configs[i], color, linestyle, markstyle, ref=configs[r]['pipeline'],labelsizescale = labelsizescales[r], lastpad = (r==NRef-1))
                ratio.GetYaxis().SetTitle('b-Jet / LF-Jet' if tails else '<#mu> = 140/<#mu> = 200')
                ratio.Draw('same')

            drawRefLine(r, ratios, reflines)

    var = cfg['histo'] if not cfg['category'] == 'Efficiencies/Technical' else cfg['histo'].replace('eff','tech_eff')
    canv.SaveAs(f"{args.output}/{args.tag+'_' if args.tag != '' else ''}{var}.png")

def main():

    args = GetParserArgs()
    chain = args.chain
    inputTest = args.test
    inputRef = args.ref
    pipelinesRef = args.pipelineRef
    pipelinesTest = args.pipelineTest
    trkanalysesRef = args.trkAnalysisRef
    trkanalysesTest = args.trkAnalysisTest
    doTails = (args.type == 'tails')
    doPUComparison = args.pu_comparison
    

    if not doTails and not doPUComparison:
        colors = [ ROOT.kRed, ROOT.kBlue, ROOT.kGreen+2, ROOT.kOrange+7, ROOT.kMagenta, ROOT.kCyan+1, ROOT.kViolet, ROOT.kTeal+2, ROOT.kPink+6, ROOT.kAzure+1]
        linestyles = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]
        markstyles = [ 20, 21, 22, 23, 24, 25, 26, 27, 28, 30 ]
        inputFiles = inputRef+inputTest
        pipelines = pipelinesRef+pipelinesTest
        trkanalyses = trkanalysesRef+trkanalysesTest
        inputFiles = inputRef+inputTest

        print(pipelines)
        print(trkanalyses)
        print(inputFiles)

        configs = []

        for i,pipeline in enumerate(pipelines):
            isReference = (i<len(pipelinesRef))
            f = ROOT.TFile.Open(inputFiles[i], 'READ')
            configs.append(getConfig(f, chain, pipeline, trkanalyses[i], args, colors[i], markstyles[i], linestyles[i], isReference))

        draw(args, configs)

    else:

        colors = [ROOT.kRed, ROOT.kRed, ROOT.kBlue, ROOT.kBlue] 
        linestyles = [2,1,2,1]
        markstyles = [20,0,24,0] if doTails else [20, 24, 20, 24]
        pipelines = args.pipelineRef+args.pipelineRef+args.pipelineTest+args.pipelineTest if doTails else args.pipelineRef+args.pipelineTest
        trkanalyses = args.trkAnalysisRef
        trkanalyses.extend(args.trkAnalysisTest)
        inputFiles = inputRef+inputRef+inputTest+inputTest if doTails else inputRef+inputTest
        configs = []

        for i,trkana in enumerate(trkanalyses):
            f = ROOT.TFile.Open(inputFiles[i], 'READ')
            configs.append(getConfig(f, chain, pipelines[i], trkanalyses[i], args, colors[i], markstyles[i], linestyles[i]))

        draw(args, configs, tails=doTails,pu_comparison=doPUComparison)

if __name__ == "__main__":
    main()
