/** \file VTPanoramicCalibration.h

	This file holds all the routines require for pano and ceph calibration.

	Currently the pano and ceph systems use a flat field removal calibration approach based on 
	gain and offset calculation. 

	The calibration class contains the routines for a calibration run, and for calibration application.

	@see Vt::CVtPanoramicCalibration for more details.

 * Copyright (c) 2013 by
 * All Rights Reserved
 * REVISIONS: 
 * $Log: VtPanoramicCalibration.h,v $
 * Revision 1.3  2013/05/27 17:41:57  david
 * V3
 *
 * Revision 1.2  2013/05/27 12:04:18  david
 * Ver 1
 *
 */

#ifndef __CVTPANORAMICCALIBRATION_H__
#define __CVTPANORAMICCALIBRATION_H__



//*********************************************************************
// INCLUDES
//*********************************************************************

namespace Vt {

/**
 \brief Calculate the mean of an image

 \param data a two dimensional data set which contains the image which we wish to obtain the mean values from
*/
template<typename T1> 
vt_double mean(const T1** data, const vt_int width, const vt_int height)
{
  vt_double			avg;
	vt_longdouble sum=0;
	
	for (vt_int row=0; row<height; row++)
	{
		T1 *rowptr = data[row];
		for (vt_int col=0; col<width; col++)
		{
			sum += rowptr[col];
		}
	}
	return avg = (vt_double)sum/((vt_double)width*height );
}

/**
   Calculate the means of the rows on an image.

	\param row_means The row mean values which are returned.
	\param data			 The 2-dimensional input data.
	\par 
	The input data is assumed to be a vector of pointers to vectors of type T2. Consequently
	individual elements of the data can be acccesed via 

		- data[row][col]
	
	\param width		The number of columns in the two d input data
	\param height		The number of rows in the two d intput data
 */

template<typename T1, typename T2> 
vt_double row_mean(T1 *row_means, const T2 **data, const vt_int width, const vt_int height)
{
	vt_longdouble tot_sum=0;
	
	for (vt_int row=0; row<height; row++)
	{
		vt_double	 	sum	=	0;
		const T2	*rowptr = data[row];

		for (vt_int col=0; col<width; col++)
		{
			sum += rowptr[col];
		}
		row_means[row] = sum/(vt_double)width;
		
		// add to total for overall mean
		tot_sum += sum;
	}

	return tot_sum/((vt_double)width*height);
}


template<typename T1, typename T2> 
vt_double col_mean(T1 *col_means, const T2 **data, const vt_int width, const vt_int height)
{
	vt_longdouble tot_sum=0;

	for (vt_int col=0; col<width; col++)	
	{
		vt_double		sum	=	0;
		for (vt_int row=0; row<height; row++)
		{
			sum += data[row][col];
		}
		col_means[col] = sum/(vt_double)height;
		
		// add to total for overall mean
		tot_sum += sum;
	}

	return tot_sum/((vt_double)width*height);
}

template<typename T1, typename T2> 
vt_double col_mean(T1 *col_means,const T2 **data, const vt_int width, const vt_int row_start, const vt_int row_end)
{
	vt_longdouble tot_sum=0;

	vt_double cnt = row_end - row_start;
	for (vt_int col=0; col<width; col++)	
	{
		vt_double		sum	=	0;
		for (vt_int row=row_start; row<row_end; row++)
		{
			sum += data[row][col];
		}
		col_means[col] = sum/cnt;
		
		// add to total for overall mean
		tot_sum += sum;
	}

	return tot_sum/((vt_double)width*cnt);
}


template<typename T1, typename T2> 
vt_double row_mean(T1 *row_means,const T2 **data, const vt_int width, const vt_int row_start, const vt_int row_end)
{
	vt_longdouble tot_sum=0;

	for (vt_int row=row_start; row<row_end; row++)
	{
		vt_double		sum	=	0;
		for (vt_int col=0; col<width; col++)	
		{
			sum += data[row][col];
		}
		row_means[row] = sum/width;
		
		// add to total for overall mean
		tot_sum += sum;
	}
	vt_double cnt = row_end - row_start;
	return tot_sum/((vt_double)width*cnt);
}



/**
  \image html vatech.jpg

  \par REQUIREMENTS: 
  
	This is class which implements the panoramic and cephelometric calibration functions.

	The functions may be thought of as consisting of two distince groups those realated to the calculation of the calibration data and 
	those related to the applicaton of the calibration process to acquired images. The former set consist of routines;-
		
		-# for the acquisition of source data required for calibration

		-# for calculation and storage of the calibration data.


	\todo
	Merge the CVtHalfLineCalib and the CVtLineCalib classes.
	The CVtHalfLineCalib calibration class was created to solve a problem with asymetric x-ray power in the panoramic
	system i.e. two separate calibrations were applied on different sides of the panoramic scan. However
 */

#define DF_THRESH 5.0
template<typename ImageType, typename CoefType> 
class CVtHalfLineCalib
{
private:
	typedef 	ImageType IM_TYPE;
	typedef 	CoefType  COEF_TYPE;

