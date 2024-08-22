/** \file VthdsImpAPI.h

* Copyright (c) 2013 by
* All Rights Reserved
*
* REVISIONS: 
* $Log:$
*
*/

#ifndef __CVTHDSIMPAPI_H__
#define __CVTHDSIMPAPI_H__



#include "../../HDSUSB2/Command.h" //!< \file ../../hdsusb2/command.h the command file for the hds commands only commands 

namespace Vt {
 
/**
  \image html vatech.jpg

  \par REQUIREMENTS: 
  
	This is class which implements the hds sensor interface. 

  \par SPECIFICATIONS: 
  
	
	Once created this class provide the interface to allow data to be acquire and passed to
	outside world. 

	This class uses to main classes for access to data and for manipulation of acquired data. The
	driver class (CVtUSBDriver) and the calibration class (VthdsCalib). 

	This class defines the nature of the data acquired for the hds sensor which is to be filled in by 
	the parser i.e. the type of data contained in the "dataset". A dataset is a unit of information
	which hold all the information from one data acquisition run. Consequently, for the hds sensor
	the dataset contains all the information relating to one output image. Which will be several bright 
	frames and a single dark frame.

	Initialisation. The objects returned by the system object are not automatically initialised. 
	It is the responsibility of the class user to initialise the objects. Consequently, any attempt to use this 
	class should first be proceed by a call to the generic API.init() function.


  \note
	Error handling is managed via the Vt_fail expception path. Hence, any object calling a function
	on this interface should catch any potential exceptions. 

	
	The class cannot be created directly, it can only be obained by calling GetAPI() with an appropiate 
	api specification type e.g.

	\code
	CVtAPI &API = GetAPI( CVtAPI::HDS15_API );
	CVtAPI &API = GetAPI( CVtAPI::HDS20_API );
	\endcode

	Data is acquired in a hierarchy in the system, the rawdata object (VtPipeData), object is filled 
	by the driver opject (VtDriver). Once the data has been acquire the data is formated into image data by 
	the parser object (VthdsLineParser).
*/

class CVthdsImpAPI : public CVtAPI
{
	friend class CVtSys;				// let the system access protected stuff
	friend class CVthdsCalib;		// let the calibation system access stuff in this class

public:
	/**
	The parser defines the dataset type, the implementation api merely typedef a copy of this
	definition for convenience.

	The dataset entry type refers to the type of data stored which is associated with an individual dataset image
	e.g.
	\code
	typedef struct 
	{
		CVtAPI::IM_TYPE type;
	}	DATASET_ENTRY_TYPE;
	\endcode
	*/
	typedef CVthdsLineParser::DATASET_ENTRY_TYPE	DATASET_ENTRY_TYPE;

	/**
	The Dataset entry is an alias for the, actual individual item which is stored in the dataset, normally
	this would be an association between the data set entry type and the dataset image e.g.
	\code
	typedef std::pair<T, CVtImageBaseClass*>	DATASET_ENTRY;
	\endcode
	*/
	typedef CVthdsLineParser::DATASET_ENTRY				DATASET_ENTRY;
	
	/**
	The Dataset is an alias for the dataset structure itself. Normally this would be some kind of collection class e.g.
	\code
	typedef	std::vector<DATASET_ENTRY >	DATASET;
	\endcode
	*/
	typedef CVthdsLineParser::DATASET							DATASET;
	
	//! This is reference to the dataset contained and owned by the parser.
	CVtDataset<DATASET_ENTRY_TYPE>								&m_dataset;

	//! This is a reference to the one and only driver object, this is owned by the singleton system object.
	CVtUsbDriver																	&m_driver;

	/**
	The firmware has a set of commands associated with each interface, the code pair structure associates the
	numeric code values with a symbolic string name representing the command. 
	*/
	typedef std::pair<std::string, vt_ulong>			CODE_PAIR_ENTRY;

