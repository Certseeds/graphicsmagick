/*
  Copyright (C) 2003 GraphicsMagick Group
  Copyright (C) 2002 ImageMagick Studio
  Copyright 1991-1999 E. I. du Pont de Nemours and Company
 
  This program is covered by multiple licenses, which are described in
  Copyright.txt. You should have received a copy of Copyright.txt with this
  package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 
  ImageMagick Application Programming Interface declarations.
*/
#ifndef _MAGICK_MAGICK_H
#define _MAGICK_MAGICK_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef Image
  *(*DecoderHandler)(const ImageInfo *,ExceptionInfo *);

typedef unsigned int
  (*EncoderHandler)(const ImageInfo *,Image *),
  (*MagickHandler)(const unsigned char *,const size_t);

typedef struct _MagickInfo
{
  char
    *name,
    *description,
    *note,
    *version,
    *module;

  ImageInfo
    *image_info;

  DecoderHandler
    decoder;

  EncoderHandler
    encoder;

  MagickHandler
    magick;

  void
    *client_data;

  unsigned int
    adjoin,
    raw,
    stealth,
    seekable_stream,
    blob_support,
    thread_support;

  unsigned long
    signature;

  struct _MagickInfo
    *previous,
    *next;
} MagickInfo;

/*
  Magick method declaractions.
*/
extern MagickExport char
  *MagickToMime(const char *);

extern MagickExport const char
  *GetImageMagick(const unsigned char *,const size_t);

extern MagickExport unsigned int
  IsMagickConflict(const char *),
  ListModuleMap(FILE *,ExceptionInfo *),
  ListMagickInfo(FILE *,ExceptionInfo *),
  UnregisterMagickInfo(const char *);

extern MagickExport void
  DestroyMagick(void),
  DestroyMagickInfo(void),
  InitializeMagick(const char *);

extern MagickExport const MagickInfo
  *GetMagickInfo(const char *,ExceptionInfo *exception);

extern MagickExport const MagickInfo
  **GetMagickInfoArray(ExceptionInfo *exception);

extern MagickExport MagickInfo
  *RegisterMagickInfo(MagickInfo *),
  *SetMagickInfo(const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