	CVtImage<ImageType> m_bright;
	CVtImage<ImageType> m_dark;

	vt_ulong  m_chip_height;
	vt_ulong  m_numChips;

	CoefType *m_darkC;
	CoefType *m_brightC;
	CoefType *m_coef;
	CoefType *m_bias;
	vt_ulong m_bias_width;

	vt_bool   m_initialised;
	vt_bool   m_ceph_mode;
	const vt_double m_max_coef;

	const vt_bool m_smooth;
public:
	CoefType  m_pedestal;

  /**
	\enum SMOOTH_SPAN
	\brief When the calibration coefficients are recalculated there is the option to smooth the resultant
	coefficients. i.e. to minimise the difference in applied calibration
	*/
	enum {
		SMOOTH_SPAN = 5
		, TOTAL_SPAN = 2*SMOOTH_SPAN + 1
		, DEFAULT_PEDESTAL = 1000
		, MAX_COEF				 = 4
		, EDGE_SKIP				 = 400
		, CENTRAL_EXT			 = 500
		};

  /**
   * Constructor
   */
  CVtHalfLineCalib(vt_ulong height
							, vt_ulong numChips ) 
									: m_darkC( NULL )
									, m_brightC( NULL )
									, m_coef( NULL )
									, m_chip_height( height )
									, m_numChips( numChips )
									, m_initialised( false )
									, m_pedestal( (CoefType)DEFAULT_PEDESTAL )
									, m_bias( NULL )
									, m_max_coef( MAX_COEF )
									, m_smooth( false )
	{}

	virtual ~CVtHalfLineCalib() 
	{
		if (m_darkC != NULL)
		{
			delete m_darkC;
		}
		if (m_brightC != NULL)
		{
			delete m_brightC;
		}
		if (m_coef != NULL)
		{
			delete m_coef;
		}
		if (m_bias != NULL)
		{
			delete m_bias;
		}
	}

	///
	// smooth
	//
	// Produces a smoothed version of the input vector
	// Returns smoothed version as a pointer it is the calling functions responsibility to remove
	// this vector.
	//
	CoefType * smooth(CoefType *vec, const vt_ulong length)
	{
		CoefType * smth_vec = new CoefType[length]; //larger than require

		memset( smth_vec, 0 , sizeof( smth_vec[0] )*length );

		// smooth 
		vt_double sum				 =  0;
		vt_long  smooth_span = SMOOTH_SPAN;
		for( vt_ulong pos = smooth_span; pos < length - smooth_span; pos++ )
		{
			if (smooth_span == pos) // first pixel
			{
				for( vt_long idx = -smooth_span; idx < smooth_span ; idx++)
				{
					sum += vec[pos+idx];
				}

				// deal with beginning
				CoefType mn = sum/TOTAL_SPAN;
				for(idx = 0; idx < smooth_span ; idx++)
				{
					smth_vec[idx] = mn;
				}
			}
			else
			{
				vt_ulong last_start = pos - smooth_span - 1;
				vt_ulong new_end    = pos + smooth_span;

				sum -= vec[last_start];
				sum += vec[new_end];
			}

			smth_vec[pos] = sum/TOTAL_SPAN;

			if (length - smooth_span - 1 == pos) // last pixel
			{
				// deal with end - jut tidy up last few elements of the vector
				CoefType mn = sum/TOTAL_SPAN; // this will fill the last
																			// few entries with the last
																			// valid smoothed entry
				for(vt_ulong idx = pos+1; idx < length; idx++)
				{
					smth_vec[idx] = mn;
				}
			}
		}

		// deal with the
		return smth_vec;
	}

	///
	// note set bright and dark functions merely replace the dark and bright frames
	// whereas the update_xxxx function replace the frame and recalculate the coefficients
	// i.e. the update functions are equivalent to a set and a recalc.
	//
	void set_bright(const CVtImage<ImageType>& bright_frame)
	{
		m_bright = bright_frame;
	}

