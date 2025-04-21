ATLAS WorkDir Project
=====================

This project can be used to build just a few packages from the repository
against an installed ATLAS release/nightly. That means any base project,
offline, trigger, simulation or analysis.

The project can be used in two ways.

Using a fully checked out repository
------------------------------------

If you have the entire `athena` repository checked out, you need to tell
the CMake configuration which packages you actually want to build. Otherwise
it will go ahead and attempt to build everything. Even packages not part of
the selected base release itself.

You should make a copy of the `package_filters_example.txt` file found in
this directory, and modify it to select the packages you're interested in,
for compilation. There is some documentation in the file explaining its
format.

Then, configure the build of the project like:

    asetup ...
    mkdir build
    cd build/
    cmake -DATLAS_PACKAGE_FILTER_FILE=/your/specific/package_filters.txt \
       ../athena/Projects/WorkDir/

Using a partially checked out repository
----------------------------------------

If you only checked out the packages that you want to compile, along with
this `Projects/WorkDir` directory, then you don't need to worry about
setting up a package filtering file. You can simply just do:

    asetup ...
    mkdir build
    cd build/
    cmake ../athena/Projects/WorkDir/

This will set up the build of all checked out packages.

Using the local build
---------------------

After being done with the build, you have to remember to source the
`setup.sh` file of the build area to pick up your modifications! So, you
would do something like this to set up your runtime environment from scratch:

    asetup ...
    source build/x86_64-slc6-gcc49-opt/setup.sh

Using the included Visual Studio Code Workspace
-----------------------------------------------

The project includes `WorkDir.code-workspace`, which can be used to simplify
working with this project in [VSCode](https://code.visualstudio.com/). To use
it natively on a machine using the operating system that an ATLAS nightly would
have been built against
([EL9](https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/9)
at the time of writing), you could do:

    asetup ...
    echo "+ Control/AthenaExamples/AthExHelloWorld \n- .*" > package_filters.txt
    code athena/Projects/WorkDir/WorkDir.code-workspace

With the
[C\+\+ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
installed, you can now build the project using your freshly made
`package_filters.txt` file, with the click of a button.
