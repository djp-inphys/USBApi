/** \file VtpcLineParser.h

	The main parser file for the pano and ceph data.

 * Copyright (c) 2013 by
 * Innovative Physics plc
 * All Rights Reserved
 *
 *
 */

#ifndef _VTPCLINEPARSER_H_
#define _VTPCLINEPARSER_H_

namespace Vt
{

/**
 The general class which will hold the pano data

 The pano line data is assumed to be

 SOL, D1(A), D1(B), D1(C), ..... D1356(A), D1356(A), D1356(A), EOL, SOL

 Hence, the acquisition state machine is 

	-# Looking for start of line
	-# found start of line,
	-# looking for end of line
	-# found end of line
	-# looking for start of line etc...

 The data is held in a vector. The data is added to the 
 vector by a producer thread. The producer thread will
 will continue to acquire data until the end of data 
 is detected.

 It is removed from the data by a general consumer thread.
 By default the consumer thread is the main thread.
*/
class CVtpcLineParser : public CVtParser
{
public:
	// 
	// data set stuff
	//
	typedef struct 
	{
		CVtAPI::IM_TYPE type;
		vt_ulong half_idx;
	}	DATASET_ENTRY_TYPE;

	typedef CVtDataset<DATASET_ENTRY_TYPE>::DATASET_ENTRY DATASET_ENTRY;
	typedef CVtDataset<DATASET_ENTRY_TYPE>::DATASET				DATASET;


	enum {
			HDR_MASK = 0x8000
			, HDR_SOL_EOL_MASK = 0x4000
			, HALF_INFO_MASK = 0x2000
			, SENSOR_LINE_INFO_MASK = 0x1fff
			, FRAME_LINE_INFO_MASK = 0x1fff
			, DATA_CHIP_MASK = 0x3000
		} m_Masks;
	enum {
			DATA_CHIPA_PTRN = 0x0000
			, DATA_CHIPB_PTRN = 0x1000
			, DATA_CHIPC_PTRN = 0x2000
			, CHIP_DATA_MASK = 0x0fff
			, HDR_EOL_PTRN = HDR_MASK | HDR_SOL_EOL_MASK
	} m_Patterns;
	
	enum {
		BUFFER_SAFETY_FACTOR = 3
	  , TRY_MAX = 20000
		, DATA_LENGTH = 4608
		, SENTINEL    = 0xdead
		} m_CONST;
	
	enum{
		MAX_HEIGHT = 1536
		, DEFAULT_HALF_IDX = 2000
		};

private:
	vt_ulong				m_bufferSize;  // lin buffer size

	vt_ushort				*m_pBuff;			 // movable pointers into final line buffer
	vt_ushort				*m_chipABuff;  
	vt_ushort				*m_chipBBuff;
	vt_ushort				*m_chipCBuff;

	vt_ushort				*m_AEnd;  // pointers to end of final line buffer
	vt_ushort				*m_BEnd;
	vt_ushort				*m_CBeg;  // special case
	vt_ushort				*m_CEnd;


	vt_ushort				*Buff;   // pointer to holder for final line data
	vt_ushort				*ABuff;  // pointers into final line buffers
	vt_ushort				*BBuff;
	vt_ushort				*CBuff;

	vt_ulong				 m_chip_height;
	vt_ulong				 m_numChips;

	vt_bool					 m_quiet;
	vt_bool					 m_half;

	
public:
	CVtDataset<DATASET_ENTRY_TYPE> m_dataset;

	vt_ulong				 m_half_idx;
	vt_ulong				 m_first_idx;

	vt_ulong				 m_corrCount;
	vt_ulong				 m_errCount;


	////
	// constructor.
	//
	CVtpcLineParser( CVtUSBPipeData &pipeData	) :	CVtParser( pipeData  )
			, m_numChips( 0 )
			, m_chip_height( 0 )
			, m_errCount( 0 )
			, m_corrCount( 0 )
			, m_quiet( false )	
	{
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}

	// Note height is size of a single chip i.e the total amount of 
	// data transfered per line height * 2 or height * 3.
	//
	CVtpcLineParser( CVtUSBPipeData &pipeData
									, vt_ulong numChips // ideally we wouldn't want to expose the number of chips
																		// a pano mode ceph mode would be better here.
									, vt_ulong height
									, vt_bool quiet
		) :	CVtParser( pipeData  )
			, m_numChips( numChips )
			, m_chip_height( height )
			, m_errCount( 0 )
			, m_corrCount( 0 )
			, m_quiet( quiet )
	{
		init();
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}

