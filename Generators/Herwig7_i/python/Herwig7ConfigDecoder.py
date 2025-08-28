# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Author: Lukas Kretschmann (lukas.kretschmann@cern.ch)

# Import standard Python modules for file handling, subprocess execution, and path manipulation
import subprocess
import os
from pathlib import Path

# Import Athena-specific modules for logging
from AthenaCommon import Logging
athMsgLog = Logging.logging.getLogger('Herwig7ConfigDecoder')

def extract_herwig_lines(input_file="Herwig.run", temp_file="HerwigConfigDecoder_RunCardDump.txt", skip_particles=False, skip_decays=False):
    """Extract unique lines containing '/Herwig/' (excluding 'cvmfs') from the input file and save to a temporary file.
    
    Args:
        input_file (str): Path to the input Herwig run card file (default: 'Herwig.run').
        temp_file (str): Path to the temporary output file for extracted lines (default: 'HerwigConfigDecoder_RunCardDump.txt').
        skip_particles (bool): If True, skip lines related to /Herwig/Particles, /Herwig/Masses, and /Herwig/Widths.
        skip_decays (bool): If True, skip lines related to /Herwig/Decays.
    
    Returns:
        list: List of unique Herwig configuration lines extracted.
    """
    try:
        # Read the input file and filter lines containing '/Herwig/' while excluding 'cvmfs'
        with open(input_file, 'r') as infile:
            all_lines = [line.strip() for line in infile if '/Herwig/' in line and 'cvmfs' not in line]
            herwig_lines = []
            seen_paths = set()  # Track unique paths to avoid duplicates
            skip_particles_count = 0
            skip_decays_count = 0
            for line in all_lines:
                # Skip particle-related lines if specified
                if skip_particles and ('/Herwig/Particles' in line or '/Herwig/Masses' in line or '/Herwig/Widths' in line):
                    skip_particles_count += 1
                    continue
                # Skip decay-related lines if specified
                if skip_decays and '/Herwig/Decays' in line:
                    skip_decays_count += 1
                    continue
                # Skip duplicate lines
                if line in seen_paths:
                    athMsgLog.debug(f"Skipped duplicate path: {line}")
                    continue
                seen_paths.add(line)
                herwig_lines.append(line)
            # Clean up specific Herwig path formatting issues
            for i in range(len(herwig_lines)):
                if '|}/Herwig' in herwig_lines[i]:
                    herwig_lines[i] = herwig_lines[i].replace('|}/Herwig', '/Herwig')
                elif '}/Herwig' in herwig_lines[i]:
                    herwig_lines[i] = herwig_lines[i].replace('}/Herwig', '/Herwig')
            
        # Log information about skipped lines
        skipped_count = len(all_lines) - len(herwig_lines)
        if skipped_count > 0:
            athMsgLog.debug(f"Skipped {skipped_count} lines (including {len(all_lines) - len(seen_paths) - skip_particles_count - skip_decays_count} duplicates)")
        if skip_particles_count > 0:
            athMsgLog.debug(f"Skipped {skip_particles_count} /Herwig/Particles, /Herwig/Masses and /Herwig/Widths lines due to --skip-particles")
        if skip_decays_count > 0:
            athMsgLog.debug(f"Skipped {skip_decays_count} /Herwig/Decays lines due to --skip-decays")
        
        # Write the filtered unique lines to the temporary file
        with open(temp_file, 'w') as outfile:
            outfile.writelines(line + '\n' for line in herwig_lines)
            
        athMsgLog.info(f"Successfully extracted {len(herwig_lines)} unique lines to {temp_file}")
        return herwig_lines
    except FileNotFoundError:
        athMsgLog.info(f"Error: Input file '{input_file}' not found")
        return []
    except Exception as e:
        athMsgLog.info(f"Error: An error occurred: {str(e)}")
        return []

