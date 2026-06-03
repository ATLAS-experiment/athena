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


def normalize_rows_ROOT(inputs):
    """
    Normalize each row (Y-bin) of a TH2F to a maximum of 1.
    Rows with zero entries are skipped.
    """
    hist2d = inputs[0][1][0]
    name = hist2d.GetName()

    print("normalize rows for this hist:", hist2d)

    # Clone histogram for output
    norm_hist = hist2d.Clone(name + "_row_normalized")
    norm_hist.Reset()

    n_bins_x = hist2d.GetNbinsX()
    n_bins_y = hist2d.GetNbinsY()

    for iy in range(1, n_bins_y + 1):
        # Project this row (fixed Y-bin) onto X
        proj = hist2d.ProjectionX(f"{name}_proj_{iy}", iy, iy)

        if proj.GetEntries() == 0:
            print(f"Row {iy}: zero entries, skipping")
            continue

        max_val = proj.GetMaximum()

        if max_val == 0:
            print(f"Row {iy}: max is zero, skipping")
            continue

        # Normalize this row
        for ix in range(1, n_bins_x + 1):
            val = hist2d.GetBinContent(ix, iy)
            err = hist2d.GetBinError(ix, iy)

            norm_hist.SetBinContent(ix, iy, val / max_val)
            norm_hist.SetBinError(ix, iy, err / max_val)

    norm_hist.SetTitle(hist2d.GetTitle() + " (row-normalized)")
    norm_hist.GetXaxis().SetTitle(hist2d.GetXaxis().GetTitle())
    norm_hist.GetYaxis().SetTitle(hist2d.GetYaxis().GetTitle())

    return [norm_hist]
