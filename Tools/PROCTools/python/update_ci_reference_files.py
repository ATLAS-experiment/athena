#!/bin/env python3
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


"""Updates reference files for a given MR, as well as related files (digest ref files, References.py)

This script should be run in the root directory of the athena repository, 
and you should pass in the URL of "CI Builds Summary" page for the MR you are interested in.
i.e. the link that you get from the MR under "Full details available on <this CI monitor view>"

So, for example, if you are interested in MR 66303, you would run this script as follows:
Tools/PROCTools/scripts/update_ci_reference_files.py https://bigpanda.cern.ch/ciview/?rel=MR-63410-2023-10-09-12-27

Running with --test-run will modify local files (so you can test that the changes make sense), and will also print out the commands which would have been executed. Nothing remote is changed! 
This is a good way to check that the proposed changes look rational before actually making in earnest.
"""

from collections import defaultdict
import os
import sys
import subprocess
import re
import argparse
try:
    import requests 
except ImportError:
    print('FATAL: this script needs the requests module. Either install it yourself, or run "lsetup gitlab"')

class CITest:
    def __init__(self, name, tag, mr, date, existing_ref, existing_version, new_version, new_version_directory, copied_file_path, digest_old, digest_new, type):
        self.name = name
        self.tag = tag
        self.mr = mr
        self.date = date
        self.existing_ref = existing_ref
        self.existing_version = existing_version
        self.new_version = new_version
        self.new_version_directory = new_version_directory
        self.copied_file_path = copied_file_path
        self.digest_old = digest_old
        self.digest_new = digest_new
        self.type = type
    
    def __repr__(self):
        return f'<CI Test: {self.name} tag: {self.tag} MR: {self.mr} date: {self.date} type: {self.type}>'

    def __str__(self):
        extra = ''
        if self.type == 'DiffPool':
            extra = f' Data file change :  {self.existing_version} -> {self.new_version}'
        elif self.type == 'Digest':
            extra = f' Digest change: {self.existing_ref}'
        return f'{self.name}:{self.tag} MR: {self.mr}'+extra

failing_tests = defaultdict(list) # Key is branch, value is list of CITest objects
dirs_created=[] #Used later to ensure we don't try to create the same directory twice
debug = False

def process_log_file(url, branch, test_name):
    """So now we have a URL to a failing test.
    We need to check that the test is failing for the correct reason - namely a reference file which needs updating
    The information we need to collect is:
    - the AMI tag of the failing tests
    - the merge request number
    - the location of the reference file
    - the location of the copied file
    - the name of the test
    - the new version number
    - the new version directory
    """
    page = requests.get(url)
    text = page.text

    # First check that this looks like a test whose ref files need updating, bail otherwise
    # INFO     All q442 athena steps completed successfully
    test_match = re.search(r'All (?P<ami_tag>\w+) athena steps completed successfully', text)
    ami_tag = test_match.group('ami_tag') if test_match else None

    # We have two types of tests, but lets try to extract some common information
    if not ami_tag:
        # Okay, maybe it was truncated? Try again.
        match_attempt_2 = re.search(r'AMIConfig (?P<ami_tag>\w+)', text)
        if match_attempt_2:
            ami_tag = match_attempt_2.group('ami_tag')
    
    if not ami_tag:
       print('WARNING: Did not find an AMI tag in the test "{}". Ignoring.'.format(test_name))
       return

    mr_match = re.search(r'NICOS_TestLog_MR-(?P<mr_number>\d+)-(?P<date>\d{4}-\d{2}-\d{2}-\d{2}-\d{2})', url)
    if not mr_match:
        print('FATAL: Could not process the URL as expected. Aborting.')
        print(url)
        sys.exit(1)

    mr_number = mr_match.group('mr_number')
    date = mr_match.group('date')
    human_readable_date = ':'.join(date.split('-')[0:3]) + " at " + ':'.join(date.split('-')[3:])

    if "Your change breaks the digest in test" in text:
        # Okay, we have a digest change
        failing_tests[branch].append(process_digest_change(text, ami_tag, mr_number, human_readable_date, test_name))

    if 'ERROR    Your change breaks the frozen tier0 policy in test' in text:
        failing_tests[branch].append(process_diffpool_change(text, ami_tag, mr_number, human_readable_date, test_name))

    if 'ERROR    Your change breaks the frozen derivation policy in test' in text:
        failing_tests[branch].append(process_diffpool_change(text, ami_tag, mr_number, human_readable_date, test_name))
    
    return