def run_herwig_get_commands(commands, phase="paths"):
    """Execute multiple Herwig 'get' commands in a single shell session and parse the output.
    
    Args:
        commands (list): List of commands or tuples (for interfaces) to execute.
        phase (str): Execution phase, either 'paths' for path commands or 'interfaces' for interface commands.
    
    Returns:
        list: List of tuples containing the command and its output or error message.
    """
    if not commands:
        return []
    
    # Retrieve Herwig installation path from environment variable
    herwig_path = os.getenv('HERWIG7_PATH')
    if not herwig_path:
        return [(cmd, "Error: Environment variable HERWIG7_PATH is not set") for cmd in commands]
    
    # Construct the Herwig command to read the default repository
    herwig_cmd = f"{herwig_path}/bin/Herwig read --repo={herwig_path}/share/Herwig/HerwigDefaults.rpo"
    
    try:
        # Create a temporary script file to store the commands
        temp_script = f"temp_herwig_script_{os.getpid()}.in"
        with open(temp_script, 'w') as f:
            for cmd in commands:
                if phase == "paths":
                    f.write(f"get {cmd}\n")
                else:  # interfaces
                    f.write(f"get {cmd[0]}:{cmd[1]}\n")
        
        # Execute the Herwig command with the temporary script as input
        cmd = f"cat {temp_script} | {herwig_cmd}"
        result = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=10 * len(commands)  # Scale timeout based on number of commands
        )
        
        # Process the command output
        output = result.stdout.strip()
        if result.stderr:
            output += f"\nError: {result.stderr.strip()}"
        
        # Clean up the temporary script file
        Path(temp_script).unlink()
        
        # Split output into individual command responses
        outputs = output.split("Herwig> get ")[1:] if output else []
        results = []
        for i, cmd in enumerate(commands):
            if i < len(outputs):
                raw_output = outputs[i].strip()
                # Remove the command itself from the output
                cmd_str = cmd if phase == "paths" else f"{cmd[0]}:{cmd[1]}"
                if raw_output.startswith(cmd_str):
                    raw_output = raw_output[len(cmd_str):].strip()
                if phase == "interfaces":
                    # Apply filtering for interface outputs
                    if len(raw_output.split("\n")) == 1:
                        if raw_output.split("\n")[0] != '' and raw_output.split("\n")[0] != 'Herwig>':
                            filtered_output = raw_output.split("\n")[0]
                        elif raw_output.split("\n")[0] != 'Herwig>':
                            filtered_output = "No response from the interface"
                        else:
                            filtered_output = "No response from the interface"
                    elif len(raw_output.split("\n")) == 2:
                        filtered_output = "No response from the interface"
                    elif len(raw_output.split("\n")) == 3:
                        if all("Herwig" in line for line in raw_output.split("\n")):
                            filtered_output = "No response from the interface"
                        else:
                            filtered_output = raw_output.split("\n")[1]
                    else:
                        filtered_output = raw_output
                    results.append((cmd[0], f"{cmd[0]}:{cmd[1]}", filtered_output))
                else:
                    results.append((cmd, raw_output))
            else:
                if phase == "interfaces":
                    results.append((cmd[0], f"{cmd[0]}:{cmd[1]}", "Error: No output received"))
                else:
                    results.append((cmd, "Error: No output received"))
        
        return results
    
    except subprocess.TimeoutExpired:
        return [(cmd, "Error: Command timed out") if phase == "paths" else (cmd[0], f"{cmd[0]}:{cmd[1]}", "Error: Command timed out") for cmd in commands]
    except Exception as e:
        return [(cmd, f"Error: {str(e)}") if phase == "paths" else (cmd[0], f"{cmd[0]}:{cmd[1]}", f"Error: {str(e)}") for cmd in commands]

