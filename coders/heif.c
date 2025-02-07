/*
% Copyright (C) 2023-2024 GraphicsMagick Group
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                       H   H  EEEEE  I  FFFFF                                %
%                       H   H  E      I  F                                    %
%                       HHHHH  EEEEE  I  FFFF                                 %
%                       H   H  E      I  F                                    %
%                       H   H  EEEEE  I  F                                    %
%                                                                             %
%           Read HEIF/HEIC/AVIF image formats using libheif.                  %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* Status: Support for reading a single image.
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
#include "magick/resource.h"
#include "magick/utility.h"
#include "magick/resource.h"

#if defined(HasHEIF)

/* Set to 1 to enable the currently non-functional progress monitor callbacks */
#define HEIF_ENABLE_PROGRESS_MONITOR 0

#include <libheif/heif.h>
#ifdef HAVE_HEIF_INIT
static MagickBool heif_initialized = MagickFalse;
#endif /* ifdef HAVE_HEIF_INIT */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H E I F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsHEIF returns True if the image format type, identified by the
%  magick string, is supported by this HEIF reader.
%
%  The format of the IsHEIF  method is:
%
%      MagickBool IsHEIF(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsHEIF returns MagickTrue if the image format type is HEIF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static MagickBool IsHEIF(const unsigned char *magick,const size_t length)
{
  enum heif_filetype_result
    heif_filetype;

  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Testing header for supported HEIF format");

  if (length < 12)
    return(MagickFalse);

  heif_filetype = heif_check_filetype(magick, (int) length);
  if (heif_filetype == heif_filetype_yes_supported)
    return MagickTrue;

  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Not a supported HEIF format");

  return(MagickFalse);
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d H E I F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadHEIFImage() reads an image in the HEIF image format.
%
%  The format of the ReadHEIFImage method is:
%
%      Image *ReadHEIFImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

#define HEIFReadCleanup()                                              \
  do                                                                   \
    {                                                                  \
      if (heif_image)                                                  \
        heif_image_release(heif_image);                                \
      if (heif_image_handle)                                           \
        heif_image_handle_release(heif_image_handle);                  \
      if (heif)                                                        \
        heif_context_free(heif);                                       \
      MagickFreeResourceLimitedMemory(in_buf);                         \
    } while (0);

#define ThrowHEIFReaderException(code_,reason_,image_) \
  do                                                   \
    {                                                  \
      HEIFReadCleanup();                               \
      ThrowReaderException(code_,reason_,image_);      \
    } while (0);

/*
  Read metadata (Exif and XMP)
*/
static Image *ReadMetadata(struct heif_image_handle *heif_image_handle,
                           Image *image, ExceptionInfo *exception)
{
  int
    count,
    i;

  heif_item_id
    *ids;

  struct heif_error
    err;

  /* Get number of metadata blocks attached to image */
  count=heif_image_handle_get_number_of_metadata_blocks(heif_image_handle,
                                                        /*type_filter*/ NULL);
  if (count==0)
    return image;

  ids=MagickAllocateResourceLimitedArray(heif_item_id *,count,sizeof(*ids));
  if (ids == (heif_item_id *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  /* Get list of metadata block ids */
  count=heif_image_handle_get_list_of_metadata_block_IDs(heif_image_handle, NULL,
                                                         ids,count);

  /* For each block id ... */
  for (i=0; i < count; i++)
    {
      const char
        *content_type,
        *profile_name;

      size_t
        exif_pad = 0,
        profile_size;

      unsigned char
        *profile;

      /* Access string indicating the type of the metadata (e.g. "Exif") */
      profile_name=heif_image_handle_get_metadata_type(heif_image_handle,ids[i]);

      /* Access string indicating the content type */
      content_type=heif_image_handle_get_metadata_content_type(heif_image_handle,ids[i]);

      /* Get the size of the raw metadata, as stored in the HEIF file */
      profile_size=heif_image_handle_get_metadata_size(heif_image_handle,ids[i]);

      if (image->logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Profile \"%s\" with content type \"%s\""
                              " and size %" MAGICK_SIZE_T_F "u bytes",
                              profile_name ? profile_name : "(null)",
                              content_type ? content_type : "(null)",
                              (MAGICK_SIZE_T) profile_size);

      if (NULL != profile_name && profile_size > 0)
        {
          if (strncmp(profile_name,"Exif",4) == 0)
            exif_pad=2;

          /* Allocate memory for profile */
          profile=MagickAllocateResourceLimitedArray(unsigned char*,profile_size+exif_pad,
                                                     sizeof(*profile));
          if (profile == (unsigned char*) NULL)
            {
              MagickFreeResourceLimitedMemory(ids);
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
            }

          /*
            Copy metadata into 'profile' buffer. For Exif data, you
            probably have to skip the first four bytes of the data,
            since they indicate the offset to the start of the TIFF
            header of the Exif data.
          */
          err=heif_image_handle_get_metadata(heif_image_handle,ids[i],profile+exif_pad);

          if (err.code != heif_error_Ok)
            {
              if (image->logging)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                      "heif_image_handle_get_metadata() reports error \"%s\"",
                                      err.message);
              MagickFreeResourceLimitedMemory(profile);
              MagickFreeResourceLimitedMemory(ids);
              ThrowReaderException(CorruptImageError,
                                   AnErrorHasOccurredReadingFromFile,image);
            }

          if (strncmp(profile_name,"Exif",4) == 0 && profile_size > 4)
            {
              /* Parse and skip offset to TIFF header */
              unsigned char *p = profile;
              magick_uint32_t offset;

              /* Big-endian offset decoding */
              offset = (magick_uint32_t) p[exif_pad+0] << 24 |
                       (magick_uint32_t) p[exif_pad+1] << 16 |
                       (magick_uint32_t) p[exif_pad+2] << 8 |
                       (magick_uint32_t) p[exif_pad+3];

              /*
                If the TIFF header offset is not zero, then need to
                move the TIFF data forward to the correct offset.
              */
              profile_size -= 4;
              if (offset > 0 && offset < profile_size)
                {
                  profile_size -= offset;

                  /* Strip any EOI marker if payload starts with a JPEG marker */
                  if (profile_size > 2 &&
                      (memcmp(p+exif_pad+4,"\xff\xd8",2) == 0 ||
                      memcmp(p+exif_pad+4,"\xff\xe1",2) == 0) &&
                      memcmp(p+exif_pad+4+profile_size-2,"\xff\xd9",2) == 0)
                    profile_size -= 2;

                  (void) memmove(p+exif_pad+4,p+exif_pad+4+offset,profile_size);
                }

              p[0]='E';
              p[1]='x';
              p[2]='i';
              p[3]='f';
              p[4]='\0';
              p[5]='\0';

              SetImageProfile(image,"EXIF",profile,profile_size+exif_pad+4);

              /*
                Retrieve image orientation from EXIF and store in
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
            }
          else
            {
              if (NULL != content_type && strncmp(content_type,"application/rdf+xml",19) == 0)
                SetImageProfile(image,"XMP",profile,profile_size);
            }
          MagickFreeResourceLimitedMemory(profile);
        }
    }
  MagickFreeResourceLimitedMemory(ids);
  return image;
}


/*
  Read Color Profile
*/
static Image *ReadColorProfile(struct heif_image_handle *heif_image_handle,
                               Image *image, ExceptionInfo *exception)
{
  struct heif_error
    err;

  enum heif_color_profile_type
    profile_type; /* 4 chars encoded into enum by 'heif_fourcc()' */

  unsigned char
    *profile;

  profile_type = heif_image_handle_get_color_profile_type(heif_image_handle);

  if (heif_color_profile_type_not_present == profile_type)
    return image;

  if (image->logging && (heif_color_profile_type_not_present !=profile_type))
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "Found color profile of type \"%c%c%c%c\")",
                          ((char) ((unsigned int) profile_type >> 24) & 0xff),
                          ((char) ((unsigned int) profile_type >> 16) & 0xff),
                          ((char) ((unsigned int) profile_type >> 8) & 0xff),
                          ((char) ((unsigned int) profile_type) & 0xff));

  if (heif_color_profile_type_prof == profile_type)
    {
      size_t profile_size;

      profile_size = heif_image_handle_get_raw_color_profile_size(heif_image_handle);

      if (image->logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Reading ICC profile with size %" MAGICK_SIZE_T_F "u bytes",
                              (MAGICK_SIZE_T) profile_size);

      if (profile_size > 0)
        {
          /* Allocate 'profile' buffer for profile */
          profile=MagickAllocateResourceLimitedArray(unsigned char*,profile_size,
                                                     sizeof(*profile));

          if (profile == (unsigned char*) NULL)
            ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);

          /* Copy ICC profile to 'profile' buffer */
          err = heif_image_handle_get_raw_color_profile(heif_image_handle,
                                                        profile);
          if (err.code != heif_error_Ok)
            {
              if (image->logging)
                (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                      "heif_image_handle_get_raw_color_profile() reports error \"%s\"",
                                      err.message);
              MagickFreeResourceLimitedMemory(profile);
              ThrowReaderException(CorruptImageError,
                                   AnErrorHasOccurredReadingFromFile,image);

            }
          SetImageProfile(image,"ICM",profile,profile_size);
          MagickFreeResourceLimitedMemory(profile);
        }
    }
  return image;
}

/*
  This progress monitor implementation is tentative since it is not invoked

  According to libheif issue 161
  (https://github.com/strukturag/libheif/issues/161) progress monitor
  does not actually work since the decoders it depends on do not
  support it.

  Libheif issue 546 (https://github.com/strukturag/libheif/pull/546)
  suggests changing the return type of on_progress and start_progress
  to "bool" so that one can implement cancellation support.
 */
typedef struct ProgressUserData_
{
  ExceptionInfo *exception;
  Image *image;
  enum heif_progress_step step;
  unsigned long int progress;
  unsigned long int max_progress;

} ProgressUserData;

#if HEIF_ENABLE_PROGRESS_MONITOR
/* Called when progress monitor starts.  The 'max_progress' parameter indicates the maximum value of progress */
static void start_progress(enum heif_progress_step step, int max_progress, void* progress_user_data)
{
  ProgressUserData *context= (ProgressUserData *) progress_user_data;
  Image *image=context->image;
  context->step = step;
  context->progress = 0;
  context->max_progress = max_progress;
  if (context->image->logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "start_progress: step=%d, max_progress=%d",step, max_progress);
  MagickMonitorFormatted(context->progress,context->max_progress,&image->exception,
                         "[%s] Loading image: %lux%lu...  ",
                         image->filename,
                         image->columns,image->rows);
}

/* Called for each step of progress.  The 'progress' parameter represents the progress within the span of 'max_progress' */
static void on_progress(enum heif_progress_step step, int progress, void* progress_user_data)
{
  ProgressUserData *context = (ProgressUserData *) progress_user_data;
  Image *image=context->image;
  context->step = step;
  context->progress = progress;
  if (context->image->logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "on_progress: step=%d, progress=%d",step, progress);
  MagickMonitorFormatted(context->progress,context->max_progress,&image->exception,
                         "[%s] Loading image: %lux%lu...  ",
                         image->filename,
                         image->columns,image->rows);
}

/* Called when progress monitor stops */
static void end_progress(enum heif_progress_step step, void* progress_user_data)
{
  ProgressUserData *context = (ProgressUserData *) progress_user_data;
  context->step = step;
  if (context->image->logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "end_progress: step=%d",step);
}

#endif /* if HEIF_ENABLE_PROGRESS_MONITOR */

static Image *ReadHEIFImage(const ImageInfo *image_info,
                            ExceptionInfo *exception)
{
  Image
    *image;

  struct heif_context
    *heif = NULL;

  struct heif_error
    heif_status;

  struct heif_image_handle
    *heif_image_handle = NULL;

  struct heif_image
    *heif_image = NULL;

  struct heif_decoding_options
    *decode_options;

  ProgressUserData
    progress_user_data;

  size_t
    in_len;

  int
    row_stride;

  unsigned char
    *in_buf = NULL;

  const uint8_t
    *pixels = NULL;

  long
    x,
    y;

  PixelPacket
    *q;

  MagickBool
    ignore_transformations;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

#ifdef HAVE_HEIF_INIT
  if (!heif_initialized)
    {
      /* heif_init() accepts a 'struct heif_init_params *' argument */
      heif_init((struct heif_init_params *) NULL);
      heif_initialized = MagickTrue;
    }
#endif /* HAVE_HEIF_INIT */

  /*
    Open image file.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  if (OpenBlob(image_info,image,ReadBinaryBlobMode,exception) == MagickFail)
    ThrowReaderException(FileOpenError,UnableToOpenFile,image);

  in_len=GetBlobSize(image);
  in_buf=MagickAllocateResourceLimitedArray(unsigned char *,in_len,sizeof(*in_buf));
  if (in_buf == (unsigned char *) NULL)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  if (ReadBlob(image,in_len,in_buf) != in_len)
    ThrowHEIFReaderException(CorruptImageError, UnexpectedEndOfFile, image);

#if LIBHEIF_NUMERIC_VERSION >= 0x01090000
  ignore_transformations = MagickFalse;
  {
    const char
      *value;

    if ((value=AccessDefinition(image_info,"heif","ignore-transformations")))
      if (LocaleCompare(value,"TRUE") == 0)
        ignore_transformations = MagickTrue;
  }
#else
  /* Older versions are missing required function to get real width/height. */
  ignore_transformations = MagickTrue;
#endif

  /* Init HEIF-Decoder handles */
  heif=heif_context_alloc();

#if defined(HAVE_HEIF_CONTEXT_SET_MAXIMUM_IMAGE_SIZE_LIMIT)
  {
    /* Add an image size limit */
    magick_int64_t width_limit = GetMagickResourceLimit(WidthResource);
    if (MagickResourceInfinity != width_limit)
      {
        if (width_limit > INT_MAX)
          width_limit =  INT_MAX;
        heif_context_set_maximum_image_size_limit(heif, (int) width_limit);
      }
  }
#endif /* if defined(HAVE_HEIF_CONTEXT_SET_MAXIMUM_IMAGE_SIZE_LIMIT) */

  /* FIXME: DEPRECATED: use heif_context_read_from_memory_without_copy() instead. */
  heif_status=heif_context_read_from_memory(heif, in_buf, in_len, NULL);
  if (heif_status.code == heif_error_Unsupported_filetype
      || heif_status.code == heif_error_Unsupported_feature)
    ThrowHEIFReaderException(CoderError, ImageTypeNotSupported, image);
  if (heif_status.code != heif_error_Ok)
    {
      if (image->logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "heif_context_read_from_memory() reports error \"%s\"",
                              heif_status.message);
      ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);
    }

  /* FIXME: no support for reading multiple images but should be added */
  {
    int number_of_top_level_images;
    number_of_top_level_images=heif_context_get_number_of_top_level_images(heif);
    if (image->logging)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "heif_context_get_number_of_top_level_images() reports %d images",
                            number_of_top_level_images);
    if (number_of_top_level_images != 1)
      ThrowHEIFReaderException(CoderError, NumberOfImagesIsNotSupported, image);
  }

  heif_status=heif_context_get_primary_image_handle(heif, &heif_image_handle);
  if (heif_status.code == heif_error_Memory_allocation_error)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if (heif_status.code != heif_error_Ok)
    {
      if (image->logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "heif_context_get_primary_image_handle() reports error \"%s\"",
                              heif_status.message);
      ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);
    }

  /*
    Note: Those values are preliminary but likely the upper bound
    The real image values might be rotated or cropped due to transformations
  */
  image->columns=heif_image_handle_get_width(heif_image_handle);
  image->rows=heif_image_handle_get_height(heif_image_handle);
  if (heif_image_handle_has_alpha_channel(heif_image_handle))
    image->matte=MagickTrue;

  if (image->logging)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Geometry: %lux%lu", image->columns, image->rows);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Matte: %s", image->matte ? "True" : "False");
    }

  /* Read EXIF and XMP profile */
  if (!ReadMetadata(heif_image_handle, image, exception))
    {
      HEIFReadCleanup();
      return NULL;
    }

  /* Read ICC profile */
  if (!ReadColorProfile(heif_image_handle, image, exception))
    {
      HEIFReadCleanup();
      return NULL;
    }

  /*
    When apply transformations (the default) the whole image has to be
    read to get the real dimensions.
  */
  if (image_info->ping && ignore_transformations)
    {
      image->depth = 8;
      HEIFReadCleanup();
      CloseBlob(image);
      return image;
    }

  if (CheckImagePixelLimits(image, exception) != MagickPass)
    ThrowHEIFReaderException(ResourceLimitError,ImagePixelLimitExceeded,image);

  /* Add decoding options support */
  decode_options = heif_decoding_options_alloc();
  if (decode_options == (struct heif_decoding_options*) NULL)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  progress_user_data.exception = exception;
  progress_user_data.image = image;
  progress_user_data.max_progress = 0;
  progress_user_data.progress = 0;

  /* version 1 options */
#if LIBHEIF_NUMERIC_VERSION >= 0x01090000
  decode_options->ignore_transformations = (ignore_transformations == MagickTrue) ? 1 : 0;
#else
  decode_options->ignore_transformations = 1;
#endif
#if HEIF_ENABLE_PROGRESS_MONITOR
  decode_options->start_progress = start_progress;
  decode_options->on_progress = on_progress;
  decode_options->end_progress = end_progress;
#endif /* if HEIF_ENABLE_PROGRESS_MONITOR */
  decode_options->progress_user_data = &progress_user_data;

  /* version 2 options */
#if LIBHEIF_NUMERIC_VERSION > 0x01070000
  decode_options->convert_hdr_to_8bit = 1;
#endif /* if LIBHEIF_NUMERIC_VERSION > 0x01070000 */

  /* version 3 options */

  /* When enabled, an error is returned for invalid input. Otherwise, it will try its best and
     add decoding warnings to the decoded heif_image. Default is non-strict. */
  /* uint8_t strict_decoding; */

  heif_status=heif_decode_image(heif_image_handle, &heif_image,
                                heif_colorspace_RGB, image->matte ? heif_chroma_interleaved_RGBA :
                                heif_chroma_interleaved_RGB,
                                decode_options);
  heif_decoding_options_free(decode_options);
  if (heif_status.code == heif_error_Memory_allocation_error)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if (heif_status.code != heif_error_Ok)
    {
      if (image->logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "heif_decode_image() reports error \"%s\"",
                              heif_status.message);
      ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);
    }

  /*
    Update with final values, see preliminary note above

    These functions are apparently added in libheif 1.9
  */
