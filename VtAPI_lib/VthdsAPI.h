/** \file VthdsAPI.h

	This is the main interface definition file for the hds sensor.

 Copyright Innovative Physics (c) 2013 by
 All Rights Reserved


 REVISIONS: 
 $Log: $
*
*/

#ifndef __CVTHDS_API_H__
#define __CVTHDS_API_H__

#include "VtSysdefs.h"

#ifdef WIN32
#pragma message( "C++ compile" )
#endif // WIN32

namespace Vt
{

	
//! default file names
#define HDS_DEFAULT_BASE_DIR							"c:/HDS/"
	
//! Information log filename, primarily used for 
#define HDS_DEFAULT_INFO_LOG_FNAME				HDS_DEFAULT_BASE_DIR "info.log"
#define HDS_DEFAULT_EXP_FNAME							HDS_DEFAULT_BASE_DIR "exp.log"
#define HDS_DEFAULT_DATA_LOG_FNAME				HDS_DEFAULT_BASE_DIR "data_err.log"
	
//! Default calibration filename for hds15
#define HDS_DEFAULT_HDS15_CALIB_FNAME			HDS_DEFAULT_BASE_DIR	"hds15.hcl"  
//! Default calibration filename for hds20
#define HDS_DEFAULT_HDS20_CALIB_FNAME			HDS_DEFAULT_BASE_DIR	"hds20.hcl" 

//! This defines the filename which indicates that the system is a size 1.5 detector.	
#define HDS15_PRESENT_FILE								HDS_DEFAULT_HDS15_CALIB_FNAME
//! This defines the filename which indicates that the system is a size 2.0 detector.	
#define HDS20_PRESENT_FILE								HDS_DEFAULT_HDS20_CALIB_FNAME
/**
The firmware is downloaded as required. This defines the filename for the firmware file.
*/
#define HDS_DEFAULT_HEX_FW_FNAME					HDS_DEFAULT_BASE_DIR	"HDSUSB.hex"

/**
 Used as the default base file name should a data set be saved to disc as raw files i.e.
 the filenames will follow the sequence 
		-"HDS_DEFAULT_BASE_FNAME"_00000.raw
		-"HDS_DEFAULT_BASE_FNAME"_00001.raw
		-"HDS_DEFAULT_BASE_FNAME"_00002.raw
		... etc
*/
#define HDS_DEFAULT_BASE_FNAME						"HDS_"

//! The default directory where intermediate calibration files will be stored.
#define HDS_CALIB_BASE_DIR								HDS_DEFAULT_BASE_DIR "calib"

//! Where are the bright frames stored.
#define HDS_CALIB_BRIGHT_FNAME_BASE				HDS_CALIB_BASE_DIR	 "\\"
//! Where are the dark frames stored.
#define HDS_CALIB_DARK_FNAME_BASE					HDS_CALIB_BASE_DIR	 "\\"

//! The default reset voltage
#define HDS_DEFAULT_RESET_VOLTAGE					"VR_RESET_VOLTAGES_1_9V"

/**
Sensor information, contains the information that is to be written to or read from the
EEPROM in the hds sensor connector.
*/
#define SENSOR_INFO_SIZE 16

struct VTAPI_API sensor_info 
{
	vt_byte buf[SENSOR_INFO_SIZE];

	enum {
		DAY_MASK = 0xf800
			, MONTH_MASK = 0x0780
			, YEAR_MASK  = 0x007f
	};
	unsigned __int32	serial_number;      //!< 32 serial number, this allows for 4,294,967,295 sensors.
	unsigned __int16  manufacturing_date; //!< (day) 5bit, (mon) 4bit, year (7bit - starts from 2000)
																				//!< Hence, we have every date between
																				//!< 00001, 0001, 0000000 1st Jan 2000 and
																				//!< 11111, 1100, 1111111, is 31st Dec 2127
	unsigned __int8   sensor_type;				//!< Allow for version number or product number
	unsigned __int16  row;								//!< How many rows does the final image have allowing for all asics and large pixels etc.
	unsigned __int16  col;								//!< How many columns does the final image have allowing for all asics and large pixels etc.
	unsigned __int8   size;								//!< 4bit - Allow for 16 sizes
	unsigned __int8   location;						//!< 4bit - Allow for 16 manufacturing location codes. This allows for 16 simultaneous manufacturing locations.
																				//!< this could be combined with with manufacturing date to allow for more than 16 manufacturing 
																				//!< locations over time.
	unsigned __int16  detector_batch;			//!< This allows for 65535 detector batches, this should be sufficient. However, 
																				//!< if more than this are required, this detector batch code shoud be combined with manufacturing data.
																				//!< this then gives 64K batches per day.
	unsigned __int16  asic_batch;					//!< This allows for 65535 detector batches, this should be sufficient. ditto detectors, 