	///
	// note prior to calling init. The pipedata refered to in this function must have been initialised
	//
	virtual void init()
	{
		CVtAPI &API = GetAPI();
		m_quiet				 = API.m_quiet;
		m_numChips		 = API.m_numChips;
		m_chip_height  = API.image_height()/m_numChips;

		m_bufferSize = m_chip_height;
		Buff  = new vt_ushort [ m_bufferSize * 3 + 1 ]; // temp make this bigger to avoid overrun
		Buff[m_bufferSize * 3] = SENTINEL; // put in sentinel value


		ABuff  = Buff;
		BBuff  = ABuff + m_bufferSize;
		CBuff  = BBuff + m_bufferSize;
		m_CEnd = CBuff + m_bufferSize - 1; // set Cbuff to the end
																			 // can then decrement rather than increment
	
		m_half = false;
		m_half_idx  = DEFAULT_HALF_IDX; 
		m_first_idx = 0; 

		m_corrCount = 0;
		m_errCount = 0;

		reset_ptrs();
	}
	
	virtual ~CVtpcLineParser() 
	{
		if(Buff != NULL)
		{
			delete Buff;
		}
	}

	void reset_ptrs()
	{
		// setup pointers to buffers - currently much larger than required
		//
		if (Buff[m_bufferSize*3] != SENTINEL)
			Vt_fail( "Data corruption - invalid sentinel value" );

		m_pBuff			= Buff;

		m_chipABuff = ABuff;
		m_AEnd			= &ABuff[ m_bufferSize ];

		m_chipBBuff = BBuff;
		m_BEnd			= &BBuff[ m_bufferSize ];

		m_chipCBuff = m_CEnd;
		m_CBeg			= m_BEnd; // because we are decrementing - to account for c inversion
													//
													// bdata bend
													//       cdata cdata cdata     
													//
													// Hence < m_CBeg is into b data.
													//
													// Therefore condition should >= m_CBeg and we are ok
													// still in c data
													//
	}