	///
	// set_bright frame
	//
	void set_dark(const CVtImage<ImageType>& dark_frame)
	{
		m_dark   = dark_frame;
	}

	///
	// calculate bias correction
	//
	void set_bias(const vt_double *bias, const vt_ulong width)
	{
		m_bias_width = width;
		if (m_bias != NULL)
			delete m_bias;

		m_bias = (vt_double *) bias;
	}

	//
	// default zero bias
	//
	void calc_bias(const vt_ulong width)
	{
		m_bias_width = width;
		if (m_bias != NULL)
			delete m_bias;

		m_bias = new vt_double [m_bias_width];
		memset( m_bias, 0, m_bias_width*sizeof( m_bias[0] ) );
	}

	///
	// access functions
	//
	CVtImage<ImageType>& get_bright()
	{
		return m_bright;
	}

	/**
	\brief Access function for the dark frame.
	\return the calibration dark frame.
	*/
	CVtImage<ImageType>& get_dark()
	{
			return m_dark;
	}
	/**
	\brief Add a new bright frame to the calibration 
	This function replaces the current bright frame with a new one.
	
	\param bright_frame the new bright frame.

	*/
	void update_bright(const CVtImage<ImageType>& bright_frame)
	{
		recalc(bright_frame, m_dark);
	} 

	void update_dark(const CVtImage<ImageType>& dark_frame)
	{
		recalc(m_bright, dark_frame);
	} 

	/**
	\brief Recalculate the calibration coefficients.

	Uses a dark fram and a bright frame to 

	*/
	void recalc()
	{
		recalc( m_bright, m_dark );
	}

	void recalc(const CVtImage<ImageType>& BrightFrame, const CVtImage<ImageType>& DarkFrame) 
	{
		m_bright = BrightFrame;
		m_dark   = DarkFrame;

		if (m_bright.height() != m_dark.height() )
			Vt_fail( "Invalid dark bright frames" );

		vt_ulong height = m_bright.height();

		// allocate coefficants
		if (m_coef != NULL)
			delete m_coef;
		
		m_coef = new CoefType[height];
		memset( m_coef, 0, sizeof( m_coef[0])*height  );

		// note the start row is dependent on the API type

		CVtAPI::API_TYPE api_type = GetAPI().get_api_type();
		vt_ulong start_row = 0;
		vt_ulong end_row   = 0;

		if (api_type == CVtAPI::PANO_API)
		{
			start_row = m_chip_height;
			end_row   = m_numChips*m_chip_height;
		}
		else
		{
			start_row =  0; // ceph mode has actual data in 'C' hence calculation can start from
											// row 0
			end_row   = m_numChips*m_chip_height;
		}

		// allocate dark field
		if (m_darkC != NULL)
			delete m_darkC;

		m_darkC = new CoefType[height];
		
		memset( m_darkC, 0, sizeof( m_darkC[0])*height  );

		vt_double mean_dark	= row_mean( m_darkC, (const ImageType **) m_dark.lines(), m_dark.width(), start_row, end_row );
		
		printf( "mean dark level %lf ", mean_dark );

		// smooth dark values
		if (m_smooth)
		{
			CoefType *smth_dark = smooth( m_darkC, end_row );

			delete m_darkC;
			m_darkC = smth_dark;

			// fix gap
			m_darkC[m_chip_height-1]   = m_darkC[m_chip_height-2] + (m_darkC[m_chip_height] - m_darkC[m_chip_height-2])/2;
			m_darkC[2*m_chip_height-1] = m_darkC[2*m_chip_height-2] + (m_darkC[2*m_chip_height] - m_darkC[2*m_chip_height-2])/2;
		}

		// allocate bright field
		if (m_brightC != NULL)
			delete m_brightC;

		m_brightC	= new CoefType[height];
		
		memset( m_brightC, 0, sizeof( m_darkC[0])*height  );
		vt_double mean_bright = row_mean( m_brightC, (const ImageType **)  m_bright.lines(), m_bright.width(), start_row, end_row );

		printf( "mean bright level %lf\n", mean_bright );

		vt_double	mean_signal = (mean_bright - mean_dark);

		printf( "mean signal level %lf\n", mean_signal );

		// smooth bright values
		if (m_smooth)
		{
			CoefType *smth_bright = smooth( m_brightC, end_row );

			delete m_brightC;
			m_brightC = smth_bright;

			// gap fix
			m_brightC[m_chip_height-1]   = m_brightC[m_chip_height-2] + (m_brightC[m_chip_height] - m_brightC[m_chip_height-2])/2;
			m_brightC[2*m_chip_height-1] = m_brightC[2*m_chip_height-2] + (m_brightC[2*m_chip_height] - m_brightC[2*m_chip_height-2])/2;
		}


		///
		// main calculation loop
		//
		if (mean_signal != 0)
		{
			vt_double *dfvec = new vt_double [ height ];
			vt_double dfsum   = 0;
			vt_double dfsumsq = 0;
			vt_ulong  dfcnt   = 0;
			//						chip height     total_height
			for (vt_int row=start_row+1; row<end_row; row++)
			{
				const vt_double eps = 0.000001;

				vt_double diff = m_brightC[row] - m_darkC[row];

				if (diff > eps)
				{
					m_coef[row] = mean_signal/diff;

					// sanity check coefs
					if (m_coef[row] > m_max_coef)
						m_coef[row] = 0.0;

					if (row > 0)
					{
						dfvec[row] = fabs(m_coef[row-1] - m_coef[row]);
						dfsum += dfvec[row];
						dfsumsq += dfvec[row]*dfvec[row];
						dfcnt++;
					}
				}
				else
				{
					m_coef[row] = 0.0;
				}
			}

			// sanity check coefs some more
			if (dfcnt > 0 )
			{
				vt_double df_mean = dfsum/dfcnt;
				vt_double df_std  = sqrt( dfsumsq/dfcnt - df_mean*df_mean );	
				for (row=start_row+1; end_row<height; row++)
				{
					if (dfvec[row] > 3*df_std)
					{
						m_coef[row] = 0.0;
					}
				}
			}
			if (dfvec != NULL)
			{
				delete dfvec;
			}
		}
		else
		{
			Vt_fail( "Invalid dark or bright frame" );
		}
		m_initialised = true;
	}


