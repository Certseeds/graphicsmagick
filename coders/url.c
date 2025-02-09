/*
% Copyright (C) 2003-2022 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            U   U  RRRR   L                                  %
%                            U   U  R   R  L                                  %
%                            U   U  RRRR   L                                  %
%                            U   U  R R    L                                  %
%                             UUU   R  R   LLLLL                              %
%                                                                             %
%                                                                             %
%                       Retrieve An Image Via a URL.                          %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                              Bill Radcliffe                                 %
%                                March 2000                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#if defined(HasXML)
#include "magick/blob.h"
#include "magick/confirm_access.h"
#include "magick/constitute.h"
#include "magick/magick.h"
#include "magick/tempfile.h"
#include "magick/utility.h"
#if defined(MSWINDOWS)
#  if defined(__MINGW32__)
#    if !defined(_MSC_VER)
#      define _MSC_VER 1200
#    endif
#  else
#    include <win32config.h>
#  endif
#endif
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#if defined(LIBXML_FTP_ENABLED)
#  if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT
#    include <libxml/nanoftp.h>
#  endif /* defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT */
#endif /* if defined(LIBXML_FTP_ENABLED) */
#if defined(LIBXML_HTTP_ENABLED)
#  if defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN
#    include <libxml/nanohttp.h>
#  endif /* defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN */
#endif /* if defined(LIBXML_HTTP_ENABLED) */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d U R L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadURLImage retrieves an image via a URL, decodes the image, and
%   returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadURLImage method is:
%
%      Image *ReadURLImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadURLImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(LIBXML_FTP_ENABLED)
#if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT
static void GetFTPData(void *userdata,const char *data,int length)
{
  FILE
    *file;

  file=(FILE *) userdata;
  if (file == (FILE *) NULL)
    return;
  if (length <= 0)
    return;
  (void) fwrite(data,length,1,file);
}
#endif /* if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT */
#endif /* if defined(LIBXML_FTP_ENABLED) */

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static Image *ReadURLImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
#define MaxBufferExtent  8192

  char
    filename[MaxTextExtent];

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *clone_info;

  ConfirmAccessMode
    access_mode=UndefinedConfirmAccessMode;

  image=(Image *) NULL;

  if (LocaleCompare(image_info->magick,"ftp") == 0)
    access_mode=URLGetFTPConfirmAccessMode;
  else if (LocaleCompare(image_info->magick,"http") == 0)
    access_mode=URLGetHTTPConfirmAccessMode;
  else if (LocaleCompare(image_info->magick,"file") == 0)
    access_mode=URLGetFileConfirmAccessMode;


  /* Attempt to re-compose original URL */
  (void) strlcpy(filename,image_info->magick,MaxTextExtent);
  LocaleLower(filename);
  (void) strlcat(filename,":",MaxTextExtent);
  (void) strlcat(filename,image_info->filename,MaxTextExtent);

  if (MagickConfirmAccess(access_mode,filename,exception)
      == MagickFail)
    return image;

  clone_info=CloneImageInfo(image_info);
  if (LocaleCompare(clone_info->magick,"file") == 0)
    {
      /* Skip over "//" at start of parsed filename */
      (void) strlcpy(clone_info->filename,image_info->filename+2,
                     sizeof(clone_info->filename));
      clone_info->magick[0]='\'';
      image=ReadImage(clone_info,exception);
    }
  else
    {
      clone_info->blob=(void *) NULL;
      clone_info->length=0;
      file=AcquireTemporaryFileStream(clone_info->filename,BinaryFileIOMode);
      if (file == (FILE *) NULL)
        {
          (void) strlcpy(filename,clone_info->filename,sizeof(filename));
          DestroyImageInfo(clone_info);
          ThrowReaderTemporaryFileException(filename);
        }
      if (LocaleCompare(clone_info->magick,"http") == 0)
        {
#if defined(LIBXML_HTTP_ENABLED)
#if defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN
          char
            buffer[MaxBufferExtent];

          void
            *context;

          char
            *type;

          int
            bytes;

          type=(char *) NULL;
          context=xmlNanoHTTPOpen(filename,&type);
          if (context != (void *) NULL)
            {
              while ((bytes=xmlNanoHTTPRead(context,buffer,MaxBufferExtent)) > 0)
                (void) fwrite(buffer,bytes,1,file);
              xmlNanoHTTPClose(context);
              xmlFree(type);
              xmlNanoHTTPCleanup();
            }
#endif /* if defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN */
#endif /* if defined(LIBXML_FTP_ENABLED) */
        }
      else if (LocaleCompare(clone_info->magick,"ftp") == 0)
        {
#if defined(LIBXML_FTP_ENABLED)
#if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT
          void
            *context;

          xmlNanoFTPInit();
          context=xmlNanoFTPNewCtxt(filename);
          if (context != (void *) NULL)
            {
              if (xmlNanoFTPConnect(context) >= 0)
                (void) xmlNanoFTPGet(context,GetFTPData,(void *) file,
                                     (char *) NULL);
              (void) xmlNanoFTPClose(context);
            }
#endif /* if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT */
#endif /* if defined(LIBXML_FTP_ENABLED) */
        }
      (void) fclose(file);
      if (!IsAccessibleAndNotEmpty(clone_info->filename))
        {
          (void) LiberateTemporaryFile(clone_info->filename);
          ThrowException(exception,CoderError,NoDataReturned,filename);
        }
      else
        {
          *clone_info->magick='\0';
          image=ReadImage(clone_info,exception);
        }
      (void) LiberateTemporaryFile(clone_info->filename);
    }
  DestroyImageInfo(clone_info);
  return(image);
}
#endif /* defined(HasXML) */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r U R L I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterURLImage adds attributes for the URL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterURLImage method is:
%
%      RegisterURLImage(void)
%
*/
ModuleExport void RegisterURLImage(void)
{
#if defined(HasXML)
  MagickInfo
    *entry;

  /* HTTP URLs are not encouraged on the Internet */
#if defined(LIBXML_HTTP_ENABLED)
#if defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN
  entry=SetMagickInfo("HTTP");
  entry->decoder=(DecoderHandler) ReadURLImage;
  entry->description="Uniform Resource Locator (http://)";
  entry->module="URL";
  entry->extension_treatment=IgnoreExtensionTreatment;
  entry->coder_class=UnstableCoderClass;
  (void) RegisterMagickInfo(entry);
#endif /* if defined(HAVE_XMLNANOHTTPOPEN) && HAVE_XMLNANOHTTPOPEN */
#endif /* if defined(LIBXML_HTTP_ENABLED) */

  /* FTP URLs have been deprecated for quite some time already */
#if defined(LIBXML_FTP_ENABLED)
#if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT
  entry=SetMagickInfo("FTP");
  entry->decoder=(DecoderHandler) ReadURLImage;
  entry->description="Uniform Resource Locator (ftp://)";
  entry->module="URL";
  entry->extension_treatment=IgnoreExtensionTreatment;
  entry->coder_class=UnstableCoderClass;
  (void) RegisterMagickInfo(entry);
#endif /* if defined(HAVE_XMLNANOFTPNEWCTXT) && HAVE_XMLNANOFTPNEWCTXT */
#endif /* if defined(LIBXML_FTP_ENABLED) */

  entry=SetMagickInfo("FILE");
  entry->decoder=(DecoderHandler) ReadURLImage;
  entry->description="Uniform Resource Locator (file://)";
  entry->extension_treatment=IgnoreExtensionTreatment;
  entry->module="URL";
  entry->coder_class=StableCoderClass;
  (void) RegisterMagickInfo(entry);
#endif /* defined(HasXML) */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r U R L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterURLImage removes format registrations made by the
%  URL module from the list of supported formats.
%
%  The format of the UnregisterURLImage method is:
%
%      UnregisterURLImage(void)
%
*/
ModuleExport void UnregisterURLImage(void)
{
#if defined(HasXML)
  (void) UnregisterMagickInfo("HTTP");
  (void) UnregisterMagickInfo("FTP");
  (void) UnregisterMagickInfo("FILE");
#endif /* defined(HasXML) */
}
