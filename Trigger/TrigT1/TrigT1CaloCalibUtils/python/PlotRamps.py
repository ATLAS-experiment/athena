# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


from ROOT import gROOT, gDirectory, gPad, gStyle, kYellow, kGray, TCanvas, TFile, TPaveLabel, TH1F
from optparse import OptionParser


def passesSelection(gain,offset):
    if  ((gain > 0.5 and gain <1.6) and (offset > -10 and offset < 10)):
        return True
    else:
        return False

def getCrate( coolId ):
    return (coolId & 0xf000000) >> 24

def getModule( coolId ):
    return (coolId & 0x00f0000) >> 16

def getMcm( coolId ):
    return (coolId & 0x0000f00) >>  8

def getChannel( coolId ):
    return (coolId & 0x000000f) >>  0

def getLogicalChannel(mcm, chn):
    return mcm*4+chn

def getCoolId(crt,mod,mcm,chn):
    id = 0 | (crt<<24) | (1<<20) | (mod<<16) | (mcm<<8) | (chn<<0)
    return id

def getCoolIdLog(crt,mod,logchn):
    mcm = int( logchn/4 )
    chn = logchn % 4
    return getCoolId(crt,mod,mcm,chn)

def PlotRamps(input_file_name=None):

    if input_file_name is None:
        print ("No input file name, assuming graphs.root")
        input_file_name = "graphs.root"

    gROOT.SetBatch( True )
    gStyle.SetPalette(1)
    gStyle.SetOptStat(111111)
    gStyle.SetOptFit(11)
    gStyle.SetCanvasColor(10)
    gStyle.SetFrameFillColor(10)
    gStyle.SetTitleFillColor(0)
    gStyle.SetTitleBorderSize(1)
    gStyle.SetStatBorderSize(1)
    gStyle.SetStatFontSize(0.075)
    gStyle.SetStatY(0.9)
    gStyle.SetStatX(0.5)

    gStyle.SetTitleFontSize(0.075)
    gStyle.SetPaperSize(gStyle.kA4)

    gStyle.SetPadTopMargin(0.10)
    gStyle.SetPadBottomMargin(0.12)
    gStyle.SetPadRightMargin(0.12)
    gStyle.SetPadLeftMargin(0.12)
    gStyle.SetHatchesSpacing(4.0)

    canvas = TCanvas('canvas','Ramps',200,10,1000,750)
    canvas.SetBatch( True )

    graphs = TFile(input_file_name)
    key_list = graphs.GetListOfKeys()

    pdfFileName = 'rampPlots.pdf'
    canvas.Print( pdfFileName + '[' )

    # Book and style a template histogram to later draw the graph into
    histo = TH1F("foo","foo",300,0.,300.)
    histo.SetMinimum(0.)
    histo.SetMaximum(300.)
    histo.GetXaxis().SetTitle("L1Calo energy")
    histo.GetYaxis().SetTitle("Calo energy")
    histo.GetXaxis().SetTitleSize(0.04)
    histo.GetYaxis().SetTitleSize(0.04)

    # Initialise a 2D map to indicate present modules
    modPresent = [ [ False for module in range(16) ] for crate in range(8) ]

    # creates sorted list of numerical coolIds
    list_of_histos=[]
    for key in key_list:
        keyStr = key.GetName()
        keyInt = int(keyStr, base = 16) 
        list_of_histos.append( keyInt )
        # Update list of present modules
        modPresent[getCrate(keyInt)][getModule(keyInt)] = True

    # loop over crates, modules, and channels
    for ppCrt in range(8):
        for ppMod in range(16):
            # Check that this module is present
            if not modPresent[ppCrt][ppMod]:  continue

            # Loop over 64 channels per module
            for logChn in range(64):
                coolId = getCoolIdLog( ppCrt, ppMod,logChn)
                my_graph = gDirectory.Get(hex(coolId))

                if ( logChn % 64 ) == 0: # new page
                    canvas.Clear()
                    canvas.cd()
                    gStyle.SetOptTitle(0)
                    gStyle.SetOptStat(0)
                    gPad.SetRightMargin(0.1)
                    gPad.SetLeftMargin(0.1)
                    gPad.SetTopMargin(0.0)
                    gPad.SetBottomMargin(0.1)

                    canvas.Divide(8,9,-1,-1)

                    title = "Crate %d PPM %d: L1Calo (x) vs Calo (y) Energies" %  ( ppCrt, ppMod )
                    ltit = TPaveLabel(0.35,0.90,0.65,1.0,title)
                    ltit.SetTextAlign(22)
                    ltit.SetTextSize(0.40)
                    ltit.SetFillStyle(0)
                    ltit.SetBorderSize(0)
                    ltit.Draw()
            
                canvas.cd( logChn + 9 )

                # Draw template histogram and graph, if it exists
                histo.Draw()
                
                if ( my_graph ):
                    # Fetch function results
                    function_list = my_graph.GetListOfFunctions()
                    my_fit = function_list[0]
                    offset = my_fit.GetParameter(0)
                    slope  = my_fit.GetParameter(1)
                    if not passesSelection(slope,offset):
                        gPad.SetFrameFillColor(kYellow-9)
                    #Style and draw graph
                    my_graph.SetMarkerStyle(34)
                    my_graph.SetMarkerSize(0.8)
                    my_fit.SetLineWidth(1)
                    my_fit.SetLineStyle(1)
                    my_graph.Draw("P")
                else:
                    gPad.SetFrameFillColor(kGray)

                # End of page, so print it
                if ( logChn == 63 ):
                    canvas.Print( pdfFileName )

    # closing file
    canvas.Print( pdfFileName + ']' )
    print ("Finished!")

if __name__ == "__main__":

    print ("Starting PlotRamps")

    parser = OptionParser()
    parser.add_option("-f","--InputFile",action="store",type="string",
                      dest="input_file_name",help="Name of input file")
    (options, args) = parser.parse_args()

    PlotRamps(options.input_file_name)
