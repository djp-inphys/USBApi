/** \file VtABDiff.h

	\brief The Header file for the Vt::CVtABDiff class, used by the ceph calibration system.

	The current verson of the pano and ceph system calculate the difference between adjacent tiles.
	This file contains the class which calculates the offset difference between tiles A&B. Hence,
	this class is only useful in the cephelometric system.


 * Copyright (c) 2013 by
 * All Rights Reserved
 * REVISIONS: 
 * $Log: $
 *
 */

#ifndef __CVTABDIFF_H__
#define __CVTABDIFF_H__

#include <float.h>

namespace Vt {

/**

\fn template<typename ImageType> 
vt_ulong roi_mu_std(vt_double *xbar, vt_double *var
									 , const CVtImage<ImageType> &in
									 , const Diff2D origin
									 , const Diff2D size )

\brief This function calculates the mean and standard deviation for a region of interest.


\param xbar			a pointer to the a variable which will contain the mean value on function completion.
\param var			a pointer to the a variable which will contain the variance value on function completion.
\param origin		the top right hand coordinate value or the roi. The x value contains the column, the 
y value contains the row.
\param size			the width and height of the roi. The x value contains the width the y value contains the height.
							
\sa Vt::Diff2D
*/
template<typename ImageType> 
vt_ulong roi_mu_std(vt_double *xbar, vt_double *var
									 , const CVtImage<ImageType> &in
									 , const Diff2D origin
									 , const Diff2D size )
{
	vt_longdouble sum		=0.0;
	vt_longdouble sumsq	=0.0;
	
	vt_int r_start = origin.GetY();
	vt_int r_end   = origin.GetY() + size.GetY();

	vt_int c_start = origin.GetX();
	vt_int c_end   = origin.GetX() + size.GetX();

	vt_ulong cnt = 0;
	for (vt_int row = r_start; row< r_end; row++)
	{
		const ImageType *rowptr = in[row];
		for (vt_int col = c_start; col< c_end; col++)
		{
			register ImageType val = rowptr[col];
			sum		+= val;
			sumsq += val*val;
			cnt++;
		}
	}

	if (cnt == 0)
		return 0;

	vt_double ave	= sum/cnt;

	*xbar = ave;
	*var  = sumsq/cnt - ave*ave;

	return cnt;
}

/**

\brief Calculates the pooled variance.



\param var1  the variance of the first distribution.
\param n1    the number of samples in the first distribution.
\param var2  the variance of the second distribution.
\param n2    the number of samples in the second distribution.
							
\sa VT::Diff2D

*/

vt_double pooled_var( const vt_double var1, const vt_ulong n1
										, const vt_double var2, const vt_ulong n2)
{
	if (n1 == 0  || n2 == 0)
		return -1;

	vt_double mult = (n1 + n2)/(n1*n2);
	vt_double pv   = ((n1-1)*var1 + (n2-1)*var2)/(n1 + n2 - 2);

	return mult*pv;
}

/**
* Compare stats
*/

typedef struct {
	vt_double pooled_var;
	vt_double xbar1;
	vt_double xbar2;
} VAR_STATS;

bool compare_objects(const VAR_STATS &a, const VAR_STATS &b)
{
	return a.pooled_var < b.pooled_var;
}

/**
*  rectangle
*/
class CVtRect
{
public:
	/**
  * The origin of the region of interest
	*/
  Diff2D m_origin;
 
	/**
  * The size of the region of interest
  */
	
	Diff2D m_size;

  CVtRect( const Diff2D rectorigin, const Diff2D rectsize ) 
	{
		m_origin = rectorigin;
		m_size   = rectsize;
	}

  /**
   * Destructor 
   */
  virtual ~CVtRect() {}
};

/**
*  rectangle pair pos
*/
class CVtRectPairs
{
public:
	enum {

			RECT_SIZE			 = 32
			, OFFSET			 = 3
			, RECT_SPACING = 32
			, NUM_RECTS		 = 40
	};
	typedef std::pair<CVtRect, CVtRect> REC_PAIR;
	typedef std::vector<REC_PAIR>::iterator iterator;
	typedef std::vector<REC_PAIR>::const_iterator const_iterator;

private:
	std::vector<REC_PAIR> m_recs;

	void push(const vt_ulong top_tl_x, const vt_ulong top_tl_y, const vt_ulong top_xsize, const vt_ulong top_ysize
					  , const vt_ulong bott_tl_x, const vt_ulong bott_tl_y, const vt_ulong bott_xsize, const vt_ulong bott_ysize
						)
	{
		CVtRect top( Diff2D( top_tl_x, top_tl_y ), Diff2D( top_xsize, top_ysize ) );
		CVtRect bott( Diff2D( bott_tl_x, bott_tl_y ), Diff2D( bott_xsize, bott_ysize ) );

		m_recs.push_back( REC_PAIR( top, bott ) );
	}

public:
	CVtRectPairs(	const vt_ulong ab_split )
	{
		/**
		*  trec = top rectangle
		*/
		const trec_top_row = ab_split - (RECT_SIZE + OFFSET);
		const brec_top_row = ab_split + OFFSET;

		vt_ulong tl_col = RECT_SPACING;
		for (vt_ulong rectno = 0; rectno < NUM_RECTS; rectno++, tl_col += RECT_SPACING )
		{
			push( tl_col, trec_top_row, RECT_SIZE, RECT_SIZE
					, tl_col, brec_top_row, RECT_SIZE, RECT_SIZE );
		}
	}

