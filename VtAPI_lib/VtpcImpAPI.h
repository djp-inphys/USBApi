/** \file VtpcImpAPI.h

The main implementaion file for pano ceph API interface.
*
* Copyright (c) 2013 by
* All Rights Reserved
*
* REVISIONS: 
* $Log:$
*
*/

#ifndef __CVTPCIMPAPI_H__
#define __CVTPCIMPAPI_H__


#include "../../IUSBI2/Command.h"


namespace Vt 
{
/**
 $Author: david $
 $Revision: 1.3 $
 $Date: 2013/05/27 17:41:57 $

	\class CVtpcImpAPI
 
	This class provide the implementation details for the pano ceph API interface.

*/
class CVtSys;

class CVtpcImpAPI : public CVtAPI // implementation
{
	friend class CVtSys;					 					 // let the system access private stuff

	typedef enum
	{
		TRANSFER_TIMEOUT = INFINITE
	} TIMEOUT;

public:
	typedef CVtpcLineParser::DATASET_ENTRY_TYPE DATASET_ENTRY_TYPE;
	typedef CVtpcLineParser::DATASET_ENTRY			DATASET_ENTRY;
	typedef CVtpcLineParser::DATASET						DATASET;
		
	//! The dataset is defined and owned by the parser, however a copy is kept here for convenience.

	CVtDataset<DATASET_ENTRY_TYPE>	&m_dataset;
	CVtUsbDriver										&m_driver;

	//! The pano ceph flat field correction calibration object
	CVtLineCalib<vt_acq_im_type, vt_double>	m_calib;

	/**
	 For the out width is the the number of colunns of the final out image. This value depends on the API type.
	 The value is defined in the Vt::CVtpcAPI header file VtpcAPI.h
	*/
	vt_ulong												m_out_width;
	//! The height of an individual sensor, this value will depend on the binning mode selected,
	vt_ulong												m_chip_height;

	//! If we decide the save the contents of a dataset to disk as raw data files then this
	std::string											m_fname_base;

	/**
	\brief The main constructor for this pc API object.

	\par
	This is the version of the constructor called by the single instance system class. 
	This construtor merely sets up the reference to the main system driver object and the dataset object.
	It uses the reference to the driver object to control the hardware. The data acquired through the
	driver object can then be accessed via the dataset object. 

	This constructor also setup any interface parameters which depend on the api or binmode.

	\note
	The main api implementation classes don't need actual access to the pipedata object where the raw data is acquire. Hence
	The driver will capture the data and place it into the pipe data the parser will turn this into intelligible data and the 
	driver will then add it to the dataset object.

	\param api the type of api to be instantiated, must be PANO_API or CEPH_API
	\param bin_mode the binning mode, this must be 1x1 1x2 2x1 2x2
	\param driver		a reference to the single system driver object, own and held by the singleton system object
	\param dataset	a reference to the dataset object owned by the parser. The parser knows about the kind of data that held
	should be held by the dataset. Hence this object owns and holds the current dataset object. However, the
	driver, and main api objects know when data has been acqiured or manipulated. Hence they should be responsible for
	adding data to this object. Keeping this separation allows the driver object to behave generically with regard to the
	data. Hence obviating the need for multiple hds and pc specific driver classes.
	
	*/
	CVtpcImpAPI(const API_TYPE api
							, const BIN_MODE bin_mode
							, CVtUsbDriver									 &driver
							, CVtDataset<DATASET_ENTRY_TYPE> &dataset
							) : CVtAPI( api, bin_mode )
						, m_driver( driver )
						, m_dataset( dataset )
						, m_fname_base( DEFAULT_BASE_FNAME )
						, m_out_width( PANO_DEFAULT_OUT_IMAGE_WIDTH_BINx2 )
						, m_chip_height( DEFAULT_IMAGE_HEIGHT )
						, m_calib( m_chip_height, m_numChips, true, api ) // default height number of chips and binning mode
	{
		set_binmode_params(); // parameters which depend on binning mode
		set_api_params();			// parameters which depend on api
	}



  /**
  \brief Virtual place holder to enable correct destructor called

	Currently doesn't have anything to do.
  */
  virtual ~CVtpcImpAPI()
  {
	}