	void gap_fixAB(ImageType **imptr, const vt_ulong pos, const vt_ulong width)
	{
		srand( (unsigned)time( NULL ) );
		//
		// fix gap BC
		//
		vt_long  plus  = 7; 
		vt_long  minus = 2; 
    ImageType *VecP		= imptr[pos+plus];
    ImageType *VecM		= imptr[pos-minus];

		vt_double mult_inc  = 1.0/(plus + minus);
		vt_double mult_fac  = mult_inc;
		for (vt_ulong row=pos-minus; row<pos+plus; row++, mult_fac += mult_inc)
		{
			// note not bounds checking here - needs caution
			ImageType *Vec = imptr[row];
			for (vt_ulong col=0; col<width; col++)
			{
				vt_double new_val = VecM[col] + mult_fac*(VecP[col] - VecM[col]);
				Vec[col]					= new_val;
			}
		}
	}

	void gap_fix(ImageType **imptr, const vt_ulong pos, const vt_ulong width)
	{
		//
		// fix gap 
		//
    ImageType *VecP1		= imptr[pos];
    ImageType *Vec			= imptr[pos-1];
    ImageType *VecM1		= imptr[pos-2];

		for (vt_ulong col=0; col<width; col++)
		{
			Vec[col] = VecM1[col] + (VecP1[col] - VecM1[col])/2;
		}
	}

	/**
	\brief calculate the offset between to chips, the a and b chip

	The calibration procedure is not perfect, due to dark current drift after application of the 
	calibration the 

	/
	 This merely take the average difference of a few lines above and below the chip cut line
	*/
	const CoefType ABoffset( const vt_ulong ab_split, CVtImage<ImageType> &out )
	{
		CVtABDiff<ImageType> ab_diff(  ab_split );

		return ab_diff( out );
	}

	/**
	\brief calculate the offset between to BC

	The 
	*/
	const CoefType BCoffset( ImageType **imptr, const vt_ulong width )
	{
		vt_long sum = 0;
		ImageType *prow1 = imptr[2*m_chip_height-2];
		ImageType *prow2 = imptr[2*m_chip_height+1];
	
		for(vt_ulong col=0; col < width; col++)
		{
			vt_long d1 = prow1[col];
			vt_long d2 = prow2[col];

			sum += d2 - d1;
		}
		return sum/(CoefType)width;
	}

