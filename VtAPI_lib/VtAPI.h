/** \file VtAPI.h

	This is the main API include file. 

	It contains all the definitions and the abstract class definitions which are common to all the APIs

* Copyright (c) 2013 by
* All Rights Reserved
*
*
* REVISIONS: 
* $Log: VtAPI.h,v $
* Revision 1.3  2013/05/27 17:41:57  david
* V3
*
* Revision 1.2  2013/05/27 12:04:18  david
* Ver 1
*
*
*/

#ifndef __CVTAPI_H__
#define __CVTAPI_H__

#include "VtSysdefs.h"

typedef	vt_ushort vt_acq_im_type;		//!< Used the define the data content for the acquired image type
typedef	vt_ushort vt_centre_im_type;//!< Used the define the data content for the centred image type, curently only applicable to pano and ceph.
typedef	vt_ushort vt_calib_im_type; //!< Used the define the data content for the calibrated image type
typedef vt_ushort vt_out_im_type;   //!< Used the define the data content for the output image type
typedef vt_ushort vt_recon_im_type; //!< Used the define the data content for the reconstructed image type, only applicable to pano. Not currently used.

#ifdef __cplusplus 

#ifdef WIN32
#pragma message( "C++ compile" )
#endif // WIN32

#include "VtErrors.h"
#include "VtImage.h"

#include "VtpcAPI.h"
#include "VthdsAPI.h"


namespace Vt
{
/**
*  simple we are still alive rotator
*/
void rotator();
	
/**
*  utility function to make filenames
*/
std::string Fname( std::string &base_fname, vt_ulong fileno );

/**
  Define template function for changing one-d array into 2d-array of pointers
*/
template<class PixelType>
PixelType ** allocLineArray(vt_uint width, vt_uint height);

template<class PixelType>
void freeLineArray(PixelType ** data);
//
//! is the file there
//
vt_bool file_exists( const std::string &fname );


/**

This object is a wrapper for all the paramter which can be set at the general API level. 
Any default should for these parameters should be set in the constructor of this structure. 
The main API routine contains one of these structures and exposes the internals using references. 
Consequently, when the object is created it will have all the default values outlined in the constructor.
Therefore the default values for the constructor for these parameters is the default values of
the paramters for the API parameters.

*/
typedef struct VTAPI_API CVtAPI_PARAMS {
	vt_bool  sync;				//!< Obtain data and parse data simulataneously (use synchronisation objects, currently not used ).
	vt_bool  quiet;				//!< Should we turn off the reporting of info and general information.
	vt_bool  doCommErr;		//!< Should we report communication errors, these can be turned off for debugging.

	vt_ulong image_height;//!< Image height   
	vt_ulong image_width;//!<  Image height   
	
	vt_ulong numBufs;			//!< Number of buffers to be read in 
						
												//! \sa Vt::CVtPipeData, and Vt::allocLineArray

	vt_bool darkFrameCal; //!< The remove darkframe only, calibration mode applicable to pano and ceph only.
	vt_bool calibFlag;		//!< Is this a calibration run.

	vt_char *fname;				//!< Input filename, if we wish to simulate a data capture run using input from a file.

	///
	//! acquisition mode and acquisition mode related parameters
	//
	vt_ulong numPkts;		 //!< the buffer size in number of packets, each packet is 512 bytes
	vt_bool  numPkt_override;  //!< The number of packets are set to default values. If they are set explictly then this flag is set.
	vt_char *calibFname; // current calibration filename

	CVtAPI_PARAMS() : sync( false )
						, quiet( true )
						, doCommErr( true )
						, image_height( 0 ) // needs to be initialised for each device type
						, image_width( 0 )
						, numBufs( 1 )
						, fname( NULL )
						, calibFlag( true )
						, darkFrameCal( false )
						, numPkts( 0 )
						, numPkt_override( false )
						, calibFname( NULL ) {}
} API_PARAMS;


/**

\brief Main API class

This is the main API interface class. This class defines the abstract class which 
contains the elements which common to all the APIs. This is the class of object that 
the Vt::GetAPI() function will return. Of course the class returned will really be
a CVtpcImpAPI or a CVthdsImpAPI, both of which are derived from this class.

*/

class VTAPI_API CVtAPI : public CVthdsAPI, public CVtpcAPI
{
public:
	/**	
	Place holder destructor,
	Currently does nothing, merely allows supper classes to deal with deletion of objects.
	*/
  virtual ~CVtAPI() {};