	/**
	The associations of symbolic names are stored in a map. Which is initialised when the object is created.
	This allows for the use of more  meaningful code, such as;-

	\code
	send_command( std::string( "VR_SOFTWARE_TRIGGER" ) );
	\endcode
	
	The send_command(std::string&) function can "lookup" the appropriate numeric value from the table.
	*/
	typedef std::map<std::string, vt_ulong>				CODE_PAIRS;
	
	CODE_PAIRS m_codes; //!< The table of command associations.

	/**
	A darkframe (a frame which was capture when the sensor was not illuminated by x-rays) is included in 
	each hds dataset acquired. The most recently acquired dark frame is held in this top level
	API object for convenience. Currently they are not used at this level. It may make sense to move this 
	dark frame object down to the calibration object level in the future.
	\todo
	Review moving this object down to the calibration level
	*/
	CVtImage<vt_acq_im_type>				m_dark; //!< The current dark frame

	/**
	When each hds sensor is calibrated a boolean mask image is created indicating where the 
	each hds dataset acquired. The most recently acquired dark frame is held in this top level
	API object for convenience. Currently they are not used at this level. It may make sense to move this 
	dark frame object down to the calibration object level in the future.
	\todo
	Review moving this object down to the calibration level
	*/

	CVtImage<vt_byte>								m_mask; //!< The current mask frame
	
	CVthdsCalib<vt_acq_im_type, vt_double	, vt_byte>						m_calib;

	//
	// Initialise reconstruction and globals in base class
	//
	CVthdsImpAPI(const API_TYPE api
							, CVtUsbDriver &driver
							, CVtDataset<DATASET_ENTRY_TYPE>   &dataset
							) : CVtAPI( api )
								, m_driver( driver )
								, m_dataset( dataset )
								, m_calib( m_dataset, m_dark, m_mask )
	{
		set_api_params();			// parameters which depend on api
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );
	}
  ///
  // Destructor
  //
  virtual ~CVthdsImpAPI() {}

  //
  // Initialise the system
	// Note all the main objects for the system should be initialised at this level
	// even the object embedded in other objects such as the driver object.
	//
  //
  virtual vt_bool init()
  {
		static vt_bool initialised = false;

		delete_dataset(); // get rid of previous images

		if (initialised) // only initialise a system once
			return true;

		initialised = true;


		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture:::Memory problem detected\n" );		
		///
		// initialise the driver stuff
		//
		m_driver.init(this);

		///
		// read in calibration stuff
		//
		try {
				std::ifstream CalDataStream( get_calib_fname(), std::ios_base::binary );

				Vt_precondition(CalDataStream.good() ? true : false, "CVthdsCalib::operator >> - Failed to open input file");
				if (!m_quiet)
				{
					if (m_apiType == HDS15_API)
						printf( "Read hds 1.5 calibration data....\n" );
					else
						printf( "Read hds 2.0 calibration data....\n" );
				}

				CalDataStream >> m_calib; // read in calibration data
				
				// Close the new file stream
				CalDataStream.close();
		}
		catch (std::exception &)
		{
			// could trap this here if reqd
			Vt_fail( "no calibration file available. A calibration run must be performed to obtain calibrated images" );
		}


		// OK compare hardware info read from calibration to hardware info read from hardware
		get_hw_info(); // read hardware info from eprom


		// uncommment next two lines if we wish to force current calibration files to match 
		// current sensor info.
		
		// debug m_calib.m_hw_info = m_hw_info; // copy current hw_info just optained
		// debug m_calib.save( get_calib_fname() ); // save same to disk

		if (m_hw_info != m_calib.m_hw_info)
			Vt_fail( "Calibration file hardware information does not match EPROM hardware information\n" );


		// initialise main listening pipe
		if (!m_quiet)
			printf( "Initialising pipe data....\n" );

		return true;
	}

	/**
	 simulate capture based on file image rather than grabbed from system.
	*/
	virtual void capture(std::string& fname)
	{
		// TODO implementation of file input
	}

	virtual void capture_dark()
	{
		// Capture dark frame
		// read dark frame
		// send I want one frame command
		arm( 1 );

		// send a software trigger to cause dark frame to be readout
		soft_trigger();

		// read image data
//		m_driver.read_pipe(); // read one image
		
		// OK reset FPGA
		reset();
	}

