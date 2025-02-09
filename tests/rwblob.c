/*
 * Copyright (C) 2003-2024 GraphicsMagick Group
 * Copyright (C) 2003 ImageMagick Studio
 * Copyright 1991-1999 E. I. du Pont de Nemours and Company
 *
 * This program is covered by multiple licenses, which are described in
 * Copyright.txt. You should have received a copy of Copyright.txt with this
 * package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 *
 * Test BLOB operations via write/read/write/read sequence to detect
 * any data corruption problems. This does not verify that the image is
 * correct, only that encode/decode process is repeatable.
 *
 * Written by Bob Friesenhahn <bfriesen@GraphicsMagick.org>
 *
 * The image returned by both reads must be identical in order for the
 * test to pass.
 *
 */

#include <magick/api.h>
#include <magick/enum_strings.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static void DescribeFrames(const ImageInfo *image_info, Image *list, const MagickBool ping)
{
  /* [0] AVS 70x46+072 Grayscale 8-bit adea7b1989cc5d19794a25ae3d7d0bc86f83b014f7231a869ee7b97177d54ab5 */
  static const char descr_fmt[] = "[%s] %m %wx%h%X%y %r %q-bit %#";
  static const char descr_fmt_ping[] = "[%s] %m %wx%h%X%y %r %q-bit";
  Image *list_entry = list;

  while (list_entry != (Image *) NULL)
    {
      char
        *text;

      text=TranslateText(image_info,list_entry,ping ? descr_fmt_ping : descr_fmt);
      if (text != (char *) NULL)
        {
          fprintf(stdout,"%s\n", text);
          MagickFree(text);
        }
      list_entry=GetNextImageInList(list_entry);
    }
}

