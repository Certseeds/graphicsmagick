// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002
//
// Definition of Drawable (Graphic objects)
//

#if !defined(Magick_Drawable_header)
#define Magick_Drawable_header

#include "Magick++/Include.h"

#include <functional>
#include <string>
#include <list>
#include <utility>
#include "Magick++/Color.h"
#include "Magick++/Geometry.h"

namespace Magick
{

  //
  // Representation of an x,y coordinate
  //
  class Coordinate
  {
  public:
    Coordinate ( void )
      : _x(0),
        _y(0)
      { }
    Coordinate ( double x_, double y_ )
      : _x(x_),
        _y(y_)
      { }
    virtual ~Coordinate ()
      { }

    void   x ( double x_ )
      {
        _x = x_;
      }
    double x ( void ) const
      {
        return _x;
      }

    void   y ( double y_ )
      {
        _y = y_;
      }
    double y ( void ) const
      {
        return _y;
      }

  private:
    double _x;
    double _y;
  };

  // Compare two Coordinate objects regardless of LHS/RHS
  int operator == ( const Coordinate& left_, const Coordinate& right_ );
  int operator != ( const Coordinate& left_, const Coordinate& right_ );
  int operator >  ( const Coordinate& left_, const Coordinate& right_ );
  int operator <  ( const Coordinate& left_, const Coordinate& right_ );
  int operator >= ( const Coordinate& left_, const Coordinate& right_ );
  int operator <= ( const Coordinate& left_, const Coordinate& right_ );

  //
  // Base class for all drawable objects
  //
  class DrawableBase: public std::unary_function<MagickLib::DrawContext,void>
  {
  public:
    // Constructor
    DrawableBase ( void )
      { }

    // Destructor
    virtual ~DrawableBase ( void );

    // Operator to invoke equivalent draw API call
    virtual void operator()( MagickLib::DrawContext ) const = 0;

    // Return polymorphic copy of object
    virtual DrawableBase* copy() const = 0;

  private:
  };

  //
  // Representation of a drawable surrogate object to manage drawable objects
  //
#undef Drawable  // Conflict with <X11/Xproto.h>
  class Drawable
  {
  public:

    // Constructor
    Drawable ( void );

    // Construct from DrawableBase
    Drawable ( const DrawableBase& original_ );

    // Destructor
    ~Drawable ( void );

    // Copy constructor
    Drawable ( const Drawable& original_ );

    // Assignment operator
    Drawable& operator= (const Drawable& original_ );

    // Operator to invoke contained object
    void operator()( MagickLib::DrawContext context_ ) const;

  private:
    DrawableBase* dp;
  };

  //
  // Base class for all drawable path elements for use with
  // DrawablePath
  //
  class VPathBase
  {
  public:
    // Constructor
    VPathBase ( void )
      { }

    // Destructor
    virtual ~VPathBase ( void );

    // Assignment operator
    const VPathBase& operator= (const VPathBase& original_ );

    // Operator to invoke equivalent draw API call
    virtual void operator()( MagickLib::DrawContext context_ ) const = 0;

    // Return polymorphic copy of object
    virtual VPathBase* copy() const = 0;
  };

  //
  // Representation of a drawable path element surrogate object to
  // manage drawable path elements so they may be passed as a list to
  // DrawablePath.
  //
  class VPath
  {
  public:
    // Constructor
    VPath ( void );
    
    // Construct from VPathBase
    VPath ( const VPathBase& original_ );
    
    // Destructor
    virtual ~VPath ( void );

    // Copy constructor
    VPath ( const VPath& original_ );
    
    // Assignment operator
    VPath& operator= (const VPath& original_ );

    // Operator to invoke contained object
    void operator()( MagickLib::DrawContext context_ ) const;

  private:
    VPathBase* dp;
  };

  //
  // Drawable Objects
  //

  // Affine (scaling, rotation, and translation)
  class DrawableAffine  : public DrawableBase
  {
  public:
    DrawableAffine ( double sx_, double sy_,
                     double rx_, double ry_,
                     double tx_, double ty_ )
      : _sx(sx_),
        _sy(sy_),
        _rx(rx_),
        _ry(ry_),
        _tx(tx_),
        _ty(ty_)
      {
      }

    /*virtual*/ ~DrawableAffine( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/
    DrawableBase* copy() const;
    
    void sx( double sx_ )
      {
        _sx = sx_;
      }
    double sx( void ) const
      {
        return _sx;
      }

    void sy( double sy_ )
      {
        _sy = sy_;
      }
    double sy( void ) const
      {
        return _sy;
      }

    void rx( double rx_ )
      {
        _rx = rx_;
      }
    double rx( void ) const
      {
        return _rx;
      }
  
    void ry( double ry_ )
      {
        _ry = ry_;
      }
    double ry( void ) const
      {
        return _ry;
      }
  
    void tx( double tx_ )
      {
        _tx = tx_;
      }
    double tx( void ) const
      {
        return _tx;
      }
  
    void ty( double ty_ )
      {
        _ty = ty_;
      }
    double ty( void ) const
      {
        return _ty;
      }
  
  private:
    double _sx;
    double _sy;
    double _rx;
    double _ry;
    double _tx;
    double _ty;
  };

