"""
 Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 2021 Peter Kraemer - Uni Mainz
"""


"""Implement functions for postprocessing used by histgrinder."""


def project_mean_ROOT(inputs):
    """Fit peak for every row in 2D histogram."""
    mean = inputs[0][1][0].ProjectionY().Clone()
    mean.Clear()
    sigma = inputs[0][1][0].ProjectionY().Clone()
    sigma.Clear()
    name = inputs[0][1][0].GetName()
    n_bins_y = inputs[0][1][0].GetNbinsY()
    for i in range(n_bins_y):
        tmp = inputs[0][1][0].ProjectionX(name, i,i).Clone()
        if tmp.GetEntries() == 0:
            print("zero entries in Projection")
            continue
        mean.SetBinContent(i, tmp.GetMean(1))
        mean.SetBinError(i, tmp.GetMeanError(1))
        sigma.SetBinContent(i, tmp.GetRMS(1))
        sigma.SetBinError(i, tmp.GetRMSError(1))
    mean.SetTitle(inputs[0][1][0].GetTitle()+"projection mean")
    mean.GetXaxis().SetTitle("#eta regions")
    mean.GetYaxis().SetTitle("entries")
    sigma.SetTitle(inputs[0][1][0].GetTitle()+"projection sigma")
    sigma.GetXaxis().SetTitle("#eta regions")
    sigma.GetYaxis().SetTitle("entries")
    return [mean, sigma]

def efficiencies_2d(inputs):
    """Returns 2D efficiencies from two 2D input histograms, the first the selective and the second the inclusive histogram."""
    selective  = inputs[0][1][0].Clone()
    inclusive  = inputs[0][1][1].Clone()
    efficiency = selective.Clone()
    efficiency.Reset()

    n_bins_x = selective.GetNbinsX()
    n_bins_y = selective.GetNbinsY()
    for i in range(n_bins_x):
        for j in range(n_bins_y):
            bin1 = selective.GetBinContent(i, j)
            bin2 = inclusive.GetBinContent(i, j)
            eff = float(bin1)/float(bin2) if bin2!=0 else 0
            efficiency.SetBinContent(i, j, eff)

    efficiency.SetTitle(selective.GetTitle())
    efficiency.GetXaxis().SetTitle("eta")
    efficiency.GetYaxis().SetTitle("phi")
    return [efficiency]