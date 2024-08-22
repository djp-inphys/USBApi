/** \file VthdsLineParser.h

	The file which contains the parser for the hds sensor.

	
 * Copyright (c) 2013 by
 * Innovative Physics plc
 * All Rights Reserved
 *
 *
 */

#ifndef _VTHDSLINEPARSER_H_
#define _VTHDSLINEPARSER_H_

namespace Vt
{

/**
\class CVthdsLineParser

This class is not exposed to the outside world at all. The Vt::CVthdsImpAPI class can see this class
because like this class it specialises in dealing with data from the hds sensor.

The class is responsible for understanding the nature of the data produce by the various hds sensors and 
for compiling this data into sensible chunks. In this case the sensible chunks are sensor rows.

*/
class CVthdsLineParser : public CVtParser
{
public:
	/** 
	\brief What is contained in the dataset.

	The idea behind the dataset is that it forms an association beteen image infomation and 
	an actual image. Currently the image information held in the hds sensor is merely the image
	type i.e. ACQ_IM, CALIB_IM etc. However, it could easily be extended in the future to allow
	for more complex information to be stored.
	*/
	
	typedef struct 
	{
		CVtAPI::IM_TYPE type;
	}	DATASET_ENTRY_TYPE;

	typedef CVtDataset<DATASET_ENTRY_TYPE>::DATASET_ENTRY DATASET_ENTRY;
	typedef CVtDataset<DATASET_ENTRY_TYPE>::DATASET				DATASET;

	
	CVtDataset<DATASET_ENTRY_TYPE>	m_dataset;

	enum {
			HDR_MASK								= 0x8000
			, HDR_SOL_EOL_MASK			= 0x4000
			, HALF_INFO_MASK				= 0x2000
			, SENSOR_LINE_INFO_MASK = 0x1fff
			, FRAME_LINE_INFO_MASK	= 0x1fff
			, CHIP_NO_MASK		 		  = 0x3000
		} m_Masks;
	enum {
	  TRY_MAX				= 20000
		, DATA_LENGTH = 4608
		, SENTINEL    = 0xdead
		} m_CONST;
	
	enum{
		MAX_HEIGHT				 = 1536
		, DEFAULT_HALF_IDX = 2000
		};

	enum {
		 CHIP_DATA_MASK = 0x0fff
		, HDR_EOL_PTRN = HDR_MASK | HDR_SOL_EOL_MASK
	} m_Patterns;
	


private:

	vt_ulong				m_bufferSize;  // lin buffer size
	vt_ulong				m_first_idx;
	vt_ulong				m_image_height;
	vt_bool					m_quiet;


	vt_ushort				*Buff;				 // line buffer
	vt_ushort				*m_pBuff;			 // movable pointer into final line buffer
	vt_ushort				*m_pBuffEnd;	 // end of current line buffer

public:
	vt_ulong				 m_corrCount;
	vt_ulong				 m_errCount;

	////
	// constructors.
	//
	CVthdsLineParser( CVtUSBPipeData &pipeData ) : CVtParser( pipeData )
										, m_bufferSize( 0 )
										, m_first_idx( 0 )
										, m_image_height( 0 )
										, m_errCount( 0 )
										, m_corrCount( 0 )
										, m_quiet( false ) 
										, Buff( NULL )
	{
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}

	CVthdsLineParser( CVtUSBPipeData &pipeData
										, vt_ulong height
										, vt_bool quiet
		) : CVtParser( pipeData, height, quiet )
										, m_bufferSize( 0 )
										, m_first_idx( 0 )
										, m_image_height( height )
										, m_errCount( 0 )
										, m_corrCount( 0 )
										, m_quiet( quiet ) 
										, Buff( NULL )
	{
		init();
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}

	virtual ~CVthdsLineParser() 
	{
		// clean up image data
		m_dataset.delete_dataset();

		if(Buff != NULL)
		{
			delete Buff;
		}
	}

	///
	// note prior to calling init. The pipedata refered to in this function must have been initialised
	//
	virtual void init()
	{
		CVtAPI &API = GetAPI();
		m_quiet					= API.m_quiet;
		m_image_height  = API.image_height();

		m_bufferSize = m_image_height;
		Buff  = new vt_ushort [ m_bufferSize  + 1 ]; // temp make this bigger to avoid overrun
		Buff[ m_bufferSize ] = SENTINEL; // put in sentinel value

		m_corrCount = 0;
		m_errCount = 0;

		reset_ptrs();
	}
	
	virtual void reset_ptrs()
	{
		// setup pointers to buffers - currently much larger than required
		//
		if (Buff[ m_bufferSize ] != SENTINEL)
			Vt_fail( "Data corruption - invalid sentinel value" );

		m_pBuff		 = Buff;
		m_pBuffEnd = &Buff[ m_bufferSize ];
	}


