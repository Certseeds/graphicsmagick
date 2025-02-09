/*
% Copyright (C) 2013-2024 GraphicsMagick Group
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         W   W  EEEEE  BBBB   PPPP                           %
%                         W   W  E      B   B  P   P                          %
%                         W W W  EEE    BBBB   PPPP                           %
%                         WW WW  E      B   B  P                              %
%                         W   W  EEEEE  BBBB   P                              %
%                                                                             %
%                                                                             %
%                     Read/Write Google WEBP Image Format.                    %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                  TIMEBUG                                    %
%                                January 2013                                 %
%                                                                             %
%                          Subsequent Development By                          %
%                               Bob Friesenhahn                               %
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
#include "magick/attribute.h"
#include "magick/blob.h"
#include "magick/colormap.h"
#include "magick/log.h"
#include "magick/constitute.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/pixel_cache.h"
#include "magick/profile.h"
#include "magick/tsd.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
#if defined(HasWEBP)
static unsigned int WriteWEBPImage(const ImageInfo *,Image *);
#else
 #define WebPGetEncoderVersion()  (0)
 #define WEBP_ENCODER_ABI_VERSION 0
#endif

#if defined(HasWEBP)
#include <webp/decode.h>
#include <webp/encode.h>

/*
  Release versions vs ABI versions (found in src/webp/encode.h)

    0.1.3  - 0x0002
    0.1.99 - 0x0100 <-- earliest version we support
    0.2.0  - 0x0200
    0.2.1  - 0x0200
    0.3.0  - 0x0201
    0.4.0  - 0x0202
    0.4.1  - 0x0202
    0.4.2  - 0x0202
    0.4.3  - 0x0202
    0.4.4  - 0x0202
    0.5.0  - 0x0209
    0.5.1  - 0x0209
    0.5.2  - 0x0209
    0.6.0  - 0x020e
*/

/*
  Progress indication support not added until v0.1.99
*/
#if WEBP_ENCODER_ABI_VERSION >= 0x0100 /* >= v0.1.99 */
#  define SUPPORT_PROGRESS 1
#  define SUPPORT_USER_ABORT
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x0200 /* >= 0.2.0 */
#  define SUPPORT_CONFIG_WEBP_HINT_GRAPH
#endif
/* These relate to 'struct WebPConfig' parameters */
#if WEBP_ENCODER_ABI_VERSION >= 0x0201 /* >= 0.3.0 */
#  define SUPPORT_CONFIG_EMULATE_JPEG_SIZE
#  define SUPPORT_CONFIG_THREAD_LEVEL
#  define SUPPORT_CONFIG_LOW_MEMORY
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x0202 /* >= 0.4.0 */
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x0209 /* >= 0.5.0 */
#  define SUPPORT_WEBP_MUX
#  define SUPPORT_WEBP_EXACT
#include <webp/mux.h>
#endif
#if WEBP_ENCODER_ABI_VERSION >= 0x020e /* >= 0.6.0 */
#  define SUPPORT_CONFIG_USE_SHARP_YUV
#endif