	/**
	\brief The main calibration interface for the panoramic and ceph system

	The function applies a gain and offset calibration to the panoramic and ceph images.

	\param  InFrame the input image
	\param  OutFrame the calibrated output image
	\note - these should be seperate images - this function doesn't guarentee
	in place operation
	*/
	void operator () (const CVtImage<ImageType>& InFrame
										, CVtImage<ImageType>& OutFrame
									 )
	{
		if (!m_initialised)
			return;

		///
		// apply calibration
		//
		vt_ulong				width    = InFrame.width();	
		const ImageType **inptr  = (const ImageType **)InFrame.lines();
		ImageType				**outptr = OutFrame.lines();

		///
		// apply calibration
		//
		vt_bool c1to2_done = false;
		vt_bool c2to3_done = false;
		CoefType offset		 = 0.0;

		const vt_ulong scope     = 2;
		const vt_ulong overshoot = CVtRectPairs::RECT_SIZE + CVtRectPairs::OFFSET + 2;

		for (vt_long row=m_chip_height*m_numChips-1; row>=0; row--)
		{
      const ImageType *InVec	= inptr[row];
      const CoefType   dark   = m_darkC[row];
			const CoefType   coef   = m_coef[row];

      ImageType *OutVec		= outptr[row];
      
			// calculate offsets
			if (row == m_chip_height-overshoot && !c1to2_done)
			{
				if ( GetAPI().get_api_type() == CVtAPI::PANO_API )
				{
					offset	= 0;
				}
				else
				{
					// note we skip a larger boundary with initial tile
					offset		 = ABoffset( m_chip_height, OutFrame );
				}

				row				 = m_chip_height;  // reset row to boundary
				c1to2_done = true;
			}
			else if (row == 2*m_chip_height-overshoot && !c2to3_done)
			{
				offset = BCoffset( outptr, width );

				row				 = 2*m_chip_height;  // reset row to boundary
				c2to3_done = true;
			}
			CoefType actual_offset = m_pedestal + offset;

			for (vt_ulong col=0; col<width; col++)
			{
				CoefType out = (((CoefType) InVec[col] - dark)*coef) + actual_offset;
				if (out<0)
				{
					OutVec[col] = (ImageType)0.0;
				}
				else if (out >= USHRT_MAX)
				{
					OutVec[col] = USHRT_MAX;
				}
				else
				{
					OutVec[col] = (ImageType) out;
				}
			}
		}

		//
		// fix gap 1
		//
		gap_fixAB( outptr, m_chip_height, width );

		//
		// fix gap 2
		//
		gap_fix( outptr, 2*m_chip_height, width );
	}

	//
	// Dark frame only calibration
	//
	void operator () (const CVtImage<ImageType>& InFrame
										, CVtImage<ImageType>& OutFrame
										, vt_bool dummy
									 )
	{
		if (!m_initialised)
			return;

		///
		// apply calibration
		//
		vt_ulong				width    = InFrame.width();	
		const ImageType **inptr  = (const ImageType **) InFrame.lines();
		ImageType **outptr       = OutFrame.lines();

		///
		// apply calibration
		//
		vt_bool c1to2_done = false;
		vt_bool c2to3_done = false;
		CoefType offset		 = 0.0;

		const vt_ulong scope     = 2;
		const vt_ulong overshoot = 10;
		for (vt_long row=m_chip_height*m_numChips-1; row>=0; row--)
		{
      const ImageType *InVec	= inptr[row];
      const CoefType dark = m_darkC[row];
			const CoefType coef = m_coef[row];

      ImageType *OutVec		= outptr[row];
      
			// calculate offsets
			if (row == m_chip_height-overshoot && !c1to2_done)
			{
				if (GetAPI().get_api_type() == CVtAPI::PANO_API)
				{
					offset	= 0;
				}
				else
				{
					// note we skip a larger boundary with initial tile
					offset		 = ABoffset( m_chip_height, OutFrame );
				}

				row				 = m_chip_height;  // reset row to boundary
				c1to2_done = true;
			}
			else if (row == 2*m_chip_height-overshoot && !c2to3_done)
			{
				offset = BCoffset( outptr, width );

				row				 = 2*m_chip_height;  // reset row to boundary
				c2to3_done = true;
			}
			CoefType actual_offset = m_pedestal + offset;

			for (vt_ulong col=0; col<width; col++)
			{
				CoefType out = (CoefType) InVec[col] + actual_offset;
				if (out<0)
				{
					OutVec[col] = (ImageType)0.0;
				}
				else if (out >= USHRT_MAX)
				{
					OutVec[col] = USHRT_MAX;
				}
				else
				{
					OutVec[col] = (ImageType) out;
				}
			}
		}

		//
		// fix gap 1
		//
		gap_fixAB( outptr, m_chip_height, width );

		//
		// fix gap 2
		//
		gap_fix( outptr, 2*m_chip_height, width );
	}

