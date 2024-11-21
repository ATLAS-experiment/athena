#!/bin/env python3
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Original Author: davide.gerbaudo@gmail.com, Jul 2017
# Modified by edward.moyse@cern.ch, Dec 2020
#

"""Create merge request lists for releases and sweeps. Supports two modes:
 - Release mode [default], to create release notes for a release built from nightly:
   > prepare_release_notes.py release/22.0.82 nightly/22.0/2022-08-02T2101
 - Sweep mode, to create the MR diff of the currently checked out branch:
   > prepare_release_notes.py --sweep

"""

from collections import defaultdict
from datetime import datetime
import subprocess
import re
import os
import argparse
import itertools
from functools import cache

gitlab_available = True
try:
    import gitlab
except ImportError:
    gitlab_available = False

repo = 'atlas/athena'  # change this for testing

def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-p', '--previous',
                        help='previous release wrt. which we diff')
    parser.add_argument('-o', '--output', nargs='?', metavar='FILE', const='release_notes.md',
                        help='write release notes to file [%(const)s]')
    parser.add_argument('-r', '--relaxed', action='store_true',
                        help='do not stop on dubious configurations')
    parser.add_argument('-s', '--sweep', action='store_true',
                        help='prepare notes for a sweep')
    parser.add_argument('-v', '--verbose',
                        action='store_true', help='print more info')
    parser.add_argument('--group-merge-requests', action='store_true',
                        help='Group merge requests with the same labels together.')
    parser.add_argument(
        '-t', '--token', help='Optionally pass a gitlab token to get more information.')
    parser.add_argument('target', nargs='?', help='Target release')
    parser.add_argument('nightly', nargs='?', help='Nightly tag to use')
    args = parser.parse_args()

    if args.token:
        if not gitlab_available:
            print('WARNING - passing a token but was not able to import gitlab. You probably need to setup python-gitlab first (i.e. lsetup gitlab) or install it locally (see https://python-gitlab.readthedocs.io)')
            exit(1)

    if args.sweep:
        target_release = ''         # not used
        # The base for the release notes is the parent of the last merge commit.
        # Usually this should be (HEAD^) but in case commits had to be reverted it
        # could be further back in the history.
        last_merge = get_command_output("git log --pretty=format:%H --merges -n 1")['stdout'].decode('ascii')
        previous_release = f'{last_merge}^'
        nightly_tag = 'HEAD'        # current HEAD
    else:
        if args.target is None or args.nightly is None:
            parser.error('target and nightly are required in release mode')

        target_release = args.target
        nightly_tag = args.nightly
        sanitize_args(target_release, nightly_tag, keep_going=args.relaxed)
        previous_release = guess_previous_and_check(
            target_release=target_release) if not args.previous else args.previous

    verbose = args.verbose
    pretty_format = '%b'  # perhaps some combination of '%s%n%b' ?
    cmd = "git log "+previous_release+".."+nightly_tag + \
        " --pretty=format:'"+pretty_format+"' --merges"+" --grep='See merge request'"
    if verbose:
        print("Executing:")
        print(cmd)
    output_log = get_command_output(cmd)
    if (output_log['returncode'] > 0):
        print("Git failed with: {}".format(output_log['stderr']))
        print('Are you running this script from within the athena/ directory?')
        exit(1)
    # If possible, use Gitlab
    gl = None
    gl_project = None
    if args.token and gitlab_available:
        if verbose: print('Trying to connect to gitlab using the supplied token')
        gl = gitlab.Gitlab("https://gitlab.cern.ch", args.token)
        # Check that the token is valid before we get further
        try:
            gl.auth()
        except Exception as err:
            print(f"Authentication failed. {err=}, {type(err)=}")
            exit(1)
        gl_project = gl.projects.get(repo)

    print('Retrieving the list of MRs (run with --verbose to get more output while this is happening).')
    merged_mrs = parse_mrs_from_log(output_log['stdout'].decode("utf-8"),
                                    pretty_format=pretty_format, verbose=verbose, gl_project=gl_project)
    
    # Now handle adding a descriptive message/jira link to release notes
    optional_message = ''
    if not args.sweep:
        print()
        print('Is there a ticket associated with the release build request e.g. ATLINFR-XXXX? (press return to skip)')
        ticket = input(': ')
        print('Do you wish to add a description of the release?')
        print('e.g. "Release for derivations and upgrade", or "Production release for data-taking. ')
        print('(press return to skip)')
        optional_message = input(': ')
        if ticket:
            optional_message = optional_message + '\nRelease request ticket: '+ticket

    release_notes = fill_template(sweep_template() if args.sweep else default_template(),
                                  target_release, nightly_tag, optional_message, previous_release,
                                  merged_mrs, output_filename=args.output, verbose=verbose,
                                  gl=gl, group_mrs=args.group_merge_requests)

    print()
    # Use gitlab api to create a release  (and tag) in gitlab
    if not args.sweep and args.token and gitlab_available:
        msg = 'Would you like to create the release in gitlab (i.e. make the tag and fill in the release notes)?'
        if input("%s (y/N) " % msg).lower() == 'y':
            try:
                gl_project.tags.create({'tag_name':target_release, 'ref':nightly_tag, 'message':optional_message+''})
                release = gl_project.releases.create({'name':target_release, 'tag_name':target_release, 'description':release_notes})
            except Exception as err:
                print(f"Failed to create tag or release. {err=}, {type(err)=}")
                exit(1)
            print(f'Created the following release: https://gitlab.cern.ch/{repo}/-/releases/{release.name}')

    # Create a Draft sweep MR in GitLab
    if args.sweep and args.token and gitlab_available:
        # Warn in case one of the merged MRs has the sweep:ignore label

        sweep_ignore = [mr for mr in merged_mrs if 'sweep:ignore' in mr.labels]
        if sweep_ignore:
            print('*'*80)
            print('WARNING - the following MRs have the sweep:ignore label attached')
            for mr in sweep_ignore:
                print(f'  { mr.web_url} [branch: {mr.source_branch}]')
            print()
            print('Check the MRs and if needed revert them before proceeding:')
            for mr in sweep_ignore:
                print(' ', get_revert_cmd(mr.source_branch))
            print('  git push origin')
            print('  # Then rerun prepare_release_notes.py, ignore this message and edit the MR description as needed')
            print('*'*80)
            print()

        current_branch = subprocess.check_output("git rev-parse --abbrev-ref HEAD",
                                                 shell=True).decode('ascii').strip()
        msg = f'Would you like me to create the sweep MR in gitlab from "{current_branch}"?'
        if input("%s (y/N) " % msg).lower() == 'y':
            # Get user fork (e.g. https://:@gitlab.cern.ch:8443/fwinkl/athena.git
            userid = subprocess.check_output("git remote get-url origin",
                                             shell=True).decode('ascii').split('/')[-2]
            gl_fork = gl.projects.get(f"{userid}/athena")  # user fork
            # Find the last merge commit, e.g. "Merge remote-tracking branch 'upstream/23.0' into ..."
            last_merge = subprocess.check_output("git log --merges -n1 --pretty='format:%B'",
                                                 shell=True).decode('ascii')
            try:
                source_branch = last_merge.split("'")[1].split('/')[1]
            except IndexError:
                source_branch = 'UNKNOWN'

            title = datetime.now().strftime("%Y-%m-%d") + f": merge of {source_branch} into main"
            mr = gl_fork.mergerequests.create({'source_branch' : current_branch,
                                               'target_branch' : 'main',
                                               'target_project_id' : gl_project.id,
                                               'title' : 'Draft: '+title,
                                               'description' : release_notes,
                                               'remove_source_branch' : True,
                                               'squash' : False})
            print()
            print('Draft MR has been created:', mr.web_url)


