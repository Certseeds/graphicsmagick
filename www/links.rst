.. -*- mode: rst -*-
.. This text is in reStucturedText format, so it may look a bit odd.
.. See http://docutils.sourceforge.net/rst.html for details.

=====================
Related Links
=====================

.. contents::

GraphicsMagick Topics
=====================

`Free Software Foundation <http://directory.fsf.org/project/GraphicsMagick/>`_ GraphicsMagick Entry.

`FreeBSD port <https://www.freshports.org/graphics/GraphicsMagick/>`_ for GraphicsMagick.

`NetBSD/pkgsrc port <http://pkgsrc.se/graphics/GraphicsMagick/>`_ for GraphicsMagick.

`Gentoo Sunrise <http://bugs.gentoo.org/show_bug.cgi?id=190372>`_ Bug Tracker Entry.

`Red Hat Linux <https://bugzilla.redhat.com/buglist.cgi?component=GraphicsMagick&product=Fedora>`_ GraphicsMagick related bugs.

Debian `GraphicsMagick packages <http://packages.debian.org/search?keywords=GraphicsMagick&amp;searchon=names&amp;suite=all&amp;section=main>`_.

`WikiPedia <http://en.wikipedia.org/wiki/GraphicsMagick>`_ GraphicsMagick Entry.

`Black Duck | Open HUB <https://www.openhub.net/p/graphics_magick>`_ GraphicsMagick Entry.

`Repology <https://repology.org/project/graphicsmagick/versions>`_ GraphicsMagick Package Versions in OS distributions.

`SourceForge <https://sourceforge.net/projects/graphicsmagick/>`_ GraphicsMagick Entry.

`GraphicsMagick questions on StackOverflow <http://stackoverflow.com/questions/tagged/graphicsmagick?>`_.

`GraphicsMagick ABI/API Changes <https://abi-laboratory.pro/tracker/timeline/graphicsmagick/>`_.



GraphicsMagick Vulnerabilities
==============================

Search for `GraphicsMagick`__ CVEs.

.. _CVE_GraphicsMagick : https://www.cvedetails.com/vulnerability-list/vendor_id-2802/Graphicsmagick.html

__ CVE_GraphicsMagick_

Debian vulnerabilities in `GraphicsMagick`__.

.. _Debian_GraphicsMagick : https://security-tracker.debian.org/tracker/source-package/graphicsmagick

__ Debian_GraphicsMagick_

Oss-fuzz issues pertaining to `GraphicsMagick`__.

.. _oss_fuzz_reports : https://issues.oss-fuzz.com/savedsearches/6886196

__ oss_fuzz_reports_

Oss-fuzz build status for `GraphicsMagick`__.

.. _oss_fuzz_build_status : https://oss-fuzz-build-logs.storage.googleapis.com/index.html#graphicsmagick

__ oss_fuzz_build_status_


Image Processing Topics
============================

`HyperMedia Image Processing Reference <http://homepages.inf.ed.ac.uk/rbf/HIPR2/>`_,
A guide to image processing algorithms, many of which are supported by GraphicsMagick.


Color Technology Related Topics
======================================

Charles Poynton's `Color technology FAQs <http://poynton.ca/Poynton-color.html>`_,
very useful documentation on color technologies.

`International Color Consortium <https://www.color.org/>`_,
the organization responsible for color profile standards.

`Argyll Color Management System <http://www.argyllcms.com/>`_,
a free experimental color management system.

`Littlecms <https://www.littlecms.com/>`_,
a free commercial-grade colormanagement engine in 100K (and used by GraphicsMagick).

`Bruce Lindbloom's Web Site <http://www.brucelindbloom.com/>`_,
offering interesting information related to color science and working spaces.

`sRGB <https://www.w3.org/Graphics/Color/sRGB.html>`_,
A standard default color space for the Internet.

`High Dynamic Range Image Encodings <http://www.anyhere.com/gward/hdrenc/hdr_encodings.html>`_,
an analysis by Greg Ward of various HDR encodings.

Gamma Related Topics
======================================

While most computer images are encoded with a gamma of 2.2 (really 2.2
to 2.6), GraphicsMagick does not attempt to convert images to
linear-light before applying image processing operations since it is
not possible to know for sure to know how to do so.  Some algorithms
such as resize, blur, and composition, will produce more accurate
results when performed on images encoded in a linear-light scaled
space.