	/**
	 The actual parameters structure, 
	
	 \sa Vt::API_PARAMS
	*/
	
	API_PARAMS m_api_params;

	/**
	\brief Parameter aliases. 
	@see Vt::API_PARAMS
	*/

	vt_bool  &m_sync;  
	vt_bool  &m_quiet;
	vt_bool  &m_doCommErr;
	vt_ulong &m_image_height; 
	vt_ulong &m_out_width;		
	
	vt_ulong &m_numBufs;			
	vt_bool  &m_darkFrameCal;
	vt_bool  &m_calibFlag;
	vt_char* &m_fname;

	vt_ulong &m_numPkts;
	vt_bool  &m_numPkt_override; 
	vt_char* &m_calibFname;

	/**
	\brief API types

	There are currently four main API types supported for the panoramic, cephelometric, intra-oral 1.5 and 
	intra-oral 2.0 sensors.
	*/
	typedef enum {
			INVALID_API		//!< Default API type.
			, PANO_API		//!< Panoramic API
			, CEPH_API		//!< Cephelometric API
			, HDS15_API		//!< Intra-oral 1.5 HDS sensor.
			, HDS20_API		//!< Intra-oral 2.0 HDS sensor.
			, MAX_API_NUM //!< Currently used to indicate the presence of an unitialised system.
	} API_TYPE; //!<

	
	/*
	\typedef IM_TYPE

	A dataset contains all the images required to produce a final output image. In the case of the 
	hds sensor this can consist of a dark frame plus a number of bright frames. These images are 
	processed to provide the final output image. The Vt::CVtAPI::IM_TYPE contains all the possible image
	types that can be contained in a dataset.

	\sa Vt::CVtDataset
	*/
	typedef enum {
		ACQ_IM				//!< Unprocessed acquired image.
		, CENTRE_IM		//!< Centred image, pano and ceph mode only.
		, CALIB_IM		//!< Calibrated image
		, RECON_IM		//!< Reconstructed image, pano mode only
		, OUTPUT_IM		//!< Final displayed image.
		, CALIB_COEF_IM //!< not currently used - may be use we decide to got to full field correction
	} IM_TYPE;			//!< Image type 

	typedef enum {
		START_SIG_RECEIVED			//!< In the case of the pano-ceph this means that the system is started,
														//!< in the case of the hds sensor it means that x-rays have triggered the sensor.
		, START_SIG_TIMEOUT			//!< A start signal was received almost immediately after requesting an image.
														//!< This is taken as an indication that something has gone wrong.
		, START_SIG_TOOQUICK		//!< This is taken as an indication that something has gone wrong.
	} START_SIG;	//!< States associated with the Vt::VtAPI::wait_for_start() interface.

	/**
	\fn  virtual API_TYPE get_api_type()
	\brief provides external access to the current api type. 
	
	In general much of the functionality of the API should be provide polymorphically. Hence, is possible the
	use of this function should be avoided. 
	*/

	virtual API_TYPE get_api_type()
	{
		return m_apiType;
	}

	/**
  \brief Initialises the system 

	The main system initialisation function.


	*/
	virtual vt_bool init() = 0;

	///
	// capture()
	//
	//! start main data capture thread
	//
	virtual void capture() = 0;
	virtual void capture(std::string& fname) = 0;
	
	///
	// process()
	//
	//! process the captured image
	// 
	virtual void process() = 0;
	virtual void process(IM_TYPE) = 0;
	
	///
	// save()
	//
	//! start main data save
	// 
	//
	virtual void save() = 0;
	
	///
	// calibrate()
	//
	//! apply calibration to image
	// 
	//
	virtual void calibrate() = 0;

