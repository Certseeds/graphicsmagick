/*
% Copyright (C) 2021 GraphicsMagick Group
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%		      H   H  EEEEE  I  FFFFF                                  %
%		      H	  H  E      I  F                                      %
%		      HHHHH  EEEEE  I  FFFF                                   %
%		      H   H  E      I  F                                      %
%		      H   H  EEEEE  I  F                                      %
%                                                                             %
%                     Read Heif/Heic Image Format.			      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
* Status: Support for reading a single image.
*/

#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/colormap.h"
#include "magick/log.h"
#include "magick/constitute.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/pixel_cache.h"
#include "magick/profile.h"
#include "magick/utility.h"
#include "magick/resource.h"

#if defined(HasHEIF)
#include <libheif/heif.h>

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

#define HEIFReadCleanup() \
  if(heif_image) heif_image_release(heif_image); \
  if(heif_image_handle) heif_image_handle_release(heif_image_handle); \
  if(heif) heif_context_free(heif); \
  MagickFreeResourceLimitedMemory(in_buf);

#define ThrowHEIFReaderException(code_,reason_,image_) \
{ \
  HEIFReadCleanup() \
  ThrowReaderException(code_,reason_,image_); \
}


static Image *ReadMetadata(struct heif_image_handle *heif_image_handle,
			   Image *image, ExceptionInfo *exception)
{
  int
    count;

  heif_item_id
    *ids;

  count=heif_image_handle_get_number_of_metadata_blocks(heif_image_handle, NULL);
  if(count==0)
    return image;

  ids=MagickAllocateResourceLimitedArray(heif_item_id *,count,sizeof(*ids));
  if(ids == (heif_item_id *) NULL)
    ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);

  count=heif_image_handle_get_list_of_metadata_block_IDs(heif_image_handle, NULL,
    ids,count);

  for(int i=0; i<count;i++) {
    const char*
      profile_name=heif_image_handle_get_metadata_type(heif_image_handle,ids[i]);

    size_t
      profile_size=heif_image_handle_get_metadata_size(heif_image_handle,ids[i]);

    unsigned char*
      profile;

    profile=MagickAllocateResourceLimitedArray(unsigned char*,profile_size,
        sizeof(*profile));
    if(profile == (unsigned char*) NULL) {
      MagickFreeResourceLimitedMemory(ids);
      ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    }

    struct heif_error
      err=heif_image_handle_get_metadata(heif_image_handle,ids[i],profile);

    if(err.code != heif_error_Ok) {
      MagickFreeResourceLimitedMemory(profile);
      MagickFreeResourceLimitedMemory(ids);
      ThrowReaderException(CorruptImageError,
        AnErrorHasOccurredReadingFromFile,image);
    }

    if(strncmp(profile_name,"Exif",4) == 0 && profile_size > 4) {
      // skip TIFF-Header
      SetImageProfile(image,profile_name,profile+4,profile_size-4);
    } else {
      SetImageProfile(image,profile_name,profile,profile_size);
    }
    MagickFreeResourceLimitedMemory(profile);
  }
  MagickFreeResourceLimitedMemory(ids);
  return image;
}

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

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

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

  if(ReadBlob(image,in_len,in_buf) != in_len)
    ThrowHEIFReaderException(CorruptImageError, UnexpectedEndOfFile, image);

  /* Init HEIF-Decoder handles */
  heif=heif_context_alloc();

  heif_status=heif_context_read_from_memory(heif, in_buf, in_len, NULL);
  if(heif_status.code == heif_error_Unsupported_filetype
      || heif_status.code == heif_error_Unsupported_feature)
    ThrowHEIFReaderException(CoderError, ImageTypeNotSupported, image);
  if(heif_status.code != heif_error_Ok)
    ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);

  /* no support for reading multiple images but could be added */
  if(heif_context_get_number_of_top_level_images(heif) != 1)
    ThrowHEIFReaderException(CoderError, NumberOfImagesIsNotSupported, image);

  heif_status=heif_context_get_primary_image_handle(heif, &heif_image_handle);
  if(heif_status.code == heif_error_Memory_allocation_error)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if(heif_status.code != heif_error_Ok)
    ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);

  image->columns=heif_image_handle_get_width(heif_image_handle);
  image->rows=heif_image_handle_get_height(heif_image_handle);
  if(heif_image_handle_has_alpha_channel(heif_image_handle))
    image->matte=MagickTrue;

  if(!ReadMetadata(heif_image_handle, image, exception)) {
    HEIFReadCleanup()
    return NULL;
  }

  heif_status=heif_decode_image(heif_image_handle, &heif_image,
    heif_colorspace_RGB, image->matte ? heif_chroma_interleaved_RGBA :
    heif_chroma_interleaved_RGB, NULL);
  if(heif_status.code == heif_error_Memory_allocation_error)
    ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
  if(heif_status.code != heif_error_Ok)
    ThrowHEIFReaderException(CorruptImageError, AnErrorHasOccurredReadingFromFile, image);

  image->depth=heif_image_get_bits_per_pixel(heif_image, heif_channel_interleaved);
  // the requested channel is interleaved there depth is a sum of all channels
  // split it up again:
  if(image->depth == 32 && image->matte)
    image->depth = 8;
  else if(image->depth == 24 && !image->matte)
    image->depth = 8;
  else
    ThrowHEIFReaderException(CoderError, UnsupportedBitsPerSample, image);

  pixels=heif_image_get_plane_readonly(heif_image, heif_channel_interleaved, &row_stride);
  if(!pixels)
    ThrowHEIFReaderException(CoderError, NoDataReturned, image);

  for (y=0; y < (long)image->rows; y++)
    {
      q=SetImagePixelsEx(image,0,y,image->columns,1,exception);
      if (q == (PixelPacket *) NULL)
	ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
      for (x=0; x < (long)image->columns; x++)
      {
        SetRedSample(q,ScaleCharToQuantum(*pixels++));
        SetGreenSample(q,ScaleCharToQuantum(*pixels++));
        SetBlueSample(q,ScaleCharToQuantum(*pixels++));
	if(image->matte) {
	  SetOpacitySample(q,MaxRGB-ScaleCharToQuantum(*pixels++));
	} else {
          SetOpacitySample(q,OpaqueOpacity);
	}
	q++;
      }
      if (!SyncImagePixels(image))
	ThrowHEIFReaderException(ResourceLimitError,MemoryAllocationFailed,image);
    }

  HEIFReadCleanup()
  CloseBlob(image);
  return image;
}

#endif

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

  *version='\0';

  int encoder_version=heif_get_version_number();
  heif_major=(encoder_version >> 16) & 0xff;
  heif_minor=(encoder_version >> 8) & 0xff;
  heif_revision=encoder_version & 0xff;
  (void) sprintf(version,
                  "heif v%u.%u.%u", heif_major,
                  heif_minor, heif_revision);

  entry=SetMagickInfo("HEIF");
#if defined(HasHEIF)
  entry->decoder=(DecoderHandler) ReadHEIFImage;
#endif
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue;
  if (*version != '\0')
    entry->version=version;
  entry->module="HEIF";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("HEIC");
#if defined(HasHEIF)
  entry->decoder=(DecoderHandler) ReadHEIFImage;
#endif
  entry->description=description;
  entry->adjoin=False;
  entry->seekable_stream=MagickTrue;
  if (*version != '\0')
    entry->version=version;
  entry->module="HEIF";
  entry->coder_class=PrimaryCoderClass;
  (void) RegisterMagickInfo(entry);
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
  (void) UnregisterMagickInfo("HEIF");
  (void) UnregisterMagickInfo("HEIC");
}