  /**
  \brief This is the main api initialisation function. 
	
	This function must always be called prior to using any of the api function. Currently, this function is exposed 
	to allow for the future inclusion of parameters to the initialisation sequence. If no parameters are required, 
	it might be sensible to call this function from the object creation code.

	This function currently has the following algorithmic structure
			-# delete any data which could be around from a previous instantiation.
			-# read in dark and bright frames. Currently, the calibration process uses a dark image and a bright image. For
			debugging purposes the code allows these images to be read in from existing files. Hence, the code looks for two
			files of the correct name in the correct directory. The filename and directory names are defined in the header file
			VtpcAPI.h, using the constants DEFAULT_DARK_FNAME and DEFAULT_BRIGHT_FNAME. If these files are present the data is read
			in and the calibration coefficients are automatically recalculated and saved to the default calibration filename.
			-# read in calibration files. If the dark and bright frames are not present the calibration coefficients are read in
			from a file.
  */
  virtual vt_bool init()
  {
		static vt_bool initialised = false;

		delete_dataset(); // get rid of previous images

		if (initialised) // only initialise a system once
			return true;

		initialised = true;

		///
		// read in calibration stuff
		//
		try {
			///
			// read in dark and bright frames and use these as the basis of the calibration
			// if they are available
			//
			if (read_darkbright())
			{
				if (!m_quiet)
					printf( "Recalculating coefficients....\n" );

				m_calib.recalc();

				if (!m_quiet)
					printf( "Saving coefficients...\n" );

				m_calib.save( get_calib_fname() );
			}
			else
			{
				std::ifstream CalDataStream( get_calib_fname(), std::ios_base::binary );
				if (!m_quiet)
				{
					if (m_apiType == PANO_API)
						printf( "Read pano calibration data....\n" );
					else
						printf( "Read ceph calibration data....\n" );
				}

				CalDataStream >> m_calib; // read in calibration data
				
				// Close the new file stream
				CalDataStream.close();
			}
		}
		catch (std::exception &)
		{
			// could trap this here if reqd
			Vt_fail( "no calibration file available. A calibration run must be performed to obtain calibrated images" );
		}

		// initialise main listening pipe
		if (!m_quiet)
			printf( "Initialising pipe data....\n" );

		return true;
	}

	//! Merely an access function for the current firmware filename.
	virtual char * get_fwfname()
	{
		return DEFAULT_HEX_FW_FNAME;
	}

	//
	// general control port access functions
	//
	virtual vt_byte ctrl_port() 
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );

