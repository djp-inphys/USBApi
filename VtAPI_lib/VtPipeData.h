/** \file VtPipeData.h

	The file which contains the main abstraction for the raw aquired data.

 * Copyright (c) 2013 by
 * Innovative Physics plc
 * All Rights Reserved
 *
 *
 * REVISIONS: 
 * $Log: VtPipeData.h,v $
 * Revision 1.2  2013/05/27 12:04:19  david
 * Ver 1
 *
 * Revision 1.1.1.1  2013/04/19 01:14:18  david
 * initial import
 *
 * Revision 1.1.1.1  2013/04/11 13:02:32  david
 * Initial import
 *
 *
 */

#ifndef _VTPIPEDATA_H_
#define _VTPIPEDATA_H_

namespace Vt {

/**
 When the raw data is allocated a sentinal value is placed at the end of each line of data.
 This value can be checked at any point to check for the presense of data over run. Generally the
 sentinal value should be chosen to be something easily recognised in a data dump like the hex value
 0xdead.

@see \fn PixelType** allocLineArray  (  vt_uint  width,    vt_uint  height  )  

*/
const vt_ushort gSentinel = 0xdead;

/**
\class CCriticalSection

Dummy sunchronization object - see the notes below, not currently used.

*/
class CCriticalSection
{
public:
	CCriticalSection() {}
	virtual ~CCriticalSection() {}
};

//
// \class Lock utility class
//
//! Synchronisation class
//
/*!
	The pipe data object can theoretically have information taken out from it at the same time
	as data is put into the pipedata i.e. a producer thread in the USBdriver class
*/
class CVtLock 
{

public:
	CVtLock( CCriticalSection &lck )
	{
//		Lock();
	}
	virtual ~CVtLock()
	{
//		Unlock();
	}
};


/*! Pipe date is produced by CVtUSBDriver object. 
	The pipe data is accessed by the currently parser class.
	\note The main parser object is held and destroyed by the current sys singleton. However, 
	a reference to this parser is held by the current API and the USBDriver object.

	@see CVtUSBDriver:: class
*/

class CVtUSBPipeData 
{
	enum{
		TRY_MAX = 10 //< If we are synchronised data mode this the maximum number of attempts that 
								 //< we allow for the producer thread to put data into pipe data before we say we
								 //< have reached the end of data
	};
public:


	//! This default constructor for pipe data, which is called when the pipe data 
	//! is first constructed.
	CVtUSBPipeData(const vt_bool sync) : m_sync( sync )
		, m_buffers( NULL )
		, m_numbufs( 0 )
		, m_data( NULL )
		, m_quiet( true )
		, m_eod( false )
		, m_pos( 0 )
		, m_bufno( 0 ) {}

	//
	/*! Second constructor is used to initialise the pipe with data	previously acquired. 
		Currently this in not used. It is here for convenience. The pipe is normally initialised
		via the init functions.

		\param buffers the actual data buffers, these can be allocated with the utility function 
		\param bufferSize
		\param numBufs
		\param sync
	*/

	CVtUSBPipeData(vt_ushort **buffers, const vt_ulong bufferSize, const vt_ulong numBufs, const vt_bool sync ): m_size( bufferSize )
						, m_buffers( buffers )
						, m_numbufs( numBufs )
						, m_sync( sync )
						, m_quiet( true )
						, m_pos( 0 )
						, m_bufno( 0 )
						, m_eod( false )
	{
		init( m_buffers, bufferSize, numBufs );
	}


	void reset()
	{
		if (m_sync)
			Vt_fail( "should not reset pipe in sync mode" );

		if ( m_buffers != NULL )
		{
			init(m_buffers, m_size, m_numbufs );
		}
		else
		{
			Vt_fail( "Trying to reset a pipe that has not be initialised" );
		}
	}