	virtual void capture_bright()
	{
		arm( m_dataset_size );

		// capturing bright frames
		soft_trigger();  // this would not normally  be here - trigger for bright 
										 // frames normally from x-ray.
		// wait for tx when data acquired
		wait_for_start();

		// OK read n images worth all at the same time
		m_driver.read_pipe( m_dataset_size ); // read n frame and add to dataset

		// OK read the info
		reset();
	}

	/**
	\brief Main capture routine
	*/
	virtual void capture()
	{
		Vt_postcondition( _CrtCheckMemory() == TRUE, "Capture::Memory problem detected\n" );
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );

		// set the reset voltage
		reset(); send_command( std::string( HDS_DEFAULT_RESET_VOLTAGE	) );

		capture_dark();

		capture_bright();

		if (!m_quiet)
			printf( "Control Port is %x\n",ctrl_port());
	}

	/**
	\brief Apply calibration to acquired dataset

	The calibration routines have direct access to the dataset via the reference to the current
	dataset which is held in this class. 
	*/
	
	virtual void calibrate()
	{
		// OK apply calibration to each line
		CVtImage<vt_out_im_type>* cal_im = new CVtImage<vt_out_im_type>(m_out_width, m_image_height);

		m_calib( *cal_im, m_dataset_size ); // currently default to using all the images.

		set_hw_info( m_calib.m_hw_info );
	}

	/**	
	
	A utility function used to print a particular byte as a series of 1 and 0
	
	*/
	void print_byte(const vt_byte val)
	{
		for(vt_ulong bit=0; bit < 8; bit++)
		{
			if (bit == 4) 
				printf( " " );

			if ((val << bit) & 0x80 )
				printf( "1" );
			else
				printf( "0" );
		}
	}
	/**
		Debug code
	
		Currently this routine is merely a place holder for any debugging code.

	*/
	void test( const CODE_PAIRS::iterator &cmd )
	{
		std::string cmd_str = (*cmd).first;
		vt_ulong	  cmd_code = (*cmd).second;

		// print out command string
		printf( "Command %s  ", cmd_str.c_str() );

		// print out command code
		printf( " %0x ", cmd_code ); print_byte( cmd_code );

		// send command to USB
		if (send_command( cmd_str ))
		{
			printf( " returned OK " );
		}
		else
		{
			printf( " problem " );
		}
		// print value of port
		vt_ulong ctrl = ctrl_port();
		print_byte( ctrl );

		if (ctrl != cmd_code )
		{
			printf( " mismatch!!" );
			send_command( cmd_str ); // try again
		}
		printf( "\n" );
	}

	/**
	The overall interface model is extremely simple

	acquire
	process
	save

	This function should be used to implement the process part.

	Currently its doing debug duty.
	*/

	virtual void process()
	{
		printf( "Initial interface test\n" );
		capture();

		arm( 0 );

		vt_byte port_a = m_driver.hs_port();

		printf( "port A %0x ", port_a ); print_byte( port_a ); printf( "\n");

		for (vt_byte val = 0; val < 0xff; val++)
		{
			set_port( val, 0xa );
			
			vt_byte port_a = m_driver.hs_port();

			printf( "val %0x port A %0x ", val, port_a ); print_byte( port_a ); printf( "\n");

			set_port( val, 0xe );
			
			vt_byte port_e = m_driver.data_port();

			printf( "val %0x port E %0x ", val, port_e ); print_byte( port_e ); printf( "\n");
			getchar();
		}

		wait_for_start();
		capture();

		for(CODE_PAIRS::iterator it = m_codes.begin();	 it != m_codes.end();  ++it )
		{
			if ( !(*it).first.empty())
			{
				test( it );
				getchar();
			}
		}

	}

	/**
	Main processing function

	Currently this merely consist of application of the calibration routines for the 
	intra-oral.

	\param imtype - this is not really applicable for intra-oral as they will alway be the acquired image
	types.
	*/
	virtual void process(IM_TYPE imtype)
	{
		if (!m_quiet)
			std::cout << "OK - images acquired" << std::endl;
		
		calibrate();
	}

	/**
	 main calibration run
	*/
	void calibration_run()
	{
		m_calib.calibration_run( *this );
	}

	/**
	 set the api to have required characteristics
	 for the current binning mode and api type
	*/
	virtual void set_api_params()
	{
		m_fname_base = HDS_DEFAULT_BASE_DIR;
		switch(m_apiType)
		{
			case HDS15_API:
				m_out_width			= HDS15_SIZE_WIDTH;
				m_image_height	= HDS15_SIZE_HEIGHT;
				m_dataset_size  = HDS15_DATASET_SIZE;
				break;
			case HDS20_API:
				m_out_width			= HDS20_SIZE_WIDTH;
				m_image_height	= HDS20_SIZE_HEIGHT;
				m_dataset_size  = HDS20_DATASET_SIZE;
				break;
			default:
				Vt_fail( "Invalid API type\n" );
				break;
		}

		set_num_pkts();
		set_calib_fname();
		set_command_codes();
	}

	/**
	interface type accessor
	*/
	virtual API_TYPE get_api_type()
	{
		return m_apiType;
	}
	
	/**
	 what does the hardware think that the device is?
	*/
	virtual API_TYPE  hw_device_type() 
	{
		return m_driver.hw_device_type();
	}

	virtual char * get_fwfname()
	{
		return HDS_DEFAULT_HEX_FW_FNAME;
	}

	/**
	General control port access functions
	*/
	virtual vt_byte ctrl_port() 
	{
		return m_driver.ctrl_port();
	}

	virtual vt_byte hs_port() 
	{
		return m_driver.hs_port();
	}

	virtual vt_byte data_port() 
	{
		return m_driver.data_port();
	}

	/**
	\brief Arm the sensor ready to receive x-rays
	*/ 
	virtual vt_bool arm(const vt_ulong frames )
	{
		vt_ulong cmd = get_command( std::string( "VR_ARM_INF" ) ) + frames;
		return send_command( cmd );
	}

	/**
	\brief Send the a software trigger to the sensor.
	*/ 
	virtual vt_bool soft_trigger()
	{
		return send_command( std::string( "VR_SOFTWARE_TRIGGER" ) );
	}

	/**
	\brief Send the a software trigger to the sensor.
	*/ 
	virtual vt_bool reset()
	{
		return send_command( std::string( "VR_IUSBI_RESET" ) );
	}

	/**
	\brief Send the a command to set a port to a particular value
	*/ 
	virtual vt_bool set_port(const vt_byte val, const vt_byte port )
	{ 
		return send_command( get_command( std::string( "VR_SET_PORT" ) ), val + ( port << 8 ) );
	}

	/**
	\brief Get information about the current sensor status
	
	The sensor contains an Eprom which is initialised as calibration time to contain certain information.
	This routine downloads this information from the sensor and saves the information in the member variable
	m_hw_info.
	*/
	virtual vt_bool get_hw_info()
	{
		vt_byte buffer[16];
		vt_byte					CommandCode		 = get_command( std::string( "VR_GET_SENSOR_INFO" )  );
		const vt_uint16 SubCommandCode = DEFAULT_SUB;

		vt_bool ret_code = send_command( (vt_byte *) buffer, CommandCode, SubCommandCode );
		m_hw_info =  buffer; 

		return ret_code;
	}

	/**
	\brief Set information about the current sensor status
	
	The routine initializes the sensor Eprom at calibration time to contain information about the hardware.
	This routine uploads this information to the sensor from that stored in m_hw_info; 
	m_hw_info.
	*/
	virtual vt_bool set_hw_info()
	{
		return m_driver.send_data((const vt_byte *) m_hw_info
														, SENSOR_INFO_SIZE
														, get_command( std::string( "VR_SET_SENSOR_INFO" ) ) 
														);
	}

	virtual vt_bool set_hw_info(SENSOR_INFO &hw_info )
	{
		m_hw_info = hw_info;
		return m_driver.send_data((const vt_byte *) m_hw_info
														, SENSOR_INFO_SIZE
														, get_command( std::string( "VR_SET_SENSOR_INFO" ) ) 
														);
	}


	/**
	 get image height - returns image height
	 currently all the images are the same height
	*/
	virtual vt_ulong image_width()
	{
		return m_out_width;
	}

	virtual vt_ulong image_width(IM_TYPE im_type) 
	{
		return m_dataset.image_width( im_type );
	}

	virtual vt_ulong image_height()
	{
		return m_image_height;
	}

	virtual vt_ulong image_height(IM_TYPE im_type) 
	{
		return m_dataset.image_height( im_type );
	}