	///
	//! wait for starting signal from hardware - this is trigger in the case of
	//! intra-oral - system information about rotation arm in the case of the pano 
	//
	virtual START_SIG  wait_for_start(const vt_double wait_time, const vt_double min_wait_time ) = 0;// in milliseconds

	//
	//! the amount of data to skip at the beginning of the data read
	//
	virtual vt_ulong get_header_size() = 0;

	///
	//! Additional API dependent on IM_TYPE
	// 
	virtual vt_ushort * image_ptr(IM_TYPE) = 0; // returns pointer to first image of desired type
	virtual vt_ushort ** image_ptrs(IM_TYPE) = 0; // returns pointer to first image of desired type
	
	virtual vt_ulong image_width(IM_TYPE) = 0;
	virtual vt_ulong image_width() = 0;
		
	virtual vt_ulong image_height(IM_TYPE) = 0;
	virtual vt_ulong image_height() = 0;


	//*******************************************
	//! accessor functions
	//*******************************************

	virtual void set_num_pkts()  = 0;// set default num pkts
	virtual vt_ulong get_num_pkts() = 0;
	virtual char* get_calib_fname() = 0;

	//
	//! set the api to have required characteristics
	//! for the current binning mode and api type
	//
	virtual void set_api_params() = 0;
	//
	//! control information
	//
	virtual vt_byte  ctrl_port() = 0; // queries the sensors ready pin - active low hence true = pin c0 = 0

	//
	//! Pano and ceph based functions
	//
	virtual void centre(IM_TYPE)			 // Pano and ceph only should not be called from intra-oral interface
	{
		Vt_fail( "centre(IM_TYPE)::Pano and ceph function only should not be called for any other type of interface\n" );
	}
	virtual vt_ulong half()									// Pano and ceph only should not be called from intra-oral interface
	{
		Vt_fail( "half()::Pano and ceph function only should not be called for any other type of interface\n" );
		return 0;
	}
	virtual vt_bool set_binmode_params()
	{
		Vt_fail( "set_binmode_params()::Pano and ceph function only should not be called for any other type of interface\n" );
		return false;
	}
	virtual void calibration_run()
	{
		Vt_fail( "calibration_run::Pano and ceph function only should not be called for any other type of interface\n" );
	}

	virtual char* get_fwfname() = 0;
protected:
  /*! Protected default constructor
	The default constructor for the Vt::CVtAPI class merely sets up the references to the 
	*/

	CVtAPI(const API_TYPE api, const BIN_MODE bin_mode=INVALID_BIN_MODE) 
					: CVtpcAPI( bin_mode ), m_apiType(api) 
					, m_sync( m_api_params.sync )
					, m_quiet( m_api_params.quiet )
					, m_doCommErr( m_api_params.doCommErr )
					, m_image_height( m_api_params.image_height )
					, m_out_width( m_api_params.image_width )	
					, m_numBufs( m_api_params.numBufs	)				
					, m_darkFrameCal( m_api_params.darkFrameCal )
					, m_calibFlag( m_api_params.calibFlag )
					, m_fname( m_api_params.fname )
					//
					//! acquisition mode and acquisition mode related parameters
					//
					, m_numPkts( m_api_params.numPkts	)	 // the buffer size in number of packets
					, m_numPkt_override( m_api_params.numPkt_override )
					, m_calibFname( m_api_params.calibFname ) // current calibration filename
	{
		m_api_params  = API_PARAMS(); //! set to default values, this line is not required merely here to make explicit what is happening
	}
	

	CVtAPI(const CVtAPI&);
  const CVtAPI& operator = (const CVtAPI&);

	///
	// acqisition mode should not be manipulated other than by system control 
	// functions
	//
	API_TYPE m_apiType; 

	char		*m_fname_base;
};

/**
* Instantiates the system returning the objects reference
*/
VTAPI_API CVtAPI& GetAPI( CVtAPI::API_TYPE api, Vt::BIN_MODE bin_mode = Vt::INVALID_BIN_MODE );
VTAPI_API CVtAPI& GetAPI();

/**
* Kills the system ending the run and calling the destructor
*/
VTAPI_API void CloseAPI();
} // end Vt namespace
#endif // __cplusplus 

#endif // __CVTAPI_H__

