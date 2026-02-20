#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import ROOT
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
import argparse

def load_selected_matrix(root_file, matrix_name, selected_vars, with_errors=True):
    matrix = root_file.Get(matrix_name)
    if not matrix:
        raise RuntimeError(f"Matrix '{matrix_name}' not found in ROOT file.")

    n = matrix.GetNbinsX()
    all_vars = [matrix.GetXaxis().GetBinLabel(i) for i in range(1, n+1)]
    indices = [all_vars.index(var) for var in selected_vars]

    selected_matrix = np.zeros((len(indices), len(indices)))
    errors_matrix = np.zeros((len(indices), len(indices))) if with_errors else None

    for i, ix in enumerate(indices):
        for j, iy in enumerate(indices):
            selected_matrix[i, j] = matrix.GetBinContent(ix+1, iy+1)
            if with_errors:
                errors_matrix[i, j] = matrix.GetBinError(ix+1, iy+1)

    return selected_matrix, errors_matrix

def plot_matrix(selected_matrix, errors_matrix, selected_vars, title, output_name, colorbar_label):
    min_value = np.min(selected_matrix)
    max_value = np.max(selected_matrix)

    fig, ax = plt.subplots(figsize=(46, 44))
    cax = ax.matshow(selected_matrix, cmap='coolwarm', norm=Normalize(vmin=min_value, vmax=max_value))

    for (i, j), val in np.ndenumerate(selected_matrix):
        if errors_matrix is not None:
            error = errors_matrix[i, j]
            text = f'{val:.2f}\n±{error:.2f}'
        else:
            text = f'{val:.2f}'
        ax.text(j, i, text, ha='center', va='center', color='black', fontsize=25)

    ax.set_xticks(np.arange(len(selected_vars)))
    ax.set_yticks([])
    ax.set_xticklabels(selected_vars, rotation=90, fontsize=25)
    ax.set_yticklabels([])
    plt.xticks(rotation=45)
    for tick in ax.get_xticklabels():
        tick.set_horizontalalignment('left')

    colorbar = fig.colorbar(cax)
    colorbar.ax.tick_params(labelsize=25)
    colorbar.set_label(colorbar_label, fontsize=25)

    plt.title(title, fontsize=30)
    plt.savefig(output_name, bbox_inches='tight')
    plt.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Plot correlation, counts, and L1TopoScore matrices from a ROOT file.')
    parser.add_argument('file_name', type=str, help='Path to the ROOT file.')
    parser.add_argument('selected_vars', type=str, nargs='+', help='List of selected variables.')
    args = parser.parse_args()

    file = ROOT.TFile.Open(args.file_name)

    # Plot rates_matrix 
    # Note: change the matrix name with the name of the matrix from the .root file, if ran on the grid your matrix might end with a "_combined"
    rates, rates_err = load_selected_matrix(file, "rates_matrix_combined", args.selected_vars, with_errors=True)
    plot_matrix(rates, rates_err, args.selected_vars, "Rates Matrix", "rates_matrix_selected.png", "Hz")

    # Plot counts_matrix (no errors used)
    counts, _ = load_selected_matrix(file, "counts_matrix_combined", args.selected_vars, with_errors=False)
    plot_matrix(counts, None, args.selected_vars, "Counts Matrix", "counts_matrix_selected.png", "# Events")

    # Plot L1TopoScore_matrix
    topo, topo_err = load_selected_matrix(file, "L1TopoScore_matrix_combined", args.selected_vars, with_errors=True)
    plot_matrix(topo, topo_err, args.selected_vars, "L1Topo Score Matrix", "toposcore_matrix_selected.png", "Topo Score")

