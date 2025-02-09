// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999-2024
//
// Test reading/writing BLOBs using Magick++
//

#include <Magick++.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

using namespace Magick;

//
// Some compilers (e.g. older Tru64 UNIX) lack ios::binary
//
#if defined(MISSING_STD_IOS_BINARY)
#  define IOS_IN_BINARY ios::in
#else
#  define IOS_IN_BINARY ios::in | ios::binary
#endif

// A derived Blob class to exercise updateNoCopy()
class myBlob : public Blob
{
public:
  // Construct from open binary stream
  myBlob( ifstream &stream_ )
    : Blob()
    {
      unsigned char* blobData = new unsigned char[100000];
      char* c= reinterpret_cast<char *>(blobData);
      size_t blobLen=0;
      while( (blobLen< 100000) && stream_.get(*c) )
        {
          c++;
          blobLen++;
        }
      if ((!stream_.eof()) || (blobLen == 0))
        {
          cout << "Failed to stream into blob!" << endl;
          exit(EXIT_FAILURE);
        }

      // Insert data into blob
      updateNoCopy( reinterpret_cast<void*>(blobData), blobLen,
                    Blob::NewAllocator );
    }
};


int main( int /*argc*/, char ** argv)
{
  // Initialize/Deinitialize GraphicsMagick (scope based)
  InitializeMagickSentinel sentinel(*argv);

  int failures=0;

  try
    {
      string srcdir("");
      if(getenv("SRCDIR") != 0)
        srcdir = getenv("SRCDIR");

      string testimage;

      //
      // Test reading BLOBs
      //
      {
        string signature("");
        {
          Image image(srcdir + "test_image.miff");
          signature = image.signature();
        }

        // Read raw data from file into BLOB
        testimage = srcdir + "test_image.miff";
        ifstream in( testimage.c_str(), IOS_IN_BINARY );
        if( !in )
          {
            cout << "Failed to open file " << testimage << " for input!" << endl;
            exit(1);
          }
        in.seekg(0,ios::end);
        streampos file_size = in.tellg();
        in.seekg(0,ios::beg);
        unsigned char* blobData = new unsigned char[file_size];
        char* c=reinterpret_cast<char *>(blobData);
        in.read(c,file_size);
        if (!in.good())
          {
            cout << "Failed to read file " << testimage << " for input!" << endl;
            return EXIT_FAILURE;
          }
        in.close();

        // Construct Magick++ Blob
        Blob blob(static_cast<const void*>(blobData), file_size);
        delete [] blobData;

        // If construction of image fails, an exception should be thrown
        {
          // Construct with blob data only
          Image image( blob );
          if ( image.signature() != signature )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << "  Image signature "
                   << image.signature()
                   << " != "
                   << signature << endl;
            }
        }

        {
          // Construct with image geometry and blob data
          Image image(  blob, Geometry(148,99));
          if ( image.signature() != signature )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << "  Image signature "
                   << image.signature()
                   << " != "
                   << signature << endl;
            }
        }

        {
          // Construct default image, and then read in blob data
          Image image;
          image.read( blob );
          if ( image.signature() != signature )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << "  Image signature "
                   << image.signature()
                   << " != "
                   << signature << endl;
            }
        }

        {
          // Construct default image, and then read in blob data with
          // image geometry
          Image image;
          image.read( blob, Geometry(148,99) );
          if ( image.signature() != signature )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << "  Image signature "
                   << image.signature()
                   << " != "
                   << signature << endl;
            }
        }

      }

      // Test writing BLOBs
      {
        Blob blob;
        string signature("");
        {
          Image image(srcdir + "test_image.miff");
          image.magick("MIFF");
          image.write( &blob );
          signature = image.signature();
        }
        {
          Image image(blob);
          if ( image.signature() != signature )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << "  Image signature "
                   << image.signature()
                   << " != "
                   << signature << endl;
              image.display();
            }
        }

      }
      // Test writing BLOBs via STL writeImages
      {
        Blob blob;

        list<Image> first;
        readImages( &first, srcdir + "test_image_anim.miff" );
        writeImages( first.begin(), first.end(), &blob, true );
      }

      // Test constructing a BLOB from a derived class
      {

        string signature("");
        {
          Image image(srcdir + "test_image.miff");
          signature = image.signature();
        }

        // Read raw data from file into BLOB
        testimage = srcdir + "test_image.miff";
        ifstream in( testimage.c_str(), IOS_IN_BINARY );
        if( !in )
          {
            cout << "Failed to open file for input!" << endl;
            return EXIT_FAILURE;
          }

        myBlob blob( in );
        in.close();

        Image image( blob );
        if ( image.signature() != signature )
          {
            ++failures;
            cout << "Line: " << __LINE__
                 << "  Image signature "
                 << image.signature()
                 << " != "
                 << signature << endl;
          }
      }
    }

  catch( Exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return EXIT_FAILURE;
    }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return EXIT_FAILURE;
    }

  if ( failures )
    {
      cout << failures << " failures" << endl;
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