	///
	// save calibration coefficients
	//
	void save( std::string fname )
	{
		FILE *fpout = fopen( fname.c_str(), "wb" );

		Vt_precondition(fpout != NULL, "CvtHalfLineCalib::save failed to open output file\n" );

		fwrite( (const char *) m_darkC,		sizeof( m_darkC[0] ), m_chip_height*m_numChips, fpout );
		fwrite( (const char *) m_brightC, sizeof( m_brightC[0] ), m_chip_height*m_numChips, fpout );
		fwrite( (const char *) m_coef,		sizeof( m_coef[0] ), m_chip_height*m_numChips, fpout );

		fclose(fpout);
	}

	///
	// save calibration coefficients
	//
	void save( FILE *fpout )
	{
		Vt_precondition(fpout != NULL, "CvtHalfLineCalib::save failed to open output file\n" );

		fwrite( (const char *) m_darkC,		sizeof( m_darkC[0] ), m_chip_height*m_numChips, fpout );
		fwrite( (const char *) m_brightC, sizeof( m_brightC[0] ), m_chip_height*m_numChips, fpout );
		fwrite( (const char *) m_coef,		sizeof( m_coef[0] ), m_chip_height*m_numChips, fpout );
	}

  ///
  // istream operator
  //
  friend std::istream& operator >> (std::istream& IS, CVtHalfLineCalib<ImageType, CoefType>& Object)
  {
    Vt_precondition(IS.good() ? true : false, "CvtHalfLineCalib::operator >> - Failed to open input file");
	
    // Read the header data
		if (Object.m_darkC != NULL)
		{
			delete Object.m_darkC;
		}

		Object.m_darkC = (CoefType *) new vt_byte[ Object.m_chip_height*Object.m_numChips*sizeof(CoefType) ];
		IS.read( (char *) Object.m_darkC,   Object.m_chip_height*Object.m_numChips*sizeof(CoefType) );

		// allocate and read
		if (Object.m_brightC != NULL)
		{
			delete Object.m_brightC;
		}
		Object.m_brightC = (CoefType *)  new vt_byte[ Object.m_chip_height*Object.m_numChips*sizeof(CoefType)  ];
		IS.read( (char *) Object.m_brightC, Object.m_chip_height*Object.m_numChips*sizeof(CoefType)  );
		
		// allocate and read
		if (Object.m_coef != NULL)
		{
			delete Object.m_coef;
		}
		Object.m_coef = (CoefType *)  new vt_byte[ Object.m_chip_height*Object.m_numChips*sizeof(CoefType) ];
		IS.read( (char *) Object.m_coef,   Object.m_chip_height*Object.m_numChips*sizeof(CoefType) );

		Object.m_initialised = true;

    return IS;
  }
};



/**
 *  $Author: david $
 *  $Revision: 1.3 $
 *  $Date: 2013/05/27 17:41:57 $
 *
 * REQUIREMENTS: 

 * 
 * SPECIFICATIONS: 

 * 
 * DESIGN NOTES: 

 *
 */

template<typename ImageType, typename CoefType> 
class CVtLineCalib
{
	///
	// Bias direction check
	//
	enum {
		BIAS_ROW_OFFSET = 100 // we use the bottom x rows for checking the bias direction
		, DEFAULT_BIAS_LEADER = 300
	};

	typedef vt_double *vt_double_ptr;	
public:
	///
	// bright extraction - not these values are also used for the
	// left right bias check averaging
	//
	// Note these figures assume single binning. If the binning mode is changed
	// to double binning in the column direction these figures will have to change
	// This is not good. However, no time to do it the right way.
	//
	enum {
		SKIP_COUNT	  		 = 160
		, PANO_EXT				 = 600
		, P_LEXT_START_COL = 140
		, P_LEXT_END_COL	 = P_LEXT_START_COL + PANO_EXT
		, P_REXT_START_COL = 2140
		, P_REXT_END_COL   = P_REXT_START_COL + PANO_EXT

		, CEPH_EXT				 = 500
		, C_LEXT_START_COL = 600 
		, C_LEXT_END_COL	 = C_LEXT_START_COL + CEPH_EXT
		, C_REXT_START_COL = 1700 
		, C_REXT_END_COL	 = C_REXT_START_COL + CEPH_EXT
	};

	vt_ulong		m_skip_count;
	vt_ulong		m_lext_start_col;
	vt_ulong		m_lext_end_col;
	vt_ulong		m_rext_start_col;
	vt_ulong		m_rext_end_col;

	CVtHalfLineCalib<ImageType, CoefType>		m_calib;