#if LIBHEIF_NUMERIC_VERSION >= 0x01090000
  image->columns=heif_image_get_primary_width(heif_image);
  image->rows=heif_image_get_primary_height(heif_image);

  if (image_info->ping)
    {
      image->depth = 8;
      HEIFReadCleanup();
      CloseBlob(image);
      return image;
    }
#endif

  image->depth=heif_image_get_bits_per_pixel(heif_image, heif_channel_interleaved);
  /* The requested channel is interleaved there depth is a sum of all channels
     split it up again: */
  if (image->logging)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "heif_image_get_bits_per_pixel: bits_per_pixel=%u", image->depth);
    }
  if (image->depth == 32 && image->matte)
    image->depth = 8;
  else if (image->depth == 24 && !image->matte)
    image->depth = 8;
  else
    ThrowHEIFReaderException(CoderError, UnsupportedBitsPerSample, image);

  pixels=heif_image_get_plane_readonly(heif_image, heif_channel_interleaved, &row_stride);
  if (!pixels)
    ThrowHEIFReaderException(CoderError, NoDataReturned, image);

  if (image->logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                          "heif_image_get_plane_readonly: bytes-per-line=%d",
                          row_stride);

  /* Transfer pixels to image, using row stride to find start of each row. */
  for (y=0; y < (long)image->rows; y++)
    {
      const uint8_t *line;
      q=SetImagePixelsEx(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
        ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
      line=pixels+y*row_stride;
      for (x=0; x < (long)image->columns; x++)
        {
          SetRedSample(q,ScaleCharToQuantum(*line++));
          SetGreenSample(q,ScaleCharToQuantum(*line++));
          SetBlueSample(q,ScaleCharToQuantum(*line++));
          if (image->matte) {
            SetOpacitySample(q,MaxRGB-ScaleCharToQuantum(*line++));
          } else {
            SetOpacitySample(q,OpaqueOpacity);
          }
          q++;
        }
      if (!SyncImagePixels(image))
        ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    }

  HEIFReadCleanup();
  CloseBlob(image);
  return image;
}

