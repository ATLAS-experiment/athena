#!/usr/bin/env python3
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Script to be used by release coordinators to merge release branches into main.
# The script performs the following steps:
#
#   1. Fetch remote and create local branch
#   2. `git merge` the source into the target branch
#   3. Restore files that are on the ignore list
#   4. Restore changes from sweep:ignore MRs
#
# Author: Frank Winklmeier
#
"""Merge git source branch into target applying a standard ignore list and
handling sweep:ignore MRs. Run this within a full atlas/athena checkout.
"""

import argparse
import os
import subprocess
import sys
from datetime import datetime

try:
   import gitlab
except ImportError:
   print("'gitlab' module not available (run 'lsetup gitlab')")
   sys.exit(1)

from prepare_release_notes import parse_mrs_from_log

# Files/directories to ignore in merges (can be wildcards)
ignore = ["Projects/",
          "Build/",
          "Tools/WorkflowTestRunner/python/References.py",
          ]

# Global access to command line arguments
args = None


def warn(msg):
   """Print a colored warning"""

   yellow = "\033[33m"
   reset = "\033[0m"
   print(f"{yellow}WARNING: {msg}{reset}")


def git_cmd(cmd, **kwargs):
   """Execute given cmd in shell context"""

   if args.verbose:
      print(f"$ {cmd}")

   kwargs.setdefault("check", True)  # exception on error by default
   return subprocess.run(cmd, shell=True, text=True, **kwargs)


def sweep_ignore_mrs():
   """Handle MRs that are marked with sweep:ignore"""

   # Get merge commits between the two branches
   git_log_mr = git_cmd(f"git log {args.target}..{args.source} "
                        "--pretty=format:%b --merges --grep='See merge request'",
                        encoding="utf-8", stdout=subprocess.PIPE).stdout

   # Retrieve MR information from GitLab
   gl = gitlab.Gitlab("https://gitlab.cern.ch")
   gl_project = gl.projects.get(args.repo)
   merged_mrs = parse_mrs_from_log(git_log_mr, verbose=args.verbose, gl_project=gl_project)

   print("Merging the following MRs:")
   for mr in merged_mrs:
      print(f"  {mr.web_url}")

   sweep_ignore = [mr for mr in merged_mrs if "sweep:ignore" in mr.labels]
   if not sweep_ignore:
      return

   print()
   warn("The following MRs carry the sweep:ignore label:")

   for i, mr in enumerate(sweep_ignore, start=1):
      print(f"{i}) {mr.web_url} [{mr.merge_commit_sha}]")
      name_status = git_cmd(f"git --no-pager log -1 --name-status --first-parent "
                            f"--pretty=format: {mr.merge_commit_sha}", stdout=subprocess.PIPE).stdout

      print(name_status)  # e.g. "M      path/to/my/file"

      # Attach the list of files to the MR object
      mr.diff_status = name_status.splitlines()

   # Ask user for input
   choice = input("Select MRs to ignore during merge: "
                  "a(ll), n(one) or comma-separated list of numbers: ").split(",")

   if choice[0]=="n":
      return

   # Restore selected MRs
   for i, mr in enumerate(sweep_ignore, start=1):
      if choice[0]=="a" or str(i) in choice:
         print(f"Restoring files from {mr.web_url}")
         for s in mr.diff_status:
            restore_file(mr, s)


def restore_file(mr, status):
   """Given a --name-status line, restore the file. The format is:
   A    file       # added
   M    file       # modified
   D    file       # deleted
   R040 orig new   # renamed (with similarity score)
   """

   def git_try_cmd(cmd, filename):
      """Check if filename was modified again after this MR. If not, run cmd."""
      git_diff = git_cmd(f"git diff --quiet {mr.merge_commit_sha}..{args.source} -- {filename}",
                         check=False)
      if git_diff.returncode==0:
         git_cmd(cmd)
      else:
         warn(f"{filename} was modified by another commit. Manual merge needed.")

   def find_rename(filename):
      """Try to find the original name of filename if it was renamed"""
      rename = git_cmd(f"git --no-pager log --name-status --pretty=format: --diff-filter=R "
                       f"{args.source}..{args.target} | grep {filename}", stdout=subprocess.PIPE).stdout

      return rename.split()[2] if rename else None

   status = status.split()
   s = status[0]

   f = None
   if s.startswith("A"):
      # Delete added file
      git_try_cmd(f"git rm -f -- {status[1]}", status[1])
   elif s.startswith("R"):
      # Undo rename and restore original file (below)
      f = status[1]
      git_try_cmd(f"git mv {status[2]} {f}", status[2])
   else:
      f = status[1]

   # Restore the original file
   if f is not None:
      if not os.path.exists(f):
         forig = find_rename(f)
         if forig is not None:
            print(f"{f} no longer exists on {args.target} but found {forig}")
            f = forig
         else:
            warn(f"Cannot restore {f}. Probably because file has been renamed on {args.target}. "
                 "Manual restore is required.")

      if f is not None:
         git_try_cmd(f"git restore -WS --ignore-unmerged "
                     f"--source={args.target} -- {f}", f)


def main():
   global args

   parser = argparse.ArgumentParser(description=__doc__,
                                    formatter_class=argparse.RawDescriptionHelpFormatter)

   parser.add_argument("source", help="source branch (e.g. 24.0)")
   parser.add_argument("target", nargs="?", default="main", help="target branch [%(default)s]")
   parser.add_argument("-v", "--verbose", action="store_true", help="print more info")
   parser.add_argument("--repo", default="atlas/athena", help="repository [%(default)s]")
   parser.add_argument("--remote", default="upstream", help="name of remote [%(default)s]")

   args = parser.parse_args()

   # Fetch remote and create merge branch
   git_cmd(f"git fetch {args.remote}")

   sweep_branch = f"sweep_{args.source}_{args.target}_{datetime.now().strftime('%Y-%m-%d')}"

   # Prepend the remote to source and target branch
   if args.remote:
      args.source = f"{args.remote}/{args.source}"
      args.target = f"{args.remote}/{args.target}"

   git_cmd(f"git checkout -B {sweep_branch} {args.target}")

   # Merge without commit
   git_cmd(f"git merge --no-commit {args.source}", check=False)

   # Restore modified files that are on the global ignore list
   git_cmd(f"git restore -WS --ignore-unmerged "
           f"--source={args.target} -- {' '.join(ignore)}")
   print()

   # Handle sweep:ignore MRs
   sweep_ignore_mrs()

   print(f"""
Now finalize the merge:
 1) git status and resolve any remaining conflicts
    Note: "--theirs" is {args.source}, "--ours" is {args.target}
 2) git commit OR git merge --continue
 3) git push origin
 4) ./Build/AtlasBuildScripts/prepare_release_notes.py --sweep -t GITLAB_TOKEN
""")

   return 0


if __name__ == "__main__":
   sys.exit(main())