def execute_herwig_get(herwig_lines, output_file="HerwigConfigDecoder_InterfaceDump.txt", max_lines=None):
    """Execute Herwig 'get' commands for all provided paths in a single shell session and save the results.
    
    Args:
        herwig_lines (list): List of Herwig configuration paths to process.
        output_file (str): Path to the output file for results (default: 'HerwigConfigDecoder_InterfaceDump.txt').
        max_lines (int, optional): Maximum number of lines to process (default: None, processes all lines).
    """
    try:
        # Check if Herwig path is set
        herwig_path = os.getenv('HERWIG7_PATH')
        if not herwig_path:
            athMsgLog.error("Error: Environment variable HERWIG7_PATH is not set")
            return
        
        # Limit the number of lines to process if specified
        lines_to_process = herwig_lines[:max_lines] if max_lines is not None else herwig_lines
        
        # Open the output file and process the Herwig paths
        with open(output_file, 'w') as outfile:
            total_tasks = len(lines_to_process)
            athMsgLog.info(f"Processing {total_tasks} Herwig paths")
            results = run_herwig_get_commands(lines_to_process, phase="paths")
            
            # Sort results to maintain original order
            results.sort(key=lambda x: lines_to_process.index(x[0]) if x[0] in lines_to_process else float('inf'))
            
            # Write results to the output file
            for line, output in results:
                if not line:
                    continue
                outfile.write(f"{line}\n{output}\n\n")
                athMsgLog.debug(f"Success! Parsed the options for: {line}")
        
        athMsgLog.info(f"Results saved to {output_file} (processed {len(lines_to_process)} lines)")
        
    except Exception as e:
        athMsgLog.info(f"Error: An error occurred during execution: {str(e)}")

def process_valid_interfaces(input_file="HerwigConfigDecoder_InterfaceDump.txt", output_file="HerwigConfigDecoder_ConfigParameters.txt"):
    """Process valid interfaces for each /Herwig/ path, execute 'get' commands, and save results.
    
    Args:
        input_file (str): Path to the input file containing Herwig paths and interfaces (default: 'HerwigConfigDecoder_InterfaceDump.txt').
        output_file (str): Path to the output file for interface results (default: 'HerwigConfigDecoder_ConfigParameters.txt').
    """
    try:
        # Check if Herwig path is set
        herwig_path_env = os.getenv('HERWIG7_PATH')
        if not herwig_path_env:
            athMsgLog.error("Error: Environment variable HERWIG7_PATH is not set")
            return
        
        # Read the input file and process each block of Herwig paths and interfaces
        with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
            content = infile.read()
            blocks = content.split('\n\n')
            
            all_tasks = []
            block_paths = []
            errors = []  # Buffer for "No valid interfaces" errors
            seen_paths = set()  # Track unique paths to avoid duplicates
            for block in blocks:
                lines = block.split('\n')
                if not lines or not lines[0].startswith('/Herwig/'):
                    continue
                
                herwig_path = lines[0].strip()
                if herwig_path in seen_paths:
                    athMsgLog.debug(f"Skipped duplicate path in herwig_results.txt: {herwig_path}")
                    continue
                seen_paths.add(herwig_path)
                block_paths.append(herwig_path)
                # Extract valid interfaces from the block
                valid_interfaces = [
                    line.strip()[2:] for line in lines[1:]
                    if line.strip().startswith('* ')
                ]
                
                if not valid_interfaces:
                    errors.append((herwig_path, f"No valid interfaces found for {herwig_path}"))
                    outfile.write(f"{herwig_path}\nNo valid interfaces found\n\n")
                    continue
                
                all_tasks.extend([(herwig_path, interface) for interface in valid_interfaces])
            
            if not all_tasks:
                athMsgLog.info("No interfaces to process")
                # Log any buffered errors before returning
                for _, error_msg in errors:
                    athMsgLog.debug(error_msg)
                return
            
            # Process all interfaces in a single shell session
            athMsgLog.info(f"Processing {len(all_tasks)} interfaces")
            results = run_herwig_get_commands(all_tasks, phase="interfaces")
            
            # Log buffered errors
            for _, error_msg in errors:
                athMsgLog.debug(error_msg)
            
            # Organize results by Herwig path
            path_results = {}
            for herwig_path, get_command, filtered_output in results:
                if herwig_path not in path_results:
                    path_results[herwig_path] = []
                path_results[herwig_path].append((get_command, filtered_output))
            
            # Write results to the output file
            for herwig_path in block_paths:
                if herwig_path not in path_results:
                    continue
                athMsgLog.debug(f"Processing interfaces for {herwig_path}:")
                outfile.write(f"{herwig_path}\n")
                for get_command, filtered_output in path_results[herwig_path]:
                    athMsgLog.debug(f"{get_command} ::: {filtered_output}")
                    outfile.write(f"{get_command} ::: {filtered_output}\n")
                outfile.write("\n")
        
        athMsgLog.info(f"Interface results saved to {output_file}")
        
    except FileNotFoundError:
        athMsgLog.info(f"Error: Input file '{input_file}' not found")
    except Exception as e:
        athMsgLog.info(f"Error: An error occurred during interface processing: {str(e)}")

