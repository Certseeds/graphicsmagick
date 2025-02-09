/*
% Copyright (C) 2003-2024 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
% Copyright 1991-1999 E. I. du Pont de Nemours and Company
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS  U   U  N   N                              %
%                            SS     U   U  NN  N                              %
%                             SSS   U   U  N N N                              %
%                               SS  U   U  N  NN                              %
%                            SSSSS   UUU   N   N                              %
%                                                                             %
%                                                                             %
%                   Read/Write Sun Rasterfile Image Format.                   %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
#include "magick/analyze.h"
#include "magick/blob.h"
#include "magick/colormap.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/monitor.h"
#include "magick/pixel_cache.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static unsigned int
  WriteSUNImage(const ImageInfo *,Image *);

/* Raster Types */
#define RT_STANDARD     1 /* Standard */
#define RT_ENCODED      2 /* Byte encoded */
#define RT_FORMAT_RGB   3 /* RGB format */

/* Color Map Types */
#define RMT_NONE        0 /* No color map */
#define RMT_EQUAL_RGB   1 /* RGB color map */
#define RMT_RAW         2 /* Raw color map */

/*
  Note that Sun headers described these fields as type 'int'.
*/
typedef struct _SUNInfo
{
  magick_uint32_t
    magic,      /* Magick (identification) number */
    width,      /* Width of image in pixels */
    height,     /* Height of image in pixels */
    depth,      /* Number of bits per pixel */
    length,     /* Size of image data in bytes */
    type,       /* Type of raster file */
    maptype,    /* Type of color map */
    maplength;  /* Size of the color map in bytes */
} SUNInfo;

/*
  Compute bytes per line for an unencoded
  image.

  The width of a scan line is always a multiple of 16-bits, padded
  when necessary.
*/
static size_t SUNBytesPerLine(const size_t width, const size_t depth)
{
  size_t
    bits;

  bits = MagickArraySize(width,depth);
  if (0 != bits)
      {
        size_t abits = RoundUpToAlignment(bits,16);
        if (abits < bits)
          abits=0;
        bits=abits;
      }
    return bits/8U;
}

