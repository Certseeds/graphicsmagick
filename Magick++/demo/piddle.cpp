// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000
//
// PerlMagick "piddle" demo re-implemented using Magick++ methods.
// The PerlMagick "piddle" demo is written by John Cristy
//

#include <string>
#include <iostream>

#include <Magick++.h>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  MagickIncarnate(*argv);

  string srcdir("");
  if(getenv("srcdir") != (char*)NULL)
    srcdir = getenv("srcdir") + string("/");

  // Common font to use.
  string font = string("@") + srcdir + "Generic.ttf";

  try {

    //
    // Create a 300x300 white canvas.
    //
    Image image( "300x300", "white" );

    // Drawing list
    std::list<Magick::Drawable> drawList;

    //
    // Draw blue grid
    //
    drawList.push_back(DrawableStrokeColor("#ccf"));
    for ( int i=0; i < 300; i += 10 )
      {
        drawList.push_back(DrawableLine(i,0, i,300));
        drawList.push_back(DrawableLine(0,i, 300,i));
      }

    //
    // Draw rounded rectangle.
    //
    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeColor("red"));
    drawList.push_back(DrawableRoundRectangle(15,15, 70,70, 10,10));

    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeColor("maroon"));
    drawList.push_back(DrawableStrokeWidth(4));
    drawList.push_back(DrawableRoundRectangle(15,15, 70,70, 10,10));

    //
    // Draw curve.
    //
    {
      drawList.push_back(DrawableStrokeColor("black"));
      drawList.push_back(DrawableStrokeWidth(4));
      drawList.push_back(DrawableFillColor(Color()));

      std::list<Magick::Coordinate> points;
      points.push_back(Coordinate(20,20));
      points.push_back(Coordinate(100,50));
      points.push_back(Coordinate(50,100));
      points.push_back(Coordinate(160,160));
      drawList.push_back(DrawableBezier(points));
    }

    //
    // Draw line
    //
    drawList.push_back(DrawableStrokeColor("red"));
    drawList.push_back(DrawableStrokeWidth(1));
    drawList.push_back(DrawableLine(10,200, 20,190));

    //
    // Draw arc within a circle.
    //
    drawList.push_back(DrawableStrokeColor("black"));
    drawList.push_back(DrawableFillColor("yellow"));
    drawList.push_back(DrawableStrokeWidth(4));
    drawList.push_back(DrawableCircle(160,70, 200,70));

    drawList.push_back(DrawableStrokeColor("black"));
    drawList.push_back(DrawableFillColor("blue"));
    drawList.push_back(DrawableStrokeWidth(4));
    {
      std::list<Path> path;
      path.push_back(PathMovetoAbs(Coordinate(160,70)));
      path.push_back(PathLinetoVerticalRel(-40));
      path.push_back(PathArcRel(PathArcArgs(40,40, 0, 0, 0, -40,40)));
      path.push_back(PathClosePath());
      drawList.push_back(DrawablePath(path));
    }

    //
    // Draw pentogram.
    //
    {
      drawList.push_back(DrawableStrokeColor("red"));
      drawList.push_back(DrawableFillColor("LimeGreen"));
      drawList.push_back(DrawableStrokeWidth(3));

      std::list<Magick::Coordinate> points;
      points.push_back(Coordinate(160,120));
      points.push_back(Coordinate(130,190));
      points.push_back(Coordinate(210,145));
      points.push_back(Coordinate(110,145));
      points.push_back(Coordinate(190,190));
      points.push_back(Coordinate(160,120));
      drawList.push_back(DrawablePolygon(points));
    }

    //
    // Draw rectangle.
    //
    drawList.push_back(DrawableStrokeColor("yellow"));
    drawList.push_back(DrawableStrokeWidth(5));
    drawList.push_back(DrawableFillColor(Color()));
    drawList.push_back(DrawableRectangle(200,200, 260,260));
    drawList.push_back(DrawableStrokeColor("green"));
    drawList.push_back(DrawableLine(200,260, 260,260));
    drawList.push_back(DrawableStrokeColor("red"));
    drawList.push_back(DrawableLine(260,200, 260,260));

    //
    // Draw text.
    //
    drawList.push_back(DrawableFont(font));
    drawList.push_back(DrawableFillColor("green"));
    drawList.push_back(DrawableStrokeColor("green"));
    drawList.push_back(DrawablePointSize(24));
//     drawList.push_back(DrawableRotation(-45.0));
//     drawList.push_back(DrawableTranslation(30,40));
    drawList.push_back(DrawableAffine(0.707107,0.707107,0.707107,-0.707107,30.000000,140.000000));
    drawList.push_back(DrawableText(0,0,"This is a test!"));
    
    image.draw(drawList);

//     image.font(font);
//     image.fillColor("green");
//     image.strokeColor("green");
//     image.fontPointsize(24);
//     image.transformRotation(-45.0);
//     image.annotate( "This is a test!", "+30+140");


//     image.write( "piddle.mvg" );

    cout << "Writing image \"piddle.miff\" ..." << endl;
    image.write( "piddle.miff" );

//     cout << "Display image..." << endl;
//     image.display( );

  }
  catch( Exception error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  catch( exception error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  
  return 0;
}