public:
	/************************************
		 DATASET Manipulation
	************************************/

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
	// Datasets can be any 2d set of data.
	//
	void add_dataset( DATASET_ENTRY_TYPE ent_type, CVtImageBaseClass *pdata )
	{
		m_dataset.add_dataset( ent_type, pdata );
	}

	DATASET_ENTRY pop_back( const IM_TYPE im_type )
	{
		return m_dataset.pop_back( im_type );
	}

	DATASET_ENTRY get_back( const IM_TYPE im_type )
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
		std::string fname_base( HDS_DEFAULT_BASE_FNAME );

		vt_ulong fname_cnt = 1;

		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++, fname_cnt++)
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
	
	//
	// save data for a particular image type
	//
	virtual void save(IM_TYPE imtype, std::string &fname_base)
	{
		vt_ulong fname_cnt = 1;
		
		DATASET::iterator it = m_dataset.end(); it--;
		for(;; it--, fname_cnt++)
		{
			if ( (*it).first.type == imtype)
			{
				switch(imtype)
				{
					case ACQ_IM:
					{
						CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
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
						break;
					}
					case CENTRE_IM:
					{
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
					case OUTPUT_IM:
					{
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
					case RECON_IM:
					{
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
				}
			}

			if (it == m_dataset.begin())
				return;
		}
		Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
	}
	
	
	///
	// HARDWARE CTRL ROUTINES
	//
	//
	// note: currently start is active low on the a paritcular pin of the usb
	// Hence the following code merely polls the pin looking for the active low state.
	//
	virtual START_SIG  wait_for_start(const vt_double wait_time, const vt_double min_wait_time = 0.0 )
	{
		Vt_precondition( m_driver.driver_handle() != NULL, "Device not initialised can't query ready status\n" );
		clock_t start = clock();

		while( !m_driver.hds_start() ) // active high
		{
			clock_t end = clock();
			double time_taken = ((double)(end - start))/CLOCKS_PER_SEC;

			if (time_taken > wait_time)
				return START_SIG_TIMEOUT;

			if (!m_quiet)
				rotator();
		}
		if (!m_quiet)
			printf( "OK\n" );

		return START_SIG_RECEIVED;
	}

	virtual START_SIG  wait_for_start() // in milliseconds
	{
		return wait_for_start( 1000000 );
	}

	///
	// The amount of data to skip at the beginning of acquiring each image. On the current 
	// TDI mode ccd sensor the initial frames are saturated and hence should be discarded.
	//
	virtual vt_ulong get_header_size()
	{
		return HDS_DEFAULT_HDR_SIZE;
	}

	//
	// command codes
	//
	CODE_PAIRS& get_command_codes()
	{
		return m_codes;
	}
	
	//
	// HARDWARE COMMS ROUTINES
	//
	/**
	\brief utility used by send command
	*/
	vt_byte get_command(const std::string &cmd )
	{
		CODE_PAIRS::iterator it = m_codes.find( cmd );
		
		return(*it).second;
	}

	virtual vt_bool send_command(vt_byte status[16]
									 , const vt_byte CommandCode
									 , const vt_uint16 SubCommandCode )
	{
		return m_driver.send_command( status, CommandCode, SubCommandCode );
	}

	virtual vt_bool send_command(const std::string &cmd )
	{
		vt_byte status[16];
		
		vt_byte					CommandCode		 = get_command( cmd );
		const vt_uint16 SubCommandCode = DEFAULT_SUB;

		return send_command( status, CommandCode, SubCommandCode );
	}

	virtual vt_bool send_command( const vt_byte CommandCode, const vt_uint16 SubCommandCode = DEFAULT_SUB )
	{
		vt_byte status[16];
		return m_driver.send_command( status, CommandCode, SubCommandCode );
	}

	//
	// bin mode effectively set the height and width of the image
	//
	virtual void set_calib_fname()
	{
		m_calibFname = (m_apiType == HDS15_API) ? HDS_DEFAULT_HDS15_CALIB_FNAME : HDS_DEFAULT_HDS20_CALIB_FNAME;
	}
	
	virtual char* get_calib_fname()
	{
		set_calib_fname(); //  make sure name is set to default for acquisition mode
		return m_calibFname;
	}

	///
	// private funcs - not exposed through API but useable
	//
	virtual vt_ulong get_num_pkts()
	{
		set_num_pkts(); // ensure that the number of packets is set to the default for the acqusition mode
		return m_numPkts;
	}
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
			m_numPkts = (m_apiType == HDS15_API) ? HDS15_CALIB_NUM_PKTS : HDS20_CALIB_NUM_PKTS;
		}
		else
		{
			m_numPkts = (m_apiType == HDS15_API) ? HDS15_NUM_PKTS : HDS20_NUM_PKTS;
		}
	}
private:
	//
	// set_interface_codes
	//
	void set_command_codes()
	{
		CODE_PAIR_ENTRY myPair;
		//
		// reset frequencies
		//
		myPair.first  = "VR_READOUT_FREQUENCY_2_0MHZ";
		myPair.second = VR_READOUT_FREQUENCY_2_0MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_2_5MHZ";
		myPair.second = VR_READOUT_FREQUENCY_2_5MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_3_0MHZ";
		myPair.second = VR_READOUT_FREQUENCY_3_0MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_3_5MHZ";
		myPair.second = VR_READOUT_FREQUENCY_3_5MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_4_0MHZ";
		myPair.second = VR_READOUT_FREQUENCY_4_0MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_4_5MHZ";
		myPair.second = VR_READOUT_FREQUENCY_4_5MHZ;
		m_codes.insert(myPair);

		myPair.first  = "VR_READOUT_FREQUENCY_5_0MHZ";
		myPair.second = VR_READOUT_FREQUENCY_5_0MHZ;
		m_codes.insert(myPair);
		//
		// ARM
		//
		myPair.first  = "VR_ARM_INF";
		myPair.second = VR_ARM_INF;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_1";
		myPair.second = VR_ARM_1;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_2";
		myPair.second = VR_ARM_2;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_3";
		myPair.second = VR_ARM_3;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_4";
		myPair.second = VR_ARM_4;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_5";
		myPair.second = VR_ARM_5;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_6";
		myPair.second = VR_ARM_6;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_7";
		myPair.second = VR_ARM_7;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_8";
		myPair.second = VR_ARM_8;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_9";
		myPair.second = VR_ARM_9;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_10";
		myPair.second = VR_ARM_10;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_11";
		myPair.second = VR_ARM_11;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_12";
		myPair.second = VR_ARM_12;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_13";
		myPair.second = VR_ARM_13;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_14";
		myPair.second = VR_ARM_14;
		m_codes.insert(myPair);

		myPair.first  = "VR_ARM_15";
		myPair.second = VR_ARM_15;
		m_codes.insert(myPair);
		//
		// general codes
		//
		myPair.first  = "VR_SOFTWARE_TRIGGER";
		myPair.second = VR_SOFTWARE_TRIGGER;
		m_codes.insert(myPair);

		myPair.first  = "VR_SHUTDOWN";
		myPair.second = VR_SHUTDOWN;
		m_codes.insert(myPair);

		myPair.first  = "VR_WRITE_SENSOR_INFO";
		myPair.second = VR_WRITE_SENSOR_INFO;
		m_codes.insert(myPair);

		myPair.first  = "VR_ABORT";
		myPair.second = VR_ABORT;
		m_codes.insert(myPair);

		myPair.first  = "VR_GET_SENSOR_INFO";
		myPair.second = VR_GET_SENSOR_INFO;
		m_codes.insert(myPair);
		//
		// reset voltages
		//
		myPair.first  = "VR_RESET_VOLTAGES_1_8V";
		myPair.second = VR_RESET_VOLTAGES_1_8V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_1_9V";
		myPair.second = VR_RESET_VOLTAGES_1_9V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_0V";
		myPair.second = VR_RESET_VOLTAGES_2_0V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_1V";
		myPair.second = VR_RESET_VOLTAGES_2_1V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_2V";
		myPair.second = VR_RESET_VOLTAGES_2_2V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_3V";
		myPair.second = VR_RESET_VOLTAGES_2_3V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_4V";
		myPair.second = VR_RESET_VOLTAGES_2_4V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_5V";
		myPair.second = VR_RESET_VOLTAGES_2_5V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_6V";
		myPair.second = VR_RESET_VOLTAGES_2_6V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_7V";
		myPair.second = VR_RESET_VOLTAGES_2_7V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_8V";
		myPair.second = VR_RESET_VOLTAGES_2_8V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_2_9V";
		myPair.second = VR_RESET_VOLTAGES_2_9V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_0V";
		myPair.second = VR_RESET_VOLTAGES_3_0V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_1V";
		myPair.second = VR_RESET_VOLTAGES_3_1V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_2V";
		myPair.second = VR_RESET_VOLTAGES_3_2V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_3V";
		myPair.second = VR_RESET_VOLTAGES_3_3V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_4V";
		myPair.second = VR_RESET_VOLTAGES_3_4V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_5V";
		myPair.second = VR_RESET_VOLTAGES_3_5V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_6V";
		myPair.second = VR_RESET_VOLTAGES_3_6V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_7V";
		myPair.second = VR_RESET_VOLTAGES_3_7V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_8V";
		myPair.second = VR_RESET_VOLTAGES_3_8V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_3_9V";
		myPair.second = VR_RESET_VOLTAGES_3_9V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_0V";
		myPair.second = VR_RESET_VOLTAGES_4_0V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_1V";
		myPair.second = VR_RESET_VOLTAGES_4_1V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_2V";
		myPair.second = VR_RESET_VOLTAGES_4_2V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_3V";
		myPair.second = VR_RESET_VOLTAGES_4_3V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_4V";
		myPair.second = VR_RESET_VOLTAGES_4_4V;
		m_codes.insert(myPair);

		myPair.first  = "VR_RESET_VOLTAGES_4_5V";
		myPair.second = VR_RESET_VOLTAGES_4_5V;
		m_codes.insert(myPair);

		myPair.first  = "VR_IUSBI_TEST";
		myPair.second = VR_IUSBI_TEST;
		m_codes.insert(myPair);

		myPair.first  = "VR_IUSBI_RENUM";
		myPair.second = VR_IUSBI_RENUM;
		m_codes.insert(myPair);

		myPair.first  = "VR_IUSBI_GET_USB_STATE";
		myPair.second = VR_IUSBI_GET_USB_STATE;
		m_codes.insert(myPair);

		myPair.first  = "VR_IUSBI_INITIALISE";
		myPair.second = VR_IUSBI_INITIALISE;
		m_codes.insert(myPair);

		myPair.first  = "VR_IUSBI_RESET";
		myPair.second = VR_IUSBI_RESET;
		m_codes.insert(myPair);

		//
		// port commands
		//
		myPair.first  = "VR_SET_PORT";
		myPair.second = VR_SET_PORT;
		m_codes.insert(myPair);

		myPair.first  = "VR_CLEAR_A";
		myPair.second = VR_CLEAR_A;
		m_codes.insert(myPair);

		myPair.first  = "VR_CLEAR_C";
		myPair.second = VR_CLEAR_C;
		m_codes.insert(myPair);

		myPair.first  = "VR_CLEAR_E";
		myPair.second = VR_CLEAR_E;
		m_codes.insert(myPair);

	}
};
} // end Vt namespace

#endif // __CVTHDSIMPAPI_H__
