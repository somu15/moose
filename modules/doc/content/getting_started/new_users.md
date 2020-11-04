# New Users

Welcome to MOOSE! This page describes some helpful ways to start developing with MOOSE. Before proceeding, please visit the [installation/index.md] page and follow the instructions to get the framework up and running.

## Create an Application id=create-an-app

MOOSE is designed for building custom applications, therefore to use MOOSE an application is required. An application is where code and input files are created for a particular problem or set of problems of interest.

To create an application, run the stork.sh script while sitting outside the MOOSE repository with
a single argument: the name of your application:

```bash
cd ~/projects
./moose/scripts/stork.sh YourAppName
```

Running this script will create a folder named "your_app_name" in the projects directory, this
application will automatically link against MOOSE. Obviously, the "YourAppName" should be the desired
name or your application; consider the use of an acronym. Animal names are prefered for
applications, but you are free to choose whatever name suits your needs.

!alert warning title=Execute stork from outside of the MOOSE directory
Do not attempt to run this script while inside the MOOSE repository. Doing so will result in an error.

### Compile and Test Your Application id=compile

```bash
cd ~/projects/YourAppName
make -j4
./run_tests -j4
```

If the application is working correctly, the output shows a single passing test. This indicates that
the application is ready to be further developed. Be sure to recompile and tests the application each time [MOOSE is updated](installation/index.md#update).

### Enable Physics Modules

To enable use of the various [modules/index.md] available from MOOSE in an application, modify the section of `~/projects/your_app_name/Makefile` that is demonstrated below.

```script
################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

ALL_MODULES                 := no

CHEMICAL_REACTIONS          := no
CONTACT                     := no
EXTERNAL_PETSC_SOLVER       := no
FLUID_PROPERTIES            := no
FLUID_STRUCTURE_INTERACTION := no
FUNCTIONAL_EXPANSION_TOOLS  := no
HEAT_CONDUCTION             := no
LEVEL_SET                   := no
MISC                        := no
NAVIER_STOKES               := no
PHASE_FIELD                 := no
POROUS_FLOW                 := no
RDG                         := no
RICHARDS                    := no
SOLID_MECHANICS             := no
STOCHASTIC_TOOLS            := no
TENSOR_MECHANICS            := no
XFEM                        := no
```

Then, to begin using a module's capabilities, recompile the application in the same manner described in the [#compile] section.

!alert tip title=Only enable modules needed
Each module enabled will increase compilation times, thus it is recommended to enable modules as-needed.

## Learn More

With a working application next consider looking at the [examples_and_tutorials/index.md] page for a beginning tour of how to use input
files and implement custom behavior in an application. The [first tutorial](tutorial01_app_development/index.md) demonstrates how MOOSE application development works. If you are interested in contributing to MOOSE please visit [framework_development/index.md].

## Helpful Software id=helpful-software

A text editor is necessary for creating application files. There are many options available, so please feel free to chose an any editor that meets your needs. A popular option for application developers is [Atom](https://atom.io), which has community developed add-ons specifically for MOOSE: [Atom Editor for MOOSE](Atom_Editor.md).

A graphical post-processor, particularly one that can read [ExodusII](https://prod-ng.sandia.gov/techlib-noauth/access-control.cgi/1992/922137.pdf) files, is also necessary. [!ac](MOOSE) includes a [!ac](GUI): [PEACOCK](application_usage/peacock.md). Another popular tool is [ParaView](https://www.paraview.org/). Both of these applications are free and will allow you to visualize and process the results of your simulations.

## Join the Community id=join

Join one of our mailing lists:

- [moose-users@googlegroups.com](https://groups.google.com/forum/#!forum/moose-users) - Technical Q&A (moderate traffic)
- [moose-announce@googlegroups.com](https://groups.google.com/forum/#!forum/moose-announce) - Announcements (very light traffic)

GMail users can just click the "Join group" button.
Everyone else can join by sending an email to:

- moose-users+subscribe@googlegroups.com
- moose-announce+subscribe@googlegroups.com

## Customizing MOOSE Configuration id=configure

MOOSE can be customized by running a `configure` script in
`$MOOSE_DIR`. Note that the `configure` script *must* be invoked from
`$MOOSE_DIR`. Below we summarize the configuration options available:

### Automatic Differentiation

- `--with-derivative-type`: Specify the derivative storage type to use for
  MOOSE's `DualReal` object. Options are `sparse` and `nonsparse`. `sparse`
  selects `SemiDynamicSparseNumberArray` as the derivative storage type;
  `nonsparse` selects `NumberArray`. A more detailed overview of these storage
  types can be found in the [`DualReal` documentation](/DualReal.md).
- `--with-derivative-size=<n>`: Specify the length of the underlying derivative
  storage array. The default is 50. A smaller number may be chosen for increased
  speed; a larger number may be required for 3D problems or problems with
  coupling between many variables.

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index.md
