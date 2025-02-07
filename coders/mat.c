/*
% Copyright (C) 2003-2023 GraphicsMagick Group
% Copyright (C) 2002 ImageMagick Studio
%
% This program is covered by multiple licenses, which are described in
% Copyright.txt. You should have received a copy of Copyright.txt with this
% package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                  M   M   AAA   TTTTT  L       AAA   BBBB                    %
%                  MM MM  A   A    T    L      A   A  B   B                   %
%                  M M M  AAAAA    T    L      AAAAA  BBBB                    %
%                  M   M  A   A    T    L      A   A  B   B                   %
%                  M   M  A   A    T    LLLLL  A   A  BBBB                    %
%                                                                             %
%                                                                             %
%                        Read MATLAB Image Format.                            %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                2001 - 2018                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%Currently supported formats:
%  2D matrices:      X*Y    byte, word, dword, qword, single, double
%  3D matrices: only X*Y*3  byte, word, dword, qword, single, double
%  complex:          X*Y                              single, double
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/analyze.h"
#include "magick/blob.h"
#include "magick/color.h"
#include "magick/colormap.h"
#include "magick/constitute.h"
#include "magick/log.h"
#include "magick/magick.h"
#include "magick/magick_endian.h"
#include "magick/monitor.h"
#include "magick/pixel_cache.h"
#include "magick/shear.h"
#include "magick/tempfile.h"
#include "magick/transform.h"
#include "magick/utility.h"
#if defined(HasZLIB)
#  include "zlib.h"
#endif


/* Auto coloring method, sorry this creates some artefact inside data
MinReal+j*MaxComplex = red  MaxReal+j*MaxComplex = black
MinReal+j*0 = white          MaxReal+j*0 = black
MinReal+j*MinComplex = blue  MaxReal+j*MinComplex = black
*/

typedef struct
{
  char identific[124];
  magick_uint16_t Version;
  char EndianIndicator[2];
  magick_uint32_t DataType;
  magick_uint32_t ObjectSize;
  magick_uint32_t unknown1;
  magick_uint32_t unknown2;

  magick_uint16_t unknown5;
  magick_uint8_t StructureFlag;
  magick_uint8_t StructureClass;
  magick_uint32_t unknown3;
  magick_uint32_t unknown4;
  magick_uint32_t DimFlag;

  magick_uint32_t SizeX;
  magick_uint32_t SizeY;
  magick_uint16_t Flag1;
  magick_uint16_t NameFlag;
}
MATHeader;

static const char MonthsTab[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char DayOfWTab[7][4]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char OsDesc[]=
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    "PCWIN";
#else
 #ifdef __APPLE__
    "MAC";
 #else
    "LNX86";
 #endif
#endif

typedef enum
  {
    miINT8 = 1,                 /* 8 bit signed */
    miUINT8,                    /* 8 bit unsigned */
    miINT16,                    /* 16 bit signed */
    miUINT16,                   /* 16 bit unsigned */
    miINT32,                    /* 32 bit signed */
    miUINT32,                   /* 32 bit unsigned */
    miSINGLE,                   /* IEEE 754 single precision float */
    miRESERVE1,
    miDOUBLE,                   /* IEEE 754 double precision float */
    miRESERVE2,
    miRESERVE3,
    miINT64,                    /* 64 bit signed */
    miUINT64,                   /* 64 bit unsigned */
    miMATRIX,                   /* MATLAB array */
    miCOMPRESSED,               /* Compressed Data */
    miUTF8,                     /* Unicode UTF-8 Encoded Character Data */
    miUTF16,                    /* Unicode UTF-16 Encoded Character Data */
    miUTF32                     /* Unicode UTF-32 Encoded Character Data */
  } mat5_data_type;

typedef enum
  {
    mxCELL_CLASS=1,             /* cell array */
    mxSTRUCT_CLASS,             /* structure */
    mxOBJECT_CLASS,             /* object */
    mxCHAR_CLASS,               /* character array */
    mxSPARSE_CLASS,             /* sparse array */
    mxDOUBLE_CLASS,             /* double precision array */
    mxSINGLE_CLASS,             /* single precision floating point */
    mxINT8_CLASS,               /* 8 bit signed integer */
    mxUINT8_CLASS,              /* 8 bit unsigned integer */
    mxINT16_CLASS,              /* 16 bit signed integer */
    mxUINT16_CLASS,             /* 16 bit unsigned integer */
    mxINT32_CLASS,              /* 32 bit signed integer */
    mxUINT32_CLASS,             /* 32 bit unsigned integer */
    mxINT64_CLASS,              /* 64 bit signed integer */
    mxUINT64_CLASS,             /* 64 bit unsigned integer */
    mxFUNCTION_CLASS            /* Function handle */
  } arrayclasstype;

#define FLAG_COMPLEX 0x8
#define FLAG_GLOBAL  0x4
#define FLAG_LOGICAL 0x2

static const QuantumType z2qtype[4] = {GrayQuantum, BlueQuantum, GreenQuantum, RedQuantum};


/* Add coloring to gray image. C=R+j*Q. Colors to red when Q>0 and blue for Q<0.
 Please note that this function expects gray image on input. Additional channel contents
 checking is wasting of resources only. */
static void InsertComplexDoubleRow(double *p, long y, Image *image, double MinVal,
                                  double MaxVal)
{
  double f;
  int x;
  register PixelPacket *q;

  if(MinVal >= 0)       /* Grant MinVal to be negative */
    MinVal = -1;
  if(MaxVal <= 0)       /* Grant MaxVal to be positive */
    MaxVal = 1;

  q = SetImagePixels(image, 0, y, image->columns, 1);
  if (q == (PixelPacket *) NULL)
    return;
  for (x = 0; x < (long) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (Quantum)(MaxRGB - q->red);  /* first multiplier should be in a range <0;1> */
      if (MAGICK_ISNAN(f))
        f=0.0;
      /*if(f<0) f=0; */
      if (f + q->red >= MaxRGB)
        q->red = MaxRGB;
      else
        q->red += (int) f;
      f = q->green - f/2.0;
      if (f <= 0)
        q->green = q->blue = 0;
      else
        q->green = q->blue = (int)f;
    }
    if (*p < 0)
    {
      f = (*p / MinVal) * (Quantum)(MaxRGB - q->blue); /* first multiplier should be in a range <0;1>; *p<0 and MinVal<0. */
      if (MAGICK_ISNAN(f))
        f=0.0;
      /*if(f<0) f=0; */
      if (f + q->blue >= MaxRGB)
        q->blue = MaxRGB;
      else
        q->blue += (int) f;
      f = q->green - f/2.0;
      if(f <= 0)
        q->green = q->red = 0;
      else
        q->green = q->red = (int)f;
    }
    p++;
    q++;
  }
  if (!SyncImagePixels(image))
    return;
  /*          if (image->previous == (Image *) NULL)
     if (QuantumTick(y,image->rows))
     MagickMonitor(LoadImageText,image->rows-y-1,image->rows); */
  return;
}


/* Add coloring to gray image. C=R+j*Q. Colors to red when Q>0 and blue for Q<0.
 Please note that this function expects gray image on input. Additional channel contents
 checking is wasting of resources only. */