#endif /* HasHEIF */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r H E I F I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterHEIFImage adds attributes for the HEIF image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format and a brief
%  description of the format.
%
%  The format of the RegisterHEIFImage method is:
%
%      RegisterHEIFImage(void)
%
*/
ModuleExport void RegisterHEIFImage(void)
{
#if defined(HasHEIF)
  static const char
    description[] = "HEIF Image Format";

  static char
    version[20];

  MagickInfo
    *entry;

  unsigned int
    heif_major,
    heif_minor,
    heif_revision;

  int encoder_version=heif_get_version_number();
  heif_major=(encoder_version >> 16) & 0xff;
  heif_minor=(encoder_version >> 8) & 0xff;
  heif_revision=encoder_version & 0xff;
  *version='\0';
  (void) snprintf(version, sizeof(version),
                  "heif v%u.%u.%u", heif_major,
                  heif_minor, heif_revision);

  entry=SetMagickInfo("AVIF");
  entry->decoder=(DecoderHandler) ReadHEIFImage;
  entry->magick=(MagickHandler) IsHEIF;
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue;
  if (*version != '\0')
    entry->version=version;
  entry->module="HEIF";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("HEIF");
  entry->decoder=(DecoderHandler) ReadHEIFImage;
  entry->magick=(MagickHandler) IsHEIF;
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue;
  if (*version != '\0')
    entry->version=version;
  entry->module="HEIF";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("HEIC");
  entry->decoder=(DecoderHandler) ReadHEIFImage;
  entry->magick=(MagickHandler) IsHEIF;
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue;
  if (*version != '\0')
    entry->version=version;
  entry->module="HEIF";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);
#endif /* HasHEIF */
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r H E I F I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterHEIFImage removes format registrations made by the
%  HEIF module from the list of supported formats.
%
%  The format of the UnregisterHEIFImage method is:
%
%      UnregisterHEIFImage(void)
%
*/
ModuleExport void UnregisterHEIFImage(void)
{
#if defined(HasHEIF)
  (void) UnregisterMagickInfo("AVIF");
  (void) UnregisterMagickInfo("HEIF");
  (void) UnregisterMagickInfo("HEIC");
#ifdef HAVE_HEIF_DEINIT
  if (heif_initialized)
    heif_deinit();
#endif /* HAVE_HEIF_DEINIT */
#endif /* HasHEIF */
}