		return m_driver.ctrl_port();
	}

	virtual vt_ulong half() 
	{
		return m_driver.half();
	}

	///
	// The amount of data to skip at the beginning of acquiring each image. On the current 
	// TDI mode ccd sensor the initial frames are saturated and hence should be discarded.
	//
	virtual vt_ulong get_header_size()
	{
		return DEFAULT_HDR_SIZE;
	}
	///
	// note: currently start is active low on the a paritcular pin of the usb
	// Hence the following code merely polls the pin looking for the active low state.
	//
	virtual CVtAPI::START_SIG  wait_for_start(const vt_double wait_time, const vt_double min_wait_time ) // in milliseconds
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );

		clock_t start = clock();

		double time_taken = 0.0;
		while( m_driver.pc_start() )
		{
			clock_t end = clock();
			time_taken = ((double)(end - start))/CLOCKS_PER_SEC;

			if (time_taken > wait_time)
				return CVtAPI::START_SIG_TIMEOUT;
		}
	
		// time taken must be more than min
		if (time_taken < min_wait_time)
			return CVtAPI::START_SIG_TOOQUICK;

		if (!m_quiet)
			BUGOUT( stderr, "Control port is %x\n", ctrl_port() );

		return CVtAPI::START_SIG_RECEIVED;
	}
		

	virtual CVtAPI::START_SIG  wait_for_start() // in milliseconds
	{
		// default wait a long time, in this case a long time is 
		// about 16-17 mins.
		//
		return wait_for_start(1000000.0, 0.0); 
	}

	///
	// Inverse of above
	//
	virtual vt_bool  wait_for_not_start(const vt_double wait_time)
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );
		clock_t start = clock();

		while( ! m_driver.pc_start() )
		{
			clock_t end = clock();
			double time_taken = ((double)(end - start))/CLOCKS_PER_SEC;

			if (time_taken > wait_time)
				return false;

			if (!m_quiet)
				rotator();
		}
		if (!m_quiet)
			printf( "OK\n" );

		return true;
	}
	virtual vt_bool  wait_for_not_start()
	{
		return wait_for_not_start( 40000 );
	}


	//----------------------------------------------------------
	// Main PanoCapture Interface routines Start
	//-----------------------------------------------------------
	///
	// main run thread
	//
	virtual void run()
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );
	
		m_driver.read_pipe();

		// reset the device - we don't actually want to reset the device now
		// vt_byte status[16];
		// send_command( status, m_driver.driver_handle(), VR_IUSBI_NOT_ACK, DEFAULT_SUB );
		if (!m_quiet)
			printf( "Control Port is %x\n",ctrl_port() );
	}

	template<typename T> 
	CVtImage<T>* transpose_lineim( CVtImage<T>& lineim )
	{
		//
		// transpose data
		//
		vt_long width = lineim.height(); // the output width is the input height
		CVtImage<vt_acq_im_type> *pim = new CVtImage<vt_acq_im_type> (width, m_chip_height*m_numChips);
		CVtImage<vt_acq_im_type> &im  = *pim;
		
		for( vt_ulong lineno = 0; lineno < width; lineno++)
		{
			vt_acq_im_type  *inptr  = lineim[lineno];

			vt_ulong row = 0;
			// tile a
			for(row = 0; row < m_chip_height; row++)
			{
				im[row][lineno] = *inptr++;
			}
			// tile b
			for(row = m_chip_height; row < 2*m_chip_height; row++)
			{
				im[row][lineno] = *inptr++;
			}
			// tile c
			if (m_invertC)
			{
				inptr += m_chip_height-1;
				for(row = 2*m_chip_height; row < 3*m_chip_height; row++)
				{
					im[row][lineno] = *inptr--;
				}
			}
			else
			{
				for(row = 2*m_chip_height; row < 3*m_chip_height; row++)
				{
					im[row][lineno] = *inptr++;
				}
			}
		}
		return pim;
	}
	///
	// main capture thread
	//
	// currently merely an alias for run
	//
	virtual void capture()
	{
		run();
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}

	///
	// simulate capture based on file image rather than grabbed from system.
	//
	virtual void capture(std::string& fname)
	{
		vt_ulong width;
		if (get_filesize(fname.c_str(), sizeof( vt_acq_im_type), width) == true)
		{
			std::ifstream cfile( fname.c_str(), std::ios::binary | std::ios::in );

			// opened file
			if (cfile.is_open())
			{
				if (!m_quiet)
					std::cout << "opened input file" << std::endl;

				///
				// estimate image size
				//
				CVtImage<vt_acq_im_type> *plineim = new CVtImage<vt_acq_im_type> (m_chip_height*m_numChips, width );
				CVtImage<vt_acq_im_type> &lineim  = *plineim;
				if (!m_quiet)
					std::cout << "reading input file...." << std::endl;


				// read in data 
				cfile.read( (char *) lineim.begin(), sizeof( vt_acq_im_type )*width*m_chip_height*m_numChips );

				if (!m_quiet)
					std::cout << "transposing data...." << std::endl;

				// transpose readin data
				CVtImage<vt_acq_im_type> *im = transpose_lineim( lineim );
				
				delete plineim;
				//
				DATASET_ENTRY_TYPE ent_type;
				ent_type.type			= ACQ_IM;
				ent_type.half_idx = m_out_width/2;  // !!!!! cheat value

				add_dataset( ent_type, im );
			}
			else
			{
				Vt_fail( "failed to open input image"  );
			}
		}
	}


	///
	// PROCESSING ROUTINES
	//
	// This is were the main data processing routines go. 
	// Currently this is merely reconstruction and calibration
	//
	//
	virtual void calibrate()
	{
		///
		// for each data set current stored
		//
		DATASET::iterator end = m_dataset.end(); // save current end - this allows us to add more
																							// entries in loop
		for(DATASET::iterator it = m_dataset.begin(); it != end; it++)
		{
			if ( (*it).first.type == CENTRE_IM )
			{
				CVtImage<vt_centre_im_type>* im = dynamic_cast<CVtImage<vt_centre_im_type>*>((*it).second);

				// OK apply calibration to each line
				CVtImage<vt_out_im_type>* cal_im = new CVtImage<vt_out_im_type>(im->width(), im->height());
				//
				// do calibration
				//
				if (m_darkFrameCal)
					m_calib( *im, *cal_im, (*it).first.half_idx, true );
				else
					m_calib( *im, *cal_im, (*it).first.half_idx );
		
				DATASET_ENTRY_TYPE ent_type( (*it).first );
				ent_type.type = OUTPUT_IM;

				add_dataset( ent_type, cal_im );
			}
		}

		Vt_postcondition( _CrtCheckMemory() == TRUE, "Calibrate::Memory problem detected\n" );
	}


	///
	// centres specific image types - let templates do the work
	// of generating the code for the different versions.
	//
	template<typename T>
	centre(const CVtImage<T>& im, const vt_ulong half_idx)
	{
		///
		// allocate output image
		//
		CVtImage<vt_out_im_type> *outimp = new CVtImage<vt_out_im_type>(m_out_width, im.height());
		CVtImage<vt_out_im_type>& outim  = *outimp;

		///
		// half index is relative to acquired image, the output image will be narrower than this
		// hence we wish to come back from the centre line by half the output image width.
		//
		vt_long start_idx = (vt_long)half_idx-(vt_long)m_out_width/2;

		if (start_idx < 0 )
		{
			start_idx = 0;
		}

		///
		// centred update the dataset
		//
		for( vt_ulong row = 0; row< im.height(); row++ )
		{
			const vt_calib_im_type *vec  = im[row];
			vt_out_im_type *outvec = outim[row];

			for( vt_ulong col = 0; col < m_out_width; col++)
			{
				if (col+start_idx >= im.width()) // note im.width is note necessarily m_out_width - it will
																				 // probably be larger depending on the number of packets that 
																				 // we have chosen to collect.
					break;
				
				outvec[col] = vec[col+start_idx];
			}
		}

		// OK complete - add dataset
		DATASET_ENTRY_TYPE ent_type;
		ent_type.type			= CENTRE_IM;
		ent_type.half_idx = half_idx;

		add_dataset( ent_type, outimp );
	}


	///
	// Centre the image
	// Centre takes an image type. However, currently
	// it only centres calib_ims and recon_ims
	//
	virtual void centre(IM_TYPE im_type)
	{
		///
		// for each calibrated image - produce a centred image
		//
		DATASET::iterator end = m_dataset.end(); // save current end - this allows us to add more
																							// entries in loop
		for(DATASET::iterator it = m_dataset.begin(); it != end; it++)
		{
			if ((*it).first.type == im_type)
			{
				switch(im_type)
				{
				case ACQ_IM:
					centre(*dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second), (*it).first.half_idx );
					break;

				case CALIB_IM:
					centre(*dynamic_cast<CVtImage<vt_calib_im_type>*>((*it).second), (*it).first.half_idx );
					break;
				default:
					Vt_fail( "Invalid image type for centring" );
					break;
				}
			}
		}
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Centre::Memory problem detected\n" );
	}

	//
	// generic process function this contains all the default
	// processing applied to a particular image
	//
	// currently not doing reconstruction process. The processing merely consists
	// of calibration and data access,
	//
	virtual void process()
	{
		calibrate();
	}

	virtual void process(IM_TYPE imtype)
	{
		if (!m_quiet)
			std::cout << "OK" << std::endl;
		
		centre(imtype); // centring before calib

		if (!m_quiet)
			std::cout << "Calibrating data set..." << std::endl;
		
		calibrate();
	}

	///
	// calibrate
	//
	virtual void calibration_run()
	{
		////
		// dark frames
		//
		printf( "\nCAPTURE DARK FRAMES :: Press return when ready - will wait for start\n\n" );
		getchar();

		wait_for_start();
		printf( "START...\n" );
		capture();
		DATASET_ENTRY dark_entry = pop_back( ACQ_IM );

		CVtImage<vt_acq_im_type>* dark_ptr = dynamic_cast<CVtImage<vt_acq_im_type>*>( dark_entry.second );
		Vt_postcondition( dark_ptr != NULL, "failed to obtain a valid imahe in calibration calculation routine" );
		m_calib.set_dark( *dark_ptr );

		////
		// bright frames
		//
		printf( "Waiting for stable position\n" );
		wait_for_not_start();
		printf( "CAPTURE BRIGHT FRAMES:: Press return when ready - will wait for start\n\n" );
		getchar();
		
		wait_for_start();
		printf( "START...\n" );
		capture();

		DATASET_ENTRY bright_entry  = pop_back( ACQ_IM );
		CVtImage<vt_acq_im_type> *bright_ptr = dynamic_cast<CVtImage<vt_acq_im_type>*>( bright_entry.second );

		printf( "Calculating the appropriate regions of the bright image to use....\n" );

		m_calib.set_bright( *bright_ptr, bright_entry.first.half_idx );

		printf( "OK\n" );
		///
		// calculation
		//
		printf( "Recalculating coefficients...\n" );

		m_calib.recalc();

		////
		// save
		//
		printf( "Saving coefficients...\n" );

		m_calib.save( get_calib_fname() );

		printf( "New coefficients writen to file %s\n" , get_calib_fname() );

		save(); // save source images
	}

  ///
  // Close the system safely
  //
  virtual vt_bool close_system()
  {
    return true;
  }