static void InsertComplexFloatRow(float *p, long y, Image *image, double MinVal, double MaxVal)
{
  double f;
  int x;
  register PixelPacket *q;

  if(MinVal >= 0)       /* Grant MinVal to be negative */
    MinVal = -1;
  if(MaxVal <= 0)       /* Grant MaxVal to be positive */
    MaxVal = 1;

  q = SetImagePixels(image, 0, y, image->columns, 1);
  if (q == (PixelPacket *) NULL)
    return;
  for (x = 0; x < (long) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (Quantum)(MaxRGB - q->red);
      if (MAGICK_ISNAN(f))
        f=0.0;
      /*if(f<0) f=0;    // Only for Assert, should be commented out */
      if (f + q->red < MaxRGB)
        q->red += (int)f;
      else
        q->red = MaxRGB;
      f /= 2.0;
      if(f < q->green)
        q->green = q->blue -= (int)(f);
      else
        q->green = q->blue = 0;
    }
    if (*p < 0)
    {
      f = (*p / MinVal) * (Quantum)(MaxRGB - q->blue); /* f is positive only <0; inf> */
      if (MAGICK_ISNAN(f))
        f=0.0;
      /*if(f<0) f=0;    //Only for Assert, should be commented out */
      if (f + q->blue < MaxRGB)
        q->blue += (int) f;
      else      /* 'else' branch is executed when NaN occurs. */
        q->blue = MaxRGB;

      f /= 2.0;
      if(f < q->green)
        q->green = q->red -= (int)(f);
      else      /* 'else' branch is executed when NaN occurs. */
        q->green = q->red = 0;
    }
    p++;
    q++;
  }
  if (!SyncImagePixels(image))
    return;
  /*          if (image->previous == (Image *) NULL)
     if (QuantumTick(y,image->rows))
     MagickMonitor(LoadImageText,image->rows-y-1,image->rows); */
  return;
}


/************** READERS ******************/



static void FixSignedValues(PixelPacket *q, magick_uint32_t y)
{
  if (y == 0)
    return;

  while(y-->0)
  {
     /* Please note that negative values will overflow
        Q=8; MaxRGB=255: <0;127> + 127+1 = <128; 255>
                       <-1;-128> + 127+1 = <0; 127> */
    q->red += MaxRGB/2 + 1;
    q->green += MaxRGB/ + 1;
    q->blue += MaxRGB/ + 1;
    q++;
  }
}


/** Fix whole row of logical/binary data. It means pack it. */
static void FixLogical(unsigned char *Buff,size_t ldblk)
{
  unsigned char mask=128;
  unsigned char *BuffL = Buff;
  unsigned char val = 0;

  if (ldblk == 0)
    return;

  while(ldblk-->0)
    {
      if(*Buff++ != 0)
        val |= mask;

      mask >>= 1;
      if(mask==0)
        {
          *BuffL++ = val;
          val = 0;
          mask = 128;
        }
    }
  *BuffL = val;
}

#if defined(HasZLIB)
static voidpf ZLIBAllocFunc(voidpf opaque, uInt items, uInt size) MAGICK_FUNC_MALLOC;
static voidpf ZLIBAllocFunc(voidpf opaque, uInt items, uInt size)
{
  ARG_NOT_USED(opaque);
  return MagickMallocCleared(MagickArraySize(items,size));
}
static void ZLIBFreeFunc(voidpf opaque, voidpf address)
{
  ARG_NOT_USED(opaque);
  MagickFree(address);
}

/** This procedure decompresses an image block for a new MATLAB format. */
static Image *DecompressBlock(Image *orig, magick_uint32_t *Size, ImageInfo *clone_info, ExceptionInfo *exception)
{
Image *image2;
void *cache_block, *decompress_block;
z_stream zip_info;
FILE *mat_file;
size_t magick_size;
int status;
int zip_status;
magick_off_t TotalSize = 0;

  if(clone_info==NULL) return NULL;
  if(clone_info->file)          /* Close file opened from previous transaction. */
  {
    fclose(clone_info->file);
    clone_info->file = NULL;
    (void) unlink(clone_info->filename);
  }

  cache_block = MagickAllocateResourceLimitedMemory(unsigned char *,(size_t)((*Size<16384) ? *Size : 16384));
  if(cache_block==NULL) return NULL;
  decompress_block = MagickAllocateResourceLimitedMemory(unsigned char *,(size_t)(4096));
  if(decompress_block==NULL)
  {
    MagickFreeResourceLimitedMemory(cache_block);
    return NULL;
  }

  mat_file = AcquireTemporaryFileStream(clone_info->filename,BinaryFileIOMode);
  if(!mat_file)
  {
    MagickFreeResourceLimitedMemory(cache_block);
    MagickFreeResourceLimitedMemory(decompress_block);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Cannot create file stream for decompressed image");
    return NULL;
  }

  zip_info.zalloc = ZLIBAllocFunc;
  zip_info.zfree = ZLIBFreeFunc;
  zip_info.opaque = (voidpf) NULL;
  zip_status = inflateInit(&zip_info);
  if (zip_status != Z_OK)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Failed to initialize zlib");
      ThrowException(exception,CorruptImageError, UnableToUncompressImage, orig->filename);
      MagickFreeResourceLimitedMemory(cache_block);
      MagickFreeResourceLimitedMemory(decompress_block);
      (void)fclose(mat_file);
      LiberateTemporaryFile(clone_info->filename);
      return NULL;
    }
  /* zip_info.next_out = 8*4; */

  zip_info.avail_in = 0;
  zip_info.total_out = 0;
  while(*Size>0 && !EOFBlob(orig))
  {
    magick_size = ReadBlob(orig, (*Size<16384)?*Size:16384, cache_block);
    zip_info.next_in = cache_block;
    zip_info.avail_in = (uInt) magick_size;

    while(zip_info.avail_in>0)
    {
      zip_info.avail_out = 4096;
      zip_info.next_out = decompress_block;
      zip_status = inflate(&zip_info,Z_NO_FLUSH);
      if ((zip_status != Z_OK) && (zip_status != Z_STREAM_END))
        {
          (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Corrupt inflate stream");
          inflateEnd(&zip_info);
          MagickFreeResourceLimitedMemory(cache_block);
          MagickFreeResourceLimitedMemory(decompress_block);
          (void)fclose(mat_file);
          LiberateTemporaryFile(clone_info->filename);
          ThrowException(exception,CorruptImageError, UnableToUncompressImage, orig->filename);
          return NULL;
        }
      fwrite(decompress_block, (size_t) 4096-zip_info.avail_out, 1, mat_file);
      TotalSize += (size_t) 4096-zip_info.avail_out;

      if(zip_status == Z_STREAM_END) goto DblBreak;
    }
        if (*Size >= magick_size)
                *Size -= (magick_uint32_t) magick_size;
        else
                *Size = 0U;
  }
DblBreak:

  inflateEnd(&zip_info);                        /* Release all caches used by zip. */
  (void)fclose(mat_file);
  MagickFreeResourceLimitedMemory(cache_block);
  MagickFreeResourceLimitedMemory(decompress_block);
  *Size = TotalSize;

  if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL) goto UnlinkFile;
  if((image2 = AllocateImage(clone_info))==NULL) goto EraseFile;
  image2->rows=0;
  image2->columns=0;

  status = OpenBlob(clone_info,image2,ReadBinaryBlobMode,exception);
  if (status == False)
  {
    DeleteImageFromList(&image2);
EraseFile:
    fclose(clone_info->file);
    clone_info->file = NULL;
UnlinkFile:
    LiberateTemporaryFile(clone_info->filename);
    return NULL;
  }

  return image2;
}
#endif /*  defined(HasZLIB) */