def sanitize_args(target_release, nightly_tag, keep_going=False):
    if not target_release.startswith('release/'):
        print("The target release should start with 'release/', you are using:\n%s" %
              target_release)
        if not keep_going:
            raise RuntimeWarning('Fix target release')
    if not nightly_tag.startswith('nightly/'):
        print(
            "The nightly tag should start with 'nightly/', you are using:\n%s" % nightly_tag)
        if not keep_going:
            raise RuntimeWarning('Fix nightly tag')

    rel_match = re.search(
        r'release/(?P<ver>\d+)\.(?P<maj>\d+?)\.(?P<rev>\d+?)', target_release)
    nig_match = re.search(
        r'nightly/(?P<branch>((\d+\.\d+)|master|main|22\.0-mc20))/(?P<date>\d{4}-\d{2}-\d{2})T(?P<time>\d{4})', nightly_tag)

    if not rel_match:
        print("The target release is not formatted as xx.y.z (version.major.revision semantic)")
        if not keep_going:
            raise RuntimeWarning('Fix formatting target release')
    elif not nig_match:
        print("The nightly tag is not formatted as expected, nightly/branch/yyyy-mm-ddThhmm\n%s" % nightly_tag)
        if not keep_going:
            raise RuntimeWarning('Fix formatting nightly tag')
    else:
        branch_rel = '.'.join([rel_match.group('ver'), rel_match.group('maj')])
        branch_nig = nig_match.group('branch')
        if branch_rel not in (branch_nig,'22.0') and branch_nig not in ('main','master'):
            print("You are creating a tag for %s from a nightly from %s" %
                  (branch_rel, branch_nig))
            if not keep_going:
                raise RuntimeWarning('Create a tag for the correct branch')


