#!/usr/bin/env python

import uproot
import pandas as pd
import os, sys
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--inputFile', type=str)
    args = parser.parse_args()

    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit(1)

    # Specify your input file
    input_name = args.inputFile
    f = uproot.open(input_name)

    # Specify the path to your log file
    log_file = "log.txt"

    # Make a list for branches
    list_branches=[]

    # List all the branches from CollectionTree
    for branch in f["CollectionTree"].members.get('fBranches'):
      j=branch.tojson()
      list_branches.append([j.get('fName'), j.get('fClassName')])

    # Create a DataFrame with column names
    df = pd.DataFrame(list_branches, columns=['VariableName', 'Type'])

    # AuxDyn branches are the actual variables
    # Filter the DataFrame to include only rows where the "VariableName" column contains "AuxDyn"
    filtered_df = df[df['VariableName'].str.contains("AuxDyn")]

    # Create a directory to save the csv files 
    output_directory = "generated_csv_files"
    os.makedirs(output_directory, exist_ok=True)

    # Dictionary to store rows grouped by prefix
    grouped_rows = {}

    # Open the log file in append mode
    with open(log_file, "a") as logfile:
        message = f"Getting list of variables from AuxDyn branches into csv files"
        print(message)
        logfile.write(message + "\n")

        # Iterate over each row in the filtered DataFrame
        for index, row in filtered_df.iterrows():
            # Extract the part of the "VariableName" before "AuxDyn"
            prefix = row['VariableName'].split("AuxDyn")[0]

            # Add the row to the corresponding group in the dictionary
            if prefix in grouped_rows:
                grouped_rows[prefix].append(row)
            else:
                grouped_rows[prefix] = [row]

        # Write each group of rows to a separate csv file with line numbers
        for prefix, rows in grouped_rows.items():
           # Create a DataFrame from the rows in the group
           group_df = pd.DataFrame(rows)

           # Drop the string before the dot in the VariableName
           group_df['VariableName'] = group_df['VariableName'].str.split(".").str[1]

           # Define a function to extract the sorting key from the VariableName
           def get_sort_key(variable_name):
           # Extract the part after the dot to make final variable names
               return variable_name.split(".")[-1]

           # Sort the DataFrame ignoring case
           group_df = group_df.sort_values(by='VariableName', key=lambda x: x.str.lower())

           # Add a new column for line numbers for easy inspection when compared to master files
           group_df.insert(0, 'LineNumber', range(1, len(group_df) + 1))

           # Save the group DataFrame to a csv file
           output_filename = os.path.join(output_directory, f"{prefix}.csv")

           # Print the csv output filenames and into the log
           print(output_filename)
           logfile.write(f"{output_filename}\n")
           group_df.to_csv(output_filename, index=False)    

        message = f"csv files generated from AuxDyn branches successfully"
        logfile.write(message + "\n")

    print(message)
    return 0
    
if __name__ == "__main__":
    ret = main()
    sys.exit(ret)
    
