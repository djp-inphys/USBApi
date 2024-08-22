/** \file VtImage.h

 The image abstraction file.

  \par REQUIREMENTS: 
  Define the 2d image iterators / containers for each image use.

  \par SPECIFICATIONS: 
  Contiains, mutates and access internally stored 2d coordinates which
  can be used for a veriety of image processing operations

  \par DESIGN NOTES: 
  A baic class which provides a great number of functions to 
  access and mutate its internal data items

 
 Copyright (c) 2013 by
 All Rights Reserved

 REVISIONS: 
 $Log: VtImage.h,v $
 Revision 1.2  2013/05/27 12:04:18  david
 Ver 1

 */

#ifndef __CVtImage_H__
#define __CVtImage_H__



//*********************************************************************
// INCLUDES
//*********************************************************************
#include <cmath>
#include <memory>
#include <float.h>
#include "VtSysdefs.h"


namespace Vt {
  

class VTAPI_API Diff2D
{
public:
  
  /**
   * The iterator's value type
   */
  typedef Diff2D value_type;

  /**
   * The iterator's PixelType
   */
  typedef Diff2D PixelType;
  
  /** 
   * Default Constructor. Init iterator at position (0,0)
   */
  Diff2D() : x(0), y(0) {}
  
  /** 
   * Construct at a given position
   */
  explicit Diff2D(vt_int ax, vt_int ay) : x(ax), y(ay) {}
  
  /** 
   * Copy Constructor
   */
  Diff2D(Diff2D const & v) : x(v.x), y(v.y) {}
  
  /** 
   * overloaded assignment operator
   */
  const Diff2D& operator=(Diff2D const & v);
  
  /** 
   * Unary negation.
   */
  Diff2D operator-() const { return Diff2D(-x, -y); }
  
  /** 
   * Increase coordinate by specified offset.
   */
  const Diff2D & operator+=(Diff2D const & offset);
  
  /** 
   * Decrease coordinate by specified vector.
   */
  const Diff2D & operator-=(Diff2D const & offset);
  
  /**
   * Create vector by adding specified offset.
   */
  Diff2D operator+(Diff2D const & offset) const { return Diff2D(x + offset.x, y + offset.y); }
  
  /** 
   * Create vector by subtracting specified offset.
   */
  Diff2D operator-(Diff2D const & offset) const { return Diff2D(x - offset.x, y - offset.y); }
  
  /** 
   * Calculate length of difference vector.
   */
  double magnitude() const { return sqrt((double)(x*x + y*y)); }
  
  /** 
   * Equality operator
   */
  vt_bool operator==(Diff2D const & r) const { return (x == r.x) && (y == r.y); }
  
  /** 
   * Inequality operator
   */
  vt_bool operator!=(Diff2D const & r) const { return (x != r.x) || (y != r.y); }
  
  /** 
   * Read current coordinate
   */
  Diff2D operator*() const { return *this; }
  
  /** 
   * Read coordinate at an offset
   */
  Diff2D operator()(vt_int const & dx, vt_int const & dy) const { return Diff2D(x + dx, y + dy); }
  
  /** 
   * Read coordinate at an offset
   */
  Diff2D operator[](Diff2D const & offset) const { return Diff2D(x + offset.x, y + offset.y); }

  /** 
   * Returns the X ordinate safely
   */
  vt_int GetX() const { return x; }

  /** 
   * Returns the Y ordinate safely
   */
  vt_int GetY() const { return y; }


private:

  // Internal x and y parameters
  vt_int x;
  vt_int y;
};



/**
 *  $Author: david $
 *  $Revision: 1.2 $
 *  $Date: 2013/05/27 12:04:18 $
 *
 *  \par REQUIREMENTS: 
 *  Abstract base class for all image related classes
 *
 *  \par SPECIFICATIONS: 
 *  Provides functional access to image properties
 *  Such as image: width, height, size, and origin and size of region of interest
 *
 *  \par DESIGN NOTES: 
 *  Provided to support generic image passing via RTTI
 *
 *  @see <A HREF="../concepts.htm#datasets"> Concepts - Datasets </A>
 *  @see <BR> <A HREF="../concepts.htm#datasetid"> Concepts - DatasetID, KeyImageID and ImageID</A>
 *
 */
class VTAPI_API CVtImageBaseClass
{
protected:
  
  // The origin of the region of interest
  Diff2D m_roiorigin;
  
  // The size of the region of interest
  Diff2D m_roisize;
  
  // The width (columns or x) of the image
  vt_uint m_width;
  
  // The height (rows or y) of the image
  vt_uint m_height;
  
  // Protected constructor to ensure class is not instantiated
  explicit CVtImageBaseClass(vt_uint width, vt_uint height) :
     m_roiorigin(0, 0),
     m_roisize(0, 0),
     m_width(width),
     m_height(height) {}
     
public:
  
  /**
   * Destructor remains public as its virtual
   */
  virtual ~CVtImageBaseClass() {}
  
  /**
   * Returns the width of the image
   *
   * @see <BR> <A HREF="../concepts.htm#imagewidth"> Concepts - Image Width </A>
   */
  vt_uint width() const { return m_width; }
  