  // Arc
  class DrawableArc : public DrawableBase
  {
  public:
    DrawableArc ( double startX_, double startY_,
                  double endX_, double endY_,
                  double startDegrees_, double endDegrees_ )
      : _startX(startX_),
        _startY(startY_),
        _endX(endX_),
        _endY(endY_),
        _startDegrees(startDegrees_),
        _endDegrees(endDegrees_)
      { }

    /*virtual*/ ~DrawableArc( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void startX( double startX_ )
      {
        _startX = startX_;
      }
    double startX( void ) const
      {
        return _startX;
      }
  
    void startY( double startY_ )
      {
        _startY = startY_;
      }
    double startY( void ) const
      {
        return _startY;
      }
  
    void endX( double endX_ )
      {
        _endX = endX_;
      }
    double endX( void ) const
      {
        return _endX;
      }

    void endY( double endY_ )
      {
        _endY = endY_;
      }
    double endY( void ) const
      {
        return _endY;
      }
  
    void startDegrees( double startDegrees_ )
      {
        _startDegrees = startDegrees_;
      }
    double startDegrees( void ) const
      {
        return _startDegrees;
      }

    void endDegrees( double endDegrees_ )
      {
        _endDegrees = endDegrees_;
      }
    double endDegrees( void ) const
      {
        return _endDegrees;
      }
  
  private:
    double _startX;
    double _startY;
    double _endX;
    double _endY;
    double _startDegrees;
    double _endDegrees;
  };

  // Bezier curve (Coordinate list must contain at least three members)
  class DrawableBezier : public DrawableBase
  {
  public:
    // Construct from coordinates
    DrawableBezier ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    DrawableBezier ( const DrawableBezier& original_ );

    // Destructor
    /*virtual*/ ~DrawableBezier ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;
  
  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  // Circle
  class DrawableCircle : public DrawableBase
  {
  public:
    DrawableCircle ( double originX_, double originY_,
                     double perimX_, double perimY_ )
      : _originX(originX_),
        _originY(originY_),
        _perimX(perimX_),
        _perimY(perimY_)
      {
      }

    /*virtual*/ ~DrawableCircle ( void );
    
    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void originX( double originX_ )
      {
        _originX = originX_;
      }
    double originX( void ) const
      {
        return _originX;
      }

    void originY( double originY_ )
      {
        _originY = originY_;
      }
    double originY( void ) const
      {
        return _originY;
      }

    void perimX( double perimX_ )
      {
        _perimX = perimX_;
      }
    double perimX( void ) const
      {
        return _perimX;
      }

    void perimY( double perimY_ )
      {
        _perimY = perimY_;
      }
    double perimY( void ) const
      {
        return _perimY;
      }

  private:
    double _originX;
    double _originY;
    double _perimX;
    double _perimY;
  };

  // Colorize at point using PaintMethod
  class DrawableColor : public DrawableBase
  {
  public:
    DrawableColor ( double x_, double y_,
                    PaintMethod paintMethod_ )
      : _x(x_),
        _y(y_),
        _paintMethod(paintMethod_)
      { }

    /*virtual*/ ~DrawableColor ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

    void paintMethod( PaintMethod paintMethod_ )
      {
        _paintMethod = paintMethod_;
      }
    PaintMethod paintMethod( void ) const
      {
        return _paintMethod;
      }

  private:
    double _x;
    double _y;
    PaintMethod _paintMethod;
  };

  // Draw image at point, scaled to size specified by width and height
  class Image;
  class DrawableCompositeImage : public DrawableBase
  {
  public:
    DrawableCompositeImage ( double x_, double y_,
                             const std::string &filename_ );

    DrawableCompositeImage ( double x_, double y_,
                             const Image &image_ );

    DrawableCompositeImage ( double x_, double y_,
                             double width_, double height_,
                             const std::string &filename_ );

    DrawableCompositeImage ( double x_, double y_,
                             double width_, double height_,
                             const Image &image_ );

    DrawableCompositeImage ( double x_, double y_,
                             double width_, double height_,
                             const std::string &filename_,
                             CompositeOperator composition_ );

    DrawableCompositeImage ( double x_, double y_,
                             double width_, double height_,
                             const Image &image_,
                             CompositeOperator composition_ );

    // Copy constructor
    DrawableCompositeImage ( const DrawableCompositeImage& original_ );

    // Destructor
    /*virtual*/ ~DrawableCompositeImage( void );

    // Assignment operator
    DrawableCompositeImage& operator= (const DrawableCompositeImage& original_ );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;
    
    void composition( CompositeOperator composition_ )
      {
        _composition = composition_;
      }
    CompositeOperator composition( void ) const
      {
        return _composition;
      }

    void filename( const std::string &image_ );
    std::string filename( void ) const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

    void width( double width_ )
      {
        _width = width_;
      }
    double width( void ) const
      {
        return _width;
      }

    void height( double height_ )
      {
        _height = height_;
      }
    double height( void ) const
      {
        return _height;
      }

    void image( const Image &image_ );
    Magick::Image image( void ) const;

    // Specify image format used to output Base64 inlined image data.
    void magick( std::string magick_ );
    std::string magick( void );

  private:
    CompositeOperator  _composition;
    double             _x;
    double             _y;
    double             _width;
    double             _height;
    Image*             _image;
  };