For a typical sRGB image encoded in a gamma-corrected space with gamma
2.2, the option ``-gamma 0.45`` (1/2.2 = 0.45) will remove that
encoding for subsequent algorithms so that they are done in
linear-light space.  When processing is completed, then ``-gamma 2.2``
will restore gamma-correction for viewing.  It is recommended to use a
Q16 or Q32 build of GraphicsMagick when doing this since linear-light
space encoding is not efficient and will lose accuracy if stored with
less than 14 bits per sample.

The following documents and pages provide interesting information on
gamma-related topics:

`Charles Poynton's Gamma FAQ <http://poynton.ca/GammaFAQ.html>`_,
provides an excellent description of what gamma is, why it is good,
and when you don't want it.


TIFF Related Topics
============================

`LibTIFF <https://libtiff.gitlab.io/libtiff/>`_,
Libtiff library and TIFF format mailing list.

AWare Systems `TIFF <https://www.awaresystems.be/imaging/tiff.html>`_ site.
Detailed TIFF-related information which goes beyond the TIFF specification,
list archives for the libtiff mailing list, and information regarding the emerging Big TIFF format.

`Digital Negative (DNG) <https://helpx.adobe.com/camera-raw/digital-negative.html>`_,
Adobe TIFF specification for digital camera raw images.

`LogLuv Encoding for TIFF Images <http://www.anyhere.com/gward/pixformat/tiffluv.html>`_,
A way to store HDR images using TIFF.

JPEG Related Topics
==========================

`Independent JPEG Group <http://www.ijg.org/>`_ (home of IJG JPEG library).

`Guido Vollbeding's JPEG site <http://jpegclub.org/>`_, including various patches to IJG JPEG release 6b.

DICOM Related Topics
============================

`David Clunie's Medical Image Format Site <http://www.dclunie.com/>`_,
information about medical images.

Metadata (Associated Data) Topics
=========================================

`Extensible Metadata Platform (XMP) <http://www.adobe.com/products/xmp/index.html>`_,
Adobe's XML-based embedded metadata format.

`EXIF <https://www.exif.org/>`_,
Format for metadata in images, particularly JPEG files from digital cameras.

High Dynamic Range Topics
==========================

`High Dynamic Range Image Encodings <http://www.anyhere.com/gward/hdrenc/hdr_encodings.html>`_,
An analsys by Greg Ward of various HDR encodings.

`LogLuv Encoding for TIFF Images <http://www.anyhere.com/gward/pixformat/tiffluv.html>`_,
A way to store HDR images using TIFF.

`OpenEXR <http://www.openexr.com/>`_,
library and sample tools for dealing with high dynamic-range (HDR) images.

Motion Picture Links
=========================

`Digital Cinema Initiatives <https://www.dcimovies.com/>`_,
DCI offers the first complete specification for digital cinema delivery.

`Ingex <http://ingex.sourceforge.net/index.html>`_ Tapeless video &
audio capture, transcoding and network file serving.  From the BBC.

Video Topics
=============

`Video Codecs and Pixel Formats <http://www.fourcc.org/>`_, offers a summary of YUV encoding formats.

Other Software Packages
========================

`eLynx lab <http://elynxlab.free.fr/en/index.html>`_ High resolution image processing tool.

The `GIMP <https://www.gimp.org/>`_, interactive image editing software (like Photoshop).

`ImageMagick <https://imagemagick.org/index.php>`_, the ancestor of GraphicsMagick.

`VIPS <https://github.com/libvips/>`_, an image processing system also useful with
large images, and which comes with an unusual GUI.

`FreeImage <https://freeimage.sourceforge.io/index.html>`_,
a free image processing library.

`ImageJ <https://imagej.nih.gov/ij/>`_ Image Processing and Analysis in Java.

`Pstoedit <http://www.pstoedit.net/>`_,
A Postscript to editable vector translation utility.

`UFRaw <http://ufraw.sourceforge.net/>`_,
a utility to read and manipulate raw images from digital cameras.

`LPROF <http://lprof.sourceforge.net/index.html>`_,
an open source ICC profiler with graphical user interface.

`Gallery <http://gallery.menalto.com/>`_,
a facinating web-based photo album organizer.  Works with GraphicsMagick!.

`DJV Imaging <https://darbyjohnston.github.io/DJV/>`_, professional movie
playback and image processing software for the film and computer
animation industries.

`OpenImageIO <https://sites.google.com/site/openimageio/>`_ library
for reading and writing images, and a bunch of related classes,
utilities, and applications.

Stock Photos
=============

`MorgueFile <https://morguefile.com/>`_, Free high-resolution stock photo images.
