/*
 * Copyright (C) 2003-2024 GraphicsMagick Group
 * Copyright (C) 2003 ImageMagick Studio
 * Copyright 1991-1999 E. I. du Pont de Nemours and Company
 *
 * This program is covered by multiple licenses, which are described in
 * Copyright.txt. You should have received a copy of Copyright.txt with this
 * package; otherwise see http://www.graphicsmagick.org/www/Copyright.html.
 *
 * Test file encode/decode operations via write/read/write/read
 * sequence to detect any data corruption problems.  This does not
 * verify that the image is correct, only that encode/decode process
 * is repeatable.
 *
 * Written by Bob Friesenhahn <bfriesen@GraphicsMagick.org>
 *
 * The image returned by both reads must be identical (or deemed close
 * enough) in order for the test to pass.
 * */

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

  char
    basefilespec[MaxTextExtent],
    filename[MaxTextExtent],
    format[MaxTextExtent],
    infile[MaxTextExtent],
    size[MaxTextExtent],
    filespec[MaxTextExtent];

  int
    arg = 1,
    exit_status = 0,
    pause = 0;

  unsigned long
    original_frames;

  MagickBool
    check = MagickTrue,
    check_for_added_frames = MagickTrue,
    use_stdio = MagickFalse;

  ImageInfo
    *imageInfo;

  ExceptionInfo
    exception;

  (void) memset(basefilespec,0,sizeof(basefilespec));
  (void) memset(filename,0,sizeof(filename));
  (void) memset(format,0,sizeof(format));
  (void) memset(infile,0,sizeof(infile));
  (void) memset(size,0,sizeof(size));
  (void) memset(filespec,0,sizeof(filespec));

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

  if (LocaleNCompare("rwfile",argv[0],7) == 0)
    InitializeMagick((char *) NULL);
  else
    InitializeMagick(*argv);

  imageInfo=CloneImageInfo(0);
  GetExceptionInfo( &exception );

  (void) strcpy(basefilespec,"out_%d");

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
          else if (LocaleCompare("filespec",option+1) == 0)
            {
              (void) snprintf(basefilespec, sizeof(basefilespec), "%s", argv[++arg]);
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
          else if (LocaleCompare("stdio",option+1) == 0)
            {
              use_stdio=MagickTrue;
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
                      " [-depth integer] [-define value] [-filespec spec]"
                      " [-log format] [-interlace interlace] [-nocheck]"
                      " [-quality quality] [-size geometry] [-stdio]"
                      " [-verbose] infile format\n", argv[0] );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

  (void) strncpy(infile, argv[arg], MaxTextExtent-1 );
  infile[MaxTextExtent-1]='\0';
  arg++;
  (void) strncpy( format, argv[arg], MaxTextExtent-1 );
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

  imageInfo->dither = 0;

  strcpy(imageInfo->filename,"");
  if (use_stdio)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Reading stdio image %s", infile);
      imageInfo->file=fopen(infile,"rb");
    }
  else
    {
      if (!magick_info->adjoin || !check_for_added_frames)
        (void) snprintf( imageInfo->filename, sizeof(imageInfo->filename), "%.*s%s",
                         (int)(sizeof(imageInfo->filename)-sizeof("[0]")-1), infile, "[0]");
      else
        (void) snprintf(imageInfo->filename, sizeof(imageInfo->filename), "%s", infile);
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Reading image %s", imageInfo->filename);
    }
  original = ReadImage ( imageInfo, &exception );
  if (use_stdio)
    {
      (void) fclose(imageInfo->file);
      imageInfo->file = (FILE *) NULL;
    }
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
    Get file format information.
  */
  size[0] = '\0';
  magick_info=GetMagickInfo(format,&exception);
  if (magick_info == (MagickInfo *) NULL)
    {
      fprintf(stderr, "No support for \"%s\" format.\n",format);
      exit_status = 1;
      goto program_exit;
    }

  if (magick_info->raw)
    {
      /*
       * Specify original image size (WIDTHxHEIGHT) if format requires it
       */
      FormatString( size, "%lux%lu", original->columns, original->rows );
    }
  if (IgnoreExtensionTreatment == magick_info->extension_treatment)
    {
      /*
        Prepend magic specifier if extension will be ignored.
      */
      (void) snprintf(filespec, sizeof(filespec), "%.*s:%.*s%s", 32, format,
                      (int) sizeof(filespec)-37 , basefilespec, ".%s");
    }
  else
    {
      (void) snprintf(filespec, sizeof(filespec), "%s%s", basefilespec, ".%s");
    }

  (void) snprintf( filename, sizeof(filename), filespec, 1, format );
  (void) remove(filename);

  /*
   * Save image to file
   */
  (void) strncpy( original->magick, format, MaxTextExtent );
  original->magick[MaxTextExtent-1]='\0';
  (void) fflush(stdout);

  strcpy(imageInfo->filename,"");
  if (use_stdio)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Writing stdio image %s", filename);
      imageInfo->file=fopen(filename,"wb+");
    }
  else
    {
      (void) strncpy( original->filename, filename, MaxTextExtent );
      original->filename[MaxTextExtent-1]='\0';
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Writing image %s", original->filename);
    }
  original->delay = 10;
  if (!WriteImage ( imageInfo, original ))
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&original->exception);
      exit_status = 1;
      goto program_exit;
    }
  if (use_stdio)
    {
      (void) fclose(imageInfo->file);
      imageInfo->file = (FILE *) NULL;
    }
  imageInfo->depth=original->depth;
  DestroyImageList( original );
  original = (Image*)NULL;

  /*
   * Verify that we can 'ping' the file
   */
  {
    Image
      *ping_image;

    MagickBool
      ping_error = MagickFalse;

    strcpy(imageInfo->filename,"");
    (void) strncpy( imageInfo->magick, format, MaxTextExtent );
    imageInfo->magick[MaxTextExtent-1]='\0';
    if (use_stdio)
      {
        imageInfo->file=fopen(filename,"rb");
      }
    else
      {
        strncpy( imageInfo->filename, filename, MaxTextExtent );
        imageInfo->filename[MaxTextExtent-1]='\0';
      }
    if ( size[0] != '\0' )
      (void) CloneString( &imageInfo->size, size );
    (void) fflush(stdout);
    ping_image = PingImage(imageInfo, &exception );
    if (use_stdio)
      {
        (void) fclose(imageInfo->file);
        imageInfo->file = (FILE *) NULL;
      }
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
        (void) printf ( "Failed to ping image from file \"%s\" in format %s\n",
                        filename, imageInfo->magick );
        (void) fflush(stdout);
        ping_error = MagickTrue;
      }
    else
      {
        /* Print a short description of the image to stdout */
        /* DescribeFrames(imageInfo, ping_image, MagickTrue); */
        DestroyImageList( ping_image );
      }
    if (ping_error)
      {
        exit_status = 1;
        goto program_exit;
      }
  }

  /*
   * Read image back from file
   */
  (void) strncpy( imageInfo->magick, format, MaxTextExtent );
  imageInfo->magick[MaxTextExtent-1]='\0';
  strcpy(imageInfo->filename,"");
  if (use_stdio)
    {
      imageInfo->file=fopen(filename,"rb");
    }
  else
    {
      strncpy( imageInfo->filename, filename, MaxTextExtent );
      imageInfo->filename[MaxTextExtent-1]='\0';
    }
  if ( size[0] != '\0' )
    (void) CloneString( &imageInfo->size, size );
  (void) fflush(stdout);
  original = ReadImage ( imageInfo, &exception );
  if (use_stdio)
    {
      (void) fclose(imageInfo->file);
      imageInfo->file = (FILE *) NULL;
    }
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
      (void) printf ( "Failed to read image from file in format %s\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

  /*
   * Save image to file
   */
  (void) sprintf( filename, basefilespec, 2, format );
  (void) remove(filename);

  (void) sprintf( filename, filespec, 2, format );
  (void) strncpy( original->magick, format, MaxTextExtent );
  original->magick[MaxTextExtent-1]='\0';
  strcpy(imageInfo->filename,"");
  if (use_stdio)
    {
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Writing stdio image %s", filename);
      imageInfo->file=fopen(filename,"wb+");
    }
  else
    {
      (void) strncpy( original->filename, filename, MaxTextExtent );
      original->filename[MaxTextExtent-1]='\0';
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                            "Writing stdio image %s", original->filename);
    }
  original->delay = 10;
  (void) fflush(stdout);
  if(!WriteImage (imageInfo,original))
    {
      if (exception.reason)
        (void) printf("    reason:%s\n",exception.reason);
      CatchException(&original->exception);
      exit_status = 1;
      goto program_exit;
    }
  if (use_stdio)
    {
      (void) fclose(imageInfo->file);
      imageInfo->file = (FILE *) NULL;
    }

  /*
   * Read image back from file
   */
  (void) strncpy( imageInfo->magick, format, MaxTextExtent );
  imageInfo->magick[MaxTextExtent-1]='\0';
  strcpy(imageInfo->filename,"");
  if (use_stdio)
    {
      imageInfo->file=fopen(filename,"rb");
    }
  else
    {
      (void) strncpy( imageInfo->filename, filename, MaxTextExtent );
      imageInfo->filename[MaxTextExtent-1]='\0';
    }
  if ( size[0] != '\0' )
    (void) CloneString( &imageInfo->size, size );
  (void) fflush(stdout);
  (void) LogMagickEvent(CoderEvent,GetMagickModule(),
                        "Reading image %s", imageInfo->filename);
  final = ReadImage ( imageInfo, &exception );
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
      (void) printf ( "Failed to read image from file in format %s\n",
                      imageInfo->magick );
      (void) fflush(stdout);
      exit_status = 1;
      goto program_exit;
    }

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
          (!strcmp( "JXL", format )) ||
          (!strcmp( "PAL", format )) ||
          (!strcmp( "PCD", format )) ||
          (!strcmp( "PCDS", format )) ||
          (!strcmp( "UYVY", format )) ||
          (!strcmp( "WEBP", format )) ||
          (!strcmp( "YUV", format )) ||

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
              Verify that reads from file R/W #1 and file R/W #2 did
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
  if (imageInfo->file != (FILE *) NULL)
    {
      (void) fclose(imageInfo->file);
      imageInfo->file = (FILE *) NULL;
    }

  DestroyExceptionInfo(&exception);
  DestroyImageInfo( imageInfo );
  DestroyMagick();

  if (pause)
    (void) getc(stdin);
  return exit_status;
}