  // Ellipse
  class DrawableEllipse : public DrawableBase
  {
  public:
    DrawableEllipse ( double originX_, double originY_, 
                      double radiusX_, double radiusY_,
                      double arcStart_, double arcEnd_ )
      : _originX(originX_),
        _originY(originY_),
        _radiusX(radiusX_),
        _radiusY(radiusY_),
        _arcStart(arcStart_),
        _arcEnd(arcEnd_)
      { }

    /*virtual*/ ~DrawableEllipse( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void originX( double originX_ )
      {
        _originX = originX_;
      }
    double originX( void ) const
      {
        return _originX;
      }

    void originY( double originY_ )
      {
        _originY = originY_;
      }
    double originY( void ) const
      {
        return _originY;
      }

    void radiusX( double radiusX_ )
      {
        _radiusX = radiusX_;
      }
    double radiusX( void ) const
      {
        return _radiusX;
      }

    void radiusY( double radiusY_ )
      {
        _radiusY = radiusY_;
      }
    double radiusY( void ) const
      {
        return _radiusY;
      }

    void arcStart( double arcStart_ )
      {
        _arcStart = arcStart_;
      }
    double arcStart( void ) const
      {
        return _arcStart;
      }

    void arcEnd( double arcEnd_ )
      {
        _arcEnd = arcEnd_;
      }
    double arcEnd( void ) const
      {
        return _arcEnd;
      }

  private:
    double _originX;
    double _originY; 
    double _radiusX;
    double _radiusY;
    double _arcStart;
    double _arcEnd;
  };

  // Specify drawing fill color
  class DrawableFillColor : public DrawableBase
  {
  public:
    DrawableFillColor ( const Color &color_ );

    DrawableFillColor ( const DrawableFillColor& original_ );

    /*virtual*/ ~DrawableFillColor( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void color( const Color &color_ )
      {
        _color = color_;
      }
    Color color( void ) const
      {
        return _color;
      }

  private:
    Color _color;
  };

  // Specify fill rule (fill-rule)
  class DrawableFillRule : public DrawableBase
  {
  public:
    DrawableFillRule ( const FillRule fillRule_ )
      : _fillRule(fillRule_)
      {
      }

    /*virtual*/ ~DrawableFillRule ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void fillRule( const FillRule fillRule_ )
      {
        _fillRule = fillRule_;
      }
    FillRule fillRule( void ) const
      {
        return _fillRule;
      }

  private:
    FillRule _fillRule;
  };

  // Specify drawing fill opacity
  class DrawableFillOpacity : public DrawableBase
  {
  public:
    DrawableFillOpacity ( double opacity_ )
      : _opacity(opacity_)
      {
      }

    /*virtual*/ ~DrawableFillOpacity ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void opacity( double opacity_ )
      {
        _opacity = opacity_;
      }
    double opacity( void ) const
      {
        return _opacity;
      }

  private:
    double _opacity;
  };

  // Specify text font
  class DrawableFont : public DrawableBase
  {
  public:
    DrawableFont ( const std::string &font_ );

    DrawableFont ( const std::string &family_,
                   StyleType style_,
                   const unsigned long weight_,
                   StretchType stretch_ );
    DrawableFont ( const DrawableFont& original_ );

    /*virtual*/ ~DrawableFont ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void font( const std::string &font_ )
      {
        _font = font_;
      }
    std::string font( void ) const
      {
        return _font;
      }

  private:
    std::string   _font;
    std::string   _family;
    StyleType     _style;
    unsigned long _weight;
    StretchType   _stretch;
  };

  // Specify text positioning gravity
  class DrawableGravity : public DrawableBase
  {
  public:
    DrawableGravity ( GravityType gravity_ )
      : _gravity(gravity_)
      {
      }

    /*virtual*/ ~DrawableGravity ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void gravity( GravityType gravity_ )
      {
        _gravity = gravity_;
      }
    GravityType gravity( void ) const
      {
        return _gravity;
      }

  private:
    GravityType _gravity;
  };

  // Line
  class DrawableLine : public DrawableBase
  {
  public:
    DrawableLine ( double startX_, double startY_,
                   double endX_, double endY_ )
      : _startX(startX_),
        _startY(startY_),
        _endX(endX_),
        _endY(endY_)
      { }

    /*virtual*/ ~DrawableLine ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void startX( double startX_ )
      {
        _startX = startX_;
      }
    double startX( void ) const
      {
        return _startX;
      }

    void startY( double startY_ )
      {
        _startY = startY_;
      }
    double startY( void ) const
      {
        return _startY;
      }

    void endX( double endX_ )
      {
        _endX = endX_;
      }
    double endX( void ) const
      {
        return _endX;
      }

    void endY( double endY_ )
      {
        _endY = endY_;
      }
    double endY( void ) const
      {
        return _endY;
      }

  private:
    double _startX;
    double _startY;
    double _endX;
    double _endY;
  };

  // Change pixel matte value to transparent using PaintMethod
  class DrawableMatte : public DrawableBase
  {
  public:
    DrawableMatte ( double x_, double y_,
                    PaintMethod paintMethod_ )
      : _x(x_),
        _y(y_),
        _paintMethod(paintMethod_)
      { }

    /*virtual*/ ~DrawableMatte ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

    void paintMethod( PaintMethod paintMethod_ )
      {
        _paintMethod = paintMethod_;
      }
    PaintMethod paintMethod( void ) const
      {
        return _paintMethod;
      }