def guess_previous_and_check(target_release='release/xx.y.z'):
    version_major_revision = target_release.split('.')
    revision = version_major_revision[-1] if len(
        version_major_revision) > 2 else None
    missing_revision = revision is None or not str(revision).isdigit()
    if missing_revision or int(revision) == 0:
        perhaps_previous = target_release[:-1]+'n-1'
        raise RuntimeWarning("Cannot guess previous release for '%s'" % target_release
                             + '\nPlease use something like'
                             + "\n --previous %s" % perhaps_previous)
    else:
        previous_revision = int(revision)-1
        version_major_previousrevision = version_major_revision[:-1] + [
            str(previous_revision)]
        previous_revision = '.'.join(version_major_previousrevision)
    return previous_revision


def get_command_output(command, with_current_environment=False):
    # lifted from supy (https://github.com/elaird/supy/blob/master/utils/io.py)
    env = None if not with_current_environment else os.environ.copy()
    p = subprocess.Popen(
        command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env)
    stdout, stderr = p.communicate()
    return {"stdout": stdout, "stderr": stderr, "returncode": p.returncode}


def get_revert_cmd(branch):
    """Get git revert commands required to revert the given branch name"""
    # Find the merge commit that mentions the branch name
    commit = get_command_output(f"git log --pretty=format:%H --grep \"^Merge branch '{branch}'\"")['stdout'].decode('ascii')
    return f"git revert -m1 {commit}"


class MergeRequestInfo(object):
    def __init__(self, mr=None, one_liner=''):
        self.mr = mr
        self.one_liner = one_liner

    def __str__(self):
        return (" ---- : %s" % self.one_liner if self.mr is None else
                "!%d : %s" % (int(self.mr), self.one_liner))

    def init_from_gitlab_message_lines(self, mr_description_lines=[], verbose=False):
        """Try parsing the commit msg we usually get for a gitlab MR.

        This is usually a few lines that look like:

           <some description, which can be multi-line>

           See merge request [optional branch] !<some MR ID>
        """
        if verbose:
            print("parsing\n", mr_description_lines)
        lines = mr_description_lines
        see_mr_lines = [l for l in lines if l.startswith('See merge request')]
        if len(lines) >= 2 and see_mr_lines:
            self.one_liner = lines[0]
            mr_match = re.search(
                r'See merge request.*!(?P<mr>\d+).*', see_mr_lines[-1])  # use last match in case multiple "See ..." lines
            self.mr = mr_match.group('mr') if mr_match else None
            if verbose:
                print(self.__str__())
        else:
            print("WARNING: Cannot parse these lines:\n" +
                  '\n'.join("[%02d] : '%s'" % (iL, l) for iL, l in enumerate(lines)))
        return self


