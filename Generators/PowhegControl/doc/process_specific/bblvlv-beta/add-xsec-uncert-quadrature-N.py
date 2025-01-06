import os
import re
import glob
import numpy as np
import random
import matplotlib.pyplot as plt

# Define the file pattern and the array of N values
filename_pattern = "pwg-*-st2-stat.dat"
#N_values = [50, 100,150,200,250,300,350,360,370,380,390,400]
#N_values = [350,360,370,380,385,390,392,395,396,397,398,399,400]
N_values = [50, 100,200,400,800,1000,1100,1200,1250,1260,1270,1275,1278,1279,1280]
plot_file_postfix=""
#N_values = [1200,1250,1255,1260,1265,1270,1275,1276,1277,1278,1279,1280]
#plot_file_postfix="_small_range"
num_repeats = 30
#num_repeats = 2

# List all files matching the pattern
file_list = glob.glob(filename_pattern)

# Find and remove empty files from the list
empty_files = [f for f in file_list if os.path.getsize(f) == 0]
file_list = [f for f in file_list if f not in empty_files]

# Save the number of empty files
num_empty_files = len(empty_files)

# Output the results
print(f"Number of empty files: {num_empty_files}")

# Now subtract N_values by the number of empty files
N_values = [N-num_empty_files for N in N_values]

print("N_values after subtracting empty files:")
print(N_values)

# Initialize a dictionary to store results for each N
results = {N: [] for N in N_values}

# Loop over each N in N_values
for N in N_values:
    for _ in range(num_repeats):
        # Randomly select N files from the file list
        selected_files = random.sample(file_list, min(N, len(file_list)))
        
        # Initialize a list to store extracted values for the current run
        value_list = []

        # Loop over selected files and process each one
        for file_path in selected_files:
            with open(file_path, 'r') as file:
                for line in file:
                    # Search for lines matching the required pattern
                    match = re.search(r"^ grand total total \(pos\.-\|neg\.\|\):\s+(\d+\.\d+|\d+)\s+\+-\s+(\d+\.\d+E[-+]\d+|\d+\.\d+)", line)
                    if match:
                        # Extract the scientific notation value (XXXE-00X)
                        value = float(match.group(2))
                        value_list.append(value)
        
        # Calculate the quadrature sum for this run
        if value_list:
            quadrature_sum = np.sqrt(np.sum(np.array(value_list) ** 2))/len(value_list)
            results[N].append(quadrature_sum)
        else:
            results[N].append(0)

# find upper and lower bounds
# Loop over each N in N_values
upper_bounds = []
lower_bounds = []
value_list = []
for file_path in file_list:
    with open(file_path, 'r') as file:
        for line in file:
           # Search for lines matching the required pattern
            match = re.search(r"^ grand total total \(pos\.-\|neg\.\|\):\s+(\d+\.\d+|\d+)\s+\+-\s+(\d+\.\d+E[-+]\d+|\d+\.\d+)", line)
            if match:
                # Extract the scientific notation value (XXXE-00X)
                value = float(match.group(2))
                value_list.append(value)
if value_list:
    value_list.sort()
    for N in N_values:
        upper_bound_value_list = value_list[-N:]
        lower_bound_value_list = value_list[:N]
        upper_bounds.append(np.sqrt(np.sum(np.array(upper_bound_value_list) ** 2))/len(upper_bound_value_list))
        lower_bounds.append(np.sqrt(np.sum(np.array(lower_bound_value_list) ** 2))/len(lower_bound_value_list))

#print("upper bounds")
#print(upper_bounds)
#print("lower bounds")
#print(lower_bounds)

# Calculate mean and standard deviation for each N
mean_values = []
std_dev_values = []
for N in N_values:
    mean_values.append(np.mean(results[N]))
    std_dev_values.append(np.std(results[N]))

# Plotting the results
plt.figure(figsize=(10, 6))
plt.plot(N_values,upper_bounds,label="upper bound")
plt.plot(N_values,lower_bounds,label="lower bound")
plt.errorbar(N_values, mean_values, yerr=std_dev_values, fmt='o-', capsize=5, label="x-sec uncertainty")
plt.fill_between(N_values, 
                 np.array(mean_values) - np.array(std_dev_values), 
                 np.array(mean_values) + np.array(std_dev_values), 
                 alpha=0.2, color="blue", label="Error band (std dev)")

plt.xlabel("Number of files (N)")
plt.ylabel("x-sec uncertainty")
plt.title("x-sec with error band vs. N")
plt.legend()
plt.grid(True)
plt.savefig("sum_of_squares_vs_N"+plot_file_postfix+".png", format="png", dpi=300, bbox_inches="tight")

for N in N_values:
    plt.errorbar([N]*len(results[N]),results[N],fmt="o",color='darkgreen',markersize=2)
plt.savefig("sum_of_squares_vs_N_with_val"+plot_file_postfix+".png", format="png", dpi=300, bbox_inches="tight")

#plt.show()