  private:
    double _x;
    double _y;
    PaintMethod _paintMethod;
  };

  // Drawable Path
  class DrawablePath : public DrawableBase
  {
  public:
    DrawablePath ( const std::list<Magick::VPath> &path_ );

    DrawablePath ( const DrawablePath& original_ );

    /*virtual*/ ~DrawablePath ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    std::list<Magick::VPath> _path;
  };

  // Point
  class DrawablePoint : public DrawableBase
  {
  public:
    DrawablePoint ( double x_, double y_ )
      : _x(x_),
        _y(y_)
      { }

    /*virtual*/ ~DrawablePoint ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _x;
    double _y;
  };

  // Text pointsize
  class DrawablePointSize : public DrawableBase
  {
  public:
    DrawablePointSize ( double pointSize_ )
      : _pointSize(pointSize_)
      { }

    /*virtual*/ ~DrawablePointSize ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void pointSize( double pointSize_ )
      {
        _pointSize = pointSize_;
      }
    double pointSize( void ) const
      {
        return _pointSize;
      }

  private:
    double _pointSize;
  };

  // Polygon (Coordinate list must contain at least three members)
  class DrawablePolygon : public DrawableBase
  {
  public:
    DrawablePolygon ( const std::list<Magick::Coordinate> &coordinates_ );

    DrawablePolygon ( const DrawablePolygon& original_ );

    /*virtual*/ ~DrawablePolygon ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  // Polyline (Coordinate list must contain at least three members)
  class DrawablePolyline : public DrawableBase
  {
  public:
    DrawablePolyline ( const std::list<Magick::Coordinate> &coordinates_ );

    DrawablePolyline ( const DrawablePolyline& original_ );

    /*virtual*/ ~DrawablePolyline ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  // Pop Graphic Context
  class DrawablePopGraphicContext : public DrawableBase
  {
  public:
    DrawablePopGraphicContext ( void )
      : _dummy(0)
      {
      }

    /*virtual*/ ~DrawablePopGraphicContext ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    int   _dummy;
  };

  // Push Graphic Context
  class DrawablePushGraphicContext : public DrawableBase
  {
  public:
    DrawablePushGraphicContext ( void )
      : _dummy(0)
      {
      }

    /*virtual*/ ~DrawablePushGraphicContext ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    int   _dummy;
  };

  // Pop (terminate) Pattern definition
  class DrawablePopPattern : public DrawableBase
  {
  public:
    DrawablePopPattern ( void )
      : _dummy(0)
      {
      }

    /*virtual*/ ~DrawablePopPattern ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    int   _dummy;
  };

  // Push (create) Pattern definition
  class DrawablePushPattern : public DrawableBase
  {
  public:
    DrawablePushPattern ( const std::string &id_, long x_, long y_,
                          long width_, long height_ );

    DrawablePushPattern ( const DrawablePushPattern& original_ );

    /*virtual*/ ~DrawablePushPattern ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

  private:
    std::string         _id;
    long		_x;
    long		_y;
    long		_width;
    long		_height;
  };

  // Rectangle
  class DrawableRectangle : public DrawableBase
  {
  public:
    DrawableRectangle ( double upperLeftX_, double upperLeftY_,
                        double lowerRightX_, double lowerRightY_ )
      : _upperLeftX(upperLeftX_),
        _upperLeftY(upperLeftY_),
        _lowerRightX(lowerRightX_),
        _lowerRightY(lowerRightY_)
      { }

    /*virtual*/ ~DrawableRectangle ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void upperLeftX( double upperLeftX_ )
      {
        _upperLeftX = upperLeftX_;
      }
    double upperLeftX( void ) const
      {
        return _upperLeftX;
      }

    void upperLeftY( double upperLeftY_ )
      {
        _upperLeftY = upperLeftY_;
      }
    double upperLeftY( void ) const
      {
        return _upperLeftY;
      }

    void lowerRightX( double lowerRightX_ )
      {
        _lowerRightX = lowerRightX_;
      }
    double lowerRightX( void ) const
      {
        return _lowerRightX;
      }

    void lowerRightY( double lowerRightY_ )
      {
        _lowerRightY = lowerRightY_;
      }
    double lowerRightY( void ) const
      {
        return _lowerRightY;
      }

  private:
    double _upperLeftX;
    double _upperLeftY;
    double _lowerRightX;
    double _lowerRightY;
  };

  // Apply Rotation
  class DrawableRotation : public DrawableBase
  {
  public:
    DrawableRotation ( double angle_ )
      : _angle( angle_ )
      { }

    /*virtual*/ ~DrawableRotation ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void angle( double angle_ )
      {
        _angle = angle_;
      }
    double angle( void ) const
      {
        return _angle;
      }

  private:
    double _angle;
  };

  // Round Rectangle
  class DrawableRoundRectangle : public DrawableBase
  {
  public:
    DrawableRoundRectangle ( double centerX_, double centerY_,
                             double width_, double hight_,
                             double cornerWidth_, double cornerHeight_ )
      : _centerX(centerX_),
        _centerY(centerY_),
        _width(width_),
        _hight(hight_),
        _cornerWidth(cornerWidth_),
        _cornerHeight(cornerHeight_)
      { }

    /*virtual*/ ~DrawableRoundRectangle ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void centerX( double centerX_ )
      {
        _centerX = centerX_;
      }
    double centerX( void ) const
      {
        return _centerX;
      }

