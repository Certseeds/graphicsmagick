.. -*- mode: rst -*-
.. This text is in reStucturedText format, so it may look a bit odd.
.. See http://docutils.sourceforge.net/rst.html for details.

===================================
UNIX/Cygwin/MinGW/MSYS2 Compilation
===================================

.. contents::
  :local:

Archive Formats
---------------

GraphicsMagick is distributed in a number of different archive formats.
The source code must be extracted prior to compilation as follows:

7z

    7-Zip archive format. The Z-Zip format may be extracted under Unix
    using '7za' from the P7ZIP package (http://p7zip.sourceforge.net/).
    Extract similar to::

      7za x GraphicsMagick-1.3.7z

.tar.bz2

    BZip2 compressed tar archive format. Requires that both the bzip2
    (http://www.sourceware.org/bzip2/) and tar programs to be available. Extract
    similar to::

      bzip2 -d GraphicsMagick-1.3.tar.bz | tar -xvf -

.tar.gz

    Gzip compressed tar archive format. Requires that both the gzip
    (http://www.gzip.org/) and tar programs to be available. Extract
    similar to::

      gzip -d GraphicsMagick-1.3.tar.gz | tar -xvf -

.tar.lz

    Lzip compressed tar archive format.  Requires that both the lzip
    (http://lzip.nongnu.org/lzip.html) and tar programs to be
    available. Extract similar to::

      lzip -d -c GraphicsMagick-1.3.tar.gz | tar -xvf -

.tar.xz

    LZMA compressed tar archive format. Requires that LZMA utils
    (http://tukaani.org/lzma/) and tar programs to be available. Extract
    similar to::

      xz -d GraphicsMagick-1.3.tar.xz | tar -xvf -

zip

    PK-ZIP archive format. Requires that the unzip program from Info-Zip
    (http://www.info-zip.org/UnZip.html) be available. Extract similar to::

      unzip GraphicsMagick-1.3.zip

The GraphicsMagick source code is extracted into a subdirectory
similar to 'GraphicsMagick-1.3'. After the source code extracted,
change to the new directory (using the actual directory name) using
a command similar to::

  cd GraphicsMagick-1.3


Build Configuration
-------------------

Use 'configure' to automatically configure, build, and install
GraphicsMagick. The configure script may be executed from the
GraphicsMagick source directory (e.g ./configure) or from a separate
build directory by specifying the full path to configure (e.g.
/src/GraphicsMagick-1.3/configure). The advantage of using a separate
build directory is that multiple GraphicsMagick builds may share the
same GraphicsMagick source directory while allowing each build to use a
unique set of options.  Using a separate directory also makes it easier
to keep track of any files you may have edited.

If you are willing to accept configure's default options (static
build, 8 bits/sample), and build from within the source directory,
type::

    ./configure

and watch the configure script output to verify that it finds everything
that you think it should. If it does not, then adjust your environment
so that it does.

If you are attempting to build a maximally-static build, then
use --disable-shared and execute configure like::

  PKG_CONFIG='pkg-config --static' ./configure

By default, 'make install' will install the package's files
in '/usr/local/bin', '/usr/local/man', etc. You can specify an
installation prefix other than '/usr/local' by giving 'configure'
the option '--prefix=PATH'.  This is valuable in case you don't have
privileges to install under the default paths or if you want to install
in the system directories instead.

If you are not happy with configure's choice of compiler, compilation
flags, or libraries, you can give 'configure' initial values for
variables by specifying them on the configure command line, e.g.::

    ./configure CC=c99 CFLAGS=-O2 LIBS=-lposix

Options which should be common to packages installed under the same
directory hierarchy may be supplied via a 'config.site' file located
under the installation prefix via the path ${prefix}/share/config.site
where ${prefix} is the installation prefix. This file is used for all
packages installed under that prefix. As an alternative, the CONFIG_SITE
environment variable may be used to specify the path of a site
configuration file to load. This is an example config.site file::

  # Configuration values for all packages installed under this prefix
  CC=gcc
  CXX=c++
  CPPFLAGS='-I/usr/local/include'
  LDFLAGS='-L/usr/local/lib -R/usr/local/lib'

When the 'config.site' file is being used to supply configuration
options, configure will issue a message similar to::

  configure: loading site script /usr/local/share/config.site

The configure variables you should be aware of are:

CC

    Name of C compiler (e.g. 'cc -Xa') to use

CXX

    Name of C++ compiler to use (e.g. 'CC')

CFLAGS

    Compiler flags (e.g. '-g -O2') to compile C code

CXXFLAGS

    Compiler flags (e.g. '-g -O2') to compile C++ code

CPPFLAGS

    Include paths (-I/somedir) to look for header files

LDFLAGS

    Library paths (-L/somedir) to look for libraries Systems that
    support the notion of a library run-path may require an additional
    argument in order to find shared libraries at run time. For
    example, the Solaris linker requires an argument of the form
    '-R/somedir', some Linux systems will work with '-rpath /somedir',
    while some other Linux systems who's gcc does not pass -rpath to
    the linker require an argument of the form '-Wl,-rpath,/somedir'.

LIBS

    Extra libraries (-lsomelib) required to link

Any variable (e.g. CPPFLAGS or LDFLAGS) which requires a directory
path must specify an absolute path rather than a relative path.

The build now supports a Linux-style "silent" build (default
disabled).  To enable this, add the configure option
--enable-silent-rules or invoke make like 'make V=0'.  If the build
has been configured for silent mode and it is necessary to see a
verbose build, then invoke make like 'make V=1'.

Configure can usually find the X include and library files
automatically, but if it doesn't, you can use the 'configure' options
'--x-includes=DIR' and '--x-libraries=DIR' to specify their locations.

The configure script provides a number of GraphicsMagick specific
options.  When disabling an option --disable-something is equivalent
to specifying --enable-something=no and --without-something is
equivalent to --with-something=no.  The configure options are as
follows (execute 'configure --help' to see all options).


Optional Features
~~~~~~~~~~~~~~~~~

--disable-compressed-files

    disable reading and writing of gzip/bzip stream files

    Normally support for being able to read and write gzip/bzip stream
    files (files which are additionally compressed using gzip or bzip)
    is a good thing, but for some formats it is necessary to
    decompress an entire input file before it may be validated and
    read.  Decompressing the file may take a lot of time and disk
    space.  If input files are not trustworthy, an apparently small
    file can take much more resources than expected.  Use this option
    to reject such files.

--enable-prof

    enable 'prof' profiling support (default disabled)

--enable-gprof

    enable 'gprof' profiling support (default disabled)

--enable-gcov

    enable 'gcov' profiling support (default disabled)

--disable-installed

    disable building an installed GraphicsMagick (default enabled)

--enable-broken-coders

    enable broken/dangerous file formats support

--disable-largefile

    disable support for large (64 bit) file offsets

--disable-openmp

    disable use of OpenMP (automatic multi-threaded loops) at all

--enable-openmp-slow

    enable OpenMP for algorithms which sometimes run slower

--enable-symbol-prefix

    enable prefixing library symbols with "Gm"

--enable-magick-compat

    install ImageMagick utility shortcuts (default disabled)

--enable-maintainer-mode

    enable additional Makefile rules which update generated files
    included in the distribution. Requires GNU make as well as a
    number of utilities and tools.

--enable-quantum-library-names

    shared library name includes quantum depth to allow shared
    libraries with different quantum depths to co-exist in same
    directory (only one can be used for development)


Optional Packages/Options
~~~~~~~~~~~~~~~~~~~~~~~~~

--with-quantum-depth

    number of bits in a pixel quantum (default 8).  Also see
    '--enable-quantum-library-names.'

--with-modules

    enable building dynamically loadable modules

--without-threads

    disable POSIX threads API support

--with-frozenpaths

    enable frozen delegate paths

--without-magick-plus-plus

    disable build/install of Magick++

--with-perl

    enable build/install of PerlMagick

--with-perl=PERL

    use specified Perl binary to configure PerlMagick

--with-perl-options=OPTIONS

    options to pass on command-line when generating PerlMagick's Makefile from Makefile.PL

--without-bzlib

    disable BZLIB support

--with-fpx

    enable FlashPIX support

--without-jbig

    disable JBIG support

--without-webp

    disable WEBP support

--without-jp2

    disable JPEG v2 support

--with-jxl

    enable JPEG-XL support

--without-jpeg

    disable JPEG support

--without-jp2

    disable JPEG v2 support

--without-lcms2

    disable lcms (v2.X) support

--without-lzma

    disable LZMA support

--without-png

    disable PNG support

--without-tiff

    disable TIFF support

--with-trio

    enable TRIO library support

--without-ttf

    disable TrueType support

--without-libzip

    disable libzip support

--with-tcmalloc

    enable Google perftools tcmalloc (minimal) memory allocation
    library support

--with-mtmalloc

    enable Solaris mtmalloc memory allocation library support

--with-umem

    enable Solaris libumem memory allocation library support

--without-wmf

    disable WMF support

--with-fontpath

    prepend to default font search path

--with-gs-font-dir

    directory containing Ghostscript fonts

--with-gs-font-dir

   directory containing Artifex URW Base35 OTF fonts

--with-windows-font-dir

    directory containing MS-Windows fonts

--without-xml

    disable XML support

--without-zlib

    disable ZLIB support

--without-zstd

    disable Zstd support

--with-x

    use the X Window System

--with-share-path=DIR

    Alternate path to share directory (default share/GraphicsMagick)

--with-libstdc=DIR

    use libstdc++ in DIR (for GNU C++)

GraphicsMagick options represent either features to be enabled, disabled,
or packages to be included in the build.  When a feature is enabled (via
--enable-something), it enables code already present in GraphicsMagick.
When a package is enabled (via --with-something), the configure script
will search for it, and if is is properly installed and ready to use
(headers and built libraries are found by compiler) it will be included
in the build.  The configure script is delivered with all features
disabled and all packages enabled. In general, the only reason to
disable a package is if a package exists but it is unsuitable for
the build (perhaps an old version or not compiled with the right
compilation flags).

Several configure options require special note:

--enable-shared

  The shared libraries are built and support for loading coder and
  process modules is enabled. Shared libraries are preferred because
  they allow programs to share common code, making the individual
  programs much smaller. In addition shared libraries are required in
  order for PerlMagick to be dynamically loaded by an installed PERL
  (otherwise an additional PERL (PerlMagick) must be installed. This
  option is not the default because all libraries used by
  GraphicsMagick must also be dynamic libraries if GraphicsMagick
  itself is to be dynamically loaded (such as for PerlMagick).

  GraphicsMagick built with delegates (see MAGICK PLUG-INS below)
  can pose additional challenges. If GraphicsMagick is built using
  static libraries (the default without --enable-shared) then
  delegate libraries may be built as either static libraries or
  shared libraries. However, if GraphicsMagick is built using shared
  libraries, then all delegate libraries must also be built as
  shared libraries.  Static libraries usually have the extension .a,
  while shared libraries typically have extensions like .so, .sa,
  or .dll. Code in shared libraries normally must compiled using
  a special compiler option to produce Position Independent Code
  (PIC). The only time this is not necessary is if the platform
  compiles code as PIC by default.

  PIC compilation flags differ from vendor to vendor (gcc's is
  -fPIC). However, you must compile all shared library source with
  the same flag (for gcc use -fPIC rather than -fpic). While static
  libraries are normally created using an archive tool like 'ar',
  shared libraries are built using special linker or compiler options
  (e.g. -shared for gcc).

  Building shared libraries often requires subtantial hand-editing
  of Makefiles and is only recommended for those who know what they
  are doing.

  If --enable-shared is not specified, a new PERL interpreter
  (PerlMagick) is built which is statically linked against the
  PerlMagick extension. This new interpreter is installed into the
  same directory as the GraphicsMagick utilities. If --enable-shared
  is specified, the PerlMagick extension is built as a dynamically
  loadable object which is loaded into your current PERL interpreter
  at run-time. Use of dynamically-loaded extensions is preferable over
  statically linked extensions so --enable-shared should be specified
  if possible (note that all libraries used with GraphicsMagick must
  be shared libraries!).

--disable-static

  static archive libraries (with extension .a) are not built. If you
  are building shared libraries, there is little value to building
  static libraries. Reasons to build static libraries include: 1) they
  can be easier to debug; 2) the clients do not have external
  dependencies (i.e. libMagick.so); 3) building PIC versions of the
  delegate libraries may take additional expertise and effort; 4) you
  are unable to build shared libraries.

--disable-installed

  By default the GraphicsMagick build is configured to formally install
  into a directory tree. This is the most secure and reliable way to
  install GraphicsMagick. Specifying --disable-installed configures
  GraphicsMagick so that it doesn't use hard-coded paths and locates
  support files by computing an offset path from the executable (or
  from the location specified by the MAGICK_HOME environment variable.
  The uninstalled configuration is ideal for binary distributions which
  are expected to extract and run in any location.

--enable-broken-coders

  The implementation of file format support for some formats is
  incomplete or imperfectly implemented such that file corruption or a
  security exploit might occur.  These formats are not included in the
  build by default but may be enabled using
  ``--enable-broken-coders``.  The existing implementation may still
  have value in controlled circumstances so it remains but needs to be
  enabled.  One of the formats currently controlled by this is Adobe
  Photoshop bitmap format (PSD).

--with-modules

  Image coders and process modules are built as loadable modules which
  are installed under the directory
  [prefix]/lib/GraphicsMagick-X.X.X/modules-QN (where 'N' equals 8, 16,
  or 32 depending on the quantum depth) in the subdirectories 'coders'
  and 'filters' respectively. The modules build option is only
  available in conjunction with --enable-shared. If --enable-shared is
  not also specified, then support for building modules is disabled.
  Note that if --enable-shared is specified, the module loader is
  active (allowing extending an installed GraphicsMagick by simply
  copying a module into place) but GraphicsMagick itself is not built
  using modules.

  Use of the modules build is recommended where it is possible to use
  it.  Using modules defers the overhead due to library dependencies
  (searching the filesystem for libraries, shared library relocations,
  initialized data, and constructors) until the point the libraries
  are required to be used to support the file format requested.
  Traditionally it has been thought that a 'static' program will be
  more performant than one built with shared libraries, and perhaps
  this may be true, but building a 'static' GraphicsMagick does not
  account for the many shared libraries it uses on a typical
  Unix/Linux system.  These shared libraries may impose unexpected
  overhead.  For example, it was recently noted that libxml2 is now
  often linked with the ICU (international character sets) libraries
  which are huge C++ libraries consuming almost 30MB of disk space and
  that simply linking with these libraries causes GraphicsMagick to
  start up much more slowly. By using the modules build, libxml2 (and
  therefore the huge ICU C++ libraries) are only loaded in the few
  cases (e.g. SVG format) where it is needed.

  When applications depend on the GraphicsMagick libraries, using the
  modules build lessens the linkage overhead due to using
  GraphicsMagick.

--enable-symbol-prefix

  The GraphicsMagick libraries may contain symbols which conflict with
  other libraries. Specifify this option to prefix "Gm" to all library
  symbols, and use the C pre-processor to allow dependent code to still
  compile as before.

--enable-magick-compat

  Normally GraphicsMagick installs only the 'gm' utility from which all
  commands may be accessed. Existing packages may be designed to invoke
  ImageMagick utilities (e.g. "convert"). Specify this option to
  install ImageMagick utility compatibility links to allow
  GraphicsMagick to substitute directly for ImageMagick. Take care when
  selecting this option since if there is an existing ImageMagick
  installation installed in the same directory, its utilities will be
  replaced when GraphicsMagick is installed.

--with-quantum-depth

  This option allows the user to specify the number of bits to use per
  pixel quantum (the size of the red, green, blue, and alpha pixel
  components. When an image file with less depth is read, smaller
  values are scaled up to this size for processing, and are scaled
  down from this size when a file with lower depth is written.  For
  example, "--with-quantum-depth=8" builds GraphicsMagick using 8-bit
  quantums. Most computer display adaptors use 8-bit
  quantums. Currently supported arguments are 8, 16, or 32.  The
  default is 8. This option is the most important option in
  determining the overall run-time performance of GraphicsMagick.

  The number of bits in a quantum determines how many values it may
  contain. Each quantum level supports 256 times as many values as
  the previous level. The following table shows the range available
  for various quantum sizes.

      ============  =====================  =================
      QuantumDepth  Valid Range (Decimal)  Valid Range (Hex)
      ============  =====================  =================
            8                0-255               00-FF
           16               0-65535            0000-FFFF
           32            0-4294967295      00000000-FFFFFFFF
      ============  =====================  =================

  Larger pixel quantums cause GraphicsMagick to run more slowly and to
  require more memory. For example, using sixteen-bit pixel quantums
  causes GraphicsMagick to run 15% to 50% slower (and take twice as
  much memory) than when it is built to support eight-bit pixel
  quantums.  Regardless, the GraphicsMagick authors prefer to use
  sixteen-bit pixel quantums since they support all common image
  formats and assure that there is no loss of color precision.

  The amount of virtual memory consumed by an image can be computed
  by the equation (QuantumDepth*Rows*Columns*5)/8. This is an
  important consideration when resources are limited, particularly
  since processing an image may require several images to be in
  memory at one time. The following table shows memory consumption
  values for a 1024x768 image:

      ============  ==============
      QuantumDepth  Virtual Memory
      ============  ==============
          8              3MB
         16              8MB
         32             15MB
      ============  ==============

  GraphicsMagick performs all image processing computations using
  floating point or non-lossy integer arithmetic, so results are very
  accurate.  Increasing the quantum storage size decreases the amount
  of quantization noise (usually not visible at 8 bits) and helps
  prevent countouring and posterization in the image.

  Consider also using the --enable-quantum-library-names configure
  option so that installed shared libraries include the quantum depth
  as part of their names so that shared libraries using different
  quantum depth options may co-exist in the same directory.

--without-magick-plus-plus

  Disable building Magick++, the C++ application programming interface
  to GraphicsMagick. A suitable C++ compiler is required in order to
  build Magick++. Specify the CXX configure variable to select the C++
  compiler to use (default "g++"), and CXXFLAGS to select the desired
  compiler opimization and debug flags (default "-g -O2"). Antique C++
  compilers will normally be rejected by configure tests so specifying
  this option should only be necessary if Magick++ fails to compile.

--with-frozenpaths

  Normally external program names are substituted into the
  delegates.mgk file without full paths. Specify this option to enable
  saving full paths to programs using locations determined by
  configure. This is useful for environments where programs are stored
  under multiple paths, and users may use different PATH settings than
  the person who builds GraphicsMagick.

--without-threads

  By default, the GraphicsMagick library is compiled to be fully
  thread safe by using thread APIs to implement required locking.
  This is intended to allow the GraphicsMagick library to be used by
  multi-threaded programs using native POSIX threads. If the locking
  or dependence on thread APIs is undesirable, then specify
  --without-threads.  Testing shows that the overhead from thread
  safety is virtually unmeasurable so usually there is no reason to
  disable multi-thread support.  While previous versions disabled
  OpenMP support when this option was supplied, that is no longer the
  case since then OpenMP locking APIs are used instead.

--disable-largefile

  By default, GraphicsMagick is compiled with support for large (> 2GB
  on a 32-bit CPU) files if the operating system supports large files.
  Applications which use the GraphicsMagick library might then also
  need to be compiled to support for large files (operating system
  dependent).  Normally support for large files is a good thing.  Only
  disable this option if there is a need to do so.

--disable-openmp

  By default, GraphicsMagick is compiled with support for OpenMP
  (http://www.openmp.org/) if the compilation environment supports it.
  OpenMP automatically parallelizes loops across concurrent threads
  based on instructions in pragmas. OpenMP was introduced in GCC
  4.2. OpenMP is a well-established standard and was implemented in
  some other compilers in the late '90s, long before its appearance in
  GCC. OpenMP adds additional build and linkage requirements.
  GraphicsMagick supports OpenMP version 2.0 and later, primarily
  using features defined by version 2.5, but will be optionally using
  features from version 3.1 in the future since it is commonly
  available.

  By default, GraphicsMagick enables as many threads as there are CPU
  cores (or CPU threads).  According to the OpenMP standard, the
  OMP_NUM_THREADS environment variable specifies how many threads
  should be used and GraphicsMagick also honors this request. In order
  to obtain the best single-user performance, set OMP_NUM_THREADS
  equal to the number of available CPU cores.  On a server with many
  cores and many programs running at once, there may be benefit to
  setting OMP_NUM_THREADS to a much smaller value than the number of
  cores, and sometimes values as low as two (or even one, to disable
  threading) will offer the best overall system performance.  Tuning a
  large system with OpenMP programs running in parallel (competing for
  resources) is a complex topic and some research and experimentation
  may be required in order to find the best parameters.

--enable-openmp-slow

  On some systems, memory-bound algorithms run slower (rather than
  faster) as threads are added via OpenMP.  This may be due to CPU
  cache and memory architecture implementation, or OS thread API
  implementation.  Since it is not known how a system will behave
  without testing and pre-built binaries need to work well on all
  systems, these algorithms are now disabled for OpenMP by default.
  If you are using a well-threaded OS on a CPU with a good
  high-performance memory architecture, you might consider enabling
  this option based on experimentation.

--with-perl

  Use this option to include PerlMagick in the GraphicsMagick build
  and test suite. While PerlMagick is always configured by default
  (PerlMagick/Makefile.PL is generated by the configure script),
  PerlMagick is no longer installed by GraphicsMagick's ''make
  install''.  The procedure to configure, build, install, and check
  PerlMagick is described in PerlMagick/README.txt.  When using a
  shared library build of GraphicsMagick, it is necessary to formally
  install GraphicsMagick prior to building PerlMagick in order to
  achieve a working PerlMagick since otherwise the wrong
  GraphicsMagick libraries may be used.

  If the argument ''--with-perl=/path/to/perl'' is supplied, then
  /path/to/perl will be taken as the PERL interpreter to use. This is
  important in case the 'perl' executable in your PATH is not PERL5,
  or is not the PERL you want to use.  Experience suggests that static
  PerlMagick builds may not be fully successful (at least for
  executing the test suite) for Perl versions newer than 5.8.8.

  As a convenience, the Makefile targets 'perl-build',
  'install-exec-perl', and 'perl-check' are provided.  In order to
  assure that library dependencies and search paths are correct, it is
  necessary to first install GraphicsMagick via 'make install', then
  build PerlMagick using 'make perl-build', then install PerlMagick
  using 'sudo make install-exec-perl', and then 'make perl-check' to
  make sure that it actually works.

--with-perl-options

  The PerlMagick module is normally installed using the Perl
  interpreter's installation PREFIX, rather than GraphicsMagick's. If
  GraphicsMagick's installation prefix is not the same as PERL's
  PREFIX, then you may find that PerlMagick's 'make install' step tries
  to install into a directory tree that you don't have write
  permissions to. This is common when PERL is delivered with the
  operating system or on Internet Service Provider (ISP) web servers.
  If you want PerlMagick to install elsewhere, then provide a PREFIX
  option to PERL's configuration step via
  "--with-perl-options=PREFIX=/some/place". Other options accepted by
  MakeMaker are 'LIB', 'LIBPERL_A', 'LINKTYPE', and 'OPTIMIZE'. See the
  ExtUtils::MakeMaker(3) manual page for more information on
  configuring PERL extensions.

--without-x

  By default, GraphicsMagick will use X11 libraries if they are
  available. When --without-x is specified, use of X11 is disabled. The
  display, animate, and import sub-commands are not included. The
  remaining sub-commands have reduced functionality such as no access
  to X11 fonts (consider using Postscript or TrueType fonts instead).

--with-gs-font-dir

  Specify the directory containing the Ghostscript Postscript Type 1
  font files (e.g. "n019003l.pfb") also known as the "URW Fonts" so
  that they can be rendered using the FreeType library.  These fonts
  emulate the standard 35 fonts commonly available on printers
  supporting Adobe Postscript so they are very useful to have. If the
  font files are installed using the default Ghostscript installation
  paths (${prefix}/share/ghostscript/fonts), they should be discovered
  automatically by configure and specifying this option is not
  necessary. Specify this option if the Ghostscript fonts fail to be
  located automatically, or the location needs to be overridden.

  The "Ghostscript" fonts (also known as "URW Standard postscript
  fonts (cyrillicized)") are available from

    https://sourceforge.net/projects/gs-fonts/

  These fonts may are often available as a package installed by a
  package manager and installing from a package manager is easier than
  installing from source:

  .. table:: URW Font Packages

    ==============  =====================  =============================
    Distribution    Package Name           Fonts Installation Path
    ==============  =====================  =============================
    Cygwin          urw-base35-fonts       /usr/share/ghostscript/fonts
    Debian Linux    fonts-urw-base35       /usr/share/fonts/type1/gsfonts
    Gentoo Linux    media-fonts/urw-fonts  /usr/share/fonts/ghostscript
    Illumos/pkgsrc  urw-fonts-2.0nb1       /opt/local/share/fonts/urw
    NetBSD/pkgsrc   urw-fonts-2.0nb1       /share/fonts/urw
    OpenIndiana     gnu-gs-fonts-std       /usr/share/ghostscript/fonts
    OS X/Homebrew   font-urw-base35        [ TBD ]
    Red Hat Linux   urw-fonts-2.0          /usr/share/fonts/default/Type1
    Ubuntu Linux    fonts-urw-base35       /usr/share/fonts/type1/gsfonts
    ==============  =====================  =============================

--with-urwbase35otf-font-dir

  Specify the directory containing the Artifex OpenType font files
  (e.g. 'URWGothic-Book.otf') from the urw-base35-fonts package
  available from https://github.com/ArtifexSoftware/urw-base35-fonts.
  These fonts are a modern replacement for the older 'psfonts' and
  older 'urw-base35-fonts' (which use short file names).  If Artifex
  urw-base35-fonts are available, they are used (by default) rather
  than the legacy 'psfonts'/'urw-base35-fonts' package described above
  (i.e. --with-gs-font-dir).

  .. table:: URW Font Packages

    ==============  =====================  ====================================
    Distribution    Package Name           Fonts Installation Path
    ==============  =====================  ====================================
    Debian Linux    fonts-urw-base35       /usr/share/fonts/opentype/urw-base35
    Ubuntu Linux    fonts-urw-base35       /usr/share/fonts/opentype/urw-base35
    ==============  =====================  ====================================

--with-windows-font-dir

  Specify the directory containing MS-Windows-compatible fonts. This is
  not necessary when GraphicsMagick is running under MS-Windows.

--with-tcmalloc

  The GNU libc malloc and some other mallocs exhibits poor concurrency
  in multi-threaded OpenMP programs and this can severely impact
  OpenMP speedup.  The 'tcmalloc' library provided as part of Google
  `gperftools <https://github.com/gperftools/gperftools>`_ has been
  observed to perform far better than the default GNU libc memory
  allocator for multi-threaded use, and also for single-threaded use.
  Overall benchmark performance improvements of up to a factor of two
  are observed for some algorithms (even with just 12 cores) and it is
  expected that the improvements will become much more apparent with
  larger numbers of cores (e.g. 64 cores).  Using tcmalloc may improve
  performance dramatically for some work-loads on modern multi-core
  systems.

--with-umem

  The default Solaris memory allocator exhibits poor concurrency in
  multi-threaded programs and this can impact OpenMP speedup under
  Solaris (and systems derived from it such as Illumos).  Use this
  convenience option to enable use of the umem memory allocation
  library, which is observed to be more performant in multi-threaded
  programs.  There is a port of umem available for Linux so this
  option is not specific to Solaris.

--with-mtmalloc

  The default Solaris memory allocator exhibits poor concurrency in
  multi-threaded programs and this can impact OpenMP speedup under
  Solaris (and systems derived from it such as Illumos).  Use this
  convenience option to enable use of the mtmalloc memory allocation
  library, which is more performant in multi-threaded programs than
  the default libc memory allocator, and more performant in
  multi-threaded programs than umem, but is less memory efficient.

JPEG XL
-------

JPEG XL seems to be a work in progress, with previous APIs being
deprecated, and replacement APIs introduced.  We have taken an
approach to use the latest recommended APIs.  For development testing
with it (0.7.0 or later) we build it as described on its git page
(https://github.com/libjxl/libjxl), but configure and build it like::

  git clone https://github.com/libjxl/libjxl.git --recursive --shallow-submodules
  cd ./libjxl
  mkdir build
  cd ./build
  export CC=clang-12 CXX=clang++-12
  cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/usr/local ..
  cmake --build . -- -j$(nproc)
  make test
  [ check for 100% tests passed ]
  make install

While the JPEG XL project recommends using Clang, it is observed to
work without known issues when compiled using GCC 9.4.0.

Popular Distribution Packages
-----------------------------

We will attempt to document the package names useful for
GraphicsMagick on various primordial operating system distributions
here.

Debian Packages
~~~~~~~~~~~~~~~

These packages may be installed on a Debian Linux system (or one that
derives from Debian such as Ubuntu or Mint) in order to quickly build
a full GraphicsMagick.  Most of these are optional depending on the
features desired::

  gcc (and/or clang), make, libbz2-dev, libfreetype6-dev, libjbig-dev,
  liblcms2-dev, liblzma-dev, libpng-dev, libtiff-dev, libtool,
  libwebp-dev, libwmf-dev, libx11-dev, libxdmcp-dev, libxext-dev,
  libxft-dev, libxml2-dev, libxt-dev, libzstd-dev, zlib1g-dev,
  libperl-dev

These additional packages are useful in order to improve the run-time
features of the software (and could be installed prior to building
GraphicsMagick)::

  dcraw, fonts-urw-base35, ghostscript, hp2xx,
  ttf-mscorefonts-installer

These additional packages are useful in order to maintain
GraphicsMagick itself::

  autoconf, automake, graphviz, libtool, docutils-common, python, m4


Building under Cygwin
---------------------

GraphicsMagick may be built under the Windows Cygwin Unix-emulation
environment available for free from

    http://www.cygwin.com/

It is suggested that the X11R6 package be installed since this enables
GraphicsMagick's X11 support (animate, display, and import
sub-commands will work) and it includes the Freetype v2 DLL required
to support TrueType and Postscript Type 1 fonts. Make sure that
/usr/X11R6/bin is in your PATH prior to running configure.

If you are using Cygwin version 1.3.9 or later, you may specify the
configure option '--enable-shared' to build Cygwin DLLs, and
additionally '--with-modules' to enable use of loadable
modules. Specifying '--enable-shared' is required if you want to build
PerlMagick under Cygwin because Cygwin does not provide the libperl.a
static library required to create a static PerlMagick.  Note that
older Cygwin compilers may not generate code which supports reliably
catching C++ exceptions thrown by DLL code.  The Magick++ library
requires that it be possible to catch C++ exceptions thrown from DLLs.
The test suite ``make check`` includes several tests to verify that
C++ exceptions are working properly.

Building under MinGW-W64 & MSYS2
--------------------------------

GraphicsMagick may easily be built using the free `MSYS2
<https://www.msys2.org/>`_ distribution which provides GCC compilers,
libraries, and headers, targeting native Windows along with a
Unix-like command shell and a package manager ('pacman') to install
pre-compiled components.  Using the pre-compiled packages, it is as
easy to compile GraphicsMagick under MSYS2 as it is under Linux!

When using MSYS2, requesting to install these packages using 'pacman
-S' should result in getting up to speed very quicky with a featureful
64-bit build:

mingw-w64-x86_64-toolchain, mingw-w64-x86_64-bzip2,
mingw-w64-x86_64-freetype, mingw-w64-x86_64-ghostscript,
mingw-w64-x86_64-jasper, mingw-w64-x86_64-jbigkit,
mingw-w64-x86_64-lcms2, mingw-w64-x86_64-libheif,
mingw-w64-x86_64-libjpeg-turbo, mingw-w64-x86_64-libjxl,
mingw-w64-x86_64-libpng, mingw-w64-x86_64-libtiff,
mingw-w64-x86_64-libtool, mingw-w64-x86_64-libwebp,
mingw-w64-x86_64-libwmf, mingw-w64-x86_64-libxml2,
mingw-w64-x86_64-libzip, mingw-w64-x86_64-zlib,

and/or use the following to add support for a 32-bit build:

mingw-w64-i686-toolchain, mingw-w64-i686-bzip2,
mingw-w64-i686-freetype, mingw-w64-i686-ghostscript,
mingw-w64-i686-jasper, mingw-w64-i686-jbigkit,
mingw-w64-i686-lcms2, mingw-w64-i686-libheif,
mingw-w64-i686-libjpeg-turbo, mingw-w64-i686-libjxl,
mingw-w64-i686-libpng, mingw-w64-i686-libtiff,
mingw-w64-i686-libtool, mingw-w64-i686-libwebp,
mingw-w64-i686-libwmf, mingw-w64-i686-libxml2,
mingw-w64-i686-libzip, mingw-w64-i686-zlib,

Note that the default installation prefix is MSYS's notion of
``/usr/local`` which installs the package into a MSYS directory. To
install outside of the MSYS directory tree, you may specify an
installation prefix like ``/c/GraphicsMagick`` which causes the package
to be installed under the Windows directory ``C:\GraphicsMagick``. The
installation directory structure will look very much like the Unix
installation layout (e.g. ``C:\GraphicsMagick\bin``,
``C:\GraphicsMagick\lib``, ``C:\GraphicsMagick\share``, etc.). Paths
which may be embedded in libraries and configuration files are
transformed into Windows paths so they don't depend on MSYS.

Cross-compilation On Unix/Linux Host
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Given a modern and working MinGW32 or mingw-w64 installation, it is
easy to cross-compile GraphicsMagick from a Unix-type host to produce
Microsoft Windows executables.

This incantation produces a static WIN32 `gm.exe` executable on an
Ubuntu Linux host with the i686-w64 cross-compiler installed::

  ./configure '--host=i686-w64-mingw32' '--disable-shared'

and this incantation produces a static WIN64 `gm.exe` executable on an
Ubuntu Linux host with the x86_64-w64 cross-compiler installed::

  ./configure '--host=x86_64-w64-mingw32' '--disable-shared'

For a full-fledged GraphicsMagick program, normally one will want to
pre-install or cross-compile the optional libraries that
GraphicsMagick may depend on and install them where the cross-compiler
will find them, or add extra `CPPFLAGS` and `LDFLAGS` options so that
the compiler searches for header files and libraries in the correct
place.

Configuring for building with shared libraries (libGraphicsMagick,
libGraphicsMagickWand, and libGraphicsMagick++ DLLs) and modules
(coders as DLLs) is also supported by the cross-builds.  A cross-built
libtool libltdl needs to be built in advance in order to use the
`--with-modules` modules option.

After configuring the software for cross-compilation, the software is
built using `make` as usual and everything should be as with native
compilation except that `make check` is likely not available (testing
might be possible on build system via WINE, not currently
tested/supported by GraphicsMagick authors).

Use of the `DESTDIR` approach as described in the `Build & Install`_
section is recommended in order to install the build products into a
formal directory tree before preparing to copy onto the Windows target
system (e.g. by packaging via an installer).

Dealing with configuration failures
-----------------------------------

While configure is designed to ease installation of GraphicsMagick, it
often discovers problems that would otherwise be encountered later
when compiling GraphicsMagick. The configure script tests for headers
and libraries by executing the compiler (CC) with the specified
compilation flags (CFLAGS), pre-processor flags (CPPFLAGS), and linker
flags (LDFLAGS). Any errors are logged to the file 'config.log'. If
configure fails to discover a header or library please review this
log file to determine why, however, please be aware that *errors
in the config.log are normal* because configure works by trying
something and seeing if it fails. An error in config.log is only a
problem if the test should have passed on your system. After taking
corrective action, be sure to remove the 'config.cache' file before
running configure so that configure will re-inspect the environment
rather than using cached values.

Common causes of configure failures are:

1) A delegate header is not in the header include path (CPPFLAGS -I
   option).

2) A delegate library is not in the linker search/run path (LDFLAGS
   -L/-R option).

3) A delegate library is missing a function (old version?).OB

4) The compilation environment is faulty.

If all reasonable corrective actions have been tried and the problem
appears to be due to a flaw in the configure script, please send a
bug report to the configure script maintainer (currently
bfriesen@graphicsmagick.org). All bug reports should contain the
operating system type (as reported by 'uname -a') and the
compiler/compiler-version. A copy of the configure script output
and/or the config.log file may be valuable in order to find the
problem. If you send a config.log, please also send a script of the
configure output and a description of what you expected to see (and
why) so the failure you are observing can be identified and resolved.

Makefile Build Targets
----------------------

Once GraphicsMagick is configured, these standard build targets are
available from the generated Makefiles:

  'make'

     Build the package

  'make install'

     Install the package

  'make check'

     Run tests using the uninstalled software. On some systems, 'make
     install' must be done before the test suite will work but usually
     the software can be tested prior to installation.

     The test suite requires sufficient RAM memory to run.  The memory
     requirement is 128MB for the Q8 build, or 256MB for the Q16
     build, or 512MB for the Q32 build.

  'make clean'

     Remove everything in the build directory created by 'make'

  'make distclean'

     Remove everything in the build directory created by 'configure'
     and 'make'. This is useful if you want to start over from scratch.

  'make uninstall'

     Remove all files from the system which are (or would be) installed
     by 'make install' using the current configuration. Note that this
     target does not work for PerlMagick since Perl no longer supports
     an 'uninstall' target.

Build & Install
---------------

Now that GraphicsMagick is configured, type ::

     make

to build the package and ::

     make install

to install it.

To install under a specified directory using the install directory
tree layout (e.g. as part of the process for packaging the built
software), specify DESTDIR like ::

  make DESTDIR=/my/dest/dir install

Verifying The Build
-------------------

To confirm your installation of the GraphicsMagick distribution was
successful, ensure that the installation directory is in your executable
search path and type ::

  gm display

The GraphicsMagick logo should be displayed on your X11 display.

Verify that the expected image formats are supported by executing ::

  gm convert -list formats

Verify that the expected fonts are available by executing ::

  gm convert -list fonts

Verify that delegates (external programs) are configured as expected
by executing ::

  gm convert -list delegates

Verify that color definitions may be loaded by executing ::

  gm convert -list colors

If GraphicsMagick is built to use loadable coder modules, then verify
that the modules load via ::

  gm convert -list modules

Verify that GraphicsMagick is properly identifying the resources of
your machine via ::

  gm convert -list resources

For a thorough test, you should run the GraphicsMagick test suite by
typing ::

  make check

Note that due to differences between the developer's environment and
your own, it is possible that some tests may be indicated as failed
even though the results are ok.  Such failures should be rare, and if
they do occur, they should be reported as a bug.  Differences between
the developer's environment environment and your own may include the
compiler, the CPU type, and the library versions used. The
GraphicsMagick developers use the current release of all dependent
libraries.
