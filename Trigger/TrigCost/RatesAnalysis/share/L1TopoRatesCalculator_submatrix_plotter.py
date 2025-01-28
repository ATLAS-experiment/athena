#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import ROOT
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import Normalize
import argparse

def plot_correlation_matrix(file_name, matrix_name, selected_vars, output_name):
    # Open the ROOT file
    root_file = ROOT.TFile.Open(file_name)
    
    # Retrieve the correlation matrix
    rates_matrix = root_file.Get(matrix_name)
    
    # Extract the list of variables
    n = rates_matrix.GetNbinsX()  # Assuming the number of bins is the same in both X and Y axes
    variables = [rates_matrix.GetXaxis().GetBinLabel(i) for i in range(1, n+1)]
   
    # Filter the selected variables
    indices = [variables.index(var) for var in selected_vars]
    
    # Create a numpy array to store the selected values
    selected_matrix = np.zeros((len(indices), len(indices)))
    errors_matrix = np.zeros((len(indices), len(indices))) 
    
    for i, ix in enumerate(indices):
        for j, iy in enumerate(indices):
            selected_matrix[i, j] = rates_matrix.GetBinContent(ix+1, iy+1)
            errors_matrix[i, j] = rates_matrix.GetBinError(ix+1, iy+1) 
    # Find the minimum and maximum values in the selected matrix
    min_value = np.min(selected_matrix)
    max_value = np.max(selected_matrix)
    
    # Create the plot
    fig, ax = plt.subplots(figsize=(24, 22))
    cax = ax.matshow(selected_matrix, cmap='coolwarm', norm=Normalize(vmin=min_value, vmax=max_value))
    
    # Add text inside the cells
    for (i, j), val in np.ndenumerate(selected_matrix):
        error = errors_matrix[i, j]
        ax.text(j, i, f'{val:.2f}\n±{error:.2f}', ha='center', va='center', color='black', fontsize=25)
    
    # Set the axis labels
    ax.set_xticks(np.arange(len(indices)))
    ax.set_yticks([])
    ax.set_xticklabels([selected_vars[i] for i in range(len(indices))], rotation=90, fontsize=25)
    ax.set_yticklabels([])
    plt.xticks(rotation=45)
 
    # Adjust labels to align at the start of each bin
    for tick in ax.get_xticklabels():
        tick.set_horizontalalignment('left')  # Align left
    # Add a color bar
    colorbar = fig.colorbar(cax)
    colorbar.ax.tick_params(labelsize=25)
    colorbar.set_label('Hz', fontsize=25)

    # Save the image as a PNG file
    plt.savefig(output_name, bbox_inches='tight')
    plt.close()

if __name__ == '__main__':
    # Set up argument parser
    parser = argparse.ArgumentParser(description='Plot a correlation matrix from a ROOT file.')
    parser.add_argument('file_name', type=str, help='Path to the ROOT file.')
    parser.add_argument('matrix_name', type=str, help='Name of the matrix in the ROOT file.')
    parser.add_argument('selected_vars', type=str, nargs='+', help='List of selected variables.')
    parser.add_argument('output_name', type=str, help='Name of the output PNG file.')

    # Parse the arguments
    args = parser.parse_args()

    # Call the function with the parsed arguments
    plot_correlation_matrix(args.file_name, args.matrix_name, args.selected_vars, args.output_name)

# Example usage
# python L1TopoRatesCalculator_submatrix_plotter.py RatesHistograms.root rates_matrix L1_eTAU12 L1_jTAU20 L1_gXEJWOJ70 L1_JPSI-1M5-eEM15 L1_jJ40p30ETA49 correlation_matrix.png
#file_name = 'RatesHistograms.root'  # Path to the ROOT file
#matrix_name = 'rates_matrix'  # Name of the matrix in the ROOT file
#selected_vars = ["L1_eTAU12","L1_jTAU20","L1_gXEJWOJ70","L1_JPSI-1M5-eEM15","L1_jJ40p30ETA49"]
#output_name = 'correlation_matrix.png'  # Name of the output file

#plot_correlation_matrix(file_name, matrix_name, selected_vars, output_name)