def remove_duplicate_blocks(input_file="HerwigConfigDecoder_ConfigParameters.txt"):
    """Remove duplicate blocks in the configuration parameters file, keeping the first occurrence.
    
    Args:
        input_file (str): Path to the input file to process (default: 'HerwigConfigDecoder_ConfigParameters.txt').
    """
    try:
        # Read the input file and split into blocks
        with open(input_file, 'r') as infile:
            content = infile.read()
            blocks = content.rstrip().split('\n\n')
        
        # Keep the first block for each unique Herwig path
        path_blocks = {}
        for block in blocks:
            lines = block.split('\n')
            if not lines or not lines[0].startswith('/Herwig/'):
                continue
            herwig_path = lines[0].strip()
            if herwig_path not in path_blocks:
                path_blocks[herwig_path] = block
            else:
                athMsgLog.debug(f"Removed duplicate block for {herwig_path}")
        
        # Collect unique blocks in their original order
        unique_blocks = []
        seen_paths = set()
        for block in blocks:
            lines = block.split('\n')
            if not lines or not lines[0].startswith('/Herwig/'):
                continue
            herwig_path = lines[0].strip()
            if herwig_path not in seen_paths:
                seen_paths.add(herwig_path)
                unique_blocks.append(path_blocks[herwig_path])
        
        # Write the unique blocks back to the file
        with open(input_file, 'w') as outfile:
            outfile.write('\n\n'.join(unique_blocks) + '\n')
        
        athMsgLog.info(f"Removed duplicate blocks in {input_file}")
        athMsgLog.info(f"Retained {len(unique_blocks)} unique blocks")
        
    except FileNotFoundError:
        athMsgLog.info(f"Error: Input file '{input_file}' not found")
    except Exception as e:
        athMsgLog.info(f"Error: An error occurred while removing duplicate blocks: {str(e)}")

def DecodeRunCard(input_file='Herwig.run', skip_particles=False, skip_decays=False):
    """Main function to decode a Herwig run card by extracting paths, executing commands, and processing interfaces.
    
    Args:
        input_file (str): Path to the Herwig run card file (default: 'Herwig.run').
        skip_particles (bool): If True, skip particle-related lines.
        skip_decays (bool): If True, skip decay-related lines.
    """
    athMsgLog.info("Hello from the config-decoder!")
    
    # Step 1: Extract Herwig configuration lines
    herwig_lines = extract_herwig_lines(
        input_file=input_file,
        skip_particles=skip_particles,
        skip_decays=skip_decays
    )
    
    # Step 2: Execute Herwig 'get' commands for the extracted paths
    if herwig_lines:
        execute_herwig_get(herwig_lines)
    
    # Step 3: Process valid interfaces for the paths
    process_valid_interfaces()
    
    # Step 4: Remove any duplicate blocks in the final output
    remove_duplicate_blocks()
    
    athMsgLog.info("Goodbye from the config-decoder!")