// *********************************************************************
#ifndef SVGOBJECTS_HPP
#define SVGOBJECTS_HPP

#include <cassert>
#include <cmath>
#include <vector>
#include <string> // std::string, std::stoi
#include <iostream>
#include <fstream> // std::fstream
#include <sstream> // std::stringstream
#include <memory> //std::shared_ptr

#include <Eigen/Dense>



// *****************************************************************************
// SVGContext:
class SVGContext
{
  public:
  std::ostream * os ;
  SVGContext() ;
} ;

// *****************************************************************************
// clase para styles de polígonos y quizás otros tipos de objetos

class PathStyle
{
  public:
  PathStyle();
  void writeSVG( SVGContext & ctx ); // write style attrs to an svg file

  Eigen::Matrix<double,3,1>  lines_color,      // lines color, when lines are drawn (draw_lines == true )
        fill_color ;      // fill color, when fill is drawn (draw_filled == true)
  bool  draw_lines,       // draw lines joining points (yes/no)
        close_lines,      // when draw_lines == true, join last point with first (yes/no)
        draw_filled ;     // fill the polygon (yes(no)
  float  lines_width ;
  float  fill_opacity ;    // fill opacity when draw_filled == true (0->transparent; 1->opaque)
  bool  dashed_lines  ;   // when draw_lines == true, draw dashed lines (yes/no)

  bool use_grad_fill ; // use gradient fill (when draw_filled == true)
  std::string grad_fill_name ; // name to use for gradient fill


} ;

// *****************************************************************************
// An abstract class for things which can be drawn to an SVG file and can be
// projected to 2d (must be projected before drawn)

class MSVG_Object
{
  public:
  MSVG_Object();
  virtual void drawSVG( SVGContext & ctx ) = 0 ;
  virtual void minmax( ) = 0 ;
  virtual ~MSVG_Object() ;
  Eigen::Matrix<double,2,1> min,max ;
} ;

// *****************************************************************************
// class Point
// A class for an isolated point, drawn with a given radius

class MSVG_Point : public MSVG_Object
{
  public:
  MSVG_Point( Eigen::Matrix<double,2,1> ppos2D, Eigen::Matrix<double,3,1> color );
  virtual void drawSVG( SVGContext & ctx ) override;
  virtual void minmax() override;

  Eigen::Matrix<double,3,1> color ;
  Eigen::Matrix<double,2,1> pos2D ;
  float radius ; // 0.03 por defecto, se puede cambiar
};

// *****************************************************************************
// class Polygon
// Any Object which is described by a sequence of points
// it can be a polyline, a polygon, a filled polygon.

class MSVG_Polygon : public MSVG_Object // secuencia de points
{
   public:
   MSVG_Polygon();
   virtual void minmax() override;
   virtual void drawSVG( SVGContext & ctx ) override;
   virtual ~MSVG_Polygon() override;

   PathStyle         style ;
   std::vector<Eigen::Matrix<double,2,1>> points2D ; // projected points
};
// *****************************************************************************
// class ObjectsSet
// A set of various objects

class MSVG_ObjectsSet : public MSVG_Object
{
   public:
   MSVG_ObjectsSet();
   virtual void minmax() override;
   virtual void drawSVG( SVGContext & ctx ) override;
   virtual ~MSVG_ObjectsSet() override;
   void add( const std::shared_ptr<MSVG_Object>& pobj );  // add one object

   std::vector<std::shared_ptr<MSVG_Object>> objetos ;
} ;


// *****************************************************************************
// class Segment
// A polygon with just two points.

class MSVG_Segment : public MSVG_Polygon
{
   public:
   MSVG_Segment( const Eigen::Matrix<double,2,1> & p0, const Eigen::Matrix<double,2,1> & vd, const Eigen::Matrix<double,3,1> & color, float width ) ;
   MSVG_Segment( const Eigen::Matrix<double,2,1> & p0, const Eigen::Matrix<double,2,1> & p1  );
   MSVG_Segment( const MSVG_Point & p0, const MSVG_Point & p1, float width );

};
// *****************************************************************************
// class Ellipse
// A planar ellipse in 3D

class MSVG_Ellipse : public MSVG_Polygon
{
   public:
   MSVG_Ellipse( unsigned n, const Eigen::Matrix<double,2,1> & center, const Eigen::Matrix<double,2,1> & eje1, const Eigen::Matrix<double,2,1> eje2  );
};
// *****************************************************************************
// class Figure
// A container for a set of objects which can be drawn to a SVG file.
// It can be written to a SVG file, the output includes the whole SVG elements
// (headers, footers, bounding box, etc....)
/*
class Figure
{
   public:

   Figure( ) ;
   void drawSVG( const std::string & nombre_arch ) ;

   ObjectsSet objetos ;  // set of objects in the figure
   float       width_cm ; // width (in centimeters) in the SVG header
   bool       flip_axes ; // true to flip axes (see Axes::Axes), false by default

   std::vector< std::string > rad_fill_grad_names ; // name of radial fill gradients to output
} ;
*/
#endif