private:
	///
	// send a command to the device
	//
	void send_command(vt_byte status[16]
									 , vt_byte CommandCode
									 , vt_uint16 SubCommandCode )
	{
		m_driver.send_command(status, CommandCode, SubCommandCode );
	}

	////
	// Send reset command
	//
	// Note - send command throws and exception if it doesn't receive expected 
	// return from transmitting data.
	//
	void Reset(const HANDLE hDevice)
	{
			vt_byte status[16];
			send_command( status, VR_IUSBI_RESET, DEFAULT_SUB );
	}

	////
	// Send ready command
	//
	void Ready(const HANDLE hDevice)
	{
			vt_byte status[16];
			send_command( status, VR_IUSBI_READY, DEFAULT_SUB );
	}

	////
	// Send not ready command
	//
	void NotReady(const HANDLE hDevice)
	{
			vt_byte status[16];
			send_command( status, VR_IUSBI_NOT_READY, DEFAULT_SUB );
	}

	//----------------------------------------------------------
	// Main PanoCapture Interface routines Start
	//-----------------------------------------------------------
	//
	// bin mode effectively set the height and width of the image
	//
	virtual vt_ulong get_num_pkts()
	{
		set_num_pkts(); // ensure that the number of packets is set to the default for the acqusition mode
		return m_numPkts;
	}

	virtual void set_calib_fname()
	{
		m_calibFname = (m_apiType == PANO_API) ? DEFAULT_PANO_CALIB_FNAME : DEFAULT_CEPH_CALIB_FNAME;
	}

	virtual char* get_calib_fname()
	{
		set_calib_fname(); //  make sure name is set to default for acquisition mode
		return m_calibFname;
	}

	// interface type accessor
	virtual API_TYPE get_api_type()
	{
		return m_apiType;
	}


	///
	// what does the hardware think that the device is?
	//
	virtual API_TYPE  hw_device_type() 
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );

		return m_driver.hw_device_type();
	}

	///
	// private funcs - not exposed through API but useable
	//
	virtual void set_num_pkts(vt_ulong num_pkts) // explicitly set num pkts
	{
		m_numPkt_override = true;
		m_numPkts = num_pkts;
	}
	virtual void set_num_pkts(vt_bool dummy) // turn off override and set default num pkts
	{
		m_numPkt_override = false;
		set_num_pkts();
	}
	virtual void set_num_pkts() // set default num pkts
	{
		if (m_numPkt_override)
			return;

		if (m_calibFlag)
		{
			m_numPkts = (m_apiType == PANO_API) ? PANO_CALIB_NUM_PKTS : CEPH_CALIB_NUM_PKTS;
		}
		else
		{
			m_numPkts = (m_apiType == PANO_API) ? PANO_NUM_PKTS : CEPH_NUM_PKTS;
		}
		// are we horizontal binning
		if ( (m_bin_mode == BIN1x1) || (m_bin_mode == BIN2x1) )
		{
			// not hbinning
			m_numPkts *= 2;
		}
		// else do nothing for binned case

	}