	///
	// Utility iterators
	//
	void MOVA()
	{
		// chip 1
		register vt_short data = *m_pipeData;
		if ((data & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN )
		{
			*m_chipABuff++ = 0;

			Vt_fail( "a:short line detected" );
		}

		if (m_chipABuff < m_AEnd ) { // make sure we don't write beyond the end of the data
			*m_chipABuff++ = data; 
			++m_pipeData; // chip mask not required for a
		}
		else {
			Vt_fail( "a:long line detected::data overrun" );
		}
	}
	void MOVB()
	{
		register vt_short data = *m_pipeData;

		if ((data & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN )
		{
			*m_chipBBuff++ = 0;

			Vt_fail( "b:short line detected" );
		}

		if (m_chipBBuff < m_BEnd ) {// make sure we don't write beyond the end of output buff
			*m_chipBBuff++ = data & CHIP_DATA_MASK; 
			++m_pipeData;

		}
		else {
			Vt_fail( "b:long line detected::data overrun" );
		}
	}
	//
	// account for the fact that C is inverted
	//
	void MOVC()
	{
		register vt_short data = *m_pipeData;

		if ((data & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN )
		{
			*m_chipCBuff-- = 0;
			Vt_fail( "c:short line detected" );
		}

		if (m_chipCBuff >= m_CBeg ) {// make sure we don't write beyond the end of output buff
			*m_chipCBuff-- = data & CHIP_DATA_MASK; 
			++m_pipeData;
		}
		else	{
			Vt_fail( "c:long line detected::data overrun" );
		}
	}

	// align boundary
	//
	// The boundaries of the rx usb buffers are not aligned on the triple 
	// or double data boundaries. Hence, do a little shuffle to ensure that
	// the boundaries are aligned in the following
	//
	vt_bool align()
	{
		vt_ushort line; // don't care
		return align( line );
	}

	vt_bool align(vt_ushort &line)
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

			if (!m_half && ((curr_data & HALF_INFO_MASK) == HALF_INFO_MASK))
			{
				m_half = true;
				if (m_first_idx > line)
				{
					// allow for wrap around of line index
					m_half_idx = (FRAME_LINE_INFO_MASK - m_first_idx) + line;
				}
				else
				{
					m_half_idx = line - m_first_idx;
				}
				m_half_idx = (m_chip_height < MAX_HEIGHT) ? m_half_idx/2 : m_half_idx;

				if (!m_quiet)
					printf( "\nhalf found @ %d\n", m_half_idx );
			}
		}
		
		while( curr_data & HDR_MASK)
		{
			++m_pipeData;
			curr_data = *m_pipeData; // get new data
		}

		//
		// now look for type A pixel type
		// 
		if (m_numChips > 2)
		{
			// three cases
			vt_ushort test = *m_pipeData & DATA_CHIP_MASK;
			switch(test)
			{
			case DATA_CHIPA_PTRN:
				// already aligned exit
				return true;

			case DATA_CHIPB_PTRN:
				MOVB();
				MOVC();
				return true;

			case DATA_CHIPC_PTRN:
				MOVC();
				return true;
			
			default:
				Vt_fail( "chip data type not found" );
				return false;
			}
		}
		else if (m_numChips == 2)
		{
			// two cases
			switch(*m_pipeData & DATA_CHIP_MASK)
			{
			case DATA_CHIPA_PTRN:
				// already aligned exit
				return true;
			case DATA_CHIPB_PTRN:
				MOVB();
				return true;
			default:
				Vt_fail( "chip data type not found" );
				return false;
			}		
		}
		else
		{
			// 1 chip already aligned
			return true;
		}

		return false; // should never get here
	}

	///
	//
	// utility to find a header in the current stream
	//
	//
	vt_bool find_hdr(vt_ulong &length)
	{
		vt_bool hdr_found = false;
		for(length = 0;!hdr_found && length < CVtpcLineParser::TRY_MAX; length++)
		{
			hdr_found = ((*m_pipeData &  HDR_MASK )== HDR_MASK );	++m_pipeData;			
		}
		return hdr_found;
	}

	//
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

			correct_length = (length == (m_chip_height*m_numChips+1));
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
	virtual vt_ulong count_lines( const vt_long total )
	{
		// current position of data in pipe
		vt_ulong curr_pos = m_pipeData.get_gpos();
		
		return (total - curr_pos)/(m_chip_height*m_numChips);
	}

	///
	// Get the next line
	//
	virtual vt_bool get_line()
	{
		vt_ushort line_num = 0;
		vt_ulong  count		= 0;
		vt_bool eol_found = false;
		try
		{
			reset_ptrs();
			
			align( line_num );
	
			eol_found = ((*m_pipeData & ( HDR_MASK | HDR_SOL_EOL_MASK ))== HDR_EOL_PTRN );
			while( !eol_found )
			{
				// no hence add the next three elements 
				//
				switch( m_numChips )
				{
				case 1:
					MOVA(); // move a buff by 1
					break;
				case 2:
					MOVA(); // move a buff by 1
					MOVB(); // move b buff by 1
					break;
				case 3:
					MOVA(); // move a buff by 1
					MOVB(); // move b buff by 1
					MOVC(); // move c buff by 1
					break;
				default:
					Vt_fail( "Unsupported number of chips" );
				}
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
				find_hdr(trys);
			}
			else if (exp.find("EOD", 0) != std::string::npos)
			{
				return false;
			}
		}
		
		if (count == m_chip_height)
		{
//			if (m_corrCount++ % 100 == 0 )
			if (!m_quiet)
			{
				printf( "EOL CORRECT : %d %d %d %x\n",  count, m_corrCount, line_num, line_num );
			}
		}
		else
		{
//			if (m_errCount++ % 100 == 0 )
			if (!m_quiet)
			{
				printf( "EOL ERROR ERROR : %d %d %d %x\n",  count, m_errCount++, line_num, line_num );
			}
		}
		
		return eol_found;
	}
	

	///
	// Get the next line
	//
	virtual vt_bool get_line(vt_bool dummy)
	{
		vt_ushort line_num = 0;
		vt_ulong  count		= 0;
		vt_bool hdr_found = false;
		try
		{
			reset_ptrs();
			
			align( line_num );
			
			vt_ushort test = 0;
			
			hdr_found = ((*m_pipeData &  HDR_MASK )== HDR_MASK );
			while( !hdr_found )
			{
				// no hence add the next three elements 
				//
				switch( m_numChips )
				{
				case 1:
					MOVA(); // move a buff by 1
					break;
				case 2:
					MOVA(); // move a buff by 1
					MOVB(); // move b buff by 1
					break;
				case 3:
					MOVA(); // move a buff by 1
					MOVB(); // move b buff by 1
					MOVC(); // move c buff by 1
					break;
				default:
					Vt_fail( "Unsupported number of chips" );
				}
				// test for end of line
				hdr_found = ((*m_pipeData &  HDR_MASK )== HDR_MASK );
				count++;
			}
		}
		catch (std::exception &e)
		{
			std::string exp(e.what());
			std::cout << exp << std::endl; 

			if (exp.find("short line detected", 0) != std::string::npos)
			{
				hdr_found = true;
			}
			else if (exp.find("long line detected", 0) != std::string::npos)
			{
				vt_ulong trys;
				find_hdr(trys);
			}
			else if (exp.find("EOD", 0) != std::string::npos)
			{
				return false;
			}
		}
		
		if (count == m_chip_height)
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
				printf( "EOL ERROR ERROR : %d %d %d %x\n",  count, m_corrCount++, line_num, line_num );
			}
		}
		