def process_diffpool_change(text, ami_tag, mr_number, human_readable_date, test_name):
    eos_path_root = '/eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/'

    # Copied file path
    # e.g. from ERROR    Copied '../SimulationRun3FullSim/run_s4006/myHITS.pool.root' to '/eos/atlas/atlascerngroupdisk/proj-sit/gitlabci/MR63410_a84345c776e93f0d7f25d00c9e91e35bcb965d09/SimulationRun3FullSimChecks'
    copied_file_match = re.search(r'^ERROR    Copied.*', text, flags=re.MULTILINE)
    if not copied_file_match:
        print("FATAL: Could not find matching copied file")
        sys.exit(1)
    copied_file_path = copied_file_match.group().split('to')[1].strip().strip("'")+'/'

    # Reference file paths
    ref_file_match = re.search(r'INFO     Reading the reference file from location.*', text)
    if not ref_file_match:
        print("FATAL: Could not find matching reference file")
        sys.exit(1)

    ref_file_path = ref_file_match.group().split('location')[1].strip()
    existing_version_number= ref_file_path.split('/')[-2]
    branch = ref_file_path.split('/')[-4]
    new_version_number = 'v'+str(int(existing_version_number[1:])+1)
    new_version_directory = eos_path_root+branch+'/'+ami_tag+'/'+new_version_number 
    old_version_directory = eos_path_root+branch+'/'+ami_tag+'/'+existing_version_number 
    # Copied file path
    # e.g. from ERROR    Copied '../SimulationRun3FullSim/run_s4006/myHITS.pool.root' to '/eos/atlas/atlascerngroupdisk/proj-sit/gitlabci/MR63410_a84345c776e93f0d7f25d00c9e91e35bcb965d09/SimulationRun3FullSimChecks'
    copied_file_match = re.search(r'^ERROR    Copied.*', text, flags=re.MULTILINE)
    if not copied_file_match:
        print("FATAL: Could not find matching copied file")
        sys.exit(1)
    
    # Sanity checks
    ami_tag_check = ref_file_path.split('/')[-3].strip()
    if ami_tag_check!=ami_tag:
        print('FATAL: Sanity check: "{}" from reference file path "{}" does not match ami tag "{}" extracted previously.'.format(ami_tag_check, ref_file_path, ami_tag))
        sys.exit(1)


    test = CITest(name=test_name, tag=ami_tag, mr=mr_number, date=human_readable_date, existing_ref = old_version_directory, existing_version = existing_version_number, new_version = new_version_number, new_version_directory = new_version_directory, copied_file_path = copied_file_path, digest_old=None, digest_new=None, type='DiffPool')
    return test

def process_digest_change(text, ami_tag, mr_number, human_readable_date, test_name):    
    # Some things aren't so relevant for digest changes
    existing_version_number = None
    new_version_directory = None
    copied_file_path = None
    new_version_number=None

    #  differs from the reference 'q447_AOD_digest.ref' (<):
    ref_file_match = re.search(r'(.*differs from the reference \')(.*)(\')', text)
    if not ref_file_match:
        print("FATAL: Could not find matching reference file")
        sys.exit(1)
    ref_file_path = ref_file_match.groups()[1]

    old_diff_lines = []
    new_diff_lines = []
    diff_started = False # Once we hit the beginning of the diff, we start recording
    # Diff starts with e.g. 
    # ERROR    The output 'q449_AOD_digest.txt' (>) differs from the reference 'q449_AOD_digest.ref' (<):
    # and ends with next INFO line

    for line in text.split('\n'):
        if 'differs from the reference' in line:
            # Start of the diff
            diff_started = True
        elif diff_started:
          if line.startswith('&lt;'):
              old_diff_lines.append(line)
          elif line.startswith('&gt;'):
              new_diff_lines.append(line)
          elif 'INFO' in line:
            # End of the diff
            break

    test = CITest(name=test_name, tag=ami_tag, mr=mr_number, date=human_readable_date, existing_ref = ref_file_path, existing_version = existing_version_number, new_version = new_version_number, new_version_directory = new_version_directory, copied_file_path = copied_file_path, digest_old=old_diff_lines, digest_new=new_diff_lines, type='Digest')
    return test