#define ThrowMATReaderException(code_,reason_,image_) \
{ \
  if (clone_info) \
    DestroyImageInfo(clone_info);    \
  ThrowReaderException(code_,reason_,image_); \
}


#define ThrowImg2MATReaderException(code_,reason_,image_) \
{ \
   if(image2!=NULL && image2!=image)    /* Does shadow temporary decompressed image exist? */ \
   { \
     CloseBlob(image2); \
     DeleteImageFromList(&image2); \
  } \
  if(clone_info) \
    DestroyImageInfo(clone_info);    \
  MagickFreeResourceLimitedMemory(BImgBuff); \
  ThrowReaderException(code_,reason_,image_); \
}




typedef struct {
        unsigned char Type[4];
        magick_uint32_t nRows;
        magick_uint32_t nCols;
        magick_uint32_t imagf;
        magick_uint32_t nameLen;
} MAT4_HDR;


#define RET_CHECK(image)  return((image->previous==NULL)?NULL:image)


/* Load Matlab V4 file. */
static Image *ReadMATImageV4(const ImageInfo *image_info, Image *image, ImportPixelAreaOptions *import_options,
                             ExceptionInfo *exception, const int logging)
{
MAT4_HDR HDR;
Image *rotated_image;
size_t ldblk;
int sample_size;
void *BImgBuff = NULL;
double MinVal_c, MaxVal_c;
int i;
BlobInfo *blob;
PixelPacket *q;
magick_uint32_t (*ReadBlobXXXLong)(Image *image);
size_t (*ReadBlobXXXDoubles)(Image *image, size_t len, double *data);
size_t (*ReadBlobXXXFloats)(Image *image, size_t len, float *data);

 if (SeekBlob(image,0,SEEK_SET) != (magick_off_t) 0)
   RET_CHECK(image);
  while(!EOFBlob(image)) /* object parser loop */
  {
    ldblk = ReadBlobLSBLong(image);
    if(EOFBlob(image)) break;
    if(ldblk>9999 /*|| ldblk<0*/) RET_CHECK(image); /* Zero is allowed! */
    HDR.Type[3] = ldblk % 10;   ldblk /= 10;    /* T digit */
    HDR.Type[2] = ldblk % 10;   ldblk /= 10;    /* P digit */
    HDR.Type[1] = ldblk % 10;   ldblk /= 10;    /* O digit */
    HDR.Type[0] = (unsigned char)ldblk;         /* M digit */

    if(HDR.Type[3]!=0) RET_CHECK(image);  /* Data format */
    if(HDR.Type[2]!=0) RET_CHECK(image);  /* Always 0 */

    ImportPixelAreaOptionsInit(import_options);

    switch(HDR.Type[0])
    {
      case 0: ReadBlobXXXLong = ReadBlobLSBLong;
              ReadBlobXXXDoubles = ReadBlobLSBDoubles;
              ReadBlobXXXFloats = ReadBlobLSBFloats;
              import_options->endian = LSBEndian;
              break;
      case 1: ReadBlobXXXLong = ReadBlobMSBLong;
              ReadBlobXXXDoubles = ReadBlobMSBDoubles;
              ReadBlobXXXFloats = ReadBlobMSBFloats;
              import_options->endian = MSBEndian;
              break;
      default: RET_CHECK(image);      /* Unsupported endian */
    }

    HDR.nRows = ReadBlobXXXLong(image);
    HDR.nCols = ReadBlobXXXLong(image);

    if (image->logging)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"MAT v4 %ux%u", HDR.nCols, HDR.nRows);

    if (HDR.nRows == 0 || HDR.nCols == 0) RET_CHECK(image);

    HDR.imagf = ReadBlobXXXLong(image);
    if(HDR.imagf!=0 && HDR.imagf!=1) RET_CHECK(image);

    HDR.nameLen = ReadBlobXXXLong(image);
    if(HDR.nameLen>0xFFFF) RET_CHECK(image);
    if (SeekBlob(image,HDR.nameLen,SEEK_CUR) < 0) /* Skip a matrix name. */
      RET_CHECK(image);

    switch(HDR.Type[1])
    {
      case 0: sample_size = 64;                         /* double-precision (64-bit) floating point numbers */
              image->depth = Min(QuantumDepth,32);        /* double type cell */
              import_options->sample_type = FloatQuantumSampleType;
              if(sizeof(double) != 8) RET_CHECK(image);      /* incompatible double size */
              ldblk = MagickArraySize(8,HDR.nRows);
              break;

      case 1: sample_size = 32;                         /* single-precision (32-bit) floating point numbers */
              image->depth = Min(QuantumDepth,32);        /* double type cell */
              import_options->sample_type = FloatQuantumSampleType;
#if 0
              if(sizeof(float) != 4)
                ThrowMATReaderException(CoderError, IncompatibleSizeOfFloat, image);
#endif
              ldblk = MagickArraySize(4,HDR.nRows);
              break;

      case 2: sample_size = 32;                         /* 32-bit signed integers */
              image->depth = Min(QuantumDepth,32);        /* Dword type cell */
              ldblk = MagickArraySize(4,HDR.nRows);
              import_options->sample_type = UnsignedQuantumSampleType;
              break;

      case 3:                           /* 16-bit signed integers */
      case 4:                           /* 16-bit unsigned integers */
              sample_size = 16;
              image->depth = Min(QuantumDepth,16);        /* Word type cell */
              ldblk = MagickArraySize(2,HDR.nRows);
              import_options->sample_type = UnsignedQuantumSampleType;
              break;

      case 5: sample_size = 8;          /* 8-bit unsigned integers */
              image->depth = Min(QuantumDepth,8);         /* Byte type cell */
              import_options->sample_type = UnsignedQuantumSampleType;
              ldblk = HDR.nRows;
              break;

      default: RET_CHECK(image);
    }

    if(ldblk==0) RET_CHECK(image);

    image->columns = HDR.nRows;
    image->rows = HDR.nCols;
    image->colors = 1l << image->depth;
    if(image->columns == 0 || image->rows == 0) RET_CHECK(image);
    if(CheckImagePixelLimits(image, exception) != MagickPass)
    {
      RET_CHECK(image);
      /* ThrowReaderException(ResourceLimitError,ImagePixelLimitExceeded,image); */
    }

    /* If ping is true, then only set image size and colors without reading any image data. */
    if(image_info->ping)
    {
      unsigned long temp = image->columns;  /* The true image is rotater 90 degs. Do rotation without data. */
      size_t offset;
      image->columns = image->rows;
      image->rows = temp;
      if(HDR.imagf==1) ldblk=MagickArraySize(2,ldblk); /*ldblk *= 2;*/
      offset=MagickArraySize(HDR.nCols,ldblk);
      if(offset==0) RET_CHECK(image);
      if (SeekBlob(image, offset, SEEK_CUR) < 0)
        RET_CHECK(image);
      goto skip_reading_current;
    }

        /* ----- Load raster data ----- */
    BImgBuff = MagickAllocateResourceLimitedMemory(unsigned char *,ldblk);    /* Ldblk was set in the check phase */
    if(BImgBuff == NULL) RET_CHECK(image);

    if(HDR.Type[1]==0)          /* Find Min and Max Values for doubles */
    {
      if (MagickFindRawImageMinMax(image, import_options->endian, HDR.nRows,
                                      HDR.nCols, DoublePixel, (unsigned int)ldblk, BImgBuff,
                                      &import_options->double_minvalue,
                                   &import_options->double_maxvalue) != MagickPass)
        goto skip_reading_current;
    }
    if(HDR.Type[1]==1)          /* Find Min and Max Values for floats */
    {
      if (MagickFindRawImageMinMax(image, import_options->endian, HDR.nRows,
                                      HDR.nCols, FloatPixel, (unsigned int)ldblk, BImgBuff,
                                      &import_options->double_minvalue,
                                      &import_options->double_maxvalue) != MagickPass)
        goto skip_reading_current;
    }

        /* Main reader loop. */
    for(i=0; i<(long)HDR.nCols; i++)
    {
      q = SetImagePixels(image, 0, HDR.nCols-i-1, image->columns, 1);
      if(q == (PixelPacket *)NULL)
      {
        if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                    "  MAT set image pixels returns unexpected NULL on a row %u.", (unsigned)i);
        goto skip_reading_current;              /* Skip image rotation, when cannot set image pixels */
      }

      if(ReadBlob(image,ldblk,(char *)BImgBuff) != (size_t)ldblk)
      {
        if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                   "  MAT cannot read scanrow %u from a file.", (unsigned)(i));
        ThrowException(exception,CorruptImageError,UnexpectedEndOfFile,image->filename);
        DestroyImagePixels(image);              /* The unread data contains crap in memory, erase current image data. */
        image->columns = image->rows = 0;
        goto ExitLoop;
      }

      if(ImportImagePixelArea(image,GrayQuantum,sample_size,BImgBuff,import_options,0) == MagickFail)
            goto ImportImagePixelAreaFailed;

      if(HDR.Type[1]==2 || HDR.Type[1]==3)
          FixSignedValues(q,HDR.nRows);

      if(!SyncImagePixels(image))
      {
        if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
             "  MAT failed to sync image pixels for a row %u", (unsigned)i);
        goto ExitLoop;
      }
    }

        /* Read complex part of data. */
    if(HDR.imagf==1)
    {
      MinVal_c = MaxVal_c = 0;
      if(HDR.Type[1]==0)                /* Find Min and Max Values for doubles */
      {
        if (MagickFindRawImageMinMax(image, import_options->endian, HDR.nRows,
                                     HDR.nCols, DoublePixel, (unsigned int)ldblk, BImgBuff,
                                     &MinVal_c, &MaxVal_c) != MagickPass)
          goto skip_reading_current;
        for(i=0; i<(long)HDR.nCols; i++)
        {
          if(ReadBlobXXXDoubles(image, ldblk, (double *)BImgBuff) != (size_t) ldblk)
          {
            if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(), "Cannot read data.");
            break;
          }
          InsertComplexDoubleRow((double *)BImgBuff, i, image, MinVal_c, MaxVal_c);
        }
      }

      if(HDR.Type[1]==1)                /* Find Min and Max Values for floats */
      {
        if (MagickFindRawImageMinMax(image, import_options->endian, HDR.nRows,
                                     HDR.nCols, FloatPixel, (unsigned int)ldblk, BImgBuff,
                                     &MinVal_c, &MaxVal_c) != MagickPass)
          goto skip_reading_current;
        for(i=0; i<(long)HDR.nCols; i++)
        {
          if(ReadBlobXXXFloats(image, ldblk, (float *)BImgBuff) != (size_t) ldblk)
          {
            if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(), "Cannot read data.");
            break;
          }
          InsertComplexFloatRow((float *)BImgBuff, i, image, MinVal_c, MaxVal_c);
        }
      }
    }

        /*  Rotate image. */
    rotated_image = RotateImage(image, 90.0, exception);
    if(rotated_image != (Image *) NULL)
    {
        /* Remove page offsets added by RotateImage */
      rotated_image->page.x=0;
      rotated_image->page.y=0;

      blob = rotated_image->blob;
      rotated_image->blob = image->blob;
      rotated_image->colors = image->colors;
      image->blob = blob;
      ReplaceImageInList(&image,rotated_image);
      image = rotated_image;
    }

    StopTimer(&image->timer);

