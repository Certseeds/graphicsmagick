//
// Demonstrate using the 'analyze' process module to compute
// image statistics.
//
// Copyright Bob Friesenhahn, 2003-2024
//
// Usage: analyze file...
//

#include <Magick++.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <list>
using namespace std;
using namespace Magick;
int main(int argc,char **argv)
{
  if ( argc < 2 )
    {
      cout << "Usage: " << argv[0] << " file..." << endl;
      return EXIT_FAILURE;
    }

  // Initialize/Deinitialize GraphicsMagick (scope based)
  InitializeMagickSentinel sentinel(*argv);

  {
    std::list<std::string> attributes;

    attributes.push_back("TopLeftColor");
    attributes.push_back("TopRightColor");
    attributes.push_back("BottomLeftColor");
    attributes.push_back("BottomRightColor");
    attributes.push_back("BrightnessMean");
    attributes.push_back("BrightnessStddev");
    attributes.push_back("SaturationMean");
    attributes.push_back("SaturationStddev");

    char **arg = &argv[1];
    while ( *arg )
      {
        string fname(*arg);
        try {
          cout << "File: " << fname << endl;
          Image image( fname );

          /* Analyze module does not require an argument list */
          image.process("Analyze",0,0);

          list<std::string>::iterator pos = attributes.begin();
          while(pos != attributes.end())
            {
              cout << "  " << setw(16) << setfill(' ') << setiosflags(ios::left)
                   << *pos << " = " << image.attribute(*pos) << endl;
              pos++;
            }
        }
        catch( Exception &error_ )
          {
            cout << error_.what() << endl;
            return EXIT_FAILURE;
          }
        ++arg;
      }
  }

  return EXIT_SUCCESS;
}