static void LogSUNInfo(const SUNInfo *sun_info,const char *mode)
{
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "%s SunHeader:\n"
                        "    Magic:     0x%04X\n"
                        "    Width:     %u\n"
                        "    Height:    %u\n"
                        "    Depth:     %u\n"
                        "    Length:    %u\n"
                        "    Type:      %u (%s)\n"
                        "    MapType:   %u (%s)\n"
                        "    MapLength: %u\n",
                        mode,
                        sun_info->magic,
                        sun_info->width,
                        sun_info->height,
                        sun_info->depth,
                        sun_info->length,
                        sun_info->type,
                        (sun_info->type == RT_STANDARD ? "Standard (RT_STANDARD)" :
                         (sun_info->type == RT_ENCODED ? "RLE encoded (RT_ENCODED)" :
                          (sun_info->type == RT_FORMAT_RGB ? "RGB format (RT_FORMAT_RGB)" :
                           "?"))),
                        sun_info->maptype,
                        (sun_info->maptype == RMT_NONE ? "No color map (RMT_NONE)" :
                         (sun_info->maptype == RMT_EQUAL_RGB ? "RGB color map (RMT_EQUAL_RGB)" :
                          (sun_info->maptype == RMT_RAW ? "Raw color map (RMT_RAW)" :
                           "?"))),
                        sun_info->maplength
                        );
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S U N                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsSUN returns True if the image format type, identified by the
%  magick string, is SUN.
%
%  The format of the IsSUN method is:
%
%      unsigned int IsSUN(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsSUN returns True if the image format type is SUN.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
static unsigned int IsSUN(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(False);
  if (memcmp(magick,"\131\246\152\225",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DecodeImage unpacks the packed image pixels into
%  runlength-encoded pixel packets.
%
%  The format of the DecodeImage method is:
%
%      MagickPassFail DecodeImage(const unsigned char *compressed_pixels,
%        const size_t length,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o status:  Method DecodeImage returns True (MagickPass) if all the
%      pixels are uncompressed without error, otherwise False (MagickFail).
%
%    o compressed_pixels:  The address of a byte (8 bits) array of compressed
%      pixel data.
%
%    o compressed_size:  An integer value that is the total number of bytes
%      of the source image (as just read by ReadBlob)
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the uncompression process.  The number of bytes in this array
%      must be at least equal to the number columns times the number of rows
%      of the source pixels.
%
%    o pixels_size: Decompressed pixels buffer size.
%
%
*/
static MagickPassFail
DecodeImage(const unsigned char *compressed_pixels,
            const size_t compressed_size,
            unsigned char *pixels,
            const size_t pixels_size)
{
  register const unsigned char
    *p;

  register int
    count;

  register unsigned char
    *q;

  unsigned char
    byte;

  assert(compressed_pixels != (unsigned char *) NULL);
  assert(pixels != (unsigned char *) NULL);
  p=compressed_pixels;
  q=pixels;
  while (((size_t) (p-compressed_pixels) < compressed_size) &&
         ((size_t) (q-pixels) < pixels_size))
    {
      byte=(*p++);
      if (byte != 128U)
        {
          /*
            Stand-alone byte
          */
          *q++=byte;
        }
      else
        {
          /*
            Runlength-encoded packet: <count><byte>
          */
          if (((size_t) (p-compressed_pixels) >= compressed_size))
            break;
          count=(*p++);
          if (count > 0)
            {
              if (((size_t) (p-compressed_pixels) >= compressed_size))
                break;
              byte=(*p++);
            }
          while ((count >= 0) && ((size_t) (q-pixels) < pixels_size))
            {
              *q++=byte;
              count--;
            }
        }
    }
  return (((size_t) (q-pixels) == pixels_size) ? MagickPass : MagickFail);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S U N I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadSUNImage reads a SUN image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSUNImage method is:
%
%      Image *ReadSUNImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadSUNImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadSUNImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image
    *image;

  int
    bit;

  long
    y;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  register long
    i;

  register unsigned char
    *p;

  size_t
    bytes_per_image,
    bytes_per_line,
    count,
    sun_data_length;

  SUNInfo
    sun_info;

  unsigned char
    *sun_data,
    *sun_pixels;

  unsigned int
    index;

  unsigned int
    status;

  MagickBool
    logging;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AllocateImage(image_info);
  logging=image->logging;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == False)
    ThrowReaderException(FileOpenError,UnableToOpenFile,image);
  /*
    Read SUN raster header.
  */
  (void) memset(&sun_info,0,sizeof(sun_info));
  sun_info.magic=ReadBlobMSBLong(image);
  do
    {
      /*
        Verify SUN identifier.
      */
      if (sun_info.magic != 0x59a66a95)
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
      sun_info.width=ReadBlobMSBLong(image);
      sun_info.height=ReadBlobMSBLong(image);
      sun_info.depth=ReadBlobMSBLong(image);
      sun_info.length=ReadBlobMSBLong(image);
      sun_info.type=ReadBlobMSBLong(image);
      sun_info.maptype=ReadBlobMSBLong(image);
      sun_info.maplength=ReadBlobMSBLong(image);
      if (logging)
        LogSUNInfo(&sun_info, "Read");
      if (EOFBlob(image))
        ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
      /*
        Verify that width, height, depth, and length are not zero
      */
      if ((sun_info.width == 0) || (sun_info.height == 0) ||
          (sun_info.depth == 0) || (sun_info.length == 0))
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

      /*
        Verify that header values are in positive numeric range of a
        32-bit 'int' even though we store them in an unsigned value.
      */
      if ((sun_info.magic | sun_info.width | sun_info.height | sun_info.depth |
           sun_info.type | sun_info.maptype | sun_info.maplength) & (1U << 31))
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
      /*
        Verify that we support the image sub-type
      */
      if ((sun_info.type != RT_STANDARD) &&
          (sun_info.type != RT_ENCODED) &&
          (sun_info.type != RT_FORMAT_RGB))
        ThrowReaderException(CoderError,DataEncodingSchemeIsNotSupported,image);
      /*
        Verify that we support the colormap type
      */
      if ((sun_info.maptype != RMT_NONE) &&
          (sun_info.maptype != RMT_EQUAL_RGB))
        ThrowReaderException(CoderError,ColormapTypeNotSupported,image);
      /*
        Insist that map length is zero if there is no colormap.
      */
      if ((sun_info.maptype == RMT_NONE) && (sun_info.maplength != 0))
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
      /*
        Insist on a supported depth
      */
      if ((sun_info.depth != 1) &&
          (sun_info.depth != 8) &&
          (sun_info.depth != 24) &&
          (sun_info.depth != 32))
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

      image->columns=sun_info.width;
      image->rows=sun_info.height;
      if (((unsigned long) ((long) image->columns) != image->columns) ||
          ((unsigned long) ((long) image->rows) != image->rows))
        ThrowReaderException(CoderError,ImageColumnOrRowSizeIsNotSupported,image);
      if (CheckImagePixelLimits(image, exception) != MagickPass)
        ThrowReaderException(ResourceLimitError,ImagePixelLimitExceeded,image);
      image->depth=sun_info.depth <= 8 ? 8 : QuantumDepth;
      if (sun_info.depth < 24)
        {
          image->colors=sun_info.maplength;
          if (sun_info.maptype == RMT_NONE)
            image->colors=1U << sun_info.depth;
          if (sun_info.maptype == RMT_EQUAL_RGB)
            image->colors=sun_info.maplength/3U;
        }

      switch (sun_info.maptype)
        {
        case RMT_NONE:
          {
            if (sun_info.depth < 24)
              {
                /*
                  Create linear color ramp.
                */
                if (!AllocateImageColormap(image,image->colors))
                  ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                       image);
                if (logging)
                  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                        "Allocated colormap with %u colors", image->colors);
              }
            break;
          }
        case RMT_EQUAL_RGB:
          {
            unsigned char
              *sun_colormap;

            /*
              Read SUN raster colormap.
            */
            if (!AllocateImageColormap(image,image->colors))
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                   image);
            sun_colormap=MagickAllocateResourceLimitedMemory(unsigned char *,image->colors);
            if (sun_colormap == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                   image);
            do
              {
                if (ReadBlob(image,image->colors,(char *) sun_colormap) !=
                    image->colors)
                  {
                    status = MagickFail;
                    break;
                  }
                for (i=0; i < (long) image->colors; i++)
                  image->colormap[i].red=ScaleCharToQuantum(sun_colormap[i]);
                if (ReadBlob(image,image->colors,(char *) sun_colormap) !=
                    image->colors)
                  {
                    status = MagickFail;
                    break;
                  }
                for (i=0; i < (long) image->colors; i++)
                  image->colormap[i].green=ScaleCharToQuantum(sun_colormap[i]);
                if (ReadBlob(image,image->colors,(char *) sun_colormap) !=
                    image->colors)
                  {
                    status = MagickFail;
                    break;
                  }
                for (i=0; i < (long) image->colors; i++)
                  image->colormap[i].blue=ScaleCharToQuantum(sun_colormap[i]);
                break;
              } while (1);
            MagickFreeResourceLimitedMemory(sun_colormap);
            if (MagickFail == status)
              ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
            if (logging)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                    "Read colormap with %u colors", image->colors);
            break;
          }
        case RMT_RAW:
          {
            unsigned char
              *sun_colormap;

            /*
              Read SUN raster colormap.
            */
            if (!AllocateImageColormap(image,image->colors))
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                   image);
            sun_colormap=MagickAllocateResourceLimitedMemory(unsigned char *,sun_info.maplength);
            if (sun_colormap == (unsigned char *) NULL)
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                   image);
            if (ReadBlob(image,sun_info.maplength,(char *) sun_colormap) !=
                sun_info.maplength)
              status = MagickFail;
            MagickFreeResourceLimitedMemory(sun_colormap);
            if (MagickFail == status)
              ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
            if (logging)
              (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                    "Read colormap with %u colors (length %u)",
                                    image->colors, sun_info.maplength);
            break;
          }
        default:
          ThrowReaderException(CoderError,ColormapTypeNotSupported,image);
        }
      image->matte=(sun_info.depth == 32);
      image->columns=sun_info.width;
      image->rows=sun_info.height;
      image->depth=8;
      if (sun_info.depth < 8)
        image->depth=sun_info.depth;

      if (image_info->ping)
        {
          CloseBlob(image);
          return(image);
        }

      /*
        Compute bytes per line and bytes per image for an unencoded
        image.

        "The width of a scan line is always 16-bits, padded when necessary."
      */
      bytes_per_line = SUNBytesPerLine(sun_info.width,sun_info.depth);
      if (logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Bytes per line: %" MAGICK_SIZE_T_F "u",
                              (MAGICK_SIZE_T) bytes_per_line);
      if (bytes_per_line == 0)
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

      bytes_per_image=MagickArraySize(sun_info.height,bytes_per_line);

      if (logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Bytes per image: %" MAGICK_SIZE_T_F "u",
                              (MAGICK_SIZE_T) bytes_per_image);

      if (bytes_per_image == 0)
        ThrowReaderException(CorruptImageError,ImproperImageHeader,image);

      if (sun_info.type == RT_ENCODED)
        sun_data_length=(size_t) sun_info.length;
      else
        sun_data_length=bytes_per_image;

      if (logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Sun data length: %" MAGICK_SIZE_T_F "u",
                              (MAGICK_SIZE_T) sun_data_length);

      /*
        Verify that data length claimed by header is supported by file size
      */
      if (sun_info.type == RT_ENCODED)
        {
          if (sun_data_length < bytes_per_image/255U)
            ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
        }
      else
        {
          if ((size_t) sun_info.length < bytes_per_image)
            ThrowReaderException(CorruptImageError,ImproperImageHeader,image);
        }
      if (BlobIsSeekable(image))
        {
          const magick_off_t file_size = GetBlobSize(image);
          const magick_off_t current_offset = TellBlob(image);
          if ((file_size > 0) &&
              (current_offset > 0) &&
              (file_size >= current_offset))
            {
              const magick_off_t remaining = file_size-current_offset;

              if ((remaining == 0) || (remaining < (magick_off_t) sun_info.length))
                {
                  ThrowReaderException(CorruptImageError,UnexpectedEndOfFile,image);
                }
            }
        }

      /*
        Read raster data into allocated buffer.
      */
      sun_data=MagickAllocateResourceLimitedMemory(unsigned char *,sun_info.length);
      if (sun_data == (unsigned char *) NULL)
        ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,image);
      if ((count=ReadBlob(image,sun_info.length,(char *) sun_data))
          != sun_info.length)
        {
          MagickFreeResourceLimitedMemory(sun_data);
          ThrowReaderException(CorruptImageError,UnableToReadImageData,image);
        }
      sun_pixels=sun_data;
      if (sun_info.type == RT_ENCODED)
        {
          /*
            Read run-length encoded raster pixels
          */
          sun_pixels=MagickAllocateResourceLimitedMemory(unsigned char *,
                                                         bytes_per_image+image->rows);
          if (sun_pixels == (unsigned char *) NULL)
            {
              MagickFreeResourceLimitedMemory(sun_data);
              ThrowReaderException(ResourceLimitError,MemoryAllocationFailed,
                                   image);
            }
          status &= DecodeImage(sun_data,sun_data_length,sun_pixels,bytes_per_image);
          MagickFreeResourceLimitedMemory(sun_data);
          if (status != MagickPass)
            {
              MagickFreeResourceLimitedMemory(sun_pixels);
              ThrowReaderException(CorruptImageError,UnableToRunlengthDecodeImage,image);
            }
        }
      /*
        Convert SUN raster image to pixel packets.
      */
      p=sun_pixels;
      if (sun_info.depth == 1)
        {
          /*
            Bilevel
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Reading bilevel image...");
          for (y=0; y < (long) image->rows; y++)
            {
              q=SetImagePixels(image,0,y,image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=AccessMutableIndexes(image);
              for (x=0; x < ((long) image->columns-7); x+=8)
                {
                  for (bit=7; bit >= 0; bit--)
                    {
                      index=((*p) & (0x01 << bit) ? 0x00 : 0x01);
                      VerifyColormapIndex(image,index);
                      indexes[x+7-bit]=index;
                      q[x+7-bit]=image->colormap[index];
                    }
                  p++;
                }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (long) (8-(image->columns % 8)); bit--)
                    {
                      index=((*p) & (0x01 << bit) ? 0x00 : 0x01);
                      VerifyColormapIndex(image,index);
                      indexes[x+7-bit]=index;
                      q[x+7-bit]=image->colormap[index];
                    }
                  p++;
                }
              if ((((image->columns/8)+(image->columns % 8 ? 1 : 0)) % 2) != 0)
                p++;
              if (!SyncImagePixels(image))
                break;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,exception,
                                              LoadImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      else if (image->storage_class == PseudoClass)
        {
          /*
            Colormapped
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Reading colormapped image...");
          for (y=0; y < (long) image->rows; y++)
            {
              q=SetImagePixels(image,0,y,image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              indexes=AccessMutableIndexes(image);
              for (x=0; x < (long) image->columns; x++)
                {
                  index=(*p++);
                  VerifyColormapIndex(image,index);
                  indexes[x]=index;
                  q[x]=image->colormap[index];
                }
              if ((image->columns % 2) != 0)
                p++;
              if (!SyncImagePixels(image))
                break;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,exception,
                                              LoadImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      else
        {
          /*
            (A)BGR or (A)RGB
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Reading truecolor image...");
          for (y=0; y < (long) image->rows; y++)
            {
              q=SetImagePixels(image,0,y,image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (long) image->columns; x++)
                {
                  if (image->matte)
                    q->opacity=(Quantum) (MaxRGB-ScaleCharToQuantum(*p++));
                  if (sun_info.type == RT_STANDARD)
                    {
                      q->blue=ScaleCharToQuantum(*p++);
                      q->green=ScaleCharToQuantum(*p++);
                      q->red=ScaleCharToQuantum(*p++);
                    }
                  else
                    {
                      q->red=ScaleCharToQuantum(*p++);
                      q->green=ScaleCharToQuantum(*p++);
                      q->blue=ScaleCharToQuantum(*p++);
                    }
                  if (image->colors != 0)
                    {
                      q->red=image->colormap[q->red].red;
                      q->green=image->colormap[q->green].green;
                      q->blue=image->colormap[q->blue].blue;
                    }
                  q++;
                }
              if (((image->columns % 2) != 0) && (image->matte == False))
                p++;
              if (!SyncImagePixels(image))
                break;
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,exception,
                                              LoadImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      MagickFreeResourceLimitedMemory(sun_pixels);
      if (EOFBlob(image))
        {
          ThrowException(exception,CorruptImageError,UnexpectedEndOfFile,
                         image->filename);
          break;
        }
      StopTimer(&image->timer);
      /*
        Proceed to next image.
      */
      if (image_info->subrange != 0)
        if (image->scene >= (image_info->subimage+image_info->subrange-1))
          break;
      sun_info.magic=ReadBlobMSBLong(image);
      if (sun_info.magic == 0x59a66a95)
        {
          /*
            Allocate next image structure.
          */
          AllocateNextImage(image_info,image);
          if (image->next == (Image *) NULL)
            {
              DestroyImageList(image);
              return((Image *) NULL);
            }
          image=SyncNextImageInList(image);
          if (!MagickMonitorFormatted(TellBlob(image),GetBlobSize(image),
                                      exception,LoadImagesText,
                                      image->filename))
            break;
        }
    } while (sun_info.magic == 0x59a66a95);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S U N I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterSUNImage adds attributes for the SUN image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSUNImage method is:
%
%      RegisterSUNImage(void)
%
*/
ModuleExport void RegisterSUNImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("IM1");
  entry->decoder=(DecoderHandler) ReadSUNImage;
  entry->encoder=(EncoderHandler) WriteSUNImage;
  entry->magick=(MagickHandler) IsSUN;
  entry->description="SUN Rasterfile (1 bit)";
  entry->module="SUN";
  entry->stealth=MagickTrue; /* Don't list in '-list format' output */
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("IM8");
  entry->decoder=(DecoderHandler) ReadSUNImage;
  entry->encoder=(EncoderHandler) WriteSUNImage;
  entry->magick=(MagickHandler) IsSUN;
  entry->description="SUN Rasterfile (8 bit)";
  entry->module="SUN";
  entry->stealth=MagickTrue; /* Don't list in '-list format' output */
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("IM24");
  entry->decoder=(DecoderHandler) ReadSUNImage;
  entry->encoder=(EncoderHandler) WriteSUNImage;
  entry->magick=(MagickHandler) IsSUN;
  entry->description="SUN Rasterfile (24 bit)";
  entry->module="SUN";
  entry->stealth=MagickTrue; /* Don't list in '-list format' output */
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("RAS");
  entry->decoder=(DecoderHandler) ReadSUNImage;
  entry->encoder=(EncoderHandler) WriteSUNImage;
  entry->magick=(MagickHandler) IsSUN;
  entry->description="SUN Rasterfile";
  entry->module="SUN";
  (void) RegisterMagickInfo(entry);

  entry=SetMagickInfo("SUN");
  entry->decoder=(DecoderHandler) ReadSUNImage;
  entry->encoder=(EncoderHandler) WriteSUNImage;
  entry->description="SUN Rasterfile";
  entry->module="SUN";
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S U N I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterSUNImage removes format registrations made by the
%  SUN module from the list of supported formats.
%
%  The format of the UnregisterSUNImage method is:
%
%      UnregisterSUNImage(void)
%
*/
ModuleExport void UnregisterSUNImage(void)
{
  (void) UnregisterMagickInfo("IM1");
  (void) UnregisterMagickInfo("IM8");
  (void) UnregisterMagickInfo("IM24");
  (void) UnregisterMagickInfo("RAS");
  (void) UnregisterMagickInfo("SUN");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S U N I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteSUNImage writes an image in the SUN rasterfile format.
%
%  The format of the WriteSUNImage method is:
%
%      unsigned int WriteSUNImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteSUNImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%
*/
static unsigned int WriteSUNImage(const ImageInfo *image_info,Image *image)
{
  long
    y;

  register const PixelPacket
    *p;

  register const IndexPacket
    *indexes;

  unsigned char
    *pixels;

  register unsigned char
    *q;

  register long
    x;

  register long
    i;

  SUNInfo
    sun_info;

  unsigned int
    status;

  unsigned long
    scene;

  size_t
    bytes_per_image,
    bytes_per_line,
    image_list_length;

  MagickBool
    logging;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  image_list_length=GetImageListLength(image);
  logging=image->logging;
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == False)
    ThrowWriterException(FileOpenError,UnableToOpenFile,image);
  scene=0;
  do
    {
      ImageCharacteristics
        characteristics;

      /*
        Ensure that image is in an RGB space.
      */
      (void) TransformColorspace(image,RGBColorspace);
      /*
        Analyze image to be written.
      */
      if (!GetImageCharacteristics(image,&characteristics,
                                   (OptimizeType == image_info->type),
                                   &image->exception))
        {
          CloseBlob(image);
          return MagickFail;
        }
      /*
        Initialize SUN raster file header.
      */
      sun_info.magic=0x59a66a95;
      sun_info.width=image->columns;
      sun_info.height=image->rows;
      sun_info.depth=0;
      sun_info.length=0;
      sun_info.type=
        (image->storage_class == DirectClass ? RT_FORMAT_RGB : RT_STANDARD);
      sun_info.maptype=RMT_NONE;
      sun_info.maplength=0;

      if (characteristics.monochrome)
        {
          /*
            Monochrome SUN raster.
          */
          sun_info.depth=1;
        }
      else if (characteristics.palette)
        {
          /*
            Colormapped SUN raster.
          */
          sun_info.depth=8;
          sun_info.maptype=RMT_EQUAL_RGB;
          sun_info.maplength=image->colors*3;
        }
      else
        {
          /*
            Full color SUN raster.
          */
          sun_info.depth=(image->matte ? 32U : 24U);
        }

      bytes_per_line=SUNBytesPerLine(sun_info.width,sun_info.depth);
      if (logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Bytes per line: %" MAGICK_SIZE_T_F "u",
                              (MAGICK_SIZE_T) bytes_per_line);
      if (0 == bytes_per_line)
        ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);
      bytes_per_image=MagickArraySize(sun_info.height,bytes_per_line);
      if (logging)
        (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                              "Bytes per image: %" MAGICK_SIZE_T_F "u",
                              (MAGICK_SIZE_T) bytes_per_image);
      if ((size_t) ((magick_uint32_t) bytes_per_image) != bytes_per_image)
        ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);
      sun_info.length=(magick_uint32_t) bytes_per_image;

      /*
        Allocate memory for pixels.
      */
      pixels=MagickAllocateResourceLimitedClearedMemory(unsigned char *,
                                                        bytes_per_line);
      if (pixels == (unsigned char *) NULL)
        ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,
                             image);

      /*
        Write SUN header.
      */
      LogSUNInfo(&sun_info, "Write");
      (void) WriteBlobMSBLong(image,sun_info.magic);
      (void) WriteBlobMSBLong(image,sun_info.width);
      (void) WriteBlobMSBLong(image,sun_info.height);
      (void) WriteBlobMSBLong(image,sun_info.depth);
      (void) WriteBlobMSBLong(image,sun_info.length);
      (void) WriteBlobMSBLong(image,sun_info.type);
      (void) WriteBlobMSBLong(image,sun_info.maptype);
      (void) WriteBlobMSBLong(image,sun_info.maplength);
      /*
        Convert MIFF to SUN raster pixels.
      */
      x=0;
      y=0;
      if (characteristics.monochrome)
        {
          /*
            Monochrome SUN raster.
          */
          register unsigned char
            bit,
            byte,
            polarity;

          /*
            Convert PseudoClass image to a SUN monochrome image.
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Writing SUN monochrome frame %lu...",image->scene);
          (void) SetImageType(image,BilevelType);
          polarity=PixelIntensityToQuantum(&image->colormap[0]) > (MaxRGB/2);
          if (image->colors == 2)
            polarity=PixelIntensityToQuantum(&image->colormap[0]) >
              PixelIntensityToQuantum(&image->colormap[1]);
          for (y=0; y < (long) image->rows; y++)
            {
              p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              q=pixels;
              indexes=AccessImmutableIndexes(image);
              bit=0;
              byte=0;
              for (x=0; x < (long) image->columns; x++)
                {
                  byte<<=1;
                  if (indexes[x] == polarity)
                    byte|=0x01;
                  bit++;
                  if (bit == 8)
                    {
                      *q++=byte;
                      bit=0;
                      byte=0;
                    }
                  p++;
                }
              if (bit != 0)
                *q++=(byte << (8-bit));
              (void) WriteBlob(image,bytes_per_line,pixels);
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,&image->exception,
                                              SaveImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      else if (characteristics.palette)
        {
          /*
            Colormapped SUN raster.
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Writing SUN colormapped frame %lu...",image->scene);

          for (i=0; i < (long) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].red));
          for (i=0; i < (long) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].green));
          for (i=0; i < (long) image->colors; i++)
            (void) WriteBlobByte(image,ScaleQuantumToChar(image->colormap[i].blue));
          /*
            Convert PseudoClass packet to SUN colormapped pixel.
          */
          for (y=0; y < (long) image->rows; y++)
            {
              p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              q=pixels;
              indexes=AccessImmutableIndexes(image);
              for (x=0; x < (long) image->columns; x++)
                {
                  *q++=indexes[x];
                  p++;
                }
              (void) WriteBlob(image,bytes_per_line,pixels);
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,&image->exception,
                                              SaveImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      else
        {
          /*
            Full color SUN raster.
          */
          if (logging)
            (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                                  "Writing SUN truecolor frame %lu...",image->scene);
          /*
            Convert DirectClass packet to SUN (A)RGB pixel.
          */
          for (y=0; y < (long) image->rows; y++)
            {
              p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              q=pixels;
              for (x=0; x < (long) image->columns; x++)
                {
                  if (image->matte)
                    *q++=ScaleQuantumToChar(MaxRGB-p->opacity);
                  *q++=ScaleQuantumToChar(p->red);
                  *q++=ScaleQuantumToChar(p->green);
                  *q++=ScaleQuantumToChar(p->blue);
                  p++;
                }
              (void) WriteBlob(image,bytes_per_line,pixels);
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  if (!MagickMonitorFormatted(y,image->rows,&image->exception,
                                              SaveImageText,image->filename,
                                              image->columns,image->rows))
                    break;
            }
        }
      MagickFreeResourceLimitedMemory(pixels);
      if (image->next == (Image *) NULL)
        break;
      image=SyncNextImageInList(image);
      if (!MagickMonitorFormatted(scene++,image_list_length,
                                  &image->exception,SaveImagesText,
                                  image->filename))
        break;
    } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  status &= CloseBlob(image);
  return(status);
}