skip_reading_current:

    if (image_info->subrange != 0)
        if (image->scene >= (image_info->subimage+image_info->subrange-1))
          break;

    /* Allocate next image structure. */
    AllocateNextImage(image_info,image);
    if (image->next == (Image *) NULL) break;
    image=SyncNextImageInList(image);
    image->columns = image->rows = 0;
    image->colors=0;

      /* row scan buffer is no longer needed */
    MagickFreeResourceLimitedMemory(BImgBuff);
    BImgBuff = NULL;
  }

ImportImagePixelAreaFailed:
ExitLoop:

  if(BImgBuff!=NULL) MagickFreeResourceLimitedMemory(BImgBuff);

  return image;
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M A T L A B i m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadMATImage reads an MAT X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMATImage method is:
%
%      Image *ReadMATImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadMATImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadMATImage(const ImageInfo *image_info, ExceptionInfo *exception)
{
  Image *image, *image2=NULL,
   *rotated_image;
  PixelPacket *q;
  unsigned int status;
  MATHeader MATLAB_HDR;
  size_t size;
  magick_uint32_t CellType;
  ImportPixelAreaOptions import_options;
  magick_uint32_t i;
  size_t ldblk;
  unsigned char *BImgBuff = NULL;
  double MinVal_c, MaxVal_c;
  unsigned z, z2;
  unsigned Frames;
  int logging;
  int sample_size;
  magick_off_t filepos = 0x80;
  magick_off_t filesize;
  BlobInfo *blob;
  ImageInfo *clone_info = NULL;

  magick_uint32_t (*ReadBlobXXXLong)(Image *image);
  magick_uint16_t (*ReadBlobXXXShort)(Image *image);
  size_t (*ReadBlobXXXDoubles)(Image *image, size_t len, double *data);
  size_t (*ReadBlobXXXFloats)(Image *image, size_t len, float *data);

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  logging = LogMagickEvent(CoderEvent,GetMagickModule(),"enter");

  /*
     Open image file.
   */
  image = AllocateImage(image_info);
  image->rows=0;
  image->columns=0;

  status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
  if (status == False)
  {
    ThrowMATReaderException(FileOpenError, UnableToOpenFile, image);
  }

  /*
     Read MATLAB image.
   */
  if(ReadBlob(image,124,&MATLAB_HDR.identific) != 124)
  {
    ThrowMATReaderException(CorruptImageError,ImproperImageHeader,image);
  }

  if(strncmp(MATLAB_HDR.identific, "MATLAB", 6))
  {
    image2 = ReadMATImageV4(image_info,image,&import_options,exception,logging);
    if(image2==NULL) goto MATLAB_KO;
    image = image2;
    goto END_OF_READING;
  }

  MATLAB_HDR.Version = ReadBlobLSBShort(image);
  if(ReadBlob(image,2,&MATLAB_HDR.EndianIndicator) != 2)
  {
    ThrowMATReaderException(CorruptImageError,ImproperImageHeader,image);
  }

  ImportPixelAreaOptionsInit(&import_options);

  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),"  Endian %c%c",
        MATLAB_HDR.EndianIndicator[0],MATLAB_HDR.EndianIndicator[1]);
  if (!strncmp(MATLAB_HDR.EndianIndicator, "IM", 2))
  {
    ReadBlobXXXLong = ReadBlobLSBLong;
    ReadBlobXXXShort = ReadBlobLSBShort;
    ReadBlobXXXDoubles = ReadBlobLSBDoubles;
    ReadBlobXXXFloats = ReadBlobLSBFloats;
    import_options.endian = LSBEndian;
  }
  else if (!strncmp(MATLAB_HDR.EndianIndicator, "MI", 2))
  {
    ReadBlobXXXLong = ReadBlobMSBLong;
    ReadBlobXXXShort = ReadBlobMSBShort;
    ReadBlobXXXDoubles = ReadBlobMSBDoubles;
    ReadBlobXXXFloats = ReadBlobMSBFloats;
    import_options.endian = MSBEndian;
  }
  else                  /* unsupported endian */
  {
MATLAB_KO: ThrowMATReaderException(CorruptImageError,ImproperImageHeader,image);
  }

  filepos = TellBlob(image);

  filesize = GetBlobSize(image);              /* zero is returned if the size cannot be determined. */
  if(filesize>0 && BlobIsSeekable(image))
  {
    if(filesize > (magick_off_t)0xFFFFFFFF)
        filesize = (magick_off_t)0xFFFFFFFF;  /* More than 4GiB are not supported in MAT! */
  }
  else
  {
    filesize = (magick_off_t)0xFFFFFFFF;
    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "Blob is not seekable, MAT reader could fail.");
    ThrowMATReaderException(CorruptImageError,AnErrorHasOccurredReadingFromFile,image);
  }

  while(filepos<filesize && !EOFBlob(image)) /* object parser loop */
  {
    Frames = 1;
    if(filepos > filesize || filepos < 0)
    {
      ThrowMATReaderException(BlobError,UnableToObtainOffset,image);
    }
    if(SeekBlob(image,filepos,SEEK_SET) != filepos) break;
    /* printf("pos=%lX\n",(long) TellBlob(image)); */

    MATLAB_HDR.DataType = ReadBlobXXXLong(image);
    if(EOFBlob(image)) break;
    MATLAB_HDR.ObjectSize = ReadBlobXXXLong(image);
    if(EOFBlob(image)) break;

    if(MATLAB_HDR.ObjectSize+filepos >= filesize)   /* Safety check for forged and or corrupted data. */
    {
      if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
             "  MAT Object with size %u overflows file with size %u.", (unsigned)MATLAB_HDR.ObjectSize, (unsigned)(filesize));
      goto MATLAB_KO;
    }

    filepos += (magick_off_t) MATLAB_HDR.ObjectSize + 4 + 4;   /* Position of a next object, when exists. */

    image2 = image;