		return hdr_found;
	}
	

	vt_bool save_line(FILE *fpout, const char *pBuff, const vt_ulong dataBytes )
	{
		std::cout << int( pBuff[0] );
		fwrite( pBuff, 1, dataBytes, fpout );
		return true;
	}

	vt_bool save_line(FILE* fpout)
	{
		const vt_ulong dataSize = m_chip_height * sizeof( ABuff[0] );
		
		save_line( fpout, (const char *) ABuff, dataSize );
		save_line( fpout, (const char *) BBuff, dataSize );
		save_line( fpout, (const char *) CBuff, dataSize );

		return true;
	}

	vt_bool save_line(FILE* fpout, vt_bool aflag, vt_bool bflag, vt_bool cflag )
	{
		const vt_ulong dataSize = m_chip_height * sizeof( ABuff[0] );
		
		if (aflag)
		{
			save_line( fpout, (const char *) ABuff, dataSize );
		}
		if (bflag)
		{
			save_line( fpout, (const char *) BBuff, dataSize );
		}
		if (cflag)
		{
			save_line( fpout, (const char *) CBuff, dataSize );
		}

		return true;
	}

	//
	// save current line into a column of data
	//
	virtual vt_bool save_line(vt_ushort** outbuf,const vt_ulong colnum, vt_bool aflag=true, vt_bool bflag=true, vt_bool cflag=true )
	{
		const vt_ulong dataSize = m_chip_height * sizeof( ABuff[0] );
		
		if (aflag)
		{
			vt_ushort *inptr  = ABuff;
			for (vt_ulong row = 0; row < m_chip_height; row++)
			{
				outbuf[row][colnum] = *inptr++;
			}
		}
		if (bflag)
		{
			vt_ushort *inptr  = BBuff;
			for (vt_ulong row = m_chip_height; row < m_chip_height*2; row++)
			{
				outbuf[row][colnum] = *inptr++;
			}
		}
		if (cflag)
		{
			vt_ushort *inptr  = CBuff;
			for (vt_ulong row = m_chip_height*2; row < m_chip_height*3; row++)
			{
				outbuf[row][colnum] = *inptr++;
			}
		}

		return true;
	}

	virtual vt_bool save_line(vt_ushort** outbuf,const vt_ulong colnum)
	{
			return save_line( outbuf, colnum, true, true, true );
	}	

	virtual CVtDataset<DATASET_ENTRY_TYPE>& get_dataset()
	{
		return m_dataset;
	}

	virtual void add_image( CVtImageBaseClass *im )
	{
		DATASET_ENTRY_TYPE ent_type;
		ent_type.type			= CVtAPI::ACQ_IM;
		ent_type.half_idx = m_half_idx;
		m_dataset.add_dataset( ent_type, im );
	}
};

	
} // end of namespace - currently Vt - needs to be changed to Vt

#endif // _VTPCLINEPARSER_H_