	///
	// Utility iterators
	//
	void MOV()
	{
		register vt_short data = *m_pipeData;
		if ((data & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN )
		{
			*m_pBuff++ = 0;

			Vt_fail( "short line detected" );
		}

		if (m_pBuff < m_pBuffEnd ) { // make sure we don't write beyond the end of the data
			*m_pBuff++ = data & CHIP_DATA_MASK; 
			++m_pipeData; 
		}
		else {
			Vt_fail( "long line detected::data overrun" );
		}
	}
	///
	// align boundary
	//
	// The boundaries of the rx usb buffers are not aligned on the triple 
	// or double data boundaries. Hence, do a little shuffle to ensure that
	// the boundaries are aligned in the following
	//
	virtual vt_bool align()
	{
		vt_ushort line; // don't care
		return align( line );
	}

	virtual vt_bool align(vt_ushort &line)
	{
		// skip over header at current position - if we are indeed at a header
		register vt_ushort curr_data = *m_pipeData;

		if (!m_quiet)
			printf( "HEADER : " );

		if (curr_data & HDR_MASK)
		{
			line			= curr_data & FRAME_LINE_INFO_MASK; // always line  from where header first encountered
			if (!m_quiet)
				printf( "%x ", curr_data );
		}
		
		while( curr_data & HDR_MASK)
		{
			++m_pipeData;
			curr_data = *m_pipeData; // get new data
		}

		return true; // should never get here
	}

	///
	// utility to find a header in the current stream
	//
	virtual vt_bool find_hdr(vt_ulong &length)
	{
		vt_bool hdr_found = false;
		for(length = 0;!hdr_found && length < CVthdsLineParser::TRY_MAX; length++,++m_pipeData)
		{
			register vt_ushort data  = *m_pipeData;
			hdr_found = ((data &  HDR_MASK )== HDR_MASK );	;			
		}
		return hdr_found;
	}

	///
	// Sync data
	//
	// Find the start of consitent data
	//
	virtual vt_bool sync_data(vt_ulong skip_count)
	{
		vt_bool hdr_found = ((*m_pipeData &  HDR_MASK )== HDR_MASK );

		vt_bool correct_length = false;
		while( !correct_length )
		{
			align(); // skip any current header

			vt_ulong length = 0;
			hdr_found = find_hdr(length); 

			correct_length = (length == (m_image_height));
		}

		//
		// skip the required number of lines
		//
		for (vt_ulong cnt = 0; cnt < skip_count; cnt++)
		{
			align(); // skip any current header

			vt_ulong length = 0;

			hdr_found = find_hdr(length); 
		}
		
		if (hdr_found)
		{
			m_first_idx = *m_pipeData & FRAME_LINE_INFO_MASK;
			if (!m_quiet)
				printf( "FIRST LINE IDX : %d", m_first_idx );
		}
		return hdr_found;
	}

	///
	// count lines
	//
	vt_ulong count_lines( const vt_long total )
	{
		// current position of data in pipe
		vt_ulong curr_pos = m_pipeData.get_gpos();
		
		return (total - curr_pos)/(m_image_height);
	}

	///
	// Get the next line
	//
	virtual vt_bool get_line()
	{
		vt_ushort line_num = 0;
		vt_ulong  count		 = 0;
		vt_bool eol_found  = false;

		try
		{
			reset_ptrs();
			
			align( line_num );
	
			eol_found = ((*m_pipeData & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN );
			while( !eol_found )
			{
				//
				// no hence add the next three elements 
				//
				MOV(); // move a buff by 1

				// test for end of line
				eol_found = ((*m_pipeData & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN );
				count++;
			}
		}
		catch (std::exception &e)
		{
			std::string exp(e.what());
			std::cout << exp << std::endl; 

			if (exp.find("short line detected", 0) != std::string::npos)
			{
				eol_found = true;
			}
			else if (exp.find("long line detected", 0) != std::string::npos)
			{
				vt_ulong trys;

				eol_found = find_hdr(trys);
			}
			else if (exp.find("EOD", 0) != std::string::npos)
			{
				return false;
			}
		}
		
		if (count == m_image_height)
		{
			if (!m_quiet)
			{
				printf( "EOL CORRECT : %d %d %d %x\n",  count, m_corrCount, line_num, line_num );
			}
		}
		else
		{
			if (!m_quiet)
			{
				printf( "EOL ERROR ERROR : %d %d %d %x\n",  count, m_errCount++, line_num, line_num );
			}
		}
		
		return eol_found;
	}
	
	///
	// save current line into a column of data
	//
	virtual vt_bool save_line(vt_ushort** outbuf,const vt_ulong colnum)
	{
		const vt_ulong dataSize = m_image_height * sizeof( Buff[0] );
		
		vt_ushort *inptr  = Buff;
		for (vt_ulong row = 0; row < m_image_height; row++)
		{
			outbuf[row][colnum] = *inptr++;
		}
	
		return true;
	}

	virtual CVtDataset<DATASET_ENTRY_TYPE>& get_dataset()
	{
		return m_dataset;
	}

	virtual void add_image( CVtImageBaseClass *im )
	{
		DATASET_ENTRY_TYPE ent_type;
		ent_type.type			= CVtAPI::ACQ_IM;
		m_dataset.add_dataset( ent_type, im );
	}
};

} // end of namespace - currently Vt - needs to be changed to Vt

#endif // _VTHDSLINEPARSER_H_