    void centerY( double centerY_ )
      {
        _centerY = centerY_;
      }
    double centerY( void ) const
      {
        return _centerY;
      }

    void width( double width_ )
      {
        _width = width_;
      }
    double width( void ) const
      {
        return _width;
      }

    void hight( double hight_ )
      {
        _hight = hight_;
      }
    double hight( void ) const
      {
        return _hight;
      }

    void cornerWidth( double cornerWidth_ )
      {
        _cornerWidth = cornerWidth_;
      }
    double cornerWidth( void ) const
      {
        return _cornerWidth;
      }

    void cornerHeight( double cornerHeight_ )
      {
        _cornerHeight = cornerHeight_;
      }
    double cornerHeight( void ) const
      {
        return _cornerHeight;
      }

  private:
    double _centerX;
    double _centerY;
    double _width;
    double _hight;
    double _cornerWidth;
    double _cornerHeight;
  };

  // Apply Scaling
  class DrawableScaling : public DrawableBase
  {
  public:
    DrawableScaling ( double x_, double y_ )
      : _x(x_),
        _y(y_)
      { }

    /*virtual*/ ~DrawableScaling ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _x;
    double _y;
  };

  // Apply Skew in X direction
  class DrawableSkewX : public DrawableBase
  {
  public:
    DrawableSkewX ( double angle_ )
      : _angle(angle_)
      { }

    /*virtual*/ ~DrawableSkewX ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void angle( double angle_ )
      {
        _angle = angle_;
      }
    double angle( void ) const
      {
        return _angle;
      }

  private:
    double _angle;
  };

  // Apply Skew in Y direction
  class DrawableSkewY : public DrawableBase
  {
  public:
    DrawableSkewY ( double angle_ )
      : _angle(angle_)
      { }

    /*virtual*/ ~DrawableSkewY ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void angle( double angle_ )
      {
        _angle = angle_;
      }
    double angle( void ) const
      {
        return _angle;
      }

  private:
    double _angle;
  };

  // Stroke dasharray
  class DrawableDashArray : public DrawableBase
  {
  public:
    DrawableDashArray( const double* dasharray_ );
    DrawableDashArray( const unsigned int* dasharray_ ); // Deprecated
    DrawableDashArray( const Magick::DrawableDashArray &original_ );

    /*virtual*/ ~DrawableDashArray( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void dasharray( const double* dasharray_ );
    void dasharray( const unsigned int* dasharray_ ); // Deprecated

    const double* dasharray( void ) const
      {
        return _dasharray;
      }

    DrawableDashArray& operator=(const Magick::DrawableDashArray &original_);

  private:
    double* _dasharray;
  };

  // Stroke dashoffset
  class DrawableDashOffset : public DrawableBase
  {
  public:
    DrawableDashOffset ( const double offset_ )
      : _offset(offset_)
      { }

    /*virtual*/ ~DrawableDashOffset ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void offset( const double offset_ )
      {
        _offset = offset_;
      }
    double offset( void ) const
      {
        return _offset;
      }

  private:
    double _offset;
  };

  // Stroke linecap
  class DrawableStrokeLineCap : public DrawableBase
  {
  public:
    DrawableStrokeLineCap ( LineCap linecap_ )
      : _linecap(linecap_)
      { }

    /*virtual*/ ~DrawableStrokeLineCap ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void linecap( LineCap linecap_ )
      {
        _linecap = linecap_;
      }
    LineCap linecap( void ) const
      {
        return _linecap;
      }

  private:
    LineCap _linecap;
  };

  // Stroke linejoin
  class DrawableStrokeLineJoin : public DrawableBase
  {
  public:
    DrawableStrokeLineJoin ( LineJoin linejoin_ )
      : _linejoin(linejoin_)
      { }

    /*virtual*/ ~DrawableStrokeLineJoin ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void linejoin( LineJoin linejoin_ )
      {
        _linejoin = linejoin_;
      }
    LineJoin linejoin( void ) const
      {
        return _linejoin;
      }

  private:
    LineJoin _linejoin;
  };

  // Stroke miterlimit
  class DrawableMiterLimit : public DrawableBase
  {
  public:
    DrawableMiterLimit ( unsigned int miterlimit_ )
      : _miterlimit(miterlimit_)
      { }

    /*virtual*/ ~DrawableMiterLimit ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void miterlimit( unsigned int miterlimit_ )
      {
        _miterlimit = miterlimit_;
      }
    unsigned int miterlimit( void ) const
      {
        return _miterlimit;
      }

  private:
    unsigned int _miterlimit;
  };


  // Stroke antialias
  class DrawableStrokeAntialias : public DrawableBase
  {
  public:
    DrawableStrokeAntialias ( bool flag_ )
      : _flag(flag_)
      { }

    /*virtual*/ ~DrawableStrokeAntialias ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void flag( bool flag_ )
      {
        _flag = flag_;
      }
    bool flag( void ) const
      {
        return _flag;
      }

  private:
    bool _flag;
  };

  // Stroke color
  class DrawableStrokeColor : public DrawableBase
  {
  public:
    DrawableStrokeColor ( const Color &color_ );

    DrawableStrokeColor ( const DrawableStrokeColor& original_ );

