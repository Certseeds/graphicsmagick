/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                          W   W   W  PPPP     GGG                            %
%                          W  W w  W  P   P  G                                %
%                           W W W W   PPPP   G  GG                            %
%                            W   W    P      G    G                           %
%                            W   W    P       GGGGG                           %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                 June 2000                                   %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

/*
  Forward declarations.
*/
static unsigned int
  WriteWPGImage(const ImageInfo *,Image *);

#define DWORD unsigned long
#define WORD  unsigned
#define BYTE  unsigned char

typedef struct
	{
	DWORD FileId;
	DWORD DataOffset;
	WORD ProductType;
	WORD FileType;
	BYTE MajorVersion;
	BYTE MinorVersion;
	WORD EncryptKey;
	WORD Reserved;
	}WPGHeader;
typedef struct
	{
	BYTE	RecType;
	DWORD   RecordLength;
	}WPGRecord;
typedef struct
	{
	WORD Width;
	WORD Heigth;
	WORD Depth;
	WORD HorzRes;
	WORD VertRes;
	}WPGBitmapType1;
typedef struct
	{
	WORD RotAngle;
	WORD LowLeftX;
	WORD LowLeftY;
	WORD UpRightX;
	WORD UpRightY;
	WORD Width;
	WORD Heigth;
	WORD Depth;
	WORD HorzRes;
	WORD VertRes;
	}WPGBitmapType2;
typedef struct
	{
	WORD StartIndex;
	WORD NumOfEntries;
	}WPGColorMapRec;
  



static void Rd_WP_DWORD(Image *image, DWORD *d)
{
   unsigned char b;

   b=ReadByte(image);
   *d = b;
   if(b<0xFF) return;

   b=ReadByte(image);
   *d = (DWORD)b ;
   b=ReadByte(image);
   *d += (DWORD)b * 256;
   if(*d<0x8000) return;

   *d = (*d & 0x7FFF) << 16;
   b=ReadByte(image);
   *d += (DWORD)b;
   b=ReadByte(image);
   *d += (DWORD)b * 256l;
   return;
}