#if defined(HasZLIB)
    if(MATLAB_HDR.DataType == miCOMPRESSED)
    {
      if(clone_info==NULL)
        if((clone_info=CloneImageInfo(image_info)) == NULL)
                {
                  if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                                   "CloneImageInfo failed");
                  continue;
                }
      image2 = DecompressBlock(image,&MATLAB_HDR.ObjectSize,clone_info,exception);
      if(image2==NULL)
      {
        if(logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                                   "Decompression failed");
        continue;
      }
      MATLAB_HDR.DataType = ReadBlobXXXLong(image2); /* replace compressed object type. */
    }
#endif

    if(MATLAB_HDR.DataType!=miMATRIX)
    {
#if defined(HasZLIB)
      if(image2 != image)
      {
         DeleteImageFromList(&image2);  /* image2 is set to NULL */
      }
#endif
      continue;  /* skip another objects. */
    }

    MATLAB_HDR.unknown1 = ReadBlobXXXLong(image2);
    MATLAB_HDR.unknown2 = ReadBlobXXXLong(image2);

    MATLAB_HDR.unknown5 = ReadBlobXXXLong(image2);
    MATLAB_HDR.StructureClass = MATLAB_HDR.unknown5 & 0xFF;
    MATLAB_HDR.StructureFlag = (MATLAB_HDR.unknown5>>8) & 0xFF;

    MATLAB_HDR.unknown3 = ReadBlobXXXLong(image2);
    if(image!=image2)
      MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);    /* ??? don't understand why ?? */
    MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);
    MATLAB_HDR.DimFlag = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeX = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeY = ReadBlobXXXLong(image2);


    switch(MATLAB_HDR.DimFlag)
    {
      case  8: z2=z=1; break;                   /* 2D matrix*/
      case 12: z2=z = ReadBlobXXXLong(image2);  /* 3D matrix RGB*/
               (void) ReadBlobXXXLong(image2);  /* Unknown6 */
               if(z!=3) ThrowImg2MATReaderException(CoderError, MultidimensionalMatricesAreNotSupported,
                         image);
               break;
      case 16: z2=z = ReadBlobXXXLong(image2);  /* 4D matrix animation */
               if(z!=3 && z!=1)
                 ThrowImg2MATReaderException(CoderError, MultidimensionalMatricesAreNotSupported, image);
               Frames = ReadBlobXXXLong(image2);
               if (Frames == 0)
                 ThrowImg2MATReaderException(CorruptImageError,ImproperImageHeader,image2);
               break;
      default: ThrowImg2MATReaderException(CoderError, MultidimensionalMatricesAreNotSupported, image);
    }

    MATLAB_HDR.Flag1 = ReadBlobXXXShort(image2);
    MATLAB_HDR.NameFlag = ReadBlobXXXShort(image2);

    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "MATLAB_HDR.StructureClass %d",MATLAB_HDR.StructureClass);
    if (MATLAB_HDR.StructureClass != mxCHAR_CLASS &&
        MATLAB_HDR.StructureClass != mxSINGLE_CLASS &&          /* float + complex float */
        MATLAB_HDR.StructureClass != mxDOUBLE_CLASS &&          /* double + complex double */
        MATLAB_HDR.StructureClass != mxINT8_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT8_CLASS &&           /* uint8 + uint8 3D */
        MATLAB_HDR.StructureClass != mxINT16_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT16_CLASS &&          /* uint16 + uint16 3D */
        MATLAB_HDR.StructureClass != mxINT32_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT32_CLASS &&          /* uint32 + uint32 3D */
        MATLAB_HDR.StructureClass != mxINT64_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT64_CLASS)            /* uint64 + uint64 3D */
      ThrowImg2MATReaderException(CoderError,UnsupportedCellTypeInTheMatrix,image);

    switch (MATLAB_HDR.NameFlag)
    {
      case 0:
        size = ReadBlobXXXLong(image2); /* Object name string size */
        size = 4 * (((size_t) size + 3 + 1) / 4);
        if (SeekBlob(image2, size, SEEK_CUR) < 0) goto MATLAB_KO;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        (void) ReadBlob(image2, 4, &size); /* Object name string */
        break;
      default:
        goto MATLAB_KO;
    }

    CellType = ReadBlobXXXLong(image2);    /* Additional object type */
    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "MATLAB_HDR.CellType: %d",CellType);

    if(4 != ReadBlob(image2,4,&size))     /* data size */
        goto MATLAB_KO;