    /*virtual*/ ~DrawableStrokeColor ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void color( const Color& color_ )
      {
        _color = color_;
      }
    Color color( void ) const
      {
        return _color;
      }

  private:
    Color _color;
  };

  // Stroke opacity
  class DrawableStrokeOpacity : public DrawableBase
  {
  public:
    DrawableStrokeOpacity ( double opacity_ )
      : _opacity(opacity_)
      {
      }

    /*virtual*/ ~DrawableStrokeOpacity ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void opacity( double opacity_ )
      {
        _opacity = opacity_;
      }
    double opacity( void ) const
      {
        return _opacity;
      }

  private:
    double _opacity;
  };

  // Stroke width
  class DrawableStrokeWidth : public DrawableBase
  {
  public:
    DrawableStrokeWidth ( double width_ )
      : _width(width_)
      { }

    /*virtual*/ ~DrawableStrokeWidth ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void width( double width_ )
      {
        _width = width_;
      }
    double width( void ) const
      {
        return _width;
      }

  private:
    double _width;
  };

  // Draw text at point
  class DrawableText : public DrawableBase
  {
  public:
    DrawableText ( const double x_, const double y_, const std::string &text_ );

    DrawableText ( const DrawableText& original_ );

    /*virtual*/ ~DrawableText ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

    void text( const std::string &text_ )
      {
        _text = text_;
      }
    std::string text( void ) const
      {
        return _text;
      }

  private:
    double      _x;
    double      _y;
    std::string _text;
  };

  // Text antialias
  class DrawableTextAntialias : public DrawableBase
  {
  public:
    DrawableTextAntialias ( bool flag_ )
      : _flag(flag_) { }

    /*virtual*/ ~DrawableTextAntialias ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void flag( bool flag_ )
      {
        _flag = flag_;
      }
    bool flag( void ) const
      {
        return _flag;
      }

  private:
    bool _flag;
  };

  // Decoration (text decoration)
  class DrawableTextDecoration : public DrawableBase
  {
  public:
    DrawableTextDecoration ( DecorationType decoration_ )
      : _decoration(decoration_)
      { }

    /*virtual*/ ~DrawableTextDecoration( void );

    // Operator to invoke equivalent draw API call
   /*virtual*/  void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void decoration( DecorationType decoration_ )
      {
        _decoration = decoration_;
      }
    DecorationType decoration( void ) const
      {
        return _decoration;
      }

  private:
    DecorationType _decoration;
  };

  // Apply Translation
  class DrawableTranslation : public DrawableBase
  {
  public:
    DrawableTranslation ( double x_, double y_ )
      : _x(x_),
        _y(y_)
      { }

    /*virtual*/ ~DrawableTranslation ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ DrawableBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _x;
    double _y;
  };

  // Set the size of the viewbox
  class DrawableViewbox : public DrawableBase
  {
  public:
    DrawableViewbox(unsigned long x1_, unsigned long y1_,
                    unsigned long x2_, unsigned long y2_)
      : _x1(x1_),
        _y1(y1_),
        _x2(x2_),
        _y2(y2_) { }

    /*virtual*/ ~DrawableViewbox ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/
    DrawableBase* copy() const;

    void x1( unsigned long x1_ )
      {
        _x1 = x1_;
      }
    unsigned long x1( void ) const
      {
        return _x1;
      }

    void y1( unsigned long y1_ )
      {
        _y1 = y1_;
      }
    unsigned long y1( void ) const
      {
        return _y1;
      }

    void x2( unsigned long x2_ )
      {
        _x2 = x2_;
      }
    unsigned long x2( void ) const
      {
        return _x2;
      }

    void y2( unsigned long y2_ )
      {
        _y2 = y2_;
      }
    unsigned long y2( void ) const
      {
        return _y2;
      }

  private:
    unsigned long _x1;
    unsigned long _y1;
    unsigned long _x2;
    unsigned long _y2;
  };

  //
  // Path Element Classes To Support DrawablePath
  //
  class PathArcArgs
  {
  public:
    PathArcArgs( double radiusX_, double radiusY_,
                 double xAxisRotation_, bool largeArcFlag_,
                 bool sweepFlag_, double x_, double y_ )
      : _radiusX(radiusX_),
        _radiusY(radiusY_),
        _xAxisRotation(xAxisRotation_),
        _largeArcFlag(largeArcFlag_),
        _sweepFlag(sweepFlag_),
        _x(x_),
        _y(y_) { }

    ~PathArcArgs ( void )
      { }

    void radiusX( double radiusX_ )
      {
        _radiusX = radiusX_;
      }
    double radiusX( void ) const
      {
        return _radiusX;
      }

    void radiusY( double radiusY_ )
      {
        _radiusY = radiusY_;
      }
    double radiusY( void ) const
      {
        return _radiusY;
      }

    void xAxisRotation( double xAxisRotation_ )
      {
        _xAxisRotation = xAxisRotation_;
      }
    double xAxisRotation( void ) const
      {
        return _xAxisRotation;
      }

    void largeArcFlag( bool largeArcFlag_ )
      {
        _largeArcFlag = largeArcFlag_;
      }
    bool largeArcFlag( void ) const
      {
        return _largeArcFlag;
      }