def update_reference_files(actually_update=True, update_local_files=False):
    print
    print('Updating reference files')
    print('========================')
    commands = []
    for branch, tests in failing_tests.items():
        for test in tests:
            print('Processing test: {} on branch {}'.format(test.name, branch))
            if test.type == 'DiffPool':
                print(' * This is a DiffPool test, and currently has version {} of {}. Will update References.py with new version.'.format(test.existing_version, test.tag))
                if actually_update:
                    print(' -> The new version is: {}. Creating directory and copying files on EOS now.'.format(test.new_version))
                    create_dir_and_copy_refs(test, True)
                else:
                    # We will print these later, so we can sanity check them when in test mode
                    commands.extend(create_dir_and_copy_refs(test, False))
                    # Remove any duplicates, whilst preserving the order
                    commands = list(dict.fromkeys(commands))

                # Now, update local References.py file
                if update_local_files:
                    data = []
                    if debug:
                        print ('Updating local References.py file with new version {} for tag {}'.format(test.new_version, test.tag))
                    line_found = False
                    with open('Tools/WorkflowTestRunner/python/References.py', 'r') as f:
                        lines = f.readlines()
                        for line in lines:
                            if test.tag in line:
                                if test.existing_version in line:
                                    line = line.replace(test.existing_version, test.new_version)
                                else:
                                    print('')
                                    print('** WARNING: For tag {} we were looking for existing version {}, but the line in the file is: {}'.format(test.tag, test.existing_version, line), end='')
                                    print('** Are you sure your branch is up-to-date with main? We cannot update an older version of References.py!')
                                line_found = True
                            data.append(line)
                    
                    if not line_found:
                        print('** WARNING - no matching line was found for the AMI tag {} in References.py. Are you sure your branch is up-to-date with main? We cannot update an older version of References.py!'.format(test.tag))
                    
                    with open('Tools/WorkflowTestRunner/python/References.py', 'w') as f:
                        f.writelines(data)
            elif test.type == 'Digest' and update_local_files:
                print(' * This is a Digest test. Need to update reference file {}.'.format(test.existing_ref))
                data = []

                diff_line=0 # We will use this to keep track of which line in the diff we are on
                with open('Tools/PROCTools/data/'+test.existing_ref, 'r') as f:
                    lines = f.readlines()
                    for current_line, line in enumerate(lines):
                        split_curr_line = line.split()
                        if (split_curr_line[0] == 'run'): # Skip header line
                            data.append(line)
                            continue

                        # So, we expect first two numbers to be run/event respectively
                        if (not split_curr_line[0].isnumeric()) or (not split_curr_line[1].isnumeric()):
                            print('FATAL: Found a line in current digest which does not start with run/event numbers: {}'.format(line))
                            sys.exit(1)
                        
                        split_old_diff_line = test.digest_old[diff_line].split()
                        split_old_diff_line.pop(0) # Remove the < character
                        split_new_diff_line = test.digest_new[diff_line].split()
                        split_new_diff_line.pop(0) # Remove the > character

                        # Let's check to see if the run/event numbers match
                        if split_curr_line[0] == split_old_diff_line[0] and split_curr_line[1] == split_old_diff_line[1]:
                            # Okay so run/event numbers match. Let's just double-check it wasn't already updated
                           if  split_curr_line!=split_old_diff_line:
                               print('FATAL: It seems like this line was already changed.')
                               print('Line we expected: {}'.format(test.old_diff_lines[diff_line]))
                               print('Line we got     : {}'.format(line))
                               sys.exit(1)

                        # Check if the new run/event numbers match
                        if split_curr_line[0] == split_new_diff_line[0] and split_curr_line[1] == split_new_diff_line[1]:
                            #Replace the existing line with the new one, making sure we right align within 12 characters
                            data.append("".join(["{:>12}".format(x) for x in split_new_diff_line])+ '\n')
                            if ((diff_line+1)<len(test.digest_old)):
                                diff_line+=1
                            continue

                        # Otherwise, we just keep the existing line
                        data.append(line)               
                        
                print(' -> Updating PROCTools digest file {}'.format(test.existing_ref))
                with open('Tools/PROCTools/data/'+test.existing_ref, 'w') as f:
                    f.writelines(data)
    return commands
                