NEXT_FRAME:
      /* Image is gray when no complex flag is set and 2D Matrix */
    image->is_grayscale = (MATLAB_HDR.DimFlag==8) &&
           ((MATLAB_HDR.StructureFlag & FLAG_COMPLEX) == 0);

    switch (CellType)
    {
      case miINT8:
      case miUINT8:
        sample_size = 8;
        if(MATLAB_HDR.StructureFlag & FLAG_LOGICAL)
          image->depth = 1;
        else
          image->depth = Min(QuantumDepth,8);         /* Byte type cell */
        import_options.sample_type = UnsignedQuantumSampleType;
        ldblk =  MATLAB_HDR.SizeX;
        break;
      case miINT16:
      case miUINT16:
        sample_size = 16;
        image->depth = Min(QuantumDepth,16);        /* Word type cell */
        ldblk = MagickArraySize(2,MATLAB_HDR.SizeX);
        import_options.sample_type = UnsignedQuantumSampleType;
        break;
      case miINT32:
      case miUINT32:
        sample_size = 32;
        image->depth = Min(QuantumDepth,32);        /* Dword type cell */
        ldblk = MagickArraySize(4,MATLAB_HDR.SizeX);
        import_options.sample_type = UnsignedQuantumSampleType;
        break;
      case miINT64:
      case miUINT64:
        sample_size = 64;
        image->depth = Min(QuantumDepth,32);        /* Qword type cell */
        ldblk = MagickArraySize(8,MATLAB_HDR.SizeX);
        import_options.sample_type = UnsignedQuantumSampleType;
        break;
      case miSINGLE:
        sample_size = 32;
        image->depth = Min(QuantumDepth,32);        /* double type cell */
        import_options.sample_type = FloatQuantumSampleType;
#if 0
      if (sizeof(float) != 4)
        ThrowImg2MATReaderException(CoderError, IncompatibleSizeOfFloat, image);
#endif
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
        {                                           /* complex float type cell */
        }
        ldblk = MagickArraySize(4,MATLAB_HDR.SizeX);
        break;
      case miDOUBLE:
        sample_size = 64;
        image->depth = Min(QuantumDepth,32);        /* double type cell */
        import_options.sample_type = FloatQuantumSampleType;
        if (sizeof(double) != 8)
        {
          ThrowImg2MATReaderException(CoderError, IncompatibleSizeOfDouble, image);  /* this causes immediate return. */
        }
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
        {                         /* complex double type cell */
        }
        ldblk = MagickArraySize(8,MATLAB_HDR.SizeX);
        break;
      default:
        ThrowImg2MATReaderException(CoderError, UnsupportedCellTypeInTheMatrix, image)
    }

    image->columns = MATLAB_HDR.SizeX;
    image->rows = MATLAB_HDR.SizeY;
    image->colors = 1l << image->depth;
    if (image->logging)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),"MAT %lux%lu (%u colors)",
                            image->columns, image->rows, image->colors);

    if(image->columns == 0 || image->rows == 0)
      goto MATLAB_KO;
    if(MagickArraySize(ldblk,MATLAB_HDR.SizeY) == 0 ||
       MagickArraySize(ldblk,MATLAB_HDR.SizeY) > MATLAB_HDR.ObjectSize)  /* Safety check for forged and or corrupted data. */
      goto MATLAB_KO;

    if(CheckImagePixelLimits(image, exception) != MagickPass)
    {
      ThrowImg2MATReaderException(ResourceLimitError,ImagePixelLimitExceeded,image);
    }

    /* ----- Create gray palette ----- */

    if (CellType==miUINT8 && z!=3)
    {
      if(image->colors>256) image->colors = 256;

      if(AllocateImageColormap(image, image->colors) != MagickPass)
      {
         if(logging)
           (void)LogMagickEvent(CoderEvent,GetMagickModule(), "Cannot allocate colormap with %d colors.", image->colors);
NoMemory: ThrowImg2MATReaderException(ResourceLimitError, MemoryAllocationFailed, image)
      }
    }

    /*
      If ping is true, then only set image size and colors without
      reading any image data.
    */
    if (image_info->ping)
    {
      unsigned long temp = image->columns;  /* The true image is rotater 90 degs. Do rotation without data. */
      image->columns = image->rows;
      image->rows = temp;
      goto skip_reading_current;
    }

  /* ----- Load raster data ----- */
    BImgBuff = MagickAllocateResourceLimitedClearedArray(unsigned char *,ldblk,sizeof(double));    /* Ldblk was set in the check phase */
    if (BImgBuff == NULL)
      goto NoMemory;

    if (CellType==miDOUBLE)        /* Find Min and Max Values for floats */
    {
      if (MagickFindRawImageMinMax(image2, import_options.endian,MATLAB_HDR.SizeX,
                                   MATLAB_HDR.SizeY,DoublePixel, (unsigned int)ldblk, BImgBuff,
                                   &import_options.double_minvalue,
                                   &import_options.double_maxvalue) != MagickPass)
        goto skip_reading_current;
      if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                                        "Double import options: Min=%g, Max=%g",
                                        import_options.double_minvalue, import_options.double_maxvalue);
    }
    if (CellType==miSINGLE)        /* Find Min and Max Values for floats */
    {
      if (MagickFindRawImageMinMax(image2, import_options.endian,MATLAB_HDR.SizeX,
                                   MATLAB_HDR.SizeY,FloatPixel, (unsigned int)ldblk, BImgBuff,
                                   &import_options.double_minvalue,
                                   &import_options.double_maxvalue) != MagickPass)
        goto skip_reading_current;
      if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                                        "Float import options: Min=%g, Max=%g",
                                        import_options.double_minvalue, import_options.double_maxvalue);
    }

    /* Main loop for reading all scanlines */
    if(z==1) z=0; /* read grey scanlines */
                /* else read color scanlines */
    do
    {
      for(i = 0; i < MATLAB_HDR.SizeY; i++)
      {
        q = SetImagePixelsEx(image,0,MATLAB_HDR.SizeY-i-1,image->columns,1,&image->exception);
        if (q == (PixelPacket *)NULL)
        {
          if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
              "  MAT set image pixels returns unexpected NULL on a row %u.", (unsigned)(MATLAB_HDR.SizeY-i-1));
          goto skip_reading_current;  /* Skip image rotation, when cannot set image pixels */
        }
        if(ReadBlob(image2,ldblk,(char *)BImgBuff) != ldblk)
        {
          if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
             "  MAT cannot read scanrow %u from a file.", (unsigned)(MATLAB_HDR.SizeY-i-1));
          ThrowException(exception,CorruptImageError,UnexpectedEndOfFile,image->filename);
          DestroyImagePixels(image);  /* The unread data contains crap in memory, erase current image data. */
          image->columns = image->rows = 0;
          goto ExitLoop;        /* It would be great to be able to read corrupted images. */
                                /* this goto will abort reading, but there remains not fully read image
                                   in the memory. */
        }
        if((CellType==miINT8 || CellType==miUINT8) && (MATLAB_HDR.StructureFlag & FLAG_LOGICAL))
        {
          FixLogical((unsigned char *)BImgBuff,ldblk);
          if(ImportImagePixelArea(image,z2qtype[z],1,BImgBuff,&import_options,0) == MagickFail)
          {
ImportImagePixelAreaFailed:
            if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
              "  MAT failed to ImportImagePixelArea for a row %u", (unsigned)(MATLAB_HDR.SizeY-i-1));
            break;
          }
        }
        else
        {
          if(ImportImagePixelArea(image,z2qtype[z],sample_size,BImgBuff,&import_options,0) == MagickFail)
            goto ImportImagePixelAreaFailed;

          if (z<=1 &&                    /* fix only during a last pass z==0 || z==1 */
                (CellType==miINT8 || CellType==miINT16 || CellType==miINT32 || CellType==miINT64))
            FixSignedValues(q,MATLAB_HDR.SizeX);
        }

        if (!SyncImagePixelsEx(image,&image->exception))
        {
          if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
            "  MAT failed to sync image pixels for a row %u", (unsigned)(MATLAB_HDR.SizeY-i-1));
          goto ExitLoop;
        }
      }
    } while(z-- >= 2);
