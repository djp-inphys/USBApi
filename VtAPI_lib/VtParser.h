/** \file VtParser.h

  This is the main interface file for the parser api.

	Each of the parsers in the system are derived from this interface-

 * Copyright (c) 2013 by
 * Innovative Physics plc
 * All Rights Reserved
 *


 *
 */

#ifndef _VTPARSER_H_
#define _VTPARSER_H_

namespace Vt
{
class CVtParser
{
protected:
	vt_ulong				m_image_height;
	vt_bool					m_quiet;
	
public:
	vt_ulong				m_half_idx; // pano variable

	CVtUSBPipeData  &m_pipeData;

	/**
	\brief Merely initialises the pipedata object which is stored at this level.

	This is version of the parser constructor that is called by the system.
	*/
	CVtParser( CVtUSBPipeData &pipeData ) : m_pipeData( pipeData ) {}


	/**
	\brief A second constructor for convenience


	Currently not used. However, the image height and quiet flag for the parser are held at this level.
	\note the image_height variable is potentially a mis-leading name. For the pano and ceph interfaces it is 
	correct. For the hds sensor then is is a matter of whether you regard the data as being passed rowwise
	or columnwise from the sensor.
	*/

	CVtParser( CVtUSBPipeData &pipeData
						, vt_ulong image_height
						, vt_bool  quiet
	) : m_pipeData( pipeData )
			, m_image_height( image_height )
			, m_quiet( quiet ) {}

	
	virtual ~CVtParser() {};

	/**
	  \brief Sync data
	
		In the various system the initial few lines could potentially contain redundent data from the fifo buffers.
		This routines find the start of real data. The test for real data is currently that it has the correct
		number of data elements between consitent data start of line and end of line markers.

		\param skip_count There is the option to skip an number of data points before looking for valid header.
	*/
	virtual vt_bool sync_data(vt_ulong skip_count) = 0;

	/*
	\brief count lines
	
	How many lines have been acquired so far.
	*/
	virtual vt_ulong count_lines( const vt_long total ) = 0;
	
	///
	// Get the next line
	//
	virtual vt_bool get_line() = 0;

	///
	// save current line into a column of data
	//
	virtual vt_bool save_line(vt_ushort** outbuf,const vt_ulong colnum) = 0;

	
	//
	// initialisation
	//
	virtual void init() = 0;
	
	//
	// add_dataset()
	//
	virtual void add_image( CVtImageBaseClass *im ) = 0;

	///
	// pipe data access functions
	//
	virtual void reset( vt_acq_im_type **rawdata, const vt_ulong numPix, const vt_ulong numBufs ) 
	{
		m_pipeData.init( rawdata, numPix, numBufs );
	}
	virtual void reset()
	{
		m_pipeData.reset(); 
	}
};

} // end of namespace - currently Vt - needs to be changed to Vt
#endif // _VTPARSER_H_