	vt_ulong																m_chip_height;
	vt_ulong																m_numChips;

												
	CVtLineCalib(const vt_ulong height
							, const vt_ulong numChips
							, const vt_bool hbin_flag
							, const CVtAPI::API_TYPE api ) : m_calib( height, numChips )
																						, m_chip_height( height )
																						, m_numChips( numChips )
	{
		set_hbin( hbin_flag, api );
	}

	///
	// set_hbin()
	// turn on or off horizontal binning mode
	//
	void set_hbin(const vt_bool hbin_flag, const CVtAPI::API_TYPE api)
	{
		if (api == CVtAPI::PANO_API)
		{
			if (hbin_flag)
			{
				// hbin true i.e. there is horizontal binning
				m_skip_count			=	SKIP_COUNT;
				m_lext_start_col	= P_LEXT_START_COL;
				m_lext_end_col		= P_LEXT_END_COL;
				m_rext_start_col	= P_REXT_START_COL;
				m_rext_end_col		= P_REXT_END_COL;
			}
			else
			{
				// hbin false i.e. no horizontal binning
				m_skip_count			=	SKIP_COUNT;
				m_lext_start_col	= 2*P_LEXT_START_COL;
				m_lext_end_col		= 2*P_LEXT_END_COL;
				m_rext_start_col	= 2*P_REXT_START_COL;
				m_rext_end_col		= 2*P_REXT_END_COL;
			}
		}
		else
		{
			if (hbin_flag)
			{
				// hbin true i.e. there is horizontal binning
				m_skip_count			=	SKIP_COUNT;
				m_lext_start_col	= C_LEXT_START_COL;
				m_lext_end_col		= C_LEXT_END_COL;
				m_rext_start_col	= C_REXT_START_COL;
				m_rext_end_col		= C_REXT_END_COL;
			}
			else
			{
				// hbin false i.e. no horizontal binning
				m_skip_count			=	SKIP_COUNT;
				m_lext_start_col	= 2*C_LEXT_START_COL;
				m_lext_end_col		= 2*C_LEXT_END_COL;
				m_rext_start_col	= 2*C_REXT_START_COL;
				m_rext_end_col		= 2*C_REXT_END_COL;
			}
		}
	}

	///
	// Do coefficients calculation and calibration
	//
	// Try this version based on mean signal level.
	// could try an alternative based on absolute differences.
	//
	void recalc()
	{
		m_calib.recalc();
	}

	///
	// access functions
	//
	void set_dark(const CVtImage<ImageType>& dark_frame)
	{
		m_calib.set_dark(dark_frame);
	}


	void set_bright(const CVtImage<ImageType>& bright_frame, const vt_ulong half_index)
	{
		set_single_bright(bright_frame);
	}


	///
	// OK - these are the main application of the calibration functions
	//
	void operator () (const CVtImage<ImageType>& InFrame
										, CVtImage<ImageType>& OutFrame
										, const vt_ulong half_position
										, const vt_bool dark_frame_calib
										)
	{
		m_calib( InFrame, OutFrame, true );
	}

	///
	// OK - these are the main application of the calibration functions
	//
	void operator () (const CVtImage<ImageType>& InFrame
										, CVtImage<ImageType>& OutFrame
										, const vt_ulong half_position
										)
	{
		m_calib( InFrame, OutFrame );
	}

	///
	// save calibration coefficients
	//
	void save( std::string fname )
	{
		FILE *fpout = fopen( fname.c_str(), "wb" );

		Vt_precondition(fpout != NULL, "CVtLineCalib::save failed to open output file\n" );

		m_calib.save(fpout);

		fclose(fpout);
	}

