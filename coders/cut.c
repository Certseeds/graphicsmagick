/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                             CCC   U   U  TTTTT                              %
%                            C   C  U   U    T                                %
%                            C      U   U    T                                %
%                            C   C  U   U    T                                %
%                             CCC    UUU     T                                %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
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


static void InsertRow(unsigned char *p,int y,Image *image)
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
      case 2:  /* Convert PseudoColor scanline. */
           {
           q=SetImagePixels(image,0,y,image->columns,1);
           if (q == (PixelPacket *) NULL)
                 break;
           indexes=GetIndexes(image);
           for (x=0; x < ((int) image->columns-1); x+=2)
                 {
                 index=(*p >> 6) & 0x3;
		 if(index>=image->colors) index=image->colors-1;
                 indexes[x]=index;
                 *q++=image->colormap[index];
		 index=(*p >> 4) & 0x3;
		 if(index>=image->colors) index=image->colors-1;
                 indexes[x]=index;
                 *q++=image->colormap[index];
		 index=(*p >> 2) & 0x3;
		 if(index>=image->colors) index=image->colors-1;
                 indexes[x]=index;
                 *q++=image->colormap[index];
                 index=(*p) & 0x3;
		 if(index>=image->colors) index=image->colors-1;
                 indexes[x+1]=index;
                 *q++=image->colormap[index];
		 p++;
                 }
           if ((image->columns % 4) != 0)
                 {
                   index=(*p >> 6) & 0x3;
		   if(index>=image->colors) index=image->colors-1;
                   indexes[x]=index;
                   *q++=image->colormap[index];
		   if ((image->columns % 4) >= 1)

		      {
		      index=(*p >> 4) & 0x3;
		      if(index>=image->colors) index=image->colors-1;
                      indexes[x]=index;
                      *q++=image->colormap[index];
		      if ((image->columns % 4) >= 2)

		        {
		        index=(*p >> 2) & 0x3;
                        indexes[x]=index;
                        *q++=image->colormap[index];
		        }
		      }
                   p++;
                 }
           if (!SyncImagePixels(image))
                 break;
/*         if (image->previous == (Image *) NULL)
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
		 if(index>=image->colors) index=image->colors-1;
		 indexes[x]=index;
                 *q++=image->colormap[index];
                 index=(*p) & 0xf;
		 if(index>=image->colors) index=image->colors-1;
                 indexes[x+1]=index;
                 *q++=image->colormap[index];
                 p++;
                 }
           if ((image->columns % 2) != 0)
                 {
                   index=(*p >> 4) & 0xf;
		   if(index>=image->colors) index=image->colors-1;
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
           if (q == (PixelPacket *) NULL) break;
           indexes=GetIndexes(image);

	   for (x=0; x < (int) image->columns; x++)
                {
                index=(*p++);
		if(index>=image->colors) index=image->colors-1;
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



typedef struct
	{
	unsigned Width;
	unsigned Height;
	unsigned Reserved;
	}CUTHeader;			/*Dr Hallo*/
typedef struct
	{
	char FileId[2];
	unsigned Version;
	unsigned Size;
	char FileType;
	char SubType;
	unsigned BoardID;
	unsigned GraphicsMode;
	unsigned MaxIndex;
	unsigned MaxRed;
	unsigned MaxGreen;
	unsigned MaxBlue;
	char PaletteId[20];
	}CUTPalHeader;


/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C U T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadCUTImage reads an CUT X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadCUTImage method is:
%
%      Image *ReadCUTImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadCUTImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadCUTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image,*palette;
  ImageInfo *clone_info;
  unsigned int status,EncodedByte;
  unsigned char RunCount,RunValue,RunCountMasked;
  CUTHeader  Header;
  CUTPalHeader PalHeader;
  int i,j;
  long ldblk;
  unsigned char *BImgBuff=NULL,*ptrB;

  /*
    Open image file.
  */
  palette=NULL;
  clone_info=NULL;
  image=AllocateImage(image_info);
  
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    ThrowReaderException(FileOpenWarning,"Unable to open file",image);
  /*
    Read CUT image.
  */
   Header.Width=LSBFirstReadShort(image);
   Header.Height=LSBFirstReadShort(image);
   Header.Reserved=LSBFirstReadShort(image);

   if (Header.Width==0 || Header.Height==0 || Header.Reserved!=0)
CUT_KO:  ThrowReaderException(CorruptImageWarning,"Not a CUT image file",image);

/*---This code checks first line of image---*/
  EncodedByte=LSBFirstReadShort(image);
  RunCount=ReadByte(image);
  RunCountMasked=RunCount & 0x7F;
  ldblk=0;
  while(RunCountMasked>0)	/*end of line?*/
	{
	i=1;
	if(RunCount<0x80) i=RunCountMasked;
	SeekBlob(image,TellBlob(image)+i,SEEK_SET);
	if(EOFBlob(image)) goto CUT_KO;	/*wrong data*/
	EncodedByte-=i+1;
	ldblk+=RunCountMasked;

	RunCount=ReadByte(image);
	if(EOFBlob(image))  goto CUT_KO;	/*wrong data: unexpected eof in line*/
	RunCountMasked=RunCount & 0x7F;
	}
 if(EncodedByte!=1) goto CUT_KO;	/*wrong data: size incorrect*/
 i=0;				/*guess a number of bit planes*/
 if(ldblk==Header.Width)   i=8;
 if(2*ldblk==Header.Width) i=4;
 if(8*ldblk==Header.Width) i=1;
 if(i==0) goto CUT_KO;		/*wrong data: incorrect bit planes*/

 image->columns=Header.Width;
 image->rows=Header.Height;
 image->depth=i;
 image->colors=1l >> i;

//printf("CUT colors\n",image->colors);
 
/* ----- Do something with palette ----- */
 if ((clone_info=CloneImageInfo(image_info)) == NULL) goto NoPalette;
 
 printf("Image filename:%s\n",clone_info->filename);
 
 i=strlen(clone_info->filename);
 j=i;
 while(--i>0)
        {
	if(clone_info->filename[i]=='.') 
		{
		break;
		}
	if(clone_info->filename[i]=='/' || clone_info->filename[i]=='\\' ||
	   clone_info->filename[i]==':' ) 
	       {
	       i=j;
	       break;
	       }
	}
	
 strcpy(clone_info->filename+i,".PAL");
 if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL)
     {
     strcpy(clone_info->filename+i,".pal");
     if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL)
         {
	 clone_info->filename[i]=0;
	 if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL) 
             {
	     DestroyImageInfo(clone_info);
	     clone_info=NULL;
	     goto NoPalette;
	     }
	 }
     }

 if( (palette=AllocateImage(clone_info))==NULL ) goto NoPalette;
 status=OpenBlob(clone_info,palette,ReadBinaryType);
 if (status == False)
     {
ErasePalette:     
     DestroyImage(palette);
     palette=NULL;
     goto NoPalette;
     }
 
 printf("Found palette filename:%s\n",palette->magick_filename);
     
 if(palette!=NULL)
   {
   ReadBlob(palette,2,PalHeader.FileId);
   if(strncmp(PalHeader.FileId,"AH",2)) goto ErasePalette;
   PalHeader.Version=LSBFirstReadShort(palette);
   PalHeader.Size=LSBFirstReadShort(palette);
   PalHeader.FileType=ReadByte(palette);
   PalHeader.SubType=ReadByte(palette);
   PalHeader.BoardID=LSBFirstReadShort(palette);
   PalHeader.GraphicsMode=LSBFirstReadShort(palette);
   PalHeader.MaxIndex=LSBFirstReadShort(palette);
   PalHeader.MaxRed=LSBFirstReadShort(palette);
   PalHeader.MaxGreen=LSBFirstReadShort(palette);
   PalHeader.MaxBlue=LSBFirstReadShort(palette);
   ReadBlob(palette,20,PalHeader.PaletteId);
   
   if(PalHeader.MaxIndex<1) goto ErasePalette;
   image->colors=PalHeader.MaxIndex+1;
   if (!AllocateImageColormap(image,image->colors)) goto NoMemory;
   
   for(i=0;i<=PalHeader.MaxIndex;i++)
           {      /*this may be wrong- I don't know why is palette such strange*/
	   j=TellBlob(palette);
	   if((j % 512)>512-/*sizeof(pal)*/6)
	       {
	       j=((j / 512)+1)*512;
	       SeekBlob(palette,j,SEEK_SET);
	       }
	   image->colormap[i].red=UpScale(LSBFirstReadShort(palette));
	   image->colormap[i].green=UpScale(LSBFirstReadShort(palette));
	   image->colormap[i].blue=UpScale(LSBFirstReadShort(palette));       
//	   printf("<%d,%d,%d;%d>",image->colormap[i].red,image->colormap[i].green,
//	           image->colormap[i].blue,i);
	   }
   }