def create_dir_and_copy_refs(test, actually_update=False):
    """
    If called with actually_update=False, this function will return a list of commands which would have been executed.
    """
    commands = []
    if test.new_version_directory not in dirs_created:
        commands.append("mkdir " + test.new_version_directory)
        dirs_created.append(test.new_version_directory)
                
    # Copy new directory first, then copy old (in case the new MR did not touch all files)
    # Important! Use no-clobber for second copy or we will overwrite the new data with old!
    commands.append("cp " + test.copied_file_path + "* "+ test.new_version_directory+"/")
    commands.append("cp -n " + test.existing_ref + "/* "+ test.new_version_directory+"/")
    if actually_update:
        print(' -> Copying files from {} to {}'.format(test.copied_file_path, test.new_version_directory))
        try:
            for command in commands:
                try:
                    subprocess.call( command, shell=True)
                except Exception as e:
                    print('Command failed due to:', e)
                    print('Do you have EOS available on this machine?') 
        except Exception as e:
            print('FATAL: Unable to copy files due to:', e)
            sys.exit(1)

        f = open(test.new_version_directory+'/info.txt', 'w')
        f.write('Merge URL: https://gitlab.cern.ch/atlas/athena/-/merge_requests/{}\n'.format(test.mr))
        f.write('Date: {}\n'.format(test.date))
        f.write('AMI: {}\n'.format(test.tag))
        f.write('Test name: {}\n'.format(test.name)) 
        f.write('Files copied from: {}\n'.format(test.copied_file_path))
        f.close()

    return commands

def process_CI_Tests_json(data):
    # Each list entry is one column in the table.
    for row in data:
        if ('ERROR' in row[0]):
            process_log_file(strip_url(row[2]), branch = row[1], test_name=strip_href(row[2]))

def strip_url(href):
    url = href[href.find('"')+1:] # Strip everything up to first quotation mark
    url = url[:url.find('"')]
    return url

def strip_href(href):
    value = href[href.find('>')+1:] # Strip everything up to first >
    value = value[:value.find('<')]
    return value

def process_CI_Builds_Summary(project):
    # Each entry is one column in the table. 11th is the tests column.
    # URL to tests page is in form: 
    # <a href="/testsview/?nightly=MR-CI-builds&rel=MR-66303-2023-10-10-19-08&ar=x86_64-centos7-gcc112-opt&proj=AthGeneration">0 (0)</a>
    test_counts = strip_href(project[11])
    # This is e.g. '0 (0)'
    test_error_counts = int(test_counts.split(' ')[0])
    if test_error_counts > 0:
        # Okay, we have an error!
        project_url = 'https://bigpanda.cern.ch'+strip_url(project[11])
        headers = {'Accept': 'application/json'} 
        r = requests.get(project_url+'&json', headers=headers)
        data = r.json()["rows_s"]
        process_CI_Tests_json(data[1:])