/*
  Global declarations.
*/
static MagickTsdKey_t tsd_key = (MagickTsdKey_t) 0;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W E B P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadWEBPImage() reads an image in the WebP image format.
%
%  The format of the ReadWEBPImage method is:
%
%      Image *ReadWEBPImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadWEBPImage(const ImageInfo *image_info,
                            ExceptionInfo *exception)
{
  Image
    *image;

  unsigned long
    count,
    y;

  register PixelPacket
    *q;

  size_t
    length;

  register size_t
    x;

  register unsigned char
    *p;

  unsigned char
    *stream,
    *pixels;

  WebPBitstreamFeatures
    stream_features;

  int
    webp_status;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AllocateImage(image_info);
  if (OpenBlob(image_info,image,ReadBinaryBlobMode,exception) == MagickFail)
    ThrowReaderException(FileOpenError,UnableToOpenFile,image);
  /*
    Read WEBP file.
  */
  length = (size_t) GetBlobSize(image); /* FIXME, does not work with stream */
  stream=MagickAllocateResourceLimitedArray(unsigned char *,
                                            length,sizeof(*stream));
  if (stream == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  count=(long) ReadBlob(image,length,(char *) stream);
  if (count != (size_t) length)
    {
      MagickFreeResourceLimitedMemory(stream);
      ThrowReaderException(CorruptImageError,InsufficientImageDataInFile,image);
    }
  if ((webp_status=WebPGetFeatures(stream,length,&stream_features)) != VP8_STATUS_OK)
    {
      MagickFreeResourceLimitedMemory(stream);

      switch (webp_status)
        {
#if !defined(__COVERITY__)
        case VP8_STATUS_OK:
          break;
#else
        default:
          break;
#endif
        case VP8_STATUS_OUT_OF_MEMORY:
          ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
          break;
        case VP8_STATUS_INVALID_PARAM:
          ThrowReaderException(CoderError,WebPInvalidParameter,image);
          break;
        case VP8_STATUS_BITSTREAM_ERROR:
          ThrowReaderException(CorruptImageError,CorruptImage,image);
          break;
        case VP8_STATUS_UNSUPPORTED_FEATURE:
          ThrowReaderException(CoderError,DataEncodingSchemeIsNotSupported,image);
          break;
        case VP8_STATUS_SUSPENDED:
          /*
            Incremental decoder object may be left in SUSPENDED state
            if the picture is only partially decoded, pending
            additional input.  We are not doing incremental decoding
            at this time.
          */
          break;
        case VP8_STATUS_USER_ABORT:
          /*
            This is what is returned if the user terminates the
            decoding.
          */
          ThrowReaderException(CoderError,WebPDecodingFailedUserAbort,image);
          break;
        case VP8_STATUS_NOT_ENOUGH_DATA:
          ThrowReaderException(CorruptImageError,InsufficientImageDataInFile,image);
          break;
        }
      /*
        Catch-all if not handled above.
      */
      ThrowReaderException(CorruptImageError,CorruptImage,image);
    }
  image->depth=8;
  image->columns=(size_t) stream_features.width;
  image->rows=(size_t) stream_features.height;
  image->matte=(stream_features.has_alpha ? MagickTrue : MagickFalse);
  if (image->ping)
    {
      MagickFreeResourceLimitedMemory(stream);
      CloseBlob(image);
      StopTimer(&image->timer);
      return(image);
    }
  if (CheckImagePixelLimits(image, exception) != MagickPass)
    {
      MagickFreeResourceLimitedMemory(stream);
      ThrowReaderException(ResourceLimitError,ImagePixelLimitExceeded,image);
    }
  if (image->matte)
    pixels=(unsigned char *) WebPDecodeRGBA(stream,length,
                                            &stream_features.width,
                                            &stream_features.height);
  else
    pixels=(unsigned char *) WebPDecodeRGB(stream,length,
                                           &stream_features.width,
                                           &stream_features.height);
  if (pixels == (unsigned char *) NULL)
    {
      MagickFreeResourceLimitedMemory(stream);
      ThrowReaderException(CoderError,NoDataReturned,image);
    }

  p=pixels;

  for (y=0; y < (size_t) image->rows; y++)
    {
      q=SetImagePixelsEx(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        break;

      for (x=0; x < (size_t) image->columns; x++)
        {
          SetRedSample(q,ScaleCharToQuantum(*p++));
          SetGreenSample(q,ScaleCharToQuantum(*p++));
          SetBlueSample(q,ScaleCharToQuantum(*p++));
          if (image->matte)
            SetOpacitySample(q,MaxRGB-ScaleCharToQuantum(*p++));
          else
            SetOpacitySample(q,OpaqueOpacity);
          q++;
        }

      if (!SyncImagePixels(image))
        break;
    }

#if defined(SUPPORT_WEBP_MUX)
  /*
    Read features out of the WebP container
    https://developers.google.com/speed/webp/docs/container-api
  */
  {
    uint32_t webp_flags=0;
    WebPData flag_data;
    WebPData content={stream,length};
    WebPMuxError mux_error;

    WebPMux *mux=WebPMuxCreate(&content,0);
    (void) memset(&flag_data,0,sizeof(flag_data));
    WebPMuxGetFeatures(mux,&webp_flags);

    if ((webp_flags & ICCP_FLAG) &&
        ((mux_error=WebPMuxGetChunk(mux,"ICCP",&flag_data)) == WEBP_MUX_OK))
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"ICCP Profile: %lu bytes",
                              (unsigned long) flag_data.size);
        if ((flag_data.bytes != NULL) && (flag_data.size > 0))
          SetImageProfile(image,"ICC",flag_data.bytes,flag_data.size);
      }

    if ((webp_flags & EXIF_FLAG) &&
        ((mux_error=WebPMuxGetChunk(mux,"EXIF",&flag_data)) == WEBP_MUX_OK))
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"EXIF Profile: %lu bytes",
                              (unsigned long) flag_data.size);
        if ((flag_data.bytes != NULL) && (flag_data.size > 0))
          {
            const int has_app1_hdr=
              (flag_data.size >= MAGICK_JPEG_APP1_EXIF_HEADER_SIZE) &&
              (memcmp((const void *) flag_data.bytes,
                      (const void *) MAGICK_JPEG_APP1_EXIF_HEADER,
                      MAGICK_JPEG_APP1_EXIF_HEADER_SIZE) == 0);
            const size_t header_size=has_app1_hdr ? 0 : MAGICK_JPEG_APP1_EXIF_HEADER_SIZE;
            const size_t profile_size=flag_data.size+header_size;
            unsigned char *profile=MagickAllocateResourceLimitedMemory(unsigned char *,profile_size);
            if (has_app1_hdr == 0)
              (void) memcpy((void *) profile, (const void *) MAGICK_JPEG_APP1_EXIF_HEADER,
                            MAGICK_JPEG_APP1_EXIF_HEADER_SIZE);
            (void) memcpy((void *) (profile+header_size),flag_data.bytes,flag_data.size);
            SetImageProfile(image,"EXIF",profile,profile_size);
            MagickFreeResourceLimitedMemory(profile);
          }
      }

    if ((webp_flags & XMP_FLAG) &&
        ((mux_error=WebPMuxGetChunk(mux,"XMP",&flag_data)) == WEBP_MUX_OK))
      {
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),"XMP Profile: %lu bytes",
                              (unsigned long) flag_data.size);
        if ((flag_data.bytes != NULL) && (flag_data.size > 0))
          SetImageProfile(image,"XMP",flag_data.bytes,flag_data.size);
      }

    WebPMuxDelete(mux);
  }