def parse_mrs_from_log(output, pretty_format='%b', verbose=False, gl_project=None):

    if pretty_format != '%b':
        raise RuntimeWarning("parsing of pretty_format %s not implemented yet")
    mrs = []
    lines_this_mr = []

    for line in output.split('\n'):
        line = line.lstrip().strip()
        if line.startswith('See merge request'):
            lines_this_mr.append(line)
            mrs.append( MergeRequestInfo().init_from_gitlab_message_lines(lines_this_mr, verbose) )
            lines_this_mr = []
        elif line:  # skip empty
            lines_this_mr.append(line)

    # If GitLab is available return list of GitLab MRs
    if gl_project:
        gl_mrs = []
        # We retrieve the MRs in batches for speed. In principle the gitlab API should be able
        # to return the full set with one request but it seems the long list of iids then causes
        # a 502 error on the CERN web server. So we go with a reasonable batch size.
        iids = [int(m.mr) for m in mrs]
        iterator = iter(iids)
        while batch := list(itertools.islice(iterator, 25)):
            # Retrieve MRs. But order gets lost, so we sort them in the original order.
            r = sorted(gl_project.mergerequests.list(iids=batch, get_all=True),
                       key=lambda m: iids.index(m.iid))
            gl_mrs += r
        return gl_mrs
    else:
        return mrs


def default_template():
    return \
"""# Release notes for {target_release:s}
The release {target_release_link:s}
was built from the tag {nightly_tag_link:s}

{optional_message:s}

This is the list of merge requests that were included since
the previous release {previous_release_link:s}:
{formatted_list_of_merge_requests:s}

Link to the full diff between {target_release_link:s} and
{previous_release_link:s}
is available at
https://gitlab.cern.ch/{repo:s}/compare/{previous_release:s}...{target_release:s}
"""

def sweep_template():
    return \
"""This sweep contains the following MRs:
{formatted_list_of_merge_requests:s}
"""

def format_mrs_from_gitlab(merged_mrs, group_mrs=False, gl=None):

    @cache
    def allowed_labels():
        """Read domain labels from the CI repository"""
        gl_project = gl.projects.get("atlas-sit/CI")
        domains_py = gl_project.files.raw("data/domain_map.py", "master")
        namespace = {}
        exec(domains_py, namespace)
        labels = set(namespace['DOMAIN_MAP'])

        # Add/remove some labels
        labels.discard('full-unit-tests')
        labels.add('frozen-tier0-violating')
        labels.add('sweep:ignore')
        return labels

    def allowed_label(label):
        """Check if given label should be shown in release notes"""
        allowed_labels_regex = [re.compile('changes-.*'), re.compile('.*-output-changed')]
        return label in allowed_labels() or any(regex.match(label) for regex in allowed_labels_regex)

    lines = []
    if group_mrs:
        grouped_mrs = defaultdict(list)
        for mr in merged_mrs:
            labels = ['~'+label for label in mr.labels if allowed_label(label)]
            label_key = ", ".join(labels)
            grouped_mrs[label_key].append(
                '   * !{} : {}'.format(mr.get_id(), mr.title))
        for key, value in grouped_mrs.items():
            lines.append(' * {}'.format(key))
            for title in value:
                lines.append(title)
    else:
        for mr in merged_mrs:
            labels = ['~'+label for label in mr.labels if allowed_label(label)]
            lines.append(" * !{id} {title} {labels}".format(id=mr.get_id(),
                         title=mr.title, labels=", ".join(labels)))
    return '\n'.join(lines)


def fill_template(template, target_release, nightly_tag, optional_message, previous_release,
                  merged_mrs=[], output_filename='foo.md', verbose=False, gl=None, group_mrs=False):
    formatted_mrs = ""
    if gl:
        formatted_mrs = format_mrs_from_gitlab(merged_mrs, group_mrs, gl=gl)
    else:
        formatted_mrs = '\n'.join(
            [" * %s" % mr for mr in merged_mrs]) if merged_mrs else '* None'

    def formatted_tag_link(tag=''):
        base_url = f'https://gitlab.cern.ch/{repo}/tags'
        return "[%s](%s)" % (tag, base_url+'/'+tag)

    filled_template = template.format(**{'target_release': target_release,
                                         'target_release_link': formatted_tag_link(target_release),
                                         'nightly_tag': nightly_tag,
                                         'nightly_tag_link': formatted_tag_link(nightly_tag),
                                         'optional_message': optional_message,
                                         'repo' : repo,
                                         'previous_release': previous_release,
                                         'previous_release_link': formatted_tag_link(previous_release),
                                         'formatted_list_of_merge_requests': formatted_mrs})
    if output_filename:
        with open(output_filename, 'w') as f:
            f.write(filled_template)
            print("Release notes stored in '%s'" % output_filename)
    return filled_template

if __name__ == '__main__':
    main()