def extract_links_from_json(url):
    headers = {'Accept': 'application/json'} 
    r = requests.get(url+'&json', headers=headers)
    data = r.json()["rows_s"]
    # First row is header. 
    # Currently this is: 'Release', 'Platform', 'Project', 'git branch<BR>(link to MR)', 'Job time stamp', 'git clone', 'Externals build', 'CMake config', 'Build time', 'Comp. Errors (w/warnings)', 'Test time', 'CI tests errors (w/warnings)', 'Host'
    for project in data[1:]:
        process_CI_Builds_Summary(project)

def summarise_failing_tests(check_for_duplicates = True):
    print('Summary of tests which need work:')

    if not failing_tests:
        print(" -> None found. Aborting.")
        return None

    mr = None
    reference_folders = []
    for branch,tests in failing_tests.items():
        print (' * Branch: {}'.format(branch))
        for test in tests:
            print('   - ', test)
            if os.path.exists(test.new_version_directory):
                msg = f'WARNING: The directory {test.new_version_directory} already exists. Are you sure you want to overwrite the existing references?'
                if input("%s (y/N) " % msg).lower() != 'y':
                    sys.exit(1)

            if (test.existing_ref not in reference_folders):
                reference_folders.append(test.existing_ref)
            elif check_for_duplicates:
                print('FATAL: Found two tests which both change the same reference file: {}, which is not supported.'.format(test.existing_ref))
                print('Consider running again in --test-run mode, to get a copy of the copy commands that could be run.')
                print('The general advice is to take the largest file (since it will have the most events), and/or take the non-legacy one.')
                sys.exit(1)
            mr = test.mr
    return 'https://gitlab.cern.ch/atlas/athena/-/merge_requests/'+mr

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('url', help='URL to CITest (put in quotes))')
    parser.add_argument('--test-run',help='Update local text files, but do not actually touch EOS.', action='store_true')
    args = parser.parse_args()
    print('Update reference files for URL: {}'.format(args.url))
 
    if not args.url.startswith(('http://', 'https://')):
        print('invalid url - should start with http:// or https://')
        print(args.url)
        print('Aborting.')
        sys.exit(1)
 
    if args.test_run:
        print(' -> Running in test mode so will not touch EOS, but will only modify files locally (these changes can easily be reverted with "git checkout" etc).')
    
    print('========================')
    extract_links_from_json(args.url)
    mr_url = summarise_failing_tests(not args.test_run)
    if not mr_url:
        sys.exit(1)
    print('========================')
    print("The next step is to update the MR with the new content i.e. the References.py file and the digest files.")
    print(" IMPORTANT: before you do this, you must first make sure that the local repository is on same branch as the MR!")
    print("i.e. you would go to the MR: "+mr_url)
    print(" and then copy the branch name and do:")
    print(" $ git remote add <MR_AUTHOR> <URL_TO_FORK>") # TODO - automate this?
    print(" $ git fetch <MR_AUTHOR>")
    print(" $ git switch -c <MR_BRANCH> <MR_AUTHOR>/<MR_BRANCH>")
    print(" $ git rebase upstream/main") # In case there have been any changes since the MR was created
    print()

    msg = 'Would you like to (locally) update digest ref files and/or versions in References.py?'
    update_local_files = False
    if input("%s (y/N) " % msg).lower() == 'y':
        not_in_athena_dir = subprocess.call("git rev-parse --is-inside-work-tree", shell=True)
        if not_in_athena_dir:
            print('FATAL: You must run this script from within the athena directory.')
            sys.exit(1)
        update_local_files = True
        
    commands = update_reference_files(not args.test_run, update_local_files)
    
    if commands and args.test_run:
        print('')
        print(' -> In test-run mode. In normal mode we would also have executed:')
        for command in commands:
            print('    ', command)
    if not args.test_run:
        print("Finished! Before pushing, you might want to manually trigger an EOS to cvmfs copy here: https://atlas-jenkins.cern.ch/view/Install%20(Boeriu)/job/ART_data_eos2cvmfs/")