ExitLoop:

    if (i != MATLAB_HDR.SizeY)
      {
        if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
                                          "Failed to read all scanlines (failed at row %d of %u rows, z=%d)",
                                          i+1, (unsigned) MATLAB_HDR.SizeY, z);
        /* Avoid returning partially read image, which contains uninitialized data */
        /* goto skip_reading_current; */
        goto END_OF_READING;
      }


    /* Read complex part of numbers here */
    if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
    {        /* Find Min and Max Values for complex parts of floats */
      MinVal_c = MaxVal_c = 0;
      CellType = ReadBlobXXXLong(image2);    /* Additional object type */
      i = ReadBlobXXXLong(image2);           /* size of a complex part - toss away*/

      if (CellType==miDOUBLE)
      {
        if (MagickFindRawImageMinMax(image2, import_options.endian, MATLAB_HDR.SizeX,
                                     MATLAB_HDR.SizeY, DoublePixel, (unsigned int)ldblk, BImgBuff,
                                     &MinVal_c, &MaxVal_c) != MagickPass)
          ThrowImg2MATReaderException(CorruptImageError,UnexpectedEndOfFile,image);
      }
      if(CellType==miSINGLE)
      {
        if (MagickFindRawImageMinMax(image2, import_options.endian, MATLAB_HDR.SizeX,
                                     MATLAB_HDR.SizeY, FloatPixel, (unsigned int)ldblk, BImgBuff,
                                     &MinVal_c, &MaxVal_c)  != MagickPass)
          ThrowImg2MATReaderException(CorruptImageError,UnexpectedEndOfFile,image);
      }

      if (CellType==miDOUBLE)
        for (i = 0; i < MATLAB_HDR.SizeY; i++)
        {
          if (ReadBlobXXXDoubles(image2, ldblk, (double *)BImgBuff) != ldblk)
            ThrowImg2MATReaderException(CorruptImageError,UnexpectedEndOfFile,image);
          InsertComplexDoubleRow((double *)BImgBuff, i, image, MinVal_c, MaxVal_c);
        }

      if (CellType==miSINGLE)
        for (i = 0; i < MATLAB_HDR.SizeY; i++)
        {
          if (ReadBlobXXXFloats(image2, ldblk, (float *)BImgBuff) != ldblk)
            ThrowImg2MATReaderException(CorruptImageError,UnexpectedEndOfFile,image);
          InsertComplexFloatRow((float *)BImgBuff, i, image, MinVal_c, MaxVal_c);
        }
    }

      /* Image is gray when no complex flag is set and 2D Matrix AGAIN!!! */
    image->is_grayscale = (MATLAB_HDR.DimFlag==8) &&
             ((MATLAB_HDR.StructureFlag & FLAG_COMPLEX) == 0);
    image->is_monochrome = image->depth==1;
    if(image->is_monochrome)
      image->colorspace=GRAYColorspace;

    if(image2==image)           /* image2 is either native image or decompressed block. */
        image2 = NULL;          /* Remove shadow copy to an image before rotation. */

      /*  Rotate image. */
    rotated_image = RotateImage(image, 90.0, exception);
    if(rotated_image != (Image *) NULL)
    {
        /* Remove page offsets added by RotateImage */
      rotated_image->page.x=0;
      rotated_image->page.y=0;

      blob = rotated_image->blob;
      rotated_image->blob = image->blob;
      rotated_image->colors = image->colors;
      image->blob = blob;
      ReplaceImageInList(&image,rotated_image);
    }

skip_reading_current:

    if(image2==image) image2 = NULL;

    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;

    /* Allocate next image structure. */
    AllocateNextImage(image_info,image);
    if ((image == (Image *) NULL) || (image->next == (Image *) NULL)) break;
    image=SyncNextImageInList(image);
    image->columns = image->rows = 0;
    image->colors=0;

      /* row scan buffer is no longer needed */
    MagickFreeResourceLimitedMemory(BImgBuff);
    BImgBuff = NULL;

    if(--Frames>0)
    {
      z = z2;
      if(image2==NULL) image2 = image;
      if(!EOFBlob(image) && TellBlob(image)<filepos)
          goto NEXT_FRAME;
    }

    if(image2!=NULL)
      if(image2!=image)         /* Does shadow temporary decompressed image exist? */
      {
/*      CloseBlob(image2); */
        DeleteImageFromList(&image2);
        if(clone_info)
        {
          if(clone_info->file)
          {
            fclose(clone_info->file);
            clone_info->file = NULL;
            (void) unlink(clone_info->filename);
          }
        }
      }

  }