	virtual ~CVtRectPairs() {}

	iterator begin()
	{
		return m_recs.begin();
	}
	iterator end()
	{
		return m_recs.end();
	}
};

/**
	\brief A utility class used by the ceph calibration system to calculate the offset between tile a and tile b.

	The current verson of the pano and ceph system calculate the difference between adjacent tiles.
	This file contains the class which calculates the offset difference between tiles A&B. Hence,
	this class is only useful in the cephelometric system.
 */

template<typename ImageType> 
class CVtABDiff
{
private:
	enum {
		NUM_RECTS = 4			//!< number of rectangles to calculate the means
	};
	typedef 	ImageType IM_TYPE;


	CVtRectPairs m_rects;

	vt_double m_xbar1;
	vt_double m_var1;
	vt_double m_xbar2;
	vt_double m_var2;

	/**
	Student t-test value between two ROI in an image

	Not used in the current implementation, was used in an initial test verson. However, decided to leave this here 
	in case it comes in useful in future.
	*/
	vt_double ttest(const CVtImage<ImageType> &in // ínput image
									, const CVtRect &top_rect			// top rectangle
									, const CVtRect &bot_rect)		// bottom rectangle
	{
		/**
		*  mu and std for top roi
		*/
		in.SetROI( top_rect.m_origin, top_rect.m_size );
		
		vt_double m_xbar1;
		vt_double m_var1;
		//												out   out   in        
		vt_long n1 = roi_mu_std( &m_xbar1, &m_var1, in);

		/**
		*  mu and std for top roi
		*/
		in.SetROI( bot_rect.m_origin, bot_rect.m_size );
		
		vt_double m_xbar2;
		vt_double m_var2;
		//												out   out   in        
		vt_long n2 = roi_mu_std( &m_xbar2, &m_var2, in);

		/**
		*  calculate t-value
		*/
		vt_double p_var = pooled_m_var( m_var1, cnt1, m_var2, cnt2 );

		if (p_var > DBL_EPSILON)
		{
			return fabs( m_xbar1 - m_xbar2 )/sqrt( p_var );
		}
		else
		{
			return DBL_MAX; // the smallest t-values are selected - hence
											// a v-big value discounts this figure from the running
		}
	}

	/**
	Pooled variance value for two ROI's in an image

	The pooled variance the av
	*/
	vt_double roi_pooled_var(const CVtImage<ImageType> &in // ínput image
													, const CVtRect &top_rect			// top rectangle
													, const CVtRect &bot_rect)		// bottom rectangle
	{
		/**
		*  mu and std for top roi
		*/
		vt_double m_xbar1;
		vt_double m_var1;
		//												out   out   in        
		vt_long n1 = roi_mu_std( &m_xbar1, &m_var1, in, top_rect.m_origin, top_rect.m_size );

		/**
		*  mu and std for top roi
		*/
		vt_double m_xbar2;
		vt_double m_var2;
		//												out   out   in        
		vt_long n2 = roi_mu_std( &m_xbar2, &m_var2, in, top_rect.m_origin, top_rect.m_size );

		/**
		*  calculate t-value
		*/
		vt_double p_var = pooled_var( m_var1, n1, m_var2, n2 );

		return p_var;
	}

public:
  /**
   * Constructor
   */
  CVtABDiff( const vt_ulong ab_split ) : m_rects( ab_split )
																			, m_xbar1( 0.0 )
																			, m_var1( 0.0 )
																			, m_xbar2( 0.0 )
																			, m_var2( 0.0 )
	{}

	virtual ~CVtABDiff() {}

	/*!
	\brief Do coefficients calculation and calibration
	
	 Try this version based on mean signal level.
	 could try an alternative based on absolute differences.
	*/
	vt_double  operator () ( const CVtImage<ImageType> &in )
	{
		std::vector<VAR_STATS> stats;

		for(CVtRectPairs::iterator it = m_rects.begin(); it != m_rects.end(); it++)
		{
			VAR_STATS stat;

			// save tvalues
			stat.pooled_var = roi_pooled_var( in, (*it).first, (*it).second );
			stat.xbar1      = m_xbar1;
			stat.xbar2      = m_xbar2;
			
			stats.push_back( stat );
		}

		/**
		*  sort stats
		*/
		std::sort( stats.begin(), stats.end(), compare_objects  );

		/**
		*  add together the bottom four mean values
		*/
		vt_ulong	cnt = 0;
		vt_double sum1 = 0.0;
		vt_double sum2 = 0.0;
		for ( std::vector<VAR_STATS>::iterator vit = stats.begin(); cnt < NUM_RECTS; cnt++, vit++ )
		{
			sum1 += (*vit).xbar1;
			sum2 += (*vit).xbar2;
		}
		return (sum1 - sum2)/cnt; // final difference
	}
};

} // Vt namespace
#endif // __CVTABDIFF_H__
