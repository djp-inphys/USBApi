/** \file VtImage.cpp

 * Copyright (c) 2013 by
 * All Rights Reserved
 *
 *
 */



//*********************************************************************
// INCLUDES
//*********************************************************************
#include "VtImage.h"
#include "VtErrors.h"


using namespace Vt;


//*********************************************************************
// CVtImageBaseClass::SetROI
//*********************************************************************
void CVtImageBaseClass::SetROI(const Diff2D& ROIOrigin, const Diff2D& ROISize) 
{ 
	Vt_precondition(ROIOrigin.GetX() >= 0 && ROIOrigin.GetY() >= 0, 
		"CVtImageBaseClass::SetROI - Region of interest origin must be greater than or equal to zero");
	
	Vt_precondition(ROISize.GetX() >= 0 && ROISize.GetY() >= 0, 
		"CVtImageBaseClass::SetROI - Region of interest size must be greater than or equal to zero");
	
	Vt_precondition(ROIOrigin.GetX() < m_width && ROIOrigin.GetY() < m_height, 
		"CVtImageBaseClass::SetROI - Region of interest origin must not be greater than the image size");
	
	Vt_precondition(ROISize.GetX() < m_width && ROISize.GetY() < m_height, 
		"CVtImageBaseClass::SetROI - Region of interest size must not be greater than the image size");
	
	m_roiorigin = ROIOrigin;
	m_roisize = ROISize;
}


//*********************************************************************
// Diff2D::Assignment operator
//*********************************************************************
const Diff2D & Diff2D::operator=(Diff2D const & v)
{
  if(this != &v)
  {
    x = v.x;
    y = v.y;
  }
  return *this;
}


//*********************************************************************
// operator+=
//*********************************************************************
const Diff2D & Diff2D::operator+=(Diff2D const & offset)
{
  x += offset.x;
  y += offset.y;
  return *this;
}
  

//*********************************************************************
// operator-=
//*********************************************************************
const Diff2D & Diff2D::operator-=(Diff2D const & offset)
{
  x -= offset.x;
  y -= offset.y;
  return *this;
}