printf("Palette header loaded\n");
   

NoPalette:
 if(palette==NULL)
   { 
   printf("Fixup palette created\n");
   
   image->colors=256;
   if (!AllocateImageColormap(image,image->colors))
	   {
NoMemory:  ThrowReaderException(ResourceLimitWarning,"Memory allocation failed",
				image);
           }	   
   
   for (i=0; i < (int)image->colors; i++)
	   {
	   image->colormap[i].red=i;
	   image->colormap[i].green=i;
	   image->colormap[i].blue=i;
	   }
   }

printf("CUT colormap OK\n");
           
/* ----- Load RLE compressed raster ----- */
 BImgBuff=(unsigned char *) malloc(ldblk);  /*Ldblk was set in the check phase*/
 if(BImgBuff==NULL) goto NoMemory;

 SeekBlob(image,6 /*sizeof(Header)*/,SEEK_SET);
 for(i=0;i<Header.Height;i++)
      {
      EncodedByte=LSBFirstReadShort(image);

      ptrB=BImgBuff;
      j=ldblk;

      RunCount=ReadByte(image);
      RunCountMasked=RunCount & 0x7F;

      while(RunCountMasked>0)  	//end of line?
    		{
    		if(RunCountMasked>j)
			{		/*Wrong Data*/
			RunCountMasked=j;
			if(j==0) 
			    {
//			    printf("Wrong data\n");
			    break;
			    }
			}

		if(RunCount>0x80)
			{
			RunValue=ReadByte(image);
			memset(ptrB,RunValue,RunCountMasked);
			}
		else {
		     ReadBlob(image,RunCountMasked,ptrB);
		     }
		     
		ptrB+=RunCountMasked;
		j-=RunCountMasked;     
    
		if(EOFBlob(image)) goto Finish;	/* wrong data: unexpected eof in line */
		RunCount=ReadByte(image);
		RunCountMasked=RunCount & 0x7F;
    		}

//printf("Inserting row %d %d %ld\n",i,j,ldblk);
	InsertRow(BImgBuff,i,image);
   	}


Finish:
 CloseBlob(image);
 if(BImgBuff!=NULL) free(BImgBuff);
 if(palette!=NULL) DestroyImage(palette);
 if(clone_info!=NULL) DestroyImageInfo(clone_info);
 return(image);       
}



/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C U T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterCUTImage adds attributes for the CUT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCUTImage method is:
%
%      RegisterCUTImage(void)
%
*/
ModuleExport void RegisterCUTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CUT");
  entry->decoder=ReadCUTImage;
  entry->description=AllocateString("DR Hallo");
  entry->module=AllocateString("CUT");
  RegisterMagickInfo(entry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C U T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterCUTImage removes format registrations made by the
%  CUT module from the list of supported formats.
%
%  The format of the UnregisterCUTImage method is:
%
%      UnregisterCUTImage(void)
%
*/
ModuleExport void UnregisterCUTImage(void)
{
  UnregisterMagickInfo("CUT");
}
