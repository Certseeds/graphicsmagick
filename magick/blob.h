/*
  Copyright (C) 2003, 2004, 2007 GraphicsMagick Group
  Copyright (C) 2002 ImageMagick Studio
  Copyright 1991-1999 E. I. du Pont de Nemours and Company
 
  This program is covered by multiple licenses, which are described in
  Copyright.txt. You should have received a copy of Copyright.txt with this
  package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 
  Image Compression/Decompression Methods.
*/
#ifndef _MAGICK_BLOB_H
#define _MAGICK_BLOB_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "magick/image.h"

/*
  Minimum input file size before considering for memory map.
*/
#define MinBlobExtent  32768L

/*
  Forward declarations.
*/
typedef struct _BlobInfo BlobInfo;

/*
 *
 * BlobInfo methods
 *
 */

/*
  Makes a duplicate of the given blob info structure, or if blob info
  is NULL, a new one.
*/
extern MagickExport BlobInfo* CloneBlobInfo(const BlobInfo *blob_info);

/*
  Increments the reference count associated with the pixel blob,
  returning a pointer to the blob.
*/
extern MagickExport BlobInfo* ReferenceBlob(BlobInfo *blob);

/*
  Deallocate memory associated with the BlobInfo structure.
*/
extern MagickExport void DestroyBlobInfo(BlobInfo *blob);

/*
  If BLOB is a memory mapping then unmap it. Reset BlobInfo structure
  to its default state.
*/
extern MagickExport void DetachBlob(BlobInfo *blob);

/*
  Initialize a BlobInfo structure.
*/
extern MagickExport void GetBlobInfo(BlobInfo *blob);

/*
  Attach memory buffer to a BlobInfo structure.
*/
extern MagickExport void AttachBlob(BlobInfo *blob_info,
                                    const void *blob,
                                    const size_t length);

/*
 *
 * Functions for managing a BLOB (type BlobInfo) attached to an Image.
 *
 */

/*
  Deallocate all memory associated with an Image blob.
*/
extern MagickExport void DestroyBlob(Image *image);


/*
 *
 * Formatted image I/O functions
 *
 */

/*
  Read an Image from a formatted in-memory "file" image  ("BLOB").
*/
extern MagickExport Image* BlobToImage(const ImageInfo *image_info,
                                       const void *blob,
                                       const size_t length,
                                       ExceptionInfo *exception);

/*
  Return an Image populated with salient information regarding a
  formatted in-memory "file" image ("BLOB") but without reading the
  image pixels.
*/
extern MagickExport Image* PingBlob(const ImageInfo *image_info,
                                    const void *blob,
                                    const size_t length,
                                    ExceptionInfo *exception);

/*
  Writes an Image to a formatted (like a file) in-memory
  representation.
*/
extern MagickExport void *ImageToBlob(const ImageInfo *image_info,
                                      Image *image,
                                      size_t *length,
                                      ExceptionInfo *exception);

/*
 *
 * Core File or BLOB I/O functions.
 *
 */

/*
  Blob open modes.
*/
typedef enum
{
  UndefinedBlobMode,    /* Undefined */
  ReadBlobMode,         /* Open for reading (text) */ /* only locale.c */
  ReadBinaryBlobMode,   /* Open for reading (binary) */
  WriteBlobMode,        /* Open for writing (text) */ /* only mvg.c txt.c */
  WriteBinaryBlobMode   /* Open for writing (binary) */
} BlobMode;

/*
  Open an input or output stream for access.  May also use a stream
  provided via image_info->stream.
*/
extern MagickExport unsigned int OpenBlob(const ImageInfo *image_info,
                                          Image *image,
                                          const BlobMode mode,
                                          ExceptionInfo *exception);

/*
  Close I/O to the file or BLOB.
*/
extern MagickExport void CloseBlob(Image *image);


/*
  Read data from the file or BLOB into a buffer.
*/
extern MagickExport size_t ReadBlob(Image *image,
                                    const size_t length,
                                    void *data);

/*
  Read data from the file or BLOB into a buffer, but support zero-copy
  if possible.
*/
extern MagickExport size_t ReadBlobZC(Image *image,
                                      const size_t length,
                                      void **data);

/*
  Write data from a buffer to the file or BLOB.
*/
extern MagickExport size_t WriteBlob(Image *image,
                                     const size_t length,
                                     const void *data);

/*
  Move the current read or write offset position in the file or BLOB.
*/
extern MagickExport magick_off_t SeekBlob(Image *image,
                                          const magick_off_t offset,
                                          const int whence);

/*
  Obtain the current read or write offset position in the file or
  BLOB.
*/
extern MagickExport magick_off_t TellBlob(const Image *image);

/*
  Ensure that all images in an image list are pointing to the same
  BlobInfo object as the first image.  Then flush all unwritten data
  to the output file or blob.
*/
extern MagickExport int SyncBlob(Image *image);

/*
  Test to see if EOF has been detected while reading the file or BLOB.
*/
extern MagickExport int EOFBlob(const Image *image);

/*
  Test to see if an error has been encountered while doing I/O to the
  file or BLOB.
*/
extern MagickExport int GetBlobStatus(const Image *image);

/*
  Obtain the current size of the file or BLOB.  Zero is returned if
  the size can not be determined.
*/
extern MagickExport magick_off_t GetBlobSize(const Image *image);


/*
  Obtain the underlying stdio FILE* for the file (if any).
*/
extern MagickExport FILE *GetBlobFileHandle(const Image *image);

