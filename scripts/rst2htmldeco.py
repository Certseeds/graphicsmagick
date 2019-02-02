#!/usr/bin/env python
# vim:ts=4:sw=4:expandtab

#* Author: Mark Mitchell
#* Copyright 2005-2008  Mark Mitchell, All Rights Reserved
#* License: see __license__ below.


__doc__ = """
Renders reStructuredText files to HTML using the custom docutils writer
docutils_htmldeco_writer, which inserts a common header and navigation menu at
the top of all HTML pages.  The header and navigation HTML blobs are
kept in a separate python file, html_fragments.py.

Usage:
    rst2htmldeco.py [options] SRCFILE OUTFILE

    SRCFILE is the path to a restructured text file.
    For example: ./AUTHORS.txt

    OUTFILE is the path where the HTML file is written.
    For example: ./www/authors.html

    Options:

    -h --help               Print this help message
    -e --embed-stylesheet=<FILE>
                            The stylesheet will be embedded in the HTML.
    -l --link-stylesheet=<URL>
                            The HTML file will Link to the stylesheet,
                            using the URL verbatim.
    -r --url-prefix=<PATH>
                            The value of url-prefix is prefixed to all the URLs in the
                            path to get to the top directory from the OUTFILE
                            directory.  Used to create working relative URLs in the custom
                            header and navigation menu.  Defaults to empty.
                            Example: --url-prefix=../../


    -e and -l are mutually excusive.  If neither is specified, nothing
    will work.

    Note that images referenced from the stylesheet may require different
    image-dir paths depending on whether the stylesheet is embedded or linked.
    If linked, image URLs in the stylesheet are relative to the stylesheet's
    location.  If embedded, image URLs in the stylesheet are relative to the
    HTML file's path.

Embedding vs. linking stylesheet
=================================

Embedding stylesheet
----------------------
* The HTML file needs no additional files in particular locations in order
  to render nicely.
* The size of every HTML file is inflated.  Download time is increased.

Linking stylesheet
-------------------
* Best performance for files served via HTTP, as browsers will download the
  stylesheet once and cache it, and individual HTML files will be more compact.

* No need to regenerate every HTML file if the stylesheet is changed.

* If the HTML file is detached from stylesheet, it will look very plain
  when rendered by a browser.

Images are always referenced (there's two, the logo and banner background),
so will not be rendered if the HTML file is detached from the image files.
"""

__copyright__ = "2005, 2008, Mark Mitchell"

__license__ = """
Copyright 2005, 2008, Mark Mitchell

Permission is hereby granted, free of charge, to any person obtaining
a copy of this oftware and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

The Software is provided "as is", without warranty of any kind,
express or implied, including but not limited to the warranties of
merchantability, fitness for a particular purpose and noninfringement.
In no event shall the authors or copyright holders be liable for any
claim, damages or other liability, whether in an action of contract,
tort or otherwise, arising from, out of or in connection with Software
or the use or other dealings in the Software.
"""

import sys
import os, os.path
import locale
import getopt

import html_fragments


try:
    locale.setlocale(locale.LC_ALL, '')
except:
    pass

from docutils.core import publish_cmdline, default_description


# Python module name of the custom HTML writer, must be in sys.path
rst_writer = 'docutils_htmldeco_writer'

docutils_opts = [
    '--cloak-email-addresses', # Obfusticate email addresses
    '--no-generator',          # no "Generated by Docutils" credit and link at the end of the document.
    # '--date',                # Include the date at the end of the document (UTC)
    # '--time',                # Include the time & date (UTC)
    '--no-datestamp',          # Do not include a datestamp of any kind
    '--traceback',             # Enable Python tracebacks when Docutils is halted.
    '--rfc-references',        # Recognize and link to standalone RFC references (like "RFC 822")
    '--rfc-base-url=http://tools.ietf.org/html/', # Base URL for RFC references
]


def make_html(src, dest, embed_stylesheet=False,
              stylesheet=None, url_prefix=''):

    # Fix the url_prefix path in the banner HTML.
    # This Python tricky business is necessary because the custom docutils
    # writer, which inserts the HTML blobs, can't do it, because docutils
    # doesn't provide any way to pass an argument such as image_dir to the
    # writer.
    # There must be a better way, but I haven't figured it out yet.
    if url_prefix:
        html_fragments.url_prefix = url_prefix.rstrip('/') + '/'
    html_fragments.nav = html_fragments.make_nav()
    html_fragments.banner = html_fragments.make_banner()
    html_fragments.footer = html_fragments.make_footer()

    args = list(docutils_opts)
    if embed_stylesheet:
        # for embedded stylesheet
        args.append('--stylesheet-path=%s' % stylesheet)
        args.append('--embed-stylesheet')
    else:
        # for linked stylesheet
        args.append('--stylesheet=%s' % stylesheet)
        args.append('--link-stylesheet')
    if src is not None:
        args.append(src)
    if dest is not None:
        args.append(dest)

    # Invoke docutils processing of the reST document.  This call
    # to a docutils method ends up executing the custom HTML writer
    # in docutils_htmldeco_writer.py
    description = ('Generates (X)HTML documents from standalone reStructuredText '
                   'sources, with custom banner and footer.  ' + default_description)
    publish_cmdline(writer_name=rst_writer,
                    description=description,
                    argv=args)


def main(argv=None):
    if argv is None:
        argv = sys.argv[1:]

    # parse command line options
    try:
        opts, posn_args = getopt.getopt(argv, 'he:l:u:',
                                       ['help',
                                        'embed-stylesheet=',
                                        'link-stylesheet=',
                                        'url-prefix=',
                                        ])
    except getopt.GetoptError, msg:
         print msg
         print __doc__
         return 1

    # process options
    embed_stylesheet = False
    link_stylesheet = True
    stylesheet = None
    url_prefix = ''

    for opt, val in opts:
        if opt in ("-h", "--help"):
            print __doc__
            return 0

        if opt in ("-e", "--embed-stylesheet"):
            link_stylesheet = False
            embed_stylesheet = True
            stylesheet = val

        if opt in ("-l", "--link-stylesheet"):
            embed_stylesheet = False
            link_stylesheet = True
            stylesheet = val

        if opt in ("-u", "--url-prefix"):
            url_prefix = val

    #if len(posn_args) != 2:
    #    print >> sys.stderr, 'Missing arguments'
    #    print >> sys.stderr, __doc__
    #    return 1

    try:
        srcfile_path = posn_args[0]
    except IndexError:
        srcfile_path = None
    try:
        outfile_path = posn_args[1]
    except IndexError:
        outfile_path = None

    make_html(srcfile_path, outfile_path, embed_stylesheet=embed_stylesheet,
              stylesheet=stylesheet, url_prefix=url_prefix)
    return 0


if __name__ == '__main__':
    sys.exit(main())