static void InsertRow(char *p,int y,Image *image)
{
int bit,x;
register PixelPacket *q;
IndexPacket index;
register IndexPacket *indexes;

 switch (image->depth)
      {
      case 1:  /* Convert bitmap scanline. */
             {
             q=SetImagePixels(image,0,y,image->columns,1);
             if (q == (PixelPacket *) NULL)
                   break;
             indexes=GetIndexes(image);
             for (x=0; x < ((int) image->columns-7); x+=8)
                {
                for (bit=0; bit < 8; bit++)
                   {
                   index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
                   indexes[x+bit]=index;
                   *q++=image->colormap[index];
                   }
                p++;
                }
             if ((image->columns % 8) != 0)
                 {
                 for (bit=0; bit < (int) (image->columns % 8); bit++)
                     {
                     index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
                     indexes[x+bit]=index;
                     *q++=image->colormap[index];
                     }
                 p++;
                 }
              if (!SyncImagePixels(image))
                 break;
/*            if (image->previous == (Image *) NULL)
                 if (QuantumTick(y,image->rows))
                   ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
            break;
            }
      case 4:  /* Convert PseudoColor scanline. */
           {
           q=SetImagePixels(image,0,y,image->columns,1);
           if (q == (PixelPacket *) NULL)
                 break;
           indexes=GetIndexes(image);
           for (x=0; x < ((int) image->columns-1); x+=2)
                 {
                 index=(*p >> 4) & 0xf;
                 indexes[x]=index;
                 *q++=image->colormap[index];
                 index=(*p) & 0xf;
                 indexes[x+1]=index;
                 *q++=image->colormap[index];
                 p++;
                 }
           if ((image->columns % 2) != 0)
                 {
                   index=(*p >> 4) & 0xf;
                   indexes[x]=index;
                   *q++=image->colormap[index];
                   p++;
                 }
           if (!SyncImagePixels(image))
                 break;
/*         if (image->previous == (Image *) NULL)
                 if (QuantumTick(y,image->rows))
                   ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
           break;
           }
      case 8: /* Convert PseudoColor scanline. */
           {
           q=SetImagePixels(image,0,y,image->columns,1);
           if (q == (PixelPacket *) NULL)
                 break;
           indexes=GetIndexes(image);
           for (x=0; x < (int) image->columns; x++)
                {
                index=(*p++);
                indexes[x]=index;
                *q++=image->colormap[index];
                }
           if (!SyncImagePixels(image))
                 break;
/*           if (image->previous == (Image *) NULL)
                 if (QuantumTick(y,image->rows))
                   ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
           }
           break;
       }
}


#define InsertByte(b) {BImgBuff[x]=b;x++;if(x>=ldblk) {InsertRow(BImgBuff,y,image);x=0;y++;}}


static int UnpackWPGRaster(Image *image)
{
unsigned x,y,i;
BYTE bbuf,RunCount;
BYTE *BImgBuff;

long ldblk;

/*if(dirty(image)) return(-1);  //Not Enough memory */
/*if AlineProc<>nil then AlineProc^.InitPassing(p.y,'Loading WPG');*/

 x=0;
 y=0;

 ldblk=((long)image->depth*image->columns+7)/8;
 BImgBuff=malloc(ldblk);
 if(BImgBuff==NULL) return(-2);

 while(y<image->rows)
     {
     bbuf=ReadByte(image);
     
/*     if not readed byte ??????	{
	delete Raster;
	Raster=NULL;
	return(-2);
	}*/

     RunCount=bbuf & 0x7F;
     if(bbuf & 0x80)
	{
	if(RunCount)	/* repeat next byte runcount * */
		{
                bbuf=ReadByte(image);
		for(i=0;i<RunCount;i++) InsertByte(bbuf);
		}
	   else {	/* read next byte as RunCount; repeat 0xFF runcount* */
                bbuf=ReadByte(image);
		for(i=0;i<RunCount;i++) InsertByte(0xFF);
		}
	}
   else {
	if(RunCount)   /* next runcount byte are readed directly */
		{
		for(i=0;i<RunCount;i++)
			{
                        bbuf=ReadByte(image);
			InsertByte(bbuf);
			};
		}
	   else {	/* repeat previous line runcount* */
		RunCount=ReadByte(image);
		if(x) {              /* attempt to duplicate row from x position: */
                      free(BImgBuff);/* I do not know what to do here */
		      return(-3);
		      }
		for(i=0;i<RunCount;i++)
			{
			x=0;
			y++;    /* Here I need to duplicate previous row RUNCOUNT* */
                        if(y<2) continue;
			if(y>image->rows)
                                 {
                                 free(BImgBuff);
                                 return(-4);
                                 }
                        InsertRow(BImgBuff,y-1,image);
			}
                }
        }
   }
 free(BImgBuff);
 return(0);
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W P G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadWPGImage reads an WPG X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadWPGImage method is:
%
%      Image *ReadWPGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadWPGImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadWPGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image;
  unsigned char *pixels;
  unsigned int status;
  WPGHeader  Header;
  WPGRecord  Rec;
  WPGBitmapType1 BitmapHeader1;
  WPGBitmapType2 BitmapHeader2;
  unsigned long height,width;

  /*
    Open image file.
  */
  image=AllocateImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    ThrowReaderException(FileOpenWarning,"Unable to open file",image);
  /*
    Read WPG image.
  */
   Header.FileId==LSBFirstReadLong(image);
   Header.DataOffset=LSBFirstReadLong(image);
   Header.ProductType=LSBFirstReadShort(image);
   Header.FileType=LSBFirstReadShort(image);
   Header.MajorVersion=ReadByte(image);
   Header.MinorVersion=ReadByte(image);
   Header.EncryptKey=LSBFirstReadShort(image);
   Header.Reserved=LSBFirstReadShort(image);

  if (Header.FileId!=0x435057FF || Header.FileType!=1 )
      ThrowReaderException(CorruptImageWarning,"Not a WPG image file",image);
  if(Header.EncryptKey!=0 )
      ThrowReaderException(CorruptImageWarning,"Encrypted WPG image file",image);

  while(!EOFBlob(image)) /* object parser loop */
   {
   SeekBlob(image,Header.DataOffset,SEEK_SET);  /*How could I do fseek??*/
   if(EOFBlob(image)) break;

   Rec.RecType=ReadByte(image);
   Rd_WP_DWORD(image,&Rec.RecordLength);

   Header.DataOffset=TellBlob(image)+Rec.RecordLength;

   switch(Rec.RecType)
     {
     case 0x0B: /* bitmap type 1 */
     	     BitmapHeader1.Width=LSBFirstReadShort(image);
	     BitmapHeader1.Heigth=LSBFirstReadShort(image);
	     BitmapHeader1.Depth=LSBFirstReadShort(image);
	     BitmapHeader1.HorzRes=LSBFirstReadShort(image);
	     BitmapHeader1.VertRes=LSBFirstReadShort(image);

             image->columns=BitmapHeader1.Width;
             image->rows=BitmapHeader1.Heigth;
             image->depth=BitmapHeader1.Depth;
             
	     if(UnpackWPGRaster(image)<0) /* The raster cannot be unpacked */
                 {
                 ThrowReaderException(ResourceLimitWarning,"Cannot decompress raster",image);
                 }
             break;
/*	  0x0E:	// palette
	     BlockRead(f,WpPal,sizeof(WpPal),readed);

	     CreatePalette(Pal,round(ln(WpPal.NumOfEntries)/ln(2)));
	     BlockRead(F,Pal^.pal[WpPal.StartIndex],3*(WpPal.NumOfEntries-WpPal.StartIndex));
*/
     case 0x14:  /* bitmap type 2 */
             BitmapHeader2.RotAngle=LSBFirstReadShort(image);
             BitmapHeader2.LowLeftX=LSBFirstReadShort(image);
             BitmapHeader2.LowLeftY=LSBFirstReadShort(image);
             BitmapHeader2.UpRightX=LSBFirstReadShort(image);
             BitmapHeader2.UpRightY=LSBFirstReadShort(image);
             BitmapHeader2.Width=LSBFirstReadShort(image);
             BitmapHeader2.Heigth=LSBFirstReadShort(image);
             BitmapHeader2.Depth=LSBFirstReadShort(image);
             BitmapHeader2.HorzRes=LSBFirstReadShort(image);
             BitmapHeader2.VertRes=LSBFirstReadShort(image);

             image->columns=BitmapHeader1.Width;
             image->rows=BitmapHeader1.Heigth;
             image->depth=BitmapHeader1.Depth;
             
	     if(UnpackWPGRaster(image)<0) /* The raster cannot be unpacked */
                 {
                 ThrowReaderException(ResourceLimitWarning,"Cannot decompress raster",image);
                 }
	     break;
     }
  }


/* if Pal<>nil then
	{
	p.Palette:=pal;
	pal:=nil;
	p.Typ:='P';
	};*/

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
%   R e g i s t e r W P G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterWPGImage adds attributes for the WPG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWPGImage method is:
%
%      RegisterWPGImage(void)
%
*/
ModuleExport void RegisterWPGImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("WPG");
  entry->decoder=ReadWPGImage;
/*  entry->encoder=WriteWPGImage;*/
  entry->description=AllocateString("Word Perfect Graphics");
  entry->module=AllocateString("WPG");
  RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r A V S I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterWPGImage removes format registrations made by the
%  WPG module from the list of supported formats.
%
%  The format of the UnregisterWPGImage method is:
%
%      UnregisterWPGImage(void)
%
*/
ModuleExport void UnregisterWPGImage(void)
{
  UnregisterMagickInfo("WPG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e W P G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteWPGImage writes an image to a file in WPG X image format.
%
%  The format of the WriteWPGImage method is:
%
%      unsigned int WriteWPGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteWPGImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

/*static unsigned int WriteWPGImage(const ImageInfo *image_info,Image *image)
{
}
*/