#endif /* defined(SUPPORT_WEBP_MUX) */

  /*
    Free scale resource.
  */
  free(pixels);
  pixels=(unsigned char *) NULL;
  MagickFreeResourceLimitedMemory(stream);
  CloseBlob(image);

  /*
    Retrieve image orientation from EXIF (if present) and store in
    image.
  */
  {
    const ImageAttribute
      *attribute;

    attribute = GetImageAttribute(image,"EXIF:Orientation");
    if ((attribute != (const ImageAttribute *) NULL) &&
        (attribute->value != (char *) NULL))
      {
        int
          orientation;

        orientation=MagickAtoI(attribute->value);
        if ((orientation > UndefinedOrientation) &&
            (orientation <= LeftBottomOrientation))
          image->orientation=(OrientationType) orientation;
      }
  }

  StopTimer(&image->timer);
  return(image);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r W E B P I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterWEBPImage adds attributes for the WEBP image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWEBPImage method is:
%
%      RegisterWEBPImage(void)
%
*/
ModuleExport void RegisterWEBPImage(void)
{
  static const char
    description[] = "WebP Image Format";

  static char
    version[41];

  MagickInfo
    *entry;

  int
    web_encoder_version;

  unsigned int
    webp_major,
    webp_minor,
    webp_revision;

  *version='\0';

  /*
    Initialize thread specific data key.
  */
  if (tsd_key == (MagickTsdKey_t) 0)
    (void) MagickTsdKeyCreate(&tsd_key);

  /*
    Obtain the encoder's version number from the library, packed in
    hexadecimal using 8bits for each of major/minor/revision. E.g:
    v2.5.7 is 0x020507.

    Also capture the encoder ABI version from <webp/encode.h> which is
    in the form MAJOR(8b) + MINOR(8b) where ABI is related to the
    library ABI and not the package release version.
  */
  web_encoder_version=(WebPGetEncoderVersion());
  webp_major=(web_encoder_version >> 16) & 0xff;
  webp_minor=(web_encoder_version >> 8) & 0xff;
  webp_revision=web_encoder_version & 0xff;
  (void) snprintf(version,sizeof(version),
                  "libwepb v%u.%u.%u, ENCODER ABI 0x%04X", webp_major,
                  webp_minor, webp_revision, WEBP_ENCODER_ABI_VERSION);

  entry=SetMagickInfo("WEBP");
#if defined(HasWEBP)
  entry->decoder=(DecoderHandler) ReadWEBPImage;
  entry->encoder=(EncoderHandler) WriteWEBPImage;
#endif
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue; /* FIXME */
  if (*version != '\0')
    entry->version=version;
  entry->module="WEBP";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r W E B P I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterWEBPImage removes format registrations made by the
%  WEBP module from the list of supported formats.
%
%  The format of the UnregisterWEBPImage method is:
%
%      UnregisterWEBPImage(void)
%
*/
ModuleExport void UnregisterWEBPImage(void)
{
  (void) UnregisterMagickInfo("WEBP");

  /*
    Destroy thread specific data key.
  */
  if (tsd_key != (MagickTsdKey_t) 0)
    {
      (void) MagickTsdKeyDelete(tsd_key);
      tsd_key = (MagickTsdKey_t) 0;
    }
}

#if defined(HasWEBP)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e W E B P I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteWEBPImage() writes an image in the WebP image format.
%
%  The format of the WriteWEBPImage method is:
%
%      MagickPassFail WriteWEBPImage(const ImageInfo *image_info, Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
*/

#if !defined(SUPPORT_WEBP_MUX)
 /*
  Called to write data to blob ("Should return true if writing was
  successful")
*/
static int WriterCallback(const unsigned char *stream,size_t length,
                          const WebPPicture *const picture)
{
  Image
    *image;

  image=(Image *) picture->custom_ptr;
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  return (length != 0U ? (WriteBlob(image,length,stream) == length) :
          MagickTrue);
}
#endif /* !defined(SUPPORT_WEBP_MUX) */

/*
  Called to provide progress indication ("It can return false to
  request an abort of the encoding process, or true otherwise if
  everything is OK.")
*/
#if defined(SUPPORT_PROGRESS)
static int ProgressCallback(int percent, const WebPPicture* picture)
{
  Image
    *image;

  /*
    When the WebPMemoryWriter is used, it commandeers custom_ptr
    and the ProgressCallback no longer has access to image.

    We use thread specific data instead.
   */
  (void) picture;
  image=(Image *) MagickTsdGetSpecific(tsd_key);
#if 0
  image=(Image *) picture->custom_ptr;
#endif
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  return MagickMonitorFormatted(percent, 101, &image->exception,
                                SaveImageText, image->filename,
                                image->columns, image->rows);
}
#endif /* defined(SUPPORT_PROGRESS) */

static unsigned int WriteWEBPImage(const ImageInfo *image_info,Image *image)
{
  const char
    *value;

  int
    webp_status;

  unsigned int
    status;

  register PixelPacket
    *p;

  register size_t
    x;

  register unsigned char
    *q;

  unsigned long
    y;

  size_t
    per_column;

  unsigned char
    *pixels;

  WebPConfig
    configure;

  WebPPicture
    picture;

#if defined(SUPPORT_WEBP_MUX)
  WebPMemoryWriter
    writer;
#endif /* defined(SUPPORT_WEBP_MUX) */

  WebPAuxStats
    statistics;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if ((image->columns > 16383) || (image->rows > 16383))
    ThrowWriterException(ImageError,WidthOrHeightExceedsLimit,image);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFail)
    ThrowWriterException(FileOpenError,UnableToOpenFile,image);
  /*
    Initialize WebP picture. This function is declared as 'static
    inline'.  It returns false if there is a mismatch between the
    libwebp headers and library.
  */
  if (WebPPictureInit(&picture) == 0)
    ThrowWriterException(DelegateError, WebPABIMismatch, image);
  /*
    Make sure that image is in an RGB type space and DirectClass.
  */
  (void) TransformColorspace(image,RGBColorspace);
  image->storage_class=DirectClass;
  picture.use_argb = 1;

  (void) MagickTsdSetSpecific(tsd_key,(void *) image);

#if !defined(SUPPORT_WEBP_MUX)
  picture.writer=WriterCallback;
  picture.custom_ptr=(void *) image;
#else
  WebPMemoryWriterInit(&writer);
  picture.writer=WebPMemoryWrite;
  picture.custom_ptr=&writer;
#endif
#if defined(SUPPORT_PROGRESS)
  picture.progress_hook=ProgressCallback;
#endif
  picture.stats=(&statistics);
  picture.width=(int) image->columns;
  picture.height=(int) image->rows;
  if (WebPConfigInit(&configure) == 0)
    ThrowWriterException(DelegateError, WebPABIMismatch, image);
  if (image_info->quality != DefaultCompressionQuality)
    configure.quality = (float) image_info->quality;

  if ((value=AccessDefinition(image_info,"webp","lossless")))
    {
      configure.lossless=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
#if defined(SUPPORT_WEBP_EXACT)
      /* Preserve RGB channels in 100% transparent areas */
      configure.exact=1;
#endif
    }
  if ((value=AccessDefinition(image_info,"webp","method")))
    configure.method=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","image-hint")))
    {
      if (LocaleCompare(value,"default") == 0)
        configure.image_hint=WEBP_HINT_DEFAULT;
      if (LocaleCompare(value,"picture") == 0)
        configure.image_hint=WEBP_HINT_PICTURE;
      if (LocaleCompare(value,"photo") == 0)
        configure.image_hint=WEBP_HINT_PHOTO;
#if defined(SUPPORT_CONFIG_WEBP_HINT_GRAPH)
      if (LocaleCompare(value,"graph") == 0)
        configure.image_hint=WEBP_HINT_GRAPH;
#endif
    }
  if ((value=AccessDefinition(image_info,"webp","target-size")))
    configure.target_size=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","target-psnr")))
    configure.target_PSNR=(float) MagickAtoF(value);
  if ((value=AccessDefinition(image_info,"webp","segments")))
    configure.segments=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","sns-strength")))
    configure.sns_strength=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","filter-strength")))
    configure.filter_strength=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","filter-sharpness")))
    configure.filter_sharpness=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","filter-type")))
    configure.filter_type=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","auto-filter")))
    configure.autofilter=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
  if ((value=AccessDefinition(image_info,"webp","alpha-compression")))
    configure.alpha_compression=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","alpha-filtering")))
    configure.alpha_filtering=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","alpha-quality")))
    configure.alpha_quality=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","pass")))
    configure.pass=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","show-compressed")))
    configure.show_compressed=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
  if ((value=AccessDefinition(image_info,"webp","preprocessing")))
    configure.preprocessing=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","partitions")))
    configure.partitions=MagickAtoI(value);
  if ((value=AccessDefinition(image_info,"webp","partition-limit")))
    configure.partition_limit=MagickAtoI(value);
#if defined(SUPPORT_CONFIG_EMULATE_JPEG_SIZE)
  if ((value=AccessDefinition(image_info,"webp","emulate-jpeg-size")))
    configure.emulate_jpeg_size=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
#endif
#if defined(SUPPORT_CONFIG_THREAD_LEVEL)
  if ((value=AccessDefinition(image_info,"webp","thread-level")))
    configure.thread_level=MagickAtoI(value);
#endif
#if defined(SUPPORT_CONFIG_LOW_MEMORY)
  if ((value=AccessDefinition(image_info,"webp","low-memory")))
    configure.low_memory=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
#endif
#if defined(SUPPORT_CONFIG_USE_SHARP_YUV)
  if ((value=AccessDefinition(image_info,"webp","use-sharp-yuv")))
    configure.use_sharp_yuv=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
#endif
#if defined(SUPPORT_WEBP_EXACT)
  if ((value=AccessDefinition(image_info,"webp","exact")))
    {
      /* Preserve RGB channels in 100% transparent areas */
      configure.exact=(LocaleCompare(value,"TRUE") == 0 ? 1 : 0);
    }
#endif
  if (WebPValidateConfig(&configure) != 1)
    ThrowWriterException(CoderError,WebPInvalidConfiguration,image);

  if (configure.lossless == 1)
    {
      /*
        Use ARGB input for lossless (YUVA input is lossy).
      */
      webp_status = WebPPictureAlloc(&picture);

      for (y = 0; y < image->rows; y++)
        {
          magick_uint32_t *s = picture.argb + (size_t)y * picture.argb_stride;
          p=GetImagePixelsEx(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x = 0; x < image->columns; x++)
            {
              *s = ((!image->matte ? 0xff000000u : (magick_uint32_t)
                     ScaleQuantumToChar(MaxRGB-GetOpacitySample(p)) << 24) |
                    (ScaleQuantumToChar(GetRedSample(p)) << 16) |
                    (ScaleQuantumToChar(GetGreenSample(p)) << 8) |
                    (ScaleQuantumToChar(GetBlueSample(p))));
              s++;
              p++;
            }
        }
    }
  else
    {
      /*
        Allocate memory for pixels.
      */
      per_column = MagickArraySize(MagickArraySize(4,image->rows),sizeof(*pixels));
      pixels=MagickAllocateResourceLimitedArray(unsigned char *,image->columns,per_column);
      if (pixels == (unsigned char *) NULL)
        ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);

      /*
        Convert image to WebP raster pixels.
      */
      q=pixels;
      for (y=0; y < image->rows; y++)
        {
          p=GetImagePixelsEx(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          for (x=0; x < image->columns; x++)
            {
              *q++=ScaleQuantumToChar(GetRedSample(p));
              *q++=ScaleQuantumToChar(GetGreenSample(p));
              *q++=ScaleQuantumToChar(GetBlueSample(p));
              if (image->matte == MagickTrue)
                *q++=ScaleQuantumToChar(MaxRGB-GetOpacitySample(p));
              p++;
            }
        }

      /*
        "Returns false in case of memory error."
      */
      if (image->matte != MagickTrue)
        webp_status=WebPPictureImportRGB(&picture,pixels,3*picture.width);
      else
        webp_status=WebPPictureImportRGBA(&picture,pixels,4*picture.width);
      MagickFreeResourceLimitedMemory(pixels);
    }

  if (webp_status)
    {
      /*
        "Returns false in case of error, true otherwise.
        In case of error, picture->error_code is updated accordingly."
      */
      webp_status=WebPEncode(&configure, &picture);
      if (! webp_status)
        {
          int
            picture_error_code;

          picture_error_code=picture.error_code;
          WebPPictureFree(&picture);

          switch (picture_error_code)
            {
            case VP8_ENC_OK:
              break;
            case VP8_ENC_ERROR_OUT_OF_MEMORY:
              ThrowWriterException(CoderError,WebPEncodingFailedOutOfMemory,image);
              break;
            case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
              ThrowWriterException(CoderError,WebPEncodingFailedBitstreamOutOfMemory,image);
              break;
            case VP8_ENC_ERROR_NULL_PARAMETER:
              ThrowWriterException(CoderError,WebPEncodingFailedNULLParameter,image);
              break;
            case VP8_ENC_ERROR_INVALID_CONFIGURATION:
              ThrowWriterException(CoderError,WebPEncodingFailedInvalidConfiguration,image);
              break;
            case VP8_ENC_ERROR_BAD_DIMENSION:
              ThrowWriterException(CoderError,WebPEncodingFailedBadDimension,image);
              break;
            case VP8_ENC_ERROR_PARTITION0_OVERFLOW:
              ThrowWriterException(CoderError,WebPEncodingFailedPartition0Overflow,image);
              break;
            case VP8_ENC_ERROR_PARTITION_OVERFLOW:
              ThrowWriterException(CoderError,WebPEncodingFailedPartitionOverflow,image);
              break;
            case VP8_ENC_ERROR_BAD_WRITE:
              ThrowWriterException(CoderError,WebPEncodingFailedBadWrite,image);
              break;
            case VP8_ENC_ERROR_FILE_TOO_BIG:
              ThrowWriterException(CoderError,WebPEncodingFailedFileTooBig,image);
              break;
#if defined(SUPPORT_USER_ABORT)
            case VP8_ENC_ERROR_USER_ABORT:     /* Added in v0.1.99 */
              ThrowWriterException(CoderError,WebPEncodingFailedUserAbort,image);
              break;
            case VP8_ENC_ERROR_LAST:           /* Added in v0.1.99 */
              break;
#endif
            } /* switch (picture_error_code) */
          /*
            Catch-all in case a code is added that we are not
            explicitly prepared for.
          */
          ThrowWriterException(CoderError,WebPEncodingFailed,image);
        } /* if (! webp_status) */
    } /* if (webp_status) */

#if defined(SUPPORT_WEBP_MUX)
  /* Write profiles */
  if (image->profiles)
    {
      size_t idx;

      /* Mapping of GraphicsMagick->libwebp feature/profile names */
      static const char data_features[][3][6]={{"ICC", "ICCP"},{"EXIF", "EXIF"},{"XMP", "XMP"}};

      /* Prepare the WebP muxer */
      WebPMuxError mux_error;
      WebPMux *mux=WebPMuxNew();
      WebPData encoded_image={writer.mem,writer.size};

      WebPMuxSetImage(mux,&encoded_image,1);

      /* Iterate over all available features and try to push them into the WebP container */
      for (idx=0;idx<sizeof(data_features)/sizeof(data_features[0]);idx++)
        {
          /* Get feature data */
          WebPData chunk;

          (void) memset(&chunk,0,sizeof(chunk));
          chunk.bytes=GetImageProfile(image,data_features[idx][0],&chunk.size);

          if (!chunk.bytes)
            continue;

          /*
            Skip over JPEG APP1 "Exif\0\0" header if present
          */
          if ((chunk.size > MAGICK_JPEG_APP1_EXIF_HEADER_SIZE) &&
              (memcmp((const void *) chunk.bytes,
                      (const void *) MAGICK_JPEG_APP1_EXIF_HEADER,
                      MAGICK_JPEG_APP1_EXIF_HEADER_SIZE) == 0))
            {
              chunk.bytes += MAGICK_JPEG_APP1_EXIF_HEADER_SIZE;
              chunk.size -= MAGICK_JPEG_APP1_EXIF_HEADER_SIZE;
            }

          /* Write feature data */
          mux_error=WebPMuxSetChunk(mux,data_features[idx][1],&chunk,0);

          switch(mux_error) {
          case WEBP_MUX_OK:
            break;
            ;;
          case WEBP_MUX_BAD_DATA:
          case WEBP_MUX_NOT_ENOUGH_DATA:
          case WEBP_MUX_NOT_FOUND:
            ThrowWriterException(CoderError,WebPInvalidParameter,image);
            break;
            ;;
          case WEBP_MUX_INVALID_ARGUMENT:
            ThrowWriterException(CoderError,WebPEncodingFailedNULLParameter,image);
            break;
            ;;
          case WEBP_MUX_MEMORY_ERROR:
            ThrowWriterException(CoderError,WebPEncodingFailedOutOfMemory,image);
            break;
            ;;
          }
        }

      /* Create the new container */
      {
        WebPData picture_profiles={writer.mem, writer.size};
        mux_error=WebPMuxAssemble(mux,&picture_profiles);

        switch (mux_error) {
        case WEBP_MUX_OK:
          break;
          ;;
        case WEBP_MUX_BAD_DATA:
        case WEBP_MUX_NOT_ENOUGH_DATA:
        case WEBP_MUX_NOT_FOUND:
          ThrowWriterException(CoderError,WebPInvalidParameter,image);
        case WEBP_MUX_INVALID_ARGUMENT:
          ThrowWriterException(CoderError,WebPEncodingFailedNULLParameter,image);
          break;
          ;;
        case WEBP_MUX_MEMORY_ERROR:
          ThrowWriterException(CoderError,WebPEncodingFailedOutOfMemory,image);
          break;
          ;;
        }

        /* Replace the original data with the container data */
        WebPMemoryWriterClear(&writer);
        writer.size=picture_profiles.size;
        writer.mem=(unsigned char *)picture_profiles.bytes;

        /* Cleanup the muxer */
        WebPMuxDelete(mux);
      }
    } /* if (webp_status) */

  /* Write out the data to the blob and cleanup*/
  WriteBlob(image, writer.size, writer.mem);
#endif /* defined(SUPPORT_WEBP_MUX) */
  WebPPictureFree(&picture);
#if defined(SUPPORT_WEBP_MUX)
  WebPMemoryWriterClear(&writer);
#endif
  status &= (webp_status ? MagickPass : MagickFail);
  status &= CloseBlob(image);

  return (status);
}
#endif