  /**
   * Returns the height of the image
   *
   * @see <BR> <A HREF="../concepts.htm#imageheight"> Concepts - Image Height </A>
   */
  vt_uint height() const { return m_height; }
  
  /**
   * Returns the size of the image
   */
  Diff2D size() const { return Diff2D(m_width, m_height); }  
  
  /**
   * Returns the origin of the region of interest
   */
  const Diff2D& GetROIOrigin() const { return m_roiorigin; }
  
  /**
   * Returns the size of the region of interest
   */
  const Diff2D& GetROISize() const { return m_roisize; }
  
  /**
   * Sets the region of interest
   */
  void SetROI(const Diff2D& ROIOrigin, const Diff2D& ROISize);
  
};



/**
 *  $Author: david $
 *  $Revision: 1.2 $
 *  $Date: 2013/05/27 12:04:18 $
 *
 *  \par REQUIREMENTS: 
 *  Templated image class providing access and mutators for underlying data
 *
 *  \par SPECIFICATIONS: 
 *  Provide the following functions to access / manipulate the data
 *  1. Construct (and assign) an image in a variety of different ways
 *  2. Provide const and non-const access to underlying data
 *
 *  \par DESIGN NOTES: 
 *  Templated class
 *
 */
template <class PIXELTYPE> 
class VTAPI_API CVtImage : public CVtImageBaseClass
{
public:
    
  /** 
   * The Image's pixel type
   */
  typedef PIXELTYPE PixelType;
  
  /** 
   * The Image's value type
   */
  typedef PIXELTYPE value_type;
  
  /** 
   * The Image's 1D random access iterator
   */
  typedef PIXELTYPE * ScanOrderIterator;
  
  /** 
   * The Image's 1D random access const iterator
   */
  typedef PIXELTYPE const * ConstScanOrderIterator;
  
  /**
   * Pixel Type Allocator
   */
  struct Allocator
  {
    // Allocate
    static PixelType * allocate(vt_uint n) { 
      return (PixelType *)::operator new(n*sizeof(PixelType)); }
    
    // Deallocate
    static void deallocate(PixelType * p) {
      ::operator delete(p); }
  };
    
  /**
   * Default constructor (image size 0x0)
   */
  CVtImage()
    : CVtImageBaseClass(0, 0),
    m_data(0) {}
  
  /** 
   * Construct image of size width x height - allocates data
   */
  CVtImage(vt_uint width, vt_uint height)
    : CVtImageBaseClass(width, height),
    m_data(0)
  {
    resize(width, height, PixelType());
  }
  
  /**
   * Constructs an image of width x height, copies specified data - allocates data
   */
  CVtImage(vt_uint width, vt_uint height, PixelType *data)
    : CVtImageBaseClass(width, height),
    m_data(0)
  {
    resizeCopy(width, height, data);
  }
  
  /** 
   * Construct image of size Diff2D width x height - allocates data
   */
  CVtImage(Diff2D size)
    : CVtImageBaseClass(size.x, size.y),
    m_data(0)
  {
    resize(size.x, size.y, PixelType());
  }
  
  /** 
   * construct image of size width*height and initialize every
   * pixel with given data (use this constructor, if 
   * PixelType doesn't have a default constructor)
   */
  CVtImage(vt_uint width, vt_uint height, PixelType d)
    : CVtImageBaseClass(width, height),
    m_data(0)
  {
    resize(width, height, d);
  }
  
  /**
   * Copy constructor
   */
  CVtImage(const CVtImage & rhs)
    : CVtImageBaseClass(0, 0),
    m_data(0)
  {
    resizeCopy(rhs);
  }
  
  /**
   * Destructor
   */
  virtual ~CVtImage() 
  { 
    deallocate(); 
  }
  
  /**
   * Assignment operator
   */
  const CVtImage & operator=(const CVtImage & rhs)
  {
    if(this != &rhs)
    {
      if((width() != rhs.width()) || 
        (height() != rhs.height()))
      {
        resizeCopy(rhs);
      }
      else
      {
        ConstScanOrderIterator is = rhs.begin();
        ConstScanOrderIterator iend = rhs.end();
        ScanOrderIterator id = begin();
        
        for(; is != iend; ++is, ++id) *id = *is;
      }
    }
    return *this;
  }
  
  /** 
   * Set Image with const value 
   */
  const CVtImage & operator=(PixelType pixel)
  {
    ScanOrderIterator i = begin();
    ScanOrderIterator iend = end();
    
    for(; i != iend; ++i) *i = pixel;
    
    return *this;
  }
  
  /** 
   * Reset image to specified size (dimensions must not be negative)
   * (old data is destroyed) 
   */
  void resize(vt_uint width, vt_uint height)
  {
    resize(width, height, PixelType());
  }
  
  /** 
   * Reset image to specified size (dimensions must not be negative)
   * (old data is destroyed) 
   */
  void resize(Diff2D size)
  {
    resize(size.x, size.y, PixelType());
  }
  