protected:
	///
	// setup all the params which depend on the current binning mode
	//
	virtual vt_bool set_binmode_params(const vt_ulong vert, const vt_ulong horiz)
	{
		if (vert == 1)
		{	
			m_chip_height = 2*DEFAULT_CHIP_HEIGHT_BIN2x;
		}	
		else
		{	
			m_chip_height = DEFAULT_CHIP_HEIGHT_BIN2x;
		}	
		m_image_height = m_numChips*m_chip_height;

		if (m_apiType == PANO_API)
		{
			if (horiz == 1)
			{	
				m_out_width = 2*PANO_DEFAULT_OUT_IMAGE_WIDTH_BINx2;

				///
				// we are not binning in the horizontal direction - tell the calibration
				// it needs to know this so that it can work out were to take the bright 
				// regions from
				//
				m_calib.set_hbin( false, m_apiType );
			}	
			else
			{	
				m_out_width = PANO_DEFAULT_OUT_IMAGE_WIDTH_BINx2;
				///
				// we are binning in the horizontal direction - tell the calibration
				// it needs to know this so that it can work out were to take the bright 
				// regions from
				//
				m_calib.set_hbin( true, m_apiType );
			}	
		}
		else
		{
			// ceph mode
			if (horiz == 1)
			{	
				//  not binned
				m_out_width = 2*CEPH_DEFAULT_OUT_IMAGE_WIDTH_BINx2;

				///
				// we are not binning in the horizontal direction - tell the calibration
				// it needs to know this so that it can work out were to take the bright 
				// regions from
				//
				m_calib.set_hbin( false, m_apiType );
			}	
			else
			{	
				m_out_width = CEPH_DEFAULT_OUT_IMAGE_WIDTH_BINx2;
				///
				// we are binning in the horizontal direction - tell the calibration
				// it needs to know this so that it can work out were to take the bright 
				// regions from
				//
				m_calib.set_hbin( true, m_apiType );
			}	
		}

		return true;
	}

	virtual vt_bool set_binmode_params()
	{
		switch(m_bin_mode)
		{
		case BIN2x2:
			return set_binmode_params(2,2);

		case BIN2x1:
			return set_binmode_params(2,1);

		case BIN1x2:
			return set_binmode_params(1,2);

		case BIN1x1:
			return set_binmode_params(1,1);
		}
		return true;
	}

	///
	// set the api to have required characteristics
	// for the current binning mode and api type
	//
	virtual void set_api_params()
	{
		set_num_pkts();
		set_calib_fname();
	}

	/************************************
		 DATASET Manipulation
	************************************/
	///
	// IMAGE PROPERTY
	//
	///
	// Default image width is out image width
	//
	virtual vt_ulong image_width()
	{
		return m_out_width;
	}
	/// 
	// image information access functions
	//
	virtual vt_ulong image_width(IM_TYPE im_type)
	{
		return m_dataset.image_width(im_type);
	}

	///
	// get image height - returns image height
	// currently all the images are the same height
	//
	virtual vt_ulong image_height() 
	{
		return m_image_height;
	}
	virtual vt_ulong image_height(IM_TYPE) // arg not used
	{
		return m_image_height;
	}


	virtual vt_bool delete_dataset()
	{
		return m_dataset.delete_dataset();
	}

	virtual vt_bool delete_image(IM_TYPE im_type)
	{
		return m_dataset.delete_image(im_type);
	}

	///
	// image data accessors
	//
	virtual vt_ushort * image_ptr()
	{
		return m_dataset.image_ptr();
	}

	virtual vt_ushort * image_ptr(IM_TYPE im_type)
	{
		return m_dataset.image_ptr(im_type);
	}

	virtual vt_ushort ** image_ptrs(IM_TYPE im_type)
	{
		return m_dataset.image_ptrs(im_type);
	}


	///
	// dataset manipulation
	//
	// Datasets can be any 2d set of data. If we are in non-sync
	// mode then Dataset 0 is usually the raw input data buffers.
	//
	virtual void add_dataset( DATASET_ENTRY_TYPE ent_type, CVtImageBaseClass *pdata )
	{
		m_dataset.add_dataset( ent_type, pdata );
	}

	virtual DATASET_ENTRY pop_back( const IM_TYPE im_type )
	{
		return m_dataset.pop_back( im_type );
	}

	virtual DATASET_ENTRY get_back( const IM_TYPE im_type )
	{
		return m_dataset.get_back( im_type );
	}

	///
	// save
	//
	// This merely saves the data captured and processed above.
	//
	template<typename T>
	void save_imfile(T **pdata, vt_ulong pixel_size, vt_ulong width, vt_ulong height, std::string &fname, vt_bool row_wise )
	{
		FILE *fpout = fopen( fname.c_str(), "wb" );

		if (fpout != NULL)
		{
			if (row_wise)
			{
				///
				// store data row wise
				// Note the following assumes that the data is stores as one contigous buffer
				// with a row pointer vector allocated and pointing into this buffer. See the definition
				// of VtImage .
				//
				fwrite( (const char *) pdata[0], pixel_size, height*width, fpout );
			}
			else
			{
				// store column wise
				T *colbuf = new T[height];

				for( vt_ulong col=0; col < width; col++)
				{
					for( vt_ulong row=0; row < height; row++)
					{
						colbuf[row] = pdata[row][col];
					}

					fwrite( (const char *) colbuf, pixel_size, height, fpout );
				}

				delete colbuf;
			}
		}
		else
		{
			Vt_fail( "Failed to open output file\n" );
		}

		fclose(fpout);
	}

	virtual void save()
	{
		std::string fname_base( HDS_DEFAULT_BASE_DIR );

		vt_ulong fname_cnt = 1;

		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			vt_ulong pixel_size = 0;

			switch((*it).first.type)
			{
				case ACQ_IM:
				{
					std::cout << "Saving acquired image" << std::endl;
					CVtImage<vt_acq_im_type> *im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
					if (im == NULL)
					{
						Vt_fail( "Unexpected image type" );
					}
					save_imfile( im->lines()
									, sizeof( vt_acq_im_type)
									, im->width()
									, im->height()
									, Fname(fname_base, fname_cnt)
									, false ); 	
				}
				break;
				case CENTRE_IM:
				{
					std::cout << "Saving centred image" << std::endl;
					CVtImage<vt_centre_im_type>* im = dynamic_cast<CVtImage<vt_centre_im_type>*>((*it).second);
					if (im == NULL)
					{
						Vt_fail( "Unexpected image type" );
					}
					save_imfile( im->lines()
									, sizeof( vt_centre_im_type)
									, im->width()
									, im->height()
									, Fname(fname_base, fname_cnt)
									, false ); 	
					break;
				}
				case CALIB_IM:
				{
					std::cout << "Saving calibrated image" << std::endl;
					CVtImage<vt_calib_im_type>* im = dynamic_cast<CVtImage<vt_calib_im_type>*>((*it).second);
					if (im == NULL)
					{
						Vt_fail( "Unexpected image type" );
					}
					save_imfile( im->lines()
									, sizeof( vt_calib_im_type)
									, im->width()
									, im->height()
									, Fname(fname_base, fname_cnt)
									, false ); 	
					break;
				}
				case RECON_IM:
				{
					std::cout << "Saving recon image" << std::endl;
					CVtImage<vt_recon_im_type>* im = dynamic_cast<CVtImage<vt_recon_im_type>*>((*it).second);
					if (im == NULL)
					{
						Vt_fail( "Unexpected image type" );
					}
					save_imfile( im->lines()
									, sizeof( vt_recon_im_type)
									, im->width()
									, im->height()
									, Fname(fname_base, fname_cnt)
									, false ); 	
					break;
				}
				case OUTPUT_IM:
				{
					std::cout << "Saving output image" << std::endl;
					CVtImage<vt_out_im_type>* im = dynamic_cast<CVtImage<vt_out_im_type>*>((*it).second);
					if (im == NULL)
					{
						Vt_fail( "Unexpected image type" );
					}
					save_imfile( im->lines()
									, sizeof( vt_out_im_type)
									, im->width()
									, im->height()
									, Fname(fname_base, fname_cnt)
									, false ); 	
					break;
				}
				default:
					std::cout << "Invalid image type" << std::endl;
			}
		}
	}
	///
	// small utility to return the size of a particular file
	//
	vt_bool get_filesize( const std::string &fname, const vt_ulong pixel_size, vt_ulong &width )
	{
			vt_ulong fsize = 0;
			vt_bool  ret_code = false;
			
			width = 0;
			HANDLE handle = ::CreateFile(fname.c_str()
																	, GENERIC_READ
																	, FILE_SHARE_READ
																	, NULL
																	, OPEN_EXISTING
																	, 0
																	, NULL );

			if (handle != INVALID_HANDLE_VALUE)
			{
				fsize = ::GetFileSize(handle, NULL);
				::CloseHandle( handle );

				ret_code = true;
				width		 = fsize/(m_chip_height*m_numChips*pixel_size);
			}

			return ret_code;
	}

	///
	// read_darkbright()
	//
	// read in dark and bright frames and use this as the basis of the calibration
	//
	vt_bool read_darkbright()
	{
		vt_bool read_dark = false;
		vt_bool read_bright = false;
		if (file_exists( DEFAULT_DARK_FNAME ) && file_exists( DEFAULT_DARK_FNAME ) )
		{
			///
			// OK if the bright and dark frames are available open these and recalculate the
			// coeficients. If these are not available then look for the calibration file
			// .cal
			//
			vt_ulong dark_width = 0;
			if (get_filesize(DEFAULT_DARK_FNAME, sizeof(vt_acq_im_type), dark_width) == true)
			{
				if (!m_quiet)
					printf( "Dark frame found....\n" );

				std::ifstream darkframe_strm(DEFAULT_DARK_FNAME, std::ios_base::binary );
				CVtImage<vt_acq_im_type> *pdarkline = new CVtImage<vt_acq_im_type>(m_chip_height*m_numChips, dark_width);
				CVtImage<vt_acq_im_type> &darkline = *pdarkline;
				
				darkframe_strm.read( (char *) darkline.begin(), dark_width*m_chip_height*m_numChips*sizeof(vt_acq_im_type) );

				CVtImage<vt_acq_im_type> *im = transpose_lineim( darkline );
				
				delete pdarkline;

				m_calib.set_dark( *im );

				delete im;
				read_dark = true;
			}

			vt_ulong bright_width = 0;
			if (get_filesize(DEFAULT_BRIGHT_FNAME, sizeof( vt_acq_im_type), bright_width) == true)
			{
				if (!m_quiet)
					printf( "Bright frame found....\n" );

				std::ifstream brightframe_strm(DEFAULT_BRIGHT_FNAME, std::ios_base::binary );
				CVtImage<vt_acq_im_type> *pbrightline = new CVtImage<vt_acq_im_type>(m_chip_height*m_numChips, bright_width);
				CVtImage<vt_acq_im_type> &brightline = *pbrightline;				

				brightframe_strm.read( (char *) brightline.begin(), bright_width*m_chip_height*m_numChips*sizeof(vt_acq_im_type));

				CVtImage<vt_acq_im_type> *im = transpose_lineim( brightline );
				
				delete pbrightline;

				m_calib.set_bright( *im, m_out_width/2 );

				delete im;
				read_bright = true;
			}
		}
		else
		{
			return false;
		}

		return (read_dark && read_bright);
	}
};

} // end Vt namespace

#endif // __CVTPCIMAPI_H__