	sensor_info(): serial_number( 0x12345678 )		//!< Probably a good idea if the serial numbers should start with some 
																								//!< for the initial batch - no one likes to have the first sensors.
								 , manufacturing_date( 0xDC83 )	//!< 27th Sept 2013
								 , sensor_type( 0x00 ) //!< Version 0 - pre-production
								 , row( 688 )					 //!< 344*2 
								 , col( 944 )					 //!< 472*2
								 , size( 1 )					 //!< 0 -> 1, 1-> 1.5, 2 - 2.0, other size TBD
								 , location( 0 )			 //!< No locations yet defined 0 - preproduction
								 , detector_batch( 0 ) //!< Initial preproduction batches -> 0
								 , asic_batch( 0 )		 //1< Initial preproduction batches -> 0
	{}
	
	/**
	Extract the day from the manufacturing date
	*/
	vt_long day()
	{
		return ( manufacturing_date & DAY_MASK ) >> 11;
	}

	/**
	Extract the month from the manufacturing date
	*/
	vt_long month()
	{
		return ( manufacturing_date & MONTH_MASK ) >> 7;
	}

	/**
	Extract the year from the manufacturing date
	*/
	vt_long year()
	{
		return ( manufacturing_date & YEAR_MASK );
	}

	/**
	Convert a buffer read from hw
	*/
	vt_ulong byte_shift( vt_byte byte, vt_ulong bytenum )
	{
		return (byte) << 8*bytenum;   
	}

	struct sensor_info& operator = (vt_byte buf[SENSOR_INFO_SIZE])
	{
		vt_long byte_num = 0;
		vt_byte *byte = (vt_byte *) &serial_number;

		serial_number  = byte_shift( buf[byte_num++], 3 );
		serial_number += byte_shift( buf[byte_num++], 2 );
		serial_number += byte_shift( buf[byte_num++], 1 );
		serial_number += byte_shift( buf[byte_num++], 0 );

		manufacturing_date = byte_shift( buf[byte_num++], 1 );
		manufacturing_date += byte_shift( buf[byte_num++], 0 );

		sensor_type = byte_shift( buf[byte_num++], 0 );

		row  = byte_shift( buf[byte_num++], 1 );
		row += byte_shift( buf[byte_num++], 0 );

		col  = byte_shift( buf[byte_num++], 1 );
		col += byte_shift( buf[byte_num++], 0 );

		vt_byte locsize = buf[byte_num++];
		size		 = locsize & 0x0f;
		location = (locsize & 0xf0) >> 4;

	  detector_batch  = byte_shift( buf[byte_num++], 1 );
	  detector_batch += byte_shift( buf[byte_num++], 0 );

	  asic_batch  = byte_shift( buf[byte_num++], 1 );
	  asic_batch += byte_shift( buf[byte_num++], 0 );

		return *this;
	}

	operator vt_byte* ()
	{
		vt_long byte_num = 0;

		buf[byte_num++] = ( serial_number & 0xff000000 ) >> 3*8;
		buf[byte_num++] = ( serial_number & 0x00ff0000 ) >> 2*8;
		buf[byte_num++] = ( serial_number & 0x0000ff00 ) >> 1*8;
		buf[byte_num++] = ( serial_number & 0x000000ff );

		buf[byte_num++] = ( manufacturing_date & 0xff00 ) >> 1*8;
		buf[byte_num++] = ( manufacturing_date & 0x00ff );

		buf[byte_num++] = sensor_type;

		buf[byte_num++] = ( row & 0xff00 ) >> 1*8;
		buf[byte_num++] = ( row & 0x00ff );
	
		buf[byte_num++] = ( col & 0xff00 ) >> 1*8;
		buf[byte_num++] = ( col & 0x00ff );

		buf[byte_num++] = ( size & 0x0f ) | (location & 0x0f) << 4;

	  buf[byte_num++] = ( detector_batch & 0xff00 ) >> 1*8;
	  buf[byte_num++] = ( detector_batch & 0x00ff );

	  buf[byte_num++] = ( asic_batch & 0xff00 ) >> 1*8;
	  buf[byte_num++] = ( asic_batch & 0x00ff );

		return buf;
	}

	///
	// Equivalence ops
	//
	vt_bool operator != (struct sensor_info& rhs)
	{
		if (serial_number != rhs.serial_number)
			return true;

		if (manufacturing_date != rhs.manufacturing_date)
			return true;

		if (sensor_type != rhs.sensor_type)
			return true;

		if (row != rhs.row)
			return true;

		if (col != rhs.col)
			return true;

		if (size != rhs.size)
			return true;

		if (location != rhs.location)
			return true;

		if (detector_batch != rhs.detector_batch)
			return true;

		if (asic_batch != rhs.asic_batch)
			return true;

		return false;
	}

	vt_bool operator == (struct sensor_info& rhs)
	{
		if (*this != rhs) // could have done !(*this != rhs), I thought this was more transparent
			return false;

		return true;
	}
	///
	// return the structure as a buffer 
	//
	vt_byte * begin()
	{
		return *this;
	}