  /** 
   * Reset image to specified size and initialize it with
   * given data (use this if PixelType doesn't have a default
   * constructor, dimensions must not be negative, old data are destroyed) 
   */
  void resize(vt_uint width, vt_uint height, PixelType d)
  {
    PixelType * newdata = 0;
    PixelType ** newlines = 0;
    if(width*height > 0)
    {
      newdata = Allocator::allocate(width*height);
      
      std::uninitialized_fill_n(newdata, width*height, d);
      
      newlines = initLineStartArray(newdata, width, height);
    }
    
    deallocate();
    m_data = newdata;
    m_lines = newlines;
    m_width = width;
    m_height = height;
  }
  
  /**
   * Resize image to size of other image and copy it's data 
   */
  void resizeCopy(const vt_uint width, const vt_uint height, PixelType *newdata)
  {
    PixelType ** newlines = 0;
    if(width*height > 0)
    {
      newlines = initLineStartArray(newdata, width, height);
    }
    
    deallocate();
    
    m_data   = newdata;
    m_lines  = newlines;
    m_width  = width;
    m_height = height; 
  }
  
  /** 
   * Resize image to size of other image and copy it's data 
   */
  void resizeCopy(const CVtImage & rhs)
  {
    PixelType * newdata = 0;  
    PixelType ** newlines = 0;
    if(rhs.width()*rhs.height() > 0)
    {
      newdata = Allocator::allocate(rhs.width()*rhs.height());
      
      std::uninitialized_copy(rhs.begin(), rhs.end(), newdata);
      
      newlines = initLineStartArray(newdata, rhs.width(), rhs.height());
    }
    
    deallocate();
    m_data = newdata;
    m_lines = newlines;
    m_width = rhs.width();
    m_height = rhs.height(); 
  }
  
  /**
   * Returns a const data pointer to the start of the image
   */
  PixelType **lines() const { return m_lines; }
  
  /**
   * Returns a non-const data pointer to the start of the image
   */
  PixelType **lines() { return m_lines; }
  
  /** 
   * Test whether a given coordinate is inside the image
   */
  inline vt_bool isInside(Diff2D const & d) const
  {
    return d.x >= 0 && d.y >= 0 && d.x < width() && d.y < height();
  }
  
  /** 
   * non-const access pixel at given location. 
   * usage:  PixelType value = image[Diff2D(1,2)] 
   */
  inline PixelType & operator[](Diff2D const & d) { return m_lines[d.y][d.x]; }
  
  /** 
   * const access pixel at given location. 
   * usage: PixelType value = image[Diff2D(1,2)] 
   */
  inline PixelType const & operator[](Diff2D const & d) const { return m_lines[d.y][d.x]; }
  
  /** 
   * non-const access pixel at given location. 
   * usage: PixelType value = image(1,2) 
   */
  inline PixelType & operator()(vt_uint const & dx, vt_uint const & dy) { return m_lines[dy][dx]; }
  
  /** 
   * const access pixel at given location. 
   * usage:  PixelType value = image(1,2) 
   */
  inline PixelType const & operator()(vt_uint const & dx, vt_uint const & dy) const { return m_lines[dy][dx]; }
  
  /** 
   * non-const access pixel at given location. 
   * Note that the 'x' index is the trailing index. 
   * usage:  PixelType value = image[2][1] 
   */
  inline PixelType * operator[](vt_uint const & dy) { return m_lines[dy]; }
  
  /** 
   * const access pixel at given location. 
   * Note that the 'x' index is the trailing index. 
   * usage:  PixelType value = image[2][1] 
   */
  inline PixelType const * operator[](vt_uint const & dy) const { return m_lines[dy]; }

  /** 
   * init 1D random access iterator pointing to first pixel
   */
  ScanOrderIterator begin() { return m_data; }
  
  /** 
   * init 1D random access iterator pointing past the end
   */
  ScanOrderIterator end() { return m_data + width() * height(); }
  
  /** 
   * init 1D random access const iterator pointing to first pixel
   */
  ConstScanOrderIterator begin() const { return m_data; }
  
  /** 
   * init 1D random access const iterator pointing past the end
   */
  ConstScanOrderIterator end() const { return m_data + width() * height(); }
  

private:
  
  // Deallocate helper method
  void deallocate()
  {
    if(m_data) 
    {
      ScanOrderIterator i = begin();
      ScanOrderIterator iend = end();
      
      for(; i != iend; ++i) 
      {
        (*i).~PIXELTYPE();
      }
      
      Allocator::deallocate(m_data);
      delete[] m_lines;
    }
  }
  
  // initLineStartArray
  static PixelType ** initLineStartArray(PixelType * data, vt_uint width, vt_uint height)
  {
    PixelType ** lines = new PIXELTYPE*[height];
    for(vt_uint y=0; y<height; ++y) 
    {
      lines[y] = data + y*width;
    }
    return lines;
  }
  
  // Data pointers
  PIXELTYPE * m_data;
  PIXELTYPE ** m_lines;
};

} // end iX Namespace

#endif // __CVtImage_H__