	// init
	void init(vt_ushort **buffers, const vt_ulong bufferSize, const  vt_ulong numBufs)
	{
		m_pos			= 0;
		m_bufno		= 0; // current buffer number

		m_size		= bufferSize;
		m_numbufs = numBufs;
		m_buffers = buffers;

		// get rid of anything currently on the queue
		while( !m_queue.empty() )
		{
			if (m_sync)
				delete m_queue.front();

			m_queue.pop();
		}

		// and the new buffers to the queue
		for(vt_ulong bufno=0; bufno < numBufs; bufno++)
		{
			if (buffers[bufno] != NULL)
			{
				m_queue.push( buffers[bufno] );
			}
		}
		m_data = m_queue.front(); // set data to new front of queue
	}

	//
	virtual ~CVtUSBPipeData()
	{
		// delete any data still in queue
		while( !m_queue.empty() )
		{
			if (m_sync)
				delete m_queue.front();

			m_queue.pop();
		}
	}

private:
	std::queue<vt_ushort *>  m_queue;		// this is where data vectors are queue
	vt_ushort								*m_data;		// current front of queue
	vt_ulong								 m_pos;			// current position in front of queue vector
	vt_ulong								 m_bufno;		// the current buffer number
	vt_ulong								 m_size;		// buffer size

	vt_ushort							 **m_buffers;		// a copy of the current buffers - only non null in non-sync mode
	vt_ulong								 m_numbufs;	  // only valid in non sync mode
	vt_bool									 m_sync;
	vt_bool									 m_eod;
public:
	vt_bool									m_quiet; 

	vt_ulong  get_size()
	{
		return m_size;
	}
	void  set_size(const vt_ulong size)
	{
		m_size = size;
	}
	vt_ulong  get_gpos()
	{
		return m_pos + m_size*m_bufno;
	}

	vt_ushort *reqst_buffer()
	{
		CVtLock lock( CCriticalSection() );

		m_eod = false; // we have data if this function is called
		if (m_queue.empty())
		{
			m_queue.push( new vt_ushort[ m_size ] );
			m_pos = 0;
			
			if (!m_quiet)
				printf( "pe\n" );

			return m_data = m_queue.back(); 
		}
		else
		{
			m_queue.push( new vt_ushort [ m_size ] );
			
			if (!m_quiet)			
				printf( "pd" );
			return m_queue.back(); 
		}
	}

	vt_ushort *get_front()
	{
		CVtLock lock( CCriticalSection() );

		if (m_queue.size() > 1)
		{
				m_queue.pop(); // delete entry in queue
				if (m_sync)
				{
					delete m_data; // delete old data
													// non sync does its own data cleanup
				}
				m_pos = 0; 
				m_eod = false;

				// OK set m_data to the new queue front
				//
				if (!m_quiet)
					printf( "cd" );

				return m_data = m_queue.front(); // set data to new front of queue
		}
		else // we are not allowed access to the very front of the queue
		{
			if (!m_quiet)
				printf( "ce\n" );
			
			m_pos = 0;
			m_eod = true; // end of data
			return m_data;
		}
	}

	vt_ulong get_pos() const
	{
		return m_pos;
	}

	vt_ushort& operator*() const
	{	
		return m_data[m_pos];
	}

	///
	//  Define prefix increment operator.
	//	CVtUSBPipeData& operator++()
	//
	void operator++()
	{
		//
		// note - we perhaps should have a locking mechanism here.
		// - however if assume that there is only one cosumer thread
		// this guy gets access to the queue by get_front which is
		// access controlled. Hence, we should be ok.
		//
		m_pos++;
	
		if (m_pos >= m_size)
		{
			m_bufno++;
			if (m_data[m_size] != gSentinel)
			{
				Vt_fail( "Invalid sentinel" );
			}
			get_front();
			if (m_eod)
			{
				if (!m_quiet)
					printf( "eod\n" );
				if (!m_sync)
				{
					if (!m_quiet)
						printf( "-->eod\n" );
					Vt_fail( "EOD" ); // end of data
				}
				else
				{
					vt_ulong trys = 0;
					while( (m_data == NULL) && (trys++ < CVtUSBPipeData::TRY_MAX) )
					{
						Sleep(100); // give producer thread time to get in
						get_front();
					}
					if (m_eod)
						Vt_fail( "EOD" ); // end of data
				}
			}
		}
	}
};


} // end of namespace - currently Vt - needs to be changed to Vt
#endif // _VTPIPEDATA_H_