	vt_ulong length()
	{
		return SENSOR_INFO_SIZE;
	}
};

typedef sensor_info SENSOR_INFO;
/**

This object is a wrapper for all the paramter which can be set at the general hds API level. 
Any default should for these parameters should be set in the constructor of this structure. 
The main API routine contains one of these structures and exposes the internals using references. 
Consequently, when the object is created it will have all the default values outlined in the constructor.
Therefore the default values for the constructor for these parameters is the default values of
the paramters for the hds API parameters.

*/
	
typedef struct VTAPI_API CVtHDS_API_PARAMS {
	vt_ulong			m_dataset_size;	//!< How many images will be acquired in a particular dataset.
	SENSOR_INFO   m_hw_info;
	
	CVtHDS_API_PARAMS() : m_dataset_size( 0 ) //!< needs to set in set_api_params - see main class
	{}
} HDS_API_PARAMS;


/**
\brief Main hds API class

This is the main hds API interface class. This class defines the abstract class which 
contains the elements which are specific to the hds APIs. 
*/

class VTAPI_API CVthdsAPI
{
protected:
	/**
	\enum HDS20_SIZE_WIDTH 
	The image width for the size 2.0
	*/
	enum {
			HDS20_SIZE_WIDTH	= 1028 //!< The width of a complete hds20 image, this includes all asics and allowance for big pixels
		,	HDS20_SIZE_HEIGHT = 828  //!< The height of a complete hds20 image, this includes all asics and allowance for big pixels
		, HDS15_SIZE_WIDTH	= 471	 //!< Currently debug value, the size of a single hds15 tile
		,	HDS15_SIZE_HEIGHT = 342	 //!< Currently debug value, the size of a single hds15 tile
		, HDS_DEFAULT_IMAGE_WIDTH	 = 1028 // we need to put in actua values here
		, HDS_DEFAULT_IMAGE_HEIGHT = 828	
		, HDS_DEFAULT_HDR_SIZE	= 0	//!< This is used to determine how much of the initial data tranmitted in the tx data we
																//!< ignore as buffer stuff.
		, PKT_SIZE	= 512
		/**
		The number of packets are derived from the height and width values using the following formula
		(width*height*2 to give number of bytes for the data + (height*2)*2 to allow for sol eol values)
		all devided by the current packet size.
		*/
		,	HDS15_NUM_PKTS = (HDS15_SIZE_WIDTH*HDS15_SIZE_HEIGHT*2 + HDS15_SIZE_HEIGHT*4)/PKT_SIZE
		, HDS20_NUM_PKTS = (HDS20_SIZE_WIDTH*HDS20_SIZE_HEIGHT*2 + HDS20_SIZE_HEIGHT*4)/PKT_SIZE
		, HDS15_CALIB_NUM_PKTS = HDS15_NUM_PKTS // doesn't change for intra-oral
		, HDS20_CALIB_NUM_PKTS = HDS20_NUM_PKTS
		, HDS15_DATASET_SIZE = 10 //! number of images acquired in a dataset
		, HDS20_DATASET_SIZE = 4
	};	
	
	//!< Protected default constructor - not really required to prevent construction as this is an abstrace base class. However
	//!< have a protected constructor merely acts as a reminder that we shouldn't consider making one of these guys.
	CVthdsAPI() : m_dataset_size( m_hds_params.m_dataset_size ) 
							, m_hw_info( m_hds_params.m_hw_info ) 
	{
		m_hds_params = HDS_API_PARAMS(); // set to default values
	}
	virtual ~CVthdsAPI() //!< Currently just a place holder.
	{}

public:
	//! actual parameters
	HDS_API_PARAMS m_hds_params;

	//! parameter aliases
	vt_ulong			&m_dataset_size;
	SENSOR_INFO		&m_hw_info;

	/**
  Initialises the system.

	This is the generic system initialisation. Currently it doesn't take any paramters.
	I'm not sure whether this should remain the case. Perhaps it would be a good idea to include 
	a generic initialisation filename.
	*/
	virtual vt_bool init() = 0;
	
	/**
	\fn capture()
	
	\brief start main data capture thread
	*/

	virtual void capture() = 0;
	virtual void capture(std::string& fname) = 0;
	
	//
	// process()
	//
	// process the captured image
	// 
	virtual void process() = 0;
	
	//
	// save()
	//
	// start main data save
	// 
	//
	virtual void save() = 0;
	
	//
	// calibrate()
	//
	// apply calibration to image
	// 
	//
	virtual void calibrate() = 0;
	//
	// calibrate()
	//
	// start calibration run
	// 
	//
	virtual void calibration_run() = 0;

	//
	// 
	virtual vt_ushort * image_ptr() = 0;
	virtual vt_ulong image_width() = 0;
	virtual vt_ulong image_height() = 0;
	virtual vt_bool  delete_dataset() = 0;
	
	///
	// control information
	//
	virtual vt_byte  ctrl_port() = 0; // queries the sensors ready pin - active low hence true = pin c0 = 0
	
	//*******************************************
	// accessor functions
	//*******************************************
	
	virtual void set_num_pkts()  = 0;// set default num pkts
	virtual vt_ulong get_num_pkts() = 0;
	virtual char* get_calib_fname() = 0;
	
	///
	// set the api to have required characteristics
	// for the current binning mode and api type
	//
	virtual void set_api_params() = 0;
};

} // end Vt namespace
#endif // __CVTHDS_API_H__