    void sweepFlag( bool sweepFlag_ )
      {
        _sweepFlag = sweepFlag_;
      }
    bool sweepFlag( void ) const
      {
        return _sweepFlag;
      }

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double	_radiusX;	// X radius
    double	_radiusY;	// Y radius
    double	_xAxisRotation;	// Rotation relative to X axis
    bool        _largeArcFlag;	// Draw longer of the two matching arcs
    bool        _sweepFlag;	// Draw arc matching clock-wise rotation
    double	_x;		// End-point X
    double	_y;		// End-point Y
  };

  // Path Arc (Elliptical Arc)
  class PathArcAbs : public VPathBase
  {
  public:
    // Draw a single arc segment
    PathArcAbs ( const PathArcArgs &coordinates_ );

    // Draw multiple arc segments
    PathArcAbs ( const std::list<Magick::PathArcArgs> &coordinates_ );

    // Copy constructor
    PathArcAbs ( const PathArcAbs& original_ );

    // Destructor
    /*virtual*/ ~PathArcAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathArcArgs> _coordinates;
  };
  class PathArcRel : public VPathBase
  {
  public:
    // Draw a single arc segment
    PathArcRel ( const PathArcArgs &coordinates_ );

    // Draw multiple arc segments
    PathArcRel ( const std::list<Magick::PathArcArgs> &coordinates_ );

    PathArcRel ( const PathArcRel& original_ );

    /*virtual*/ ~PathArcRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathArcArgs> _coordinates;
  };

  // Path Closepath
  class PathClosePath : public VPathBase
  {
  public:
    PathClosePath ( void )
      : _dummy(0)
      {
      }

    /*virtual*/ ~PathClosePath ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    int   _dummy;
  };

  //
  // Curveto (Cubic Bezier)
  //
  class PathCurvetoArgs
  {
  public:
    PathCurvetoArgs( double x1_, double y1_,
                     double x2_, double y2_,
                     double x_, double y_ )
      : _x1(x1_),
        _y1(y1_),
        _x2(x2_),
        _y2(y2_),
        _x(x_),
        _y(y_)
      {
      }

    ~PathCurvetoArgs ( void )
      { }

    void x1( double x1_ )
      {
        _x1 = x1_;
      }
    double x1( void ) const
      {
        return _x1;
      }

    void y1( double y1_ )
      {
        _y1 = y1_;
      }
    double y1( void ) const
      {
        return _y1;
      }

    void x2( double x2_ )
      {
        _x2 = x2_;
      }
    double x2( void ) const
      {
        return _x2;
      }

    void y2( double y2_ )
      {
        _y2 = y2_;
      }
    double y2( void ) const
      {
        return _y2;
      }

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _x1;
    double _y1;
    double _x2;
    double _y2;
    double _x;
    double _y;
  };
  class PathCurvetoAbs : public VPathBase
  {
  public:
    // Draw a single curve
    PathCurvetoAbs ( const PathCurvetoArgs &args_ );

    // Draw multiple curves
    PathCurvetoAbs ( const std::list<Magick::PathCurvetoArgs> &args_ );

    // Copy constructor
    PathCurvetoAbs ( const PathCurvetoAbs& original_ );

    // Destructor
    /*virtual*/ ~PathCurvetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathCurvetoArgs> _args;
  };
  class PathCurvetoRel : public VPathBase
  {
  public:
    // Draw a single curve
    PathCurvetoRel ( const PathCurvetoArgs &args_ );

    // Draw multiple curves
    PathCurvetoRel ( const std::list<Magick::PathCurvetoArgs> &args_ );

    // Copy constructor
    PathCurvetoRel ( const PathCurvetoRel& original_ );

    /*virtual*/ ~PathCurvetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathCurvetoArgs> _args;
  };
  class PathSmoothCurvetoAbs : public VPathBase
  {
  public:
    // Draw a single curve
    PathSmoothCurvetoAbs ( const Magick::Coordinate &coordinates_ );

    // Draw multiple curves
    PathSmoothCurvetoAbs ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathSmoothCurvetoAbs ( const PathSmoothCurvetoAbs& original_ );

    /*virtual*/ ~PathSmoothCurvetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ 
    VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };
  class PathSmoothCurvetoRel : public VPathBase
  {
  public:
    // Draw a single curve
    PathSmoothCurvetoRel ( const Coordinate &coordinates_ );

    // Draw multiple curves
    PathSmoothCurvetoRel ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathSmoothCurvetoRel ( const PathSmoothCurvetoRel& original_ );

    // Destructor
    /*virtual*/ ~PathSmoothCurvetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ 
    VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  //
  // Quadratic Curveto (Quadratic Bezier)
  //
  class PathQuadraticCurvetoArgs
  {
  public:
    PathQuadraticCurvetoArgs( double x1_, double y1_,
                              double x_, double y_ )
      : _x1(x1_),
        _y1(y1_),
        _x(x_),
        _y(y_)
      {
      }

    ~PathQuadraticCurvetoArgs ( void )
      {
      }

    void x1( double x1_ )
      {
        _x1 = x1_;
      }
    double x1( void ) const
      {
        return _x1;
      }

    void y1( double y1_ )
      {
        _y1 = y1_;
      }
    double y1( void ) const
      {
        return _y1;
      }

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _x1;
    double _y1;
    double _x;
    double _y;
  };
  class PathQuadraticCurvetoAbs : public VPathBase
  {
  public:
    // Draw a single curve
    PathQuadraticCurvetoAbs ( const Magick::PathQuadraticCurvetoArgs &args_ );