END_OF_READING:
  MagickFreeResourceLimitedMemory(BImgBuff);
  CloseBlob(image);

  {
    Image *p;
    long scene=0;

    /*
      Rewind list, removing any empty images while rewinding.
    */
    p=image;
    image=NULL;
    while (p != (Image *)NULL)
      {
        Image *tmp=p;
        if ((p->rows == 0) || (p->columns == 0)) {
          p=p->previous;
          DeleteImageFromList(&tmp);
        } else {
          image=p;
          p=p->previous;
        }
      }

    /*
      Fix scene numbers
    */
    for (p=image; p != (Image *) NULL; p=p->next)
      p->scene=scene++;
  }

  if(clone_info != NULL)        /* cleanup garbage file from compression */
  {
    if(clone_info->file)
    {
      fclose(clone_info->file);
      clone_info->file = NULL;
      (void) unlink(clone_info->filename);
    }
    DestroyImageInfo(clone_info);
    clone_info = NULL;
  }
  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),"return");
  if(image==NULL)
    ThrowMATReaderException(CorruptImageError,ImageFileDoesNotContainAnyImageData,image);
  return (image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A T L A B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMATLABImage writes an Matlab matrix to a file.
%
%  The format of the WriteMATLABImage method is:
%
%      unsigned int WriteMATLABImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMATLABImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
*/
static MagickPassFail WriteMATLABImage(const ImageInfo *image_info,Image *image)
{
  long y;
  unsigned z;
  MagickPassFail status;
  int logging;
  unsigned long DataSize;
  char padding;
  char MATLAB_HDR[0x80];
  time_t current_time;
#if defined(HAVE_LOCALTIME_R)
  struct tm tm_buf;
#endif /* if defined(HAVE_LOCALTIME_R) */
  const struct tm *t;
  unsigned char *pixels;
  int is_gray;
  unsigned char ImageName = 'A';

  current_time = time((time_t *)NULL);
#if defined(HAVE_LOCALTIME_R)
  t=localtime_r(&current_time, &tm_buf);
#else
  t=localtime(&current_time); /* Thread-unsafe version */
#endif  /* if defined(HAVE_LOCALTIME_R) */

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  logging = LogMagickEvent(CoderEvent,GetMagickModule(),"enter MAT");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFail)
    ThrowWriterException(FileOpenError,UnableToOpenFile,image);

  /*
    Store MAT header.
  */
  (void) memset(MATLAB_HDR,' ',Min(sizeof(MATLAB_HDR),124));
  snprintf(MATLAB_HDR,sizeof(MATLAB_HDR),"MATLAB 5.0 MAT-file, Platform: %.8s, Created on: %.3s %.3s %2d %2d:%2d:%2d %d",
    OsDesc,
    DayOfWTab[t->tm_wday],
    MonthsTab[t->tm_mon],
    t->tm_mday,
    t->tm_hour,t->tm_min,t->tm_sec,
    t->tm_year+1900);
  MATLAB_HDR[0x7C]=0;
  MATLAB_HDR[0x7D]=1;
  MATLAB_HDR[0x7E]='I';
  MATLAB_HDR[0x7F]='M';
  (void) WriteBlob(image,sizeof(MATLAB_HDR),MATLAB_HDR);

  pixels=(unsigned char *) NULL;
  while(image!=NULL)
  {
    pixels=MagickAllocateResourceLimitedMemory(unsigned char *,image->rows);
    if (pixels == (unsigned char *) NULL)
      ThrowWriterException(ResourceLimitError,MemoryAllocationFailed,image);

    is_gray = IsGrayImage(image,&image->exception);
    z = is_gray ? 0 : 3;

    DataSize = image->rows /*Y*/ * image->columns /*X*/;
    if(!is_gray) DataSize *= 3 /*Z*/;
    padding = ((unsigned char)(DataSize-1) & 0x7) ^ 0x7;

    (void) WriteBlobLSBLong(image, miMATRIX); /* 0x80 */
    (void) WriteBlobLSBLong(image, DataSize + padding + (is_gray?48l:56l)); /* 0x84 */
    (void) WriteBlobLSBLong(image, 0x6); /* 0x88 */
    (void) WriteBlobLSBLong(image, 0x8); /* 0x8C */
    (void) WriteBlobLSBLong(image, 0x6); /* 0x90 */
    (void) WriteBlobLSBLong(image, 0);
    (void) WriteBlobLSBLong(image, 0x5); /* 0x98 */
    (void) WriteBlobLSBLong(image, is_gray?0x8:0xC); /* 0x9C - DimFlag */
    (void) WriteBlobLSBLong(image, image->rows);    /* x: 0xA0 */
    (void) WriteBlobLSBLong(image, image->columns); /* y: 0xA4 */
    if(!is_gray)
    {
      (void) WriteBlobLSBLong(image, 3); /* z: 0xA8 */
      (void) WriteBlobLSBLong(image, 0);
    }
    (void) WriteBlobLSBShort(image, 1);  /* 0xB0 */
    (void) WriteBlobLSBShort(image, 1);  /* 0xB2 */
    (void) WriteBlobLSBLong(image, ImageName++); /* 0xB4  - here is a small bug only 'A' .. 'Z images could be generated properly */
    (void) WriteBlobLSBLong(image, 0x2); /* 0xB8 */
    (void) WriteBlobLSBLong(image, DataSize); /* 0xBC */

    /*
      Store image data.
    */
    {
      magick_uint64_t
        progress_span;

      magick_int64_t
        progress_quantum;

      progress_span = image->columns;
      if(!is_gray) progress_span *= 3;
      progress_quantum = 0;

      do
      {
        for (y=0; y<(long)image->columns; y++)
        {
          progress_quantum++;
          if (AcquireImagePixels(image,y,0,1,image->rows,&image->exception) == (PixelPacket *) NULL)
            break;
          if (ExportImagePixelArea(image,z2qtype[z],8,pixels,0,0) == MagickFail)
            break;
          if (WriteBlob(image,image->rows,pixels) != image->rows)
            break;
          if (QuantumTick(progress_quantum,progress_span))
            if (!MagickMonitorFormatted(progress_quantum,progress_span,&image->exception,
                                        SaveImageText,image->filename,
                                        image->columns,image->rows))
              break;
        }
        if (y != (long)image->columns)
          {
            status=MagickFail;
            goto BreakAll;
          }

      } while(z-- >= 2);
    }
BreakAll:

    while(padding-->0) (void) WriteBlobByte(image,0);

    MagickFreeResourceLimitedMemory(pixels);
    if(status == MagickFail || image->next==NULL) break;
    image=SyncNextImageInList(image);
  }

  status &= CloseBlob(image);
  MagickFreeResourceLimitedMemory(pixels);

  if (logging)
    (void)LogMagickEvent(CoderEvent,GetMagickModule(),"return MAT");

  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M A T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterMATImage adds attributes for the MAT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMATImage method is:
%
%      RegisterMATImage(void)
%
*/
ModuleExport void RegisterMATImage(void)
{
  MagickInfo * entry;

  entry = SetMagickInfo("MAT");
  entry->decoder = (DecoderHandler) ReadMATImage;
  entry->encoder = (EncoderHandler) WriteMATLABImage;
  entry->seekable_stream = True;
  entry->description =
#if defined(HasZLIB)
                        "MATLAB Level 4.0-7.0 image formats";
#else
                        "MATLAB Level 4.0-6.0 image formats";
#endif
  entry->module = "MAT";
  entry->blob_support=False;
  (void) RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M A T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterMATImage removes format registrations made by the
%  MAT module from the list of supported formats.
%
%  The format of the UnregisterMATImage method is:
%
%      UnregisterMATImage(void)
%
*/
ModuleExport void UnregisterMATImage(void)
{
  (void) UnregisterMagickInfo("MAT");
}