/*
  Obtain a pointer to the base of where BLOB data is stored.  The data
  is only available if the data is stored on the heap, or is memory
  mapped.  Otherwise NULL is returned.
*/
extern MagickExport unsigned char *GetBlobStreamData(const Image *image);


/*
 *
 * Formatted File or BLOB I/O functions.
 *
 */

/*
  Read a single byte from the file or BLOB.  Returns an EOF character
  if EOF has been detected.
*/
extern MagickExport int ReadBlobByte(Image *image);

/*
  Read a 16-bit unsigned "short" value from the file or BLOB in
  little-endian order.
*/
extern MagickExport unsigned short ReadBlobLSBShort(Image *image);

/*
  Read a 16-bit unsigned "short" value from the file or BLOB in
  big-endian order.
*/
extern MagickExport unsigned short ReadBlobMSBShort(Image *image);

/*
  Read a 32-bit unsigned "long" value from the file or BLOB in
  little-endian order.
*/
extern MagickExport unsigned long ReadBlobLSBLong(Image *image);

/*
  Read a 32-bit unsigned "long" value from the file or BLOB in
  big-endian order.
*/
extern MagickExport unsigned long ReadBlobMSBLong(Image *image);

/*
  Read a 64-bit "double" value from the file or BLOB in
  little-endian order.
*/
extern MagickExport double ReadBlobLSBdouble(Image * image);

/*
  Read a 64-bit "double" value from the file or BLOB in
  big-endian order.
*/
extern MagickExport double ReadBlobMSBdouble(Image * image);

/*
  Read a string from the file or blob until a newline character is
  read or an end-of-file condition is encountered.
*/
extern MagickExport char *ReadBlobString(Image *image,
                                         char *string);

/*
  Write a single byte to the file or BLOB.
*/
extern MagickExport size_t WriteBlobByte(Image *image,
                                         const unsigned long value);


/*
  Write a 16-bit "short" value to the  file or BLOB in little-endian order.
*/
extern MagickExport size_t WriteBlobLSBShort(Image *image,
                                             const unsigned long value);

/*
  Write a 32-bit "long" value to the file or BLOB in little-endian order.
*/
extern MagickExport size_t WriteBlobLSBLong(Image *image,
                                            const unsigned long value);



/*
  Write a 32-bit "long" value to the file or BLOB in big-endian order.
*/
extern MagickExport size_t WriteBlobMSBLong(Image *image,
                                            const unsigned long value);

/*
  Write a 16-bit "short" value to the file or BLOB in big-endian order.
*/
extern MagickExport size_t WriteBlobMSBShort(Image *image,
                                             const unsigned long value);

/*
  Write a C string to the file or BLOB, without the terminating NULL
  byte.
*/
extern MagickExport size_t WriteBlobString(Image *image,
                                           const char *string);

/*
 *
 * BLOB attribute access.
 *
 */

/*
  Blob supports seek operations.  BlobSeek() and BlobTell() may safely
  be used.
*/
extern MagickExport unsigned int BlobIsSeekable(const Image *image);

/*
  Allow file descriptor to be closed (if True).
*/
extern MagickExport void SetBlobClosable(Image *image,
                                         unsigned int closable);

/*
  Blob is for a temporary file which should be deleted (if True).
*/
extern MagickExport void SetBlobTemporary(Image *image,
                                          unsigned int isTemporary);

/*
  Returns True if the file associated with the blob is a temporary
  file and should be removed when the associated image is destroyed.
*/
extern MagickExport unsigned int GetBlobTemporary(const Image *image);

/*
 *
 * Memory mapped file support.
 *
 */

/*
  Memory mapping modes.
*/
typedef enum
{
  ReadMode,             /* Map for read-only access */
  WriteMode,            /* Map for write-only access (useless) */
  IOMode                /* Map for read/write access */
} MapMode;

/*
  Release memory mapping for a region.
*/
extern MagickExport unsigned int UnmapBlob(void *map,
                                           const size_t length);

/*
  Perform a requested memory mapping of a file descriptor.
*/
extern MagickExport void *MapBlob(int file,
                                  const MapMode mode,
                                  magick_off_t offset,
                                  size_t length);

/*
 *
 * Buffer to File / File to Buffer functions.
 *
 */

/*
  Writes a buffer to a named file.
*/
extern MagickExport unsigned int BlobToFile(const char *filename,
                                            const void *blob,
                                            const size_t length,
                                            ExceptionInfo *exception);

/*
  Read the contents of a file into memory.
*/
extern MagickExport void *FileToBlob(const char *filename,
                                     size_t *length,
                                     ExceptionInfo *exception);

/*
 *
 * Junk yet to be categorized.
 *
 */

/*
  Returns the stream handler associated with the image.
*/
extern MagickExport StreamHandler GetBlobStreamHandler(const Image *image);


/*
  Copies data from the input stream to a file.  Useful in case it is
  necessary to perform seek operations on the input data.
*/
extern MagickExport unsigned int ImageToFile(Image *image,
                                             const char *filename,
                                             ExceptionInfo *exception);

/*
  Search for a configuration file (".mgk" file) using appropriate
  rules and return as an in-memory buffer.
*/
extern MagickExport void *GetConfigureBlob(const char *filename,
                                           char *path,
                                           size_t *length,
                                           ExceptionInfo *exception);

/*
  Converts a least-significant byte first buffer of integers to
  most-significant byte first.
*/
extern MagickExport void MSBOrderLong(unsigned char *buffer,
                                      const size_t length);

/*
  Converts a least-significant byte first buffer of integers to
  most-significant byte first.
*/
extern MagickExport void MSBOrderShort(unsigned char *p,
                                       const size_t length);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
