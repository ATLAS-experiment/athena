
# VP1 (Virtual Point 1) - The ATLAS 3D event display

### Introduction 

VP1  is the 3D event display for ATLAS. VP1 is the acronym of "or Virtual Point 1", being "Point 1" the site at CERN where the ATLAS experiment is.

VP1 is a framework, there are:

    * base packages for common functions and interfaces (VP1Base, VP1Gui, ...)
    * system packages implementing access to specific data collections  and systems and to handle all the information  from them (VP1GeometrySystems, VP1AODSystems, VP1TrackSystems, ...) 
    * plugin packages which define the usage and the visualization of systems (VP1GeometryPlugin, VP13DCocktailPlugin, ...)
    
    
### Documentation

Main website: https://atlas-vp1.web.cern.ch/atlas-vp1/home/

A hands-on tutorial focused on producing beautiful and informative event displays for physics papers: https://atlas-vp1.web.cern.ch/atlas-vp1/doc_new/tutorial_physics/index.html

The VP1 Blog, a collection of useful tips & tricks: https://atlas-vp1.web.cern.ch/atlas-vp1/blog/

User's support: for all requests and reports, feel free to file a JIRA ticket on the VP1 JIRA project: https://its.cern.ch/jira/projects/ATLASVPONE/summary

Mailing list - feel free to send us any questions about VP1 and related topics: https://atlas-vp1.web.cern.ch/atlas-vp1/home/contacts/

 
### Event display approvals

For event displays which are not part of any physics paper and which you want to use for public usage (press release, multimedia, conference, talk, ...), you can ask an express approval on this dedicated JIRA project: https://its.cern.ch/jira/projects/ATLDISPLAY/summary


### Temporary workaround for remote snapshot rendering

Some X11/GLX servers report a zero `GLX_MAX_PBUFFER_PIXELS` value. Affected
Coin versions then reduce the offscreen dimensions to an invalid size, so VP1
cannot save snapshots or compose rendered views. A pixel-limit-only shim has
restored correct snapshots in our SSH/XQuartz setup. The permanent Coin fix is
also proposed for AthenaExternals in
[MR 1490](https://gitlab.cern.ch/atlas/atlasexternals/-/merge_requests/1490).

The optional launcher below works with an existing Athena installation; no
AthenaExternals or Coin rebuild is needed.

1. Use an Athena release or local VP1 build containing this launcher change.
   For a local build, rebuild/install VP1Algs and source the build setup again.
   The support files are installed under `share/VP1Algs/offscreen-workaround`.

2. Connect with your usual working X11 forwarding (for example `ssh -Y HOST`)
   and set up Athena and your VP1 build as usual. Run VP1 inside its Linux
   environment, including the container or VM if applicable. Start from a
   shell without earlier experimental shims or custom Coin library overrides.

3. Add the option to your normal command:

   ```bash
   vp1 --fix-snapshots
   vp1 --fix-snapshots /path/to/events.pool.root
   ```

   Before importing Athena, VP1 compiles and caches the shim, then restarts
   itself with the process-local preload. Other arguments are preserved.
   The first launch requires a C compiler (`${CC:-cc}`) and OpenGL/X11 headers
   inside Linux, not on the macOS display client. On EL9/Fedora the header
   packages are typically `mesa-libGL-devel` and `libX11-devel`.
   `CC` must name a single compiler executable. Supported targets are Linux
   glibc on aarch64 and x86_64. Help remains available without compiling.

4. Save a snapshot and verify that the file contains the expected image.
   Also test image composition if you use it. To keep the terminal output:

   ```bash
   vp1 --fix-snapshots 2>&1 | tee vp1-workaround.log
   ```

   The launcher prints `enabled for this VP1 process`. When a zero query is
   changed, the shim prints `GLX_MAX_PBUFFER_PIXELS: 0 -> 16777216` once.
   This confirms interception; inspect the image to verify successful rendering.

For an older release without the new option, copy both files from
[share/offscreen-workaround](VP1Algs/share/offscreen-workaround) into one
Linux directory and run `bash /path/to/vp1-with-glx-workaround` with your
usual VP1 arguments. No AthenaExternals rebuild is needed in either case.

The shim changes only successful zero-valued `GLX_MAX_PBUFFER_PIXELS` query
results to a synthetic `4096 * 4096` value, matching Coin's existing maximum
tile dimensions. This is a temporary reporting workaround, not a guarantee
of available GPU memory or pbuffer capacity. Positive limits, width/height
queries, and context creation arguments are forwarded unchanged. It does
not force direct rendering, change EGL selection, or fix other rendering
failures. The runtime test above was performed on aarch64; other systems
should verify their output in the same way.

The launcher sets `LD_PRELOAD` only for VP1 and its child processes, preserving
any existing preload list. It does not modify the installed libraries or your
shell setup. The compiled library is cached under
`${XDG_CACHE_HOME:-$HOME/.cache}/vp1-glx-workaround`. For a home directory that
cannot load shared libraries, choose an executable local filesystem:

```bash
VP1_GLX_WORKAROUND_CACHE_DIR=/path/to/private/local-cache \
  vp1 --fix-snapshots
```

Once your release includes the Coin fix, launch ordinary `vp1` again. You can
then delete its cache directory; the installed support files can stay in place. If snapshots
still fail with this workaround, keep the log and report the failure rather
than adding the earlier force-direct shim on top.
