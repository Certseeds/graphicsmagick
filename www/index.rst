.. -*- mode: rst -*-
.. This text is in reStucturedText format, so it may look a bit odd.
.. See http://docutils.sourceforge.net/rst.html for details.

======================================
GraphicsMagick Image Processing System
======================================

.. meta::
   :description: GraphicsMagick is a robust collection of tools and
                 libraries to read, write, and manipulate an image in any
                 of the more popular image formats including GIF, JPEG,
                 PNG, PDF, and WebP. With GraphicsMagick you can
                 create GIFs dynamically making it suitable for Web
                 applications. You can also resize, rotate, sharpen,
                 color reduce, or add special effects to an image and
                 save your completed work in the same or differing image
                 format.

   :keywords: GraphicsMagick, GM, PerlMagick, Perl Magick, Perl Magic,
              image processing, software development, TclMagick, Magick++


.. _`download GraphicsMagick release` : https://sourceforge.net/projects/graphicsmagick/files/
.. _`download development snapshots` : https://sourceforge.net/projects/graphicsmagick/files/graphicsmagick-snapshots/
.. _`visit Mercurial repository`: https://sourceforge.net/p/graphicsmagick/code/

.. _programming : programming.html

===========================  =========================================================
Current Release              1.3.45 (Released August 27, 2024) `download release`__
Development Snapshots        (Updated frequently) `download development snapshots`__
Mercurial Repository         (Updated frequently) `visit Mercurial repository`__
===========================  =========================================================

__ `download GraphicsMagick release`_
__ `download development snapshots`_
__ `visit Mercurial repository`_


Check http://www.GraphicsMagick.org/ or
https://graphicsmagick.sourceforge.io/index.html for the latest
version of this page.

.. _FSF : http://www.fsf.org/
.. _`GNU GPL` :  https://www.gnu.org/licenses/license-list.html#GPLCompatibleLicenses
.. _ImageMagick : http://www.imagemagick.org/
.. _Open Source Initiative : http://www.opensource.org/
.. _`MIT License` : http://opensource.org/licenses/MIT
.. _OSSCC : http://www.osscc.net/en/index.html
.. _OpenMP : http://www.openmp.org/
.. _`read about OpenMP in GraphicsMagick` : OpenMP.html
.. _`ChangeLog` : ChangeLog.html
.. _`Flickr` : http://www.kitchensoap.com/2009/04/03/slides-from-web20-expo-2009-and-somethin-else-interestin/
.. _`Etsy` : http://codeascraft.etsy.com/2010/07/09/batch-processing-millions-of-images/
.. _`John Allspaw's presentation` : http://www.kitchensoap.com/2009/04/03/slides-from-web20-expo-2009-and-somethin-else-interestin/
.. _Mercurial : https://www.mercurial-scm.org/
.. _`NEWS` : NEWS.html
.. _`SLOCCount` : https://dwheeler.com/sloccount/
.. _`authors` : authors.html
.. _`benchmarks` : benchmarks.html
.. _`manual page` : GraphicsMagick.html
.. _`valgrind` : http://www.valgrind.org/
.. _`ASan` : https://github.com/google/sanitizers/wiki/AddressSanitizer
.. _`UBSan` : https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
.. _`oss-fuzz` : https://github.com/google/oss-fuzz
.. _`Coverity Analysis Metrics` : https://scan.coverity.com/projects/graphicsmagick

GraphicsMagick is the swiss army knife of image processing. Comprised
of 284K physical lines (according to David A. Wheeler's `SLOCCount`_)
of source code in the base package, it provides a robust and efficient
collection of tools and libraries which support reading, writing, and
manipulating an image in over 92 major formats including important
formats like DPX, GIF, JPEG, JPEG-2000, JXL, PNG, PDF, PNM, TIFF, and
WebP.

Image processing is multi-threaded using OpenMP_ (`read about OpenMP
in GraphicsMagick`_) so that CPU-bound tasks scale linearly as
processor cores are added. OpenMP_ support requires compilation with
GCC 4.2 (or later), or use of any C compiler supporting at least the
OpenMP_ 2.0 specification.

GraphicsMagick is quite portable, and compiles under almost every general
purpose operating system that runs on 32-bit or 64-bit CPUs.
GraphicsMagick is available for virtually any Unix or Unix-like system,
including Linux. It also runs under `Windows <INSTALL-windows.html>`_
Vista and later (Vista, 7, 8.X, 10, 11), and MacOS-X.

GraphicsMagick supports huge images and has been tested with
gigapixel-size images. GraphicsMagick can create new images on the
fly, making it suitable for building dynamic Web
applications. GraphicsMagick may be used to resize, rotate, sharpen,
color reduce, or add special effects to an image and save the result
in the same or different image format. Image processing operations are
available from the command line, as well as through C, C++, Lua, Perl,
PHP, Python, Tcl, Ruby, Windows .NET, or Windows COM programming
interfaces. With some modification, language extensions for
ImageMagick may be used.

GraphicsMagick is originally derived from ImageMagick_ 5.5.2 as of
November 2002 but has been completely independent of the ImageMagick
project since then. Since the fork from ImageMagick many improvements
have been made (see `NEWS`_) by many `authors`_ using an open
development model but without breaking the API or utilities operation.

Here are some reasons to prefer GraphicsMagick over ImageMagick or
other popular software:

  * GM is more efficient than ImageMagick so it gets the job done
    faster using fewer resources.

  * GM is much smaller and lighter than ImageMagick (3-5X smaller
    installation footprint).

  * GM supports classic ImageMagick interfaces such as 'convert'
    whereas ImageMagick has now removed them.

  * GM has been used to process billions of files at the world's
    largest photo sites (e.g. `Flickr`_ and `Etsy`_).

  * GM does not conflict with other installed software.

  * GM suffers from fewer security issues and exploits than ImageMagick.

  * GM participates in Google's `oss-fuzz`_ project (since February, 2018).

  * GM `valgrind`_'s 100% clean (memcheck and helgrind).

  * GM passes rigorous memory error testing using `ASan`_.

  * GM passes undefined behavior testing using `UBSan`_.

  * GM comes with a comprehensive `manual page`_.

  * GM provides API and ABI stability and managed releases that you can
    count on (ImageMagick does not).

  * GM provides detailed yet comprehensible `ChangeLog`_ and `NEWS`_
    files (ImageMagick does not).

  * GM is available for free, and may be used to support both open and
    proprietary applications.

  * GM is distributed under an X11-style license (`MIT License`_),
    approved by the `Open Source Initiative`_, recommended for use by
    the `OSSCC`_, and compatible with the `GNU GPL`_.

  * GM source code is managed in Mercurial_, a distributed source
    control management tool which supports management of local
    changes.  The repository history goes back to 1998.

  * GM has 0.00 (zero) defects per 1000 lines of code (352,673 total
    lines included) according to `Coverity Analysis Metrics`_ on July
    26th, 2024.

  * GM developers contribute to other free projects for the public good.

GraphicsMagick is `copyrighted <Copyright.html>`_ by the GraphicsMagick
Group as well as many others.

Here are just a few examples of what GraphicsMagick can do:

  * Convert an image from one format to another (e.g. TIFF to JPEG)

  * Resize, rotate, sharpen, color reduce, or add special effects to an
    image

  * Create a montage of image thumbnails

  * Create a transparent image suitable for use on the Web

  * Compare two images

  * Turn a group of images into a GIF animation sequence

  * Create a composite image by combining several separate images

  * Draw shapes or text on an image

  * Decorate an image with a border or frame

  * Describe the format and characteristics of an image