int main ( int argc, char **argv )
{
  Image
    *final = (Image *) NULL,
    *original = (Image *) NULL;

  const MagickInfo
    *magick_info;

  size_t
    blob_length = 0;

  char
    *blob = NULL,
    infile[MaxTextExtent],
    format[MaxTextExtent],
    size[MaxTextExtent];

  int
    arg = 1,
    exit_status = 0,
    rows = 0,
    columns = 0,
    pause = 0;

  unsigned long
    original_frames;

  MagickBool
    check = MagickTrue,
    check_for_added_frames = MagickTrue;

  ImageInfo
    *imageInfo;

  ExceptionInfo
    exception;

  /*
    Initialize locale from environment variables (LANG, LC_CTYPE,
    LC_NUMERIC, LC_TIME, LC_COLLATE, LC_MONETARY, LC_MESSAGES,
    LC_ALL), but require that LC_NUMERIC use common conventions.  The
    LC_NUMERIC variable affects the decimal point character and
    thousands separator character for the formatted input/output
    functions and string conversion functions.
  */
  (void) setlocale(LC_ALL,"");
  (void) setlocale(LC_NUMERIC,"C");

  if (LocaleNCompare("rwblob",argv[0],7) == 0)
    InitializeMagick((char *) NULL);
  else
    InitializeMagick(*argv);

  imageInfo=CloneImageInfo(0);
  GetExceptionInfo( &exception );

  for (arg=1; arg < argc; arg++)
    {
      char
        *option = argv[arg];

      if (*option == '-')
        {
          if (LocaleCompare("compress",option+1) == 0)
            {
              arg++;
              option=argv[arg];
              imageInfo->compression=StringToCompressionType(option);
            }
          else if (LocaleCompare("debug",option+1) == 0)
            {
              (void) SetLogEventMask(argv[++arg]);
            }
          else if (LocaleCompare("depth",option+1) == 0)
            {
              imageInfo->depth=QuantumDepth;
              arg++;
              if ((arg == argc) || !sscanf(argv[arg],"%ld",&imageInfo->depth))
                {
                  (void) printf("-depth argument missing or not integer\n");
                  (void) fflush(stdout);
                  exit_status = 1;
                  goto program_exit;
                }
              if(imageInfo->depth != 8 && imageInfo->depth != 16 &&
                 imageInfo->depth != 32)
                {
                  (void) printf("-depth (%ld) not 8, 16, or 32\n",
                                imageInfo->depth);
                  (void) fflush(stdout);
                  exit_status = 1;
                  goto program_exit;
                }
            }
          else if (LocaleCompare("define",option+1) == 0)
            {
              if (arg == argc)
                {
                  (void) printf("-define argument missing\n");
                  (void) fflush(stdout);
                  exit_status = 1;
                  goto program_exit;
                }
              if (AddDefinitions(imageInfo,argv[++arg],&exception)
                  == MagickFail)
                {
                  exit_status = 1;
                  goto program_exit;
                }
            }
          else if (LocaleCompare("log",option+1) == 0)
            {
              (void) SetLogFormat(argv[++arg]);
            }
          else if (LocaleCompare("interlace",option+1) == 0)
            {
              imageInfo->interlace=StringToInterlaceType(argv[++arg]);
            }
          else if (LocaleCompare("nocheck",option+1) == 0)
            {
              check=MagickFalse;
            }
          else if (LocaleCompare("pause",option+1) == 0)
            {
              pause=1;
            }
          else if (LocaleCompare("quality",option+1) == 0)
            {
              imageInfo->quality=atol(argv[++arg]);
            }
          else if (LocaleCompare("size",option+1) == 0)
            {
              arg++;
              if ((arg == argc) || !IsGeometry(argv[arg]))
                {
                  (void) printf("-size argument missing or not geometry\n");
                  (void) fflush(stdout);
                  exit_status = 1;
                  goto program_exit;
                }
              (void) CloneString(&imageInfo->size,argv[arg]);
            }
          else if (LocaleCompare("verbose",option+1) == 0)
            {
              imageInfo->verbose+=1;
            }
        }
      else
        {
          break;
        }
    }
  if (arg != argc-2)
    {
      (void) printf("arg=%d, argc=%d\n", arg, argc);
      (void) printf ( "Usage: %s [-compress algorithm] [-debug events]"
                      " [-depth integer] [-define value] [-log format]"
                      " [-interlace interlace] [-nocheck] [-quality quality] "
                      " [-size geometry] [-verbose]"
                      " infile format\n", argv[0] );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

  (void) strncpy(infile, argv[arg], MaxTextExtent-1);
  infile[MaxTextExtent-1]='\0';
  arg++;
  (void) strncpy( format, argv[arg], MaxTextExtent-1);
  format[MaxTextExtent-1]='\0';

  magick_info=GetMagickInfo(format,&exception);
  if (magick_info == (MagickInfo *) NULL)
    {
      fprintf(stderr, "No support for \"%s\" format.\n",format);
      exit(1);
    }

  /*
    Some formats intentionally modify the number of frames.
    FAX & JBIG write multiple frames, but read only one frame.
  */
  if ((!strcmp( "FAX", format )) ||
      (!strcmp( "JBIG", format )) ||
      (!strcmp( "MNG", format )) ||
      (!strcmp( "PSD", format )) ||
      (!strcmp( "PTIF", format )) )
    check_for_added_frames = MagickFalse;

  /*
   * Read original image
   */
  DestroyImageInfo( imageInfo );
  imageInfo=CloneImageInfo(0);
  GetExceptionInfo( &exception );

  imageInfo->dither = 0;
  if (!magick_info->adjoin && !check_for_added_frames)
    (void) snprintf( imageInfo->filename, sizeof(imageInfo->filename), "%.*s%s",
                     (int)(sizeof(imageInfo->filename)-sizeof("[0]")-1), infile, "[0]");
  else
    (void) snprintf(imageInfo->filename, sizeof(imageInfo->filename), "%s", infile);

  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Reading image %s", imageInfo->filename);
  original = ReadImage ( imageInfo, &exception );
  if (exception.severity != UndefinedException)
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&exception);
      exit_status = 1;
      goto program_exit;
    }
  if ( original == (Image *)NULL )
    {
      (void) printf ( "Failed to read original image %s\n",
                      imageInfo->filename );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

  /*
    Save first input file number of frames for later verifications.
  */
  original_frames=GetImageListLength(original);

  /*
   * Obtain original image size if format requires it
   */
  rows    = original->rows;
  columns = original->columns;
  size[0]='\0';
  if (magick_info->raw)
    FormatString( size, "%dx%d", columns, rows );

  /*
   * Save image to BLOB
   */
  blob_length = 8192;
  (void) strncpy( original->magick, format, MaxTextExtent );
  original->magick[MaxTextExtent-1]='\0';
  (void) strcpy( imageInfo->filename, "" );
  original->delay = 10;
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Writing image to BLOB");
  blob =(char *) ImageToBlob ( imageInfo, original, &blob_length, &exception );
  if (exception.severity != UndefinedException)
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&exception);
      exit_status = 1;
      goto program_exit;
    }
  if ( blob == NULL )
    {
      (void) printf ( "Failed to write BLOB in format %s (blob is NULL!)\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }
  if ( blob_length == 0)
    {
      (void) printf ( "Failed to write BLOB in format %s (blob length is 0!)\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }
  imageInfo->depth=original->depth;
  DestroyImageList( original );
  original = (Image*)NULL;

  /*
   * Verify that we can 'ping' the BLOB.
   */
  {
    Image
      *ping_image;

    MagickBool
      ping_error = MagickFalse;

    (void) strncpy( imageInfo->magick, format, MaxTextExtent );
    imageInfo->magick[MaxTextExtent-1]='\0';
    (void) strcpy( imageInfo->filename, "" );
    if ( size[0] != '\0' )
      (void) CloneString( &imageInfo->size, size );
    ping_image = PingBlob(imageInfo, blob, blob_length, &exception );
    if (exception.severity != UndefinedException)
      {
        if (exception.reason)
          (void) printf("    reason:%s\n",exception.reason);
        CatchException(&exception);
        (void) fflush(stderr);
        ping_error = MagickTrue;
      }
    if ( ping_image == (Image *)NULL )
      {
        (void) printf ( "Failed to ping image from BLOB in format %s\n",
                        imageInfo->magick );
        (void) fflush(stdout);
        ping_error = MagickTrue;
      }
    else
      {
        /* Print a short description of the image to stdout */
        /* DescribeFrames(imageInfo, ping_image, MagickTrue); */
        /* (void) fflush(stdout); */
        DestroyImageList( ping_image );
      }
    if (ping_error)
      {
        exit_status = 1;
        goto program_exit;
      }
  }

  /*
   * Read image back from BLOB
   */
  (void) strncpy( imageInfo->magick, format, MaxTextExtent );
  imageInfo->magick[MaxTextExtent-1]='\0';
  (void) strcpy( imageInfo->filename, "" );
  if ( size[0] != '\0' )
    (void) CloneString( &imageInfo->size, size );
  original = BlobToImage( imageInfo, blob, blob_length, &exception );
  if (exception.severity != UndefinedException)
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&exception);
      exit_status = 1;
      goto program_exit;
    }
  if ( original == (Image *)NULL )
    {
      (void) printf ( "Failed to read image from BLOB in format %s\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }
  MagickFree(blob);
  blob=0;

  /*
   * Save image to BLOB
   */
  blob_length = 8192;
  (void) strncpy( original->magick, format, MaxTextExtent );
  original->magick[MaxTextExtent-1]='\0';
  (void) strcpy( imageInfo->filename, "" );
  original->delay = 10;
  blob = (char *) ImageToBlob ( imageInfo, original, &blob_length,
                                &exception );
  if (exception.severity != UndefinedException)
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&exception);
      exit_status = 1;
      goto program_exit;
    }
  imageInfo->depth=original->depth;
  if ( blob == NULL )
    {
      (void) printf ( "Failed to write BLOB in format %s\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

  /*
   * Read image back from BLOB
   */
  (void) strncpy( imageInfo->magick, format, MaxTextExtent );
  imageInfo->magick[MaxTextExtent-1]='\0';
  (void) strcpy( imageInfo->filename, "" );
  if ( size[0] != '\0' )
    (void) CloneString( &imageInfo->size, size );
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Reading image from BLOB");
  final = BlobToImage( imageInfo, blob, blob_length, &exception );
  if (exception.severity != UndefinedException)
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&exception);
      exit_status = 1;
      goto program_exit;
    }
  if ( final == (Image *)NULL )
    {
      (void) printf ( "Failed to read image from BLOB in format %s\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }
  MagickFree(blob);
  blob=0;

  /* Print a short description of the image to stdout */
  DescribeFrames(imageInfo, final, MagickFalse);
  (void) fflush(stdout);

  if (check)
    {
      /*
       * Check final output
       */
      double
        fuzz_factor = 0;

      /*
        Some formats are lossy.
      */
      if ((!strcmp( "CIN", format ) && (QuantumDepth == 8)) ||
          (!strcmp( "CMYK", format )) ||
          (!strcmp( "GRAY", format )) ||
          (!strcmp( "JNG", format )) ||
          (!strcmp( "JP2", format )) ||
          (!strcmp( "JPEG", format )) ||
          (!strcmp( "JPG", format )) ||
          (!strcmp( "JPG24", format )) ||
          (!strcmp( "PAL", format )) ||
          (!strcmp( "PCD", format )) ||
          (!strcmp( "PCDS", format )) ||
          (!strcmp( "UYVY", format )) ||
          (!strcmp( "WEBP", format )) ||
          (!strcmp( "YUV", format ))  ||

          (!strcmp( "EPDF", format ))  ||
          (!strcmp( "EPI", format ))  ||
          (!strcmp( "EPS", format ))  ||
          (!strcmp( "EPSF", format ))  ||
          (!strcmp( "EPSI", format ))  ||
          (!strcmp( "EPT", format ))  ||
          (!strcmp( "PDF", format ))  ||
          (!strcmp( "PS", format ))  ||
          (!strcmp( "PS2", format ))  ||

          (final->compression == JPEGCompression))
        fuzz_factor = 0.06;

      switch(imageInfo->compression)
        {
        case JPEGCompression:
        case JPEG2000Compression:
        case WebPCompression:
          {
            fuzz_factor = 0.06;
            break;
          }
        default:
          {
          }
        }

      {
        Image
          *o,
          *f;

        unsigned long
          frame=0;

        /*
          Verify that frame pixels are identical (or close enough).
        */
        for (o=original, f=final, frame=0;
             ((o != (Image *) NULL) && (f != (Image *) NULL)) ;
             o = o->next, f = f->next, frame++)
          {
            printf("Checking frame %ld...\n",frame);
            if ( !IsImagesEqual(o, f ) &&
                 (original->error.normalized_mean_error > fuzz_factor) )
              {
                (void) printf( "R/W file check for format \"%s\" failed "
                               "(frame = %ld): %.6f/%.6f/%.6fe\n",
                               format,frame,
                               original->error.mean_error_per_pixel,
                               original->error.normalized_mean_error,
                               original->error.normalized_maximum_error);
                (void) fflush(stdout);
                exit_status = 1;
                goto program_exit;
              }
          }

        if (check_for_added_frames)
          {
            /*
              Verify that reads from BLOB R/W #1 and BLOB R/W #2 did
              return the same number of frames.
            */
            if ((o != (Image *) NULL) && (f != (Image *) NULL))
              {
                (void) printf("R/W file check for format \"%s\" failed due to "
                              "differing number of returned frames (%ld vs %ld)\n",
                              format,
                              GetImageListLength(original),
                              GetImageListLength(final));
                exit_status = 1;
                goto program_exit;
              }

            /*
              If format supports multiple frames, then we should expect
              that frames are not lost (or spuriously added) due to
              read/write of format.
            */
            if (magick_info->adjoin)
              {
                unsigned long
                  final_frames;

                final_frames=GetImageListLength(final);
                if (original_frames != final_frames)
                  {
                    (void) printf("R/W file check for format \"%s\" failed due "
                                  "to differing number of returned frames (%ld "
                                  "vs %ld) from original file\n",
                                  format,original_frames,final_frames);
                    exit_status = 1;
                    goto program_exit;
                  }
              }
          }
      }
    }

 program_exit:
  if (original)
    DestroyImageList( original );
  original = (Image*)NULL;
  if (final)
    DestroyImageList( final );
  final = (Image*)NULL;
  if (blob)
    MagickFree(blob);
  blob=0;

  DestroyExceptionInfo(&exception);
  DestroyImageInfo( imageInfo );
  DestroyMagick();

  if (pause)
    (void) getc(stdin);
  return exit_status;
}