  ///
  // istream operator
  //
  friend std::istream& operator >> (std::istream& IS, CVtLineCalib<ImageType, CoefType>& Object)
  {
    Vt_precondition(IS.good() ? true : false, "CVtLineCalib::operator >> - Failed to open input file");
	
		// OK stream out left and right
		IS >> Object.m_calib;

    return IS;
  }

private:
	void set_single_bright(const CVtImage<ImageType>& bright_frame)
	{
		double *smth_mean;
		double *diff;

		vt_ulong frm_chip_height = bright_frame.height();
		vt_ulong frm_width = bright_frame.width();

		smooth(bright_frame, smth_mean, diff);

		vt_bool *mask = new vt_bool[frm_width];
		memset( mask, false, sizeof(vt_bool) * frm_width );
		vt_ulong cols = left_bmask( mask, smth_mean, diff, frm_width);
    cols += right_bmask( mask, smth_mean, diff, frm_width);

		// allocate first half image
		CVtImage<ImageType> *brtp = new CVtImage<ImageType> ( cols, frm_chip_height );
		CVtImage<ImageType> &brt  = *brtp;
		//
		// copy image
		//
		vt_ulong cnt = 0;
		for (vt_ulong col = 0; col < frm_width; col++)
		{
			if (mask[col])
			{
				for (vt_ulong row = 0; row < frm_chip_height; row++)
				{
					ImageType data = bright_frame[row][col]; 
					brt[row][cnt] = data;
				}
				cnt++;
			}
		}
		m_calib.set_bright(brt);

		// clean up
		if (mask != NULL)
			delete mask;
		if (smth_mean != NULL)
			delete smth_mean;
		if (diff != NULL)
			delete diff;
		if( brtp != NULL )
			delete brtp; // delete this as copied to bright frame
	}

	///
	//
	// smooth
	//
	// calculate smoothed difference vector - returns 2 vectors
	// these should be deleted in the calling function.
	//
	void smooth(const CVtImage<ImageType>& bright, vt_double_ptr &smth_mean, vt_double_ptr &df)
	{
		double *colmns = new double[bright.width()];
		double bright_mn = col_mean(colmns, (const ImageType **) bright.lines(), bright.width(), m_chip_height, bright.height() );

		smth_mean = new double[bright.width()]; //larger than require

		memset( smth_mean, 0 , sizeof( smth_mean[0] )*bright.width() );

		df = new double[bright.width()]; //larger than require
		memset( df, 0 , sizeof( df[0] )*bright.width() );

		// smooth colmeans
		vt_double sum				  =  0;
		vt_long  smooth_span = CVtHalfLineCalib<ImageType, CoefType>::SMOOTH_SPAN;
		for( vt_ulong col = smooth_span; col < bright.width() - smooth_span; col++ )
		{
			if (smooth_span == col)
			{
				for( vt_long idx = -smooth_span; idx < smooth_span ; idx++)
				{
					sum += colmns[col+idx];
				}
			}
			else
			{
				vt_ulong last_start = col - smooth_span - 1;
				vt_ulong new_end    = col + smooth_span;

				sum -= colmns[last_start];
				sum += colmns[new_end];
			}
			smth_mean[col] = sum/CVtHalfLineCalib<ImageType, CoefType>::TOTAL_SPAN;
			df[col] = smth_mean[col] - smth_mean[col-1]; // remember to ignore first diff
		}
		df[smooth_span] = 0; // zap bad first diff

		if (colmns != NULL)
			delete colmns;
	}
	///
	// left_bmask()
	// create bright mask. Find the region of the image which is bright
	//
	// This code assumes that the central section of the image is elevated above.
	// It finds the works in from the outside edges i.e. find the two outside shoulders.
	//
	vt_ulong left_bmask(vt_bool mask[], const vt_double smth_mean[], const vt_double diff[], const vt_ulong width)
	{
		vt_ulong start_col = (m_lext_start_col > width - 1) ? (width-1) : m_lext_start_col;
		vt_ulong end_col   = (m_lext_end_col > width - 1) ? (width-1) : m_lext_end_col;

		printf( "strt col %d end col %d\n", start_col, end_col );

		vt_ulong cnt = 0;
		for (vt_ulong col = start_col; col < end_col; col++,cnt++)
		{
			if (diff[col] > DF_THRESH)
				break;

			mask[col] = true;
		}
		return cnt;
	}

	///
	// right_bmask()
	// create bright mask. Find the region of the image which is bright
	//
	// This code assumes that the central section of the image is elevated above.
	// It finds the works in from the outside edges i.e. find the two outside shoulders.
	//
	vt_ulong right_bmask(vt_bool mask[], const vt_double smth_mean[], const vt_double diff[], const vt_ulong width)
	{
		vt_ulong start_col = (m_rext_start_col > width - 1) ? (width-1) : m_rext_start_col;
		vt_ulong end_col   = (m_rext_end_col > width - 1) ? (width-1) : m_rext_end_col;

		printf( "strt col %d end col %d\n", start_col, end_col );

		vt_ulong cnt = 0;
		for (vt_ulong col = start_col; col < end_col; col++,cnt++)
		{
			if (diff[col] > DF_THRESH)
				break;

			mask[col] = true;
		}
		return cnt;
	}
}; // end of line calib

} // Vt namespace


#endif // __CVTPANORAMICCALIBRATION_H__