    // Draw multiple curves
    PathQuadraticCurvetoAbs ( const std::list<Magick::PathQuadraticCurvetoArgs> &args_ );

    // Copy constructor
    PathQuadraticCurvetoAbs ( const PathQuadraticCurvetoAbs& original_ );

    // Destructor
    /*virtual*/ ~PathQuadraticCurvetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathQuadraticCurvetoArgs> _args;
  };
  class PathQuadraticCurvetoRel : public VPathBase
  {
  public:
    // Draw a single curve
    PathQuadraticCurvetoRel ( const Magick::PathQuadraticCurvetoArgs &args_ );

    // Draw multiple curves
    PathQuadraticCurvetoRel ( const std::list<Magick::PathQuadraticCurvetoArgs> &args_ );

    // Copy constructor
    PathQuadraticCurvetoRel ( const PathQuadraticCurvetoRel& original_ );

    // Destructor
    /*virtual*/ ~PathQuadraticCurvetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::PathQuadraticCurvetoArgs> _args;
  };
  class PathSmoothQuadraticCurvetoAbs : public VPathBase
  {
  public:
    // Draw a single curve
    PathSmoothQuadraticCurvetoAbs ( const Magick::Coordinate &coordinate_ );

    // Draw multiple curves
    PathSmoothQuadraticCurvetoAbs ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathSmoothQuadraticCurvetoAbs ( const PathSmoothQuadraticCurvetoAbs& original_ );

    // Destructor
    /*virtual*/ ~PathSmoothQuadraticCurvetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };
  class PathSmoothQuadraticCurvetoRel : public VPathBase
  {
  public:
    // Draw a single curve
    PathSmoothQuadraticCurvetoRel ( const Magick::Coordinate &coordinate_ );

    // Draw multiple curves
    PathSmoothQuadraticCurvetoRel ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathSmoothQuadraticCurvetoRel ( const PathSmoothQuadraticCurvetoRel& original_ );

    // Destructor
    /*virtual*/ ~PathSmoothQuadraticCurvetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  //
  // Path Lineto
  //
  class PathLinetoAbs : public VPathBase
  {
  public:
    // Draw to a single point
    PathLinetoAbs ( const Magick::Coordinate& coordinate_  );

    // Draw to multiple points
    PathLinetoAbs ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathLinetoAbs ( const PathLinetoAbs& original_ );

    // Destructor
    /*virtual*/ ~PathLinetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };
  class PathLinetoRel : public VPathBase
  {
  public:
    // Draw to a single point
    PathLinetoRel ( const Magick::Coordinate& coordinate_ );

    // Draw to multiple points
    PathLinetoRel ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathLinetoRel ( const PathLinetoRel& original_ );

    // Destructor
    /*virtual*/ ~PathLinetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

  // Path Horizontal Lineto
  class PathLinetoHorizontalAbs : public VPathBase
  {
  public:
    PathLinetoHorizontalAbs ( double x_ )
      : _x(x_)
      {
      }

    /*virtual*/ ~PathLinetoHorizontalAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

  private:
    double _x;
  };
  class PathLinetoHorizontalRel : public VPathBase
  {
  public:
    PathLinetoHorizontalRel ( double x_ )
      : _x(x_)
      {
      }

    /*virtual*/ ~PathLinetoHorizontalRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

    void x( double x_ )
      {
        _x = x_;
      }
    double x( void ) const
      {
        return _x;
      }

  private:
    double _x;
  };

  // Path Vertical Lineto
  class PathLinetoVerticalAbs : public VPathBase
  {
  public:
    PathLinetoVerticalAbs ( double y_ )
      : _y(y_)
      {
      }

    /*virtual*/ ~PathLinetoVerticalAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _y;
  };
  class PathLinetoVerticalRel : public VPathBase
  {
  public:
    PathLinetoVerticalRel ( double y_ )
      : _y(y_)
      {
      }

    /*virtual*/ ~PathLinetoVerticalRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

    void y( double y_ )
      {
        _y = y_;
      }
    double y( void ) const
      {
        return _y;
      }

  private:
    double _y;
  };

  // Path Moveto
  class PathMovetoAbs : public VPathBase
  {
  public:
    // Simple moveto
    PathMovetoAbs ( const Magick::Coordinate &coordinate_ );

    // Moveto followed by implicit linetos
    PathMovetoAbs ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathMovetoAbs ( const PathMovetoAbs& original_ );

    // Destructor
    /*virtual*/ ~PathMovetoAbs ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };
  class PathMovetoRel : public VPathBase
  {
  public:
    // Simple moveto
    PathMovetoRel ( const Magick::Coordinate &coordinate_ );

    // Moveto followed by implicit linetos
    PathMovetoRel ( const std::list<Magick::Coordinate> &coordinates_ );

    // Copy constructor
    PathMovetoRel ( const PathMovetoRel& original_ );

    // Destructor
    /*virtual*/ ~PathMovetoRel ( void );

    // Operator to invoke equivalent draw API call
    /*virtual*/ void operator()( MagickLib::DrawContext context_ ) const;

    // Return polymorphic copy of object
    /*virtual*/ VPathBase* copy() const;

  private:
    std::list<Magick::Coordinate> _coordinates;
  };

} // namespace Magick

#endif // Magick_Drawable_header
