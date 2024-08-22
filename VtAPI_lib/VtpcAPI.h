/** \file VtpcAPI.h

	The main file for definition of the pano and ceph API interface.

* Copyright (c) 2013 by
* All Rights Reserved
*
*
* REVISIONS: 
* $Log: $
*
*/

#ifndef __CVTPCAPI_H__
#define __CVTPCAPI_H__

#include "VtSysdefs.h"

#ifdef WIN32
#pragma message( "C++ compile" )
#endif // WIN32

namespace Vt
{


#define DEFAULT_BASE_DIR							"c:\\Pax300\\"
#define DEFAULT_DARK_FNAME						DEFAULT_BASE_DIR "dark.raw"
#define DEFAULT_BRIGHT_FNAME					DEFAULT_BASE_DIR "bright.raw"
#define DEFAULT_IN_FNAME							DEFAULT_BASE_DIR "infile.raw"

#define DEFAULT_INFO_LOG_FNAME				DEFAULT_BASE_DIR "info.log"
#define DEFAULT_EXP_FNAME							DEFAULT_BASE_DIR "exp.log"
#define DEFAULT_DATA_LOG_FNAME				DEFAULT_BASE_DIR "data_err.log"

#define DEFAULT_PANO_CALIB_FNAME			DEFAULT_BASE_DIR "pano_calib.cal" // should this be backwards compatible with existing systems.
#define DEFAULT_CEPH_CALIB_FNAME			DEFAULT_BASE_DIR "ceph_calib.cal"

#define CEPH_PRESENT_FILE							DEFAULT_CEPH_CALIB_FNAME
#define PANO_PRESENT_FILE							DEFAULT_PANO_CALIB_FNAME

#define DEFAULT_HEX_FW_FNAME					DEFAULT_BASE_DIR "IUSBI.hex"
#define DEFAULT_BASE_FNAME						DEFAULT_BASE_DIR "CCD_"


enum {
	/**
	NUM_PKTS
	The number of 
	*/
	PANO_NUM_PKTS = 14100 //!< The number of packets acquire for a panoramic image acquisition.
		, CEPH_NUM_PKTS = 11000
		, PANO_CALIB_NUM_PKTS = 18000
		, CEPH_CALIB_NUM_PKTS = 13000
		, DEFAULT_CHIP_HEIGHT_BIN2x = 768	
		, PANO_DEFAULT_OUT_IMAGE_WIDTH_BINx2 = 2880
		, PANO_DEFAULT_HALF_INDEX = PANO_DEFAULT_OUT_IMAGE_WIDTH_BINx2/2			// acquired lines
		, CEPH_DEFAULT_OUT_IMAGE_WIDTH_BINx2 = 2500
		, CEPH_DEFAULT_HALF_INDEX = CEPH_DEFAULT_OUT_IMAGE_WIDTH_BINx2/2			// acquired lines
		, DEFAULT_NUM_CHIPS			= 3
		, DEFAULT_IMAGE_HEIGHT	= DEFAULT_CHIP_HEIGHT_BIN2x * DEFAULT_NUM_CHIPS
		, DEFAULT_HDR_SIZE = 160 // formally this was the skip count in line calib
};

/**
Panoramic binning mode
*/
typedef enum {
	INVALID_BIN_MODE
		, BIN1x1
		, BIN2x2
		, BIN1x2
		, BIN2x1
} BIN_MODE;

/**
\struct CVtPANO_API_PARAMS
\par
This object is a wrapper for all the paramters which can be set at the general PANO-CEPH API level. 
Any default for these parameters should be set in the constructor of this structure. 
The main PANO-CEPH pcAPI routine contains one of these structures and exposes the internals using references. 
Consequently, when the object is created it will have all the default values outlined in the constructor.
Therefore the default values for the constructor for these parameters is the default values of
the paramters for the API parameters.
*/

typedef struct VTAPI_API CVtPANO_API_PARAMS {
	BIN_MODE bin_mode;
	vt_ulong numLines;		//!< The number of lines per output file, this is not currently used.
	vt_ulong numChips;		//!< How many chips are currently used on the sensor? This is currently 3 for both the pano and ceph.

	vt_bool Aout;					//!< A flag which states whether or not the data from sensor A is stored to file.
	vt_bool Bout;					//!< B flag which states whether or not the data from sensor A is stored to file.
	vt_bool Cout;					//!< C flag which states whether or not the data from sensor A is stored to file.
	vt_bool invertC;			//!< If we are using a file to simulate input do we invert the C chip on input.
	
	CVtPANO_API_PARAMS() : bin_mode( BIN2x2 )
						, numLines( 20000 )
						, numChips( 3 )
						, Aout( true )
						, Bout( true )
						, Cout( true )
						, invertC( true ) {}
} PANO_API_PARAMS;


/**
\class CVtpcAPI

The main Pano and Ceph API definition class.

This is an abstract base class, which is used to define the parts of the API which relate specifically to
the PANO and CEPH systems. Currently, the API definition structure is split in a hierarchy. The general
purpose 

*/

class VTAPI_API CVtpcAPI
{
public:
		
protected:
	////
	// Protected default constructor, copy constructor,
	// assignment operator and destructor
	//
	CVtpcAPI(const BIN_MODE bin_mode = INVALID_BIN_MODE) : m_bin_mode( m_pano_params.bin_mode )
		,	m_numLines( m_pano_params.numLines )   // potential to split output over a number of files
		,	m_numChips( m_pano_params.numChips )		// how many chips in currrent sensor
		,	m_Aout( m_pano_params.Aout )           // when saving, if true save the information from chip A
		,	m_Bout( m_pano_params.Bout )						// when saving, if true save the information from chip B
		,	m_Cout( m_pano_params.Cout )						// when saving, if true save the information from chip C
		,	m_invertC( m_pano_params.invertC )			// invert the c chip data when reading in files
	{
		m_pano_params  = PANO_API_PARAMS(); // set to default values
		m_bin_mode = bin_mode;
	}
	
	CVtpcAPI(const CVtpcAPI&);
	const CVtpcAPI& operator = (const CVtpcAPI&);
	virtual ~CVtpcAPI() {};
	
public:

	// actual parameters
	PANO_API_PARAMS m_pano_params;
	
	// parameter aliases
	BIN_MODE &m_bin_mode; 

	vt_ulong &m_numLines; // number of lines per output file
	vt_ulong &m_numChips;
	
	vt_bool &m_Aout;
	vt_bool &m_Bout;
	vt_bool &m_Cout;
	
	vt_bool &m_invertC;
	
	 /**
	 * Initialises the system 
	 */
	virtual vt_bool init() = 0;
	
	///
	// capture()
	//
	// start main data capture thread
	// 
	//
	virtual void capture() = 0;
	virtual void capture(std::string& fname) = 0;
	
	///
	// process()
	//
	// process the captured image
	// 
	//
	virtual void process() = 0;
	
	
	///
	// save()
	//
	// start main data save
	// 
	//
	virtual void save() = 0;
	
	///
	// calibrate()
	//
	// calibrate currently captured image
	// 
	//
	virtual void calibrate() = 0;
	
	// calibrate()
	//
	// start calibration run
	// 
	//
	virtual void calibration_run() = 0;
	
	
	///
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
	virtual vt_bool set_binmode_params() = 0;
	virtual void set_api_params() = 0;
};

} // end Vt namespace
#endif //__CVTPCAPI_H__