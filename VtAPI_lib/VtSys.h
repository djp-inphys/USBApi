/** 
* Copyright (c) 2013 by Innovative Physics
* All Rights Reserved
*
  \mainpage

  \image html vatech.jpg

	\author David Prendergast

  \section sec1 Innovative Physics Sensor API 

  \par REQUIREMENTS: 
	The Innovative Physics API is implemented as an abstract class factory. The class factory generates the
	concrete classes corresponding to the abstract API classes. The abstract API classes 
	Vt::CVtAPI, Vt::CVtpcAPI and Vt::CVthdsAPI. are	defined in three header files, 
	VtAPI.h, VtpcAPI.h and VthdsAPI.h.

	\par
	To use the system all that is required are three steps,
	
					-# include the VtAPI.h header file.
					-# make a call to Vt::GetAPI() somewhere in your code e.g.
							CVtAPI &API = GetAPI( CVtAPI::HDS20_API );
					-# use the API reference to do something e.g.
							API.capture();


  \par SPECIFICATIONS: 
	Create the appropriate concrete objects for the implementation of the API.
	\par 
	Currently the construction of the concrete object uses the following structure
	\par	
	1) Create the pipe_data object, this provides an abstraction for the acquired data
			 The pipe data object is passed to the parser. The pipe object is generic. It is used for all
			 types of raw data ie. pano ceph and hds. The parser is specfic to the API type, there are 
			 currently two types of parser, one which understands data coming from the pano and ceph sensors and 
			 one that understands hds sensor data.
	\par	
	2) Once the parser is created this is passed to the driver object. Like the pipe data object the 
			 driver object is generic and used to access the hardware for all types of interface. The driver object 
			 encapsulates everything to do with the hardware. Effectively, the driver forms the basis of a high level 
			 hardware abstraction layer. The driver is responsible for acquiring the data, by filling in the information 
			 in pipedata. Once pipe data has been filled with data the parser is responsible for interpriting this data
		 	 and turning it into individual lines of the acquired image. Clearly. the parser must know the structure of 
			 the data coming from the various sources. Consequently, the parser type is specific to the particular
			 API type instantiated. Once the data has been parsed it is placed in a dataset by the USB driver.
  
  \note 
	Error handling is managed via the Vt_fail exception generation. Hence, any object calling GetAPI, should 
	catch any potential exceptions. 

	\par 
	The Sys object in addition to being an abstract class factory is implemented around a singleton design pattern. That
	is the sys object is a singleton. However, rather than returning an instance of itself, as might be usual, it returns 
	a copy of the generate API classes contained within itself. Consequently, any subsequent call to GetAPI after the initial
	call will return a reference to the class created on the first call.

	\par Initialisation. 
	The objects created within the system object are not automatically initialised. It is the responsibility of 
	the class user to initialise the API i.e. any attempt to use the class should first be proceed by a
	call to the generic API.init() function.


	@see Vt::CVtSys
	@see Vt::CVtAPI
	@see Vt::CVtpcAPI
	@see Vt::CVthdsAPI
	@see Vt::CVtParser
	@see Vt::CVthdsLineParser
	@see Vt::CVtpcLineParser
	@see Vt::CVtUsbDriver
*
* REVISIONS: 
* $Author: David Prendergast
* $Log: VtSys.h,v $
* Revision 1.3  2013/05/27 17:41:57  david
*
* Revision 1.2  2013/05/27 12:04:19  david
*
*/


#ifndef __CVTSYS_H__
#define __CVTSYS_H__

/**
\namespace Vt::

\brief The main namespace which contains all the Innovative Physics low level API functions.

The Innovative Physics API is split into the a number of different interfaces. These interfaces are defined in the 
following files ;-

	- VtAPI.h
	- VtpcAPI.h
	- VthdsAPI.h
	
	- VtParser.h
	- VtDrvrAPI.h

The first three files define the main api functionality. The remaining two files , VtParser.h and VtDrvr.h
are currently internal interface definitions. However, they could be extended and made available externally.

The API is split over three libraries, these libraries could if required by implemented as DLL rather than static
libraries. The libraries form a hierarchy,

	- lib, which contains the main windriver interface routines. 
	- ez_lib, which contains the cypress interface routines and the contains the main driver HAL definition.
	- VtAPI_lib, contains the main API layer for the Innovative Physics interface.
*/


namespace Vt
{
//*********************************************************************
// utilities
//*********************************************************************

//
//! simple we are still alive rotator
//
void rotator()
{
	static vt_ulong cnt = 0;

	switch(cnt++)
	{
	case 0:
		printf( "\b|");
		break;
	case 1:
		printf( "\b/");
		break;
	case 2:
		printf( "\b-");
		break;
	case 3:
		printf( "\b\\");
		cnt = 0;
		break;
	default:
		cnt = 0;
	}
}

//
//! Utility - utility to obtain current working directory name
//
std::string GetWorkingDirectory()
{
		vt_char Buffer[2056];
		return std::string ( _getcwd(Buffer, 2056) );
}

////
//! utility function to make filenames
//
std::string Fname( std::string &base_fname, vt_ulong fileno )
{
	std::string fname(GetWorkingDirectory());
	fname.append("\\data\\" + base_fname );
	
	// Add a number of leading zeros before the frame number to make sorting automatic
	vt_char subName[256];
	
	sprintf( subName, "%d", fileno );
	
	vt_long len = strlen(subName);
	
	for(vt_int Count1 = 0; Count1 < (6 - strlen(subName)); Count1++)
		fname.append("0");
	
	// Add the rest of the storage name
	fname.append(subName);
	
	fname.append(".raw");
	
	return fname;
}

//
//! define template function for changing one-d array into 2d-array of pointers
//
/*!
	This function is used to allocate the data for pipe data initialisation. The pipedata function
	has the options to be initialised to collect data in one big grab or via a number of buffers. 
	The number of buffers used is determine by the API variable numBuff.

*/
template<class PixelType>
PixelType ** allocLineArray(vt_uint width, vt_uint height)
{
	// extra line insterted for safety
		PixelType * data   = new PixelType[ (width+1)*(height) ]; // bad assume sentinel is same size as pixel ty
		PixelType ** lines = new PixelType*[  height*sizeof( PixelType* ) ];
		
		PixelType *ptr = data;
		for(vt_uint y=0; y<height; ++y, ptr += width + 1) 
		{
			lines[y] = ptr;
			PixelType *pSentinel = (PixelType *) &lines[y][width];
			*pSentinel = gSentinel; // put sentinal data
		}
		return lines;
}

///
//! free line array
//
template<class PixelType>
void freeLineArray(PixelType ** data)
{
	delete( data[0] );
	delete( data );
}

///
// is a file there or not
//
vt_bool file_exists( const std::string &fname )
{
	HANDLE handle = ::CreateFile(fname.c_str()
															, GENERIC_READ
															, FILE_SHARE_READ
															, NULL
															, OPEN_EXISTING
															, 0
															, NULL );

	if (handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		::CloseHandle( handle );
		return true;
	}
}


//!  Main System Object Class - Singleton class which contain concrete classes for API
/*!
	\class CVtSys
  
  \par SPECIFICATIONS: 
	Create the appropriate concrete objects for the implementation of the API.
	\par 
	Currently the construction of the concrete object uses the following structure
	\par	
	1) Create the pipe_data object, this provides an abstraction for the acquired data
			 The pipe data object is passed to the parser. The pipe object is generic. It is used for all
			 types of raw data ie. pano ceph and hds. The parser is specfic to the API type, there are 
			 currently two types of parser, one which understands data coming from the pano and ceph sensors and 
			 one that understands hds sensor data.
	\par	
	2) Once the parser is created this is passed to the driver object. Like the pipe data object the 
			 driver object is generic and used to access the hardware for all types of interface. The driver object 
			 encapsulates everything to do with the hardware. Effectively, the driver forms the basis of a high level 
			 hardware abstraction layer. The driver is responsible for acquiring the data, by filling in the information 
			 in pipedata. Once pipe data has been filled with data the parser is responsible for interpriting this data
		 	 and turning it into individual lines of the acquired image. Clearly. the parser must know the structure of 
			 the data coming from the various sources. Consequently, the parser type is specific to the particular
			 API type instantiated. Once the data has been parsed it is placed in a dataset by the USB driver.
  
  \note 
	Error handling is managed via the Vt_fail exception generation. Hence, any object calling GetAPI, should 
	catch any potential exceptions. 

	\par 
	The Sys object in addition to being an abstract class factory is implemented around a singleton design pattern. That
	is the sys object is a singleton. However, rather than returning an instance of itself, as might be usual, it returns 
	a copy of the generate API classes contained within itself. Consequently, any subsequent call to GetAPI after the initial
	call will return a reference to the class created on the first call.

	\par Initialisation. 
	The objects created within the system object are not automatically initialised. It is the responsibility of 
	the class user to initialise the API i.e. any attempt to use the class should first be proceed by a
	call to the generic API.init() function.


	@see Vt::CVtAPI
	@see Vt::CVtpcAPI
	@see Vt::CVthdsAPI
	@see Vt::CVtParser
	@see Vt::CVthdsLineParser
	@see Vt::CVtpcLineParser
	@see Vt::CVtUsbDriver
*/
class CVtSys 
{
private:
	FILE						 *m_fpinfo; //< Pointer to information file, primarily contain information about sensors connected to the system

	CVtUSBPipeData	  m_pipe_data; //< The generic pipe data object, this will contain the raw data
	CVtParser				 *m_parser;		 //< Parser, this will be instantiated with a pc or hds parser object
	CVtDrvrAPI			 *m_driver;    //< Main driver

	CVtAPI					 *m_API;

  CVtAPI::API_TYPE	m_api;
  Vt::BIN_MODE	m_bin_mode;

protected:
  //! Pointer to single instance object which contains the current API.
  static CVtSys* theSystem;

	//
	//! Pano-ceph API constructor
	//
	/*! This version of the constructor is used for the pano and ceph mode construction, where the 
	bin mode parameter actually make sense. Consequently, if this version of the constructor is called
	the system "knows" that it should create a pano-ceph line parser and a pano-ceph API object.

	Remember the construction of the sys object is merely to contain the main api object. In this case the
	Vt::CVtpcImpAPI object, which implements (hence Imp) the CVtpcAPI interface. Each API object makes use of 
	two other main classes,
		-# The parser class Vt::CVtpcLineParser i.e. hds specific.
		-# The driver class Vt::CVtUsbDriver (generic).
	In turn the parser class makes uses the generic pipedate class Vt::CVtPipeData to wrap around the raw aquired data.
	Additionally the line parser is responsible for defining and creating the data set which will hold all the 
	acquired data. For convenience a reference to the dataset is kept by the driver and the main API object. Hence, thet can all
	manipulate and access information in the dataset 

	\param api			The api type (will be PANO or CEPH)
  \param bin_mode The binning mode 1x1 1x2 2x1 2x2
	*/

	CVtSys( const CVtAPI::API_TYPE api
				, const Vt::BIN_MODE bin_mode ) :	m_api( api )
																					, m_bin_mode( bin_mode )
																					, m_pipe_data( false ) 
	{
		m_fpinfo = freopen(DEFAULT_INFO_LOG_FNAME, "w", stderr);
		if (m_fpinfo == NULL) {
			printf("Cannot redirect output\n");
		}

		CVtpcLineParser	*parser	= new CVtpcLineParser( m_pipe_data );
		CVtUsbDriver	  *driver	= new CVtUsbDriver( *parser );

		m_API			= new CVtpcImpAPI( api, bin_mode, *driver, parser->get_dataset() );

		m_parser	= parser;
		m_driver	= driver;

		fclose( m_fpinfo );
	}

	//! Intra-oral (hds) version of API constructor
	//
	/*! This version of the constructor is used for hds mode construction i.e. no bin mode paramater.
	Consequently, this version of the constructor "knows" that it should create an hds line parser and this the version that is
	passed to the driver.

	Remember the construction of the sys object is merely to contain the main api object. In this case the
	Vt::CVthdsImpAPI object, which implements (hence Imp) the CVthdsAPI interface. Each API object makes use of 
	two other main classes,
		-# The parser class Vt::CVthdsLineParser i.e. hds specific.
		-# The driver class Vt::CVtUsbDriver (generic).
	In turn the parser class makes uses the generic pipedate class Vt::CVtPipeData to wrap around the raw aquired data.
	Additionally the line parser is responsible for defining and creating the data set which will hold all the 
	acquired data. For convenience a reference to the dataset is kept by the driver and the main API object. Hence, thet can all
	manipulate and access information in the dataset 

	\param api			The api type (will be HDS15 or HDS20)
	*/

	CVtSys( const CVtAPI::API_TYPE api ) :	m_api( api )
																				, m_bin_mode( Vt::INVALID_BIN_MODE )
																				, m_pipe_data( false ) 
																				, m_fpinfo( NULL )
	{
		m_fpinfo = freopen(DEFAULT_INFO_LOG_FNAME, "w", stderr);
		if (m_fpinfo == NULL) {
			printf("Cannot redirect output\n");
		}

		CVthdsLineParser *parser = new CVthdsLineParser( m_pipe_data );
		CVtUsbDriver	   *driver = new CVtUsbDriver( *parser );
		
		m_API			= new CVthdsImpAPI( api, *driver, parser->get_dataset() );

		m_parser	= parser;
		m_driver	= driver;

		fclose( m_fpinfo );
	}

	//! Protected copy constructor, merely defined to prevent direct access to one of these singleton 
	//! Sys objects.
	CVtSys(const CVtSys&);

	//! Protected assignment operator, merely defined to prevent direct access to one of these singleton 
	//! Sys objects.
	const CVtSys& operator= (const CVtSys&);

	/*!
	\fn virtual ~CVtSys()
	\brief Destructor
	Destroys the agrogated objects
		-# the parser
		-# the driver
		-# the api object
	and closes the information file.
	\note the parser is responsible for delete the current dataset.

	*/
	virtual ~CVtSys()
	{
		if (m_parser != NULL)
			delete m_parser;

		if (m_driver != NULL)
			delete m_driver;
	
		if (m_API != NULL)
			delete m_API;

		if (m_fpinfo != NULL)
			fclose( m_fpinfo );
	}
	//
	//! Check certain system files are present.
	/*!
	\fn const vt_bool system_files(const CVtAPI::API_TYPE api) const

	Certain files are used to confirm the system type. For the panoramic system the pano system file
	will be the panoramic calibration file. 
	
	\param api this is the api type for which the presence of the corresponding system file will be confirmed 
	
	\par
	For example, for the ceph system, this function would be return true if the default ceph calibration were present. The names of the default 
	calibration file are defined in the appropriate header file; VtpcAPI.h for the pano and ceph systems, VthdsAPI.h for the hds systems.

	
	 @see #define PANO_PRESENT_FILE	
	 @see #define CEPH_PRESENT_FILE	
	 @see #define HDS15_PRESENT_FILE	
	 @see #define HDS20_PRESENT_FILE	
	*/
	const vt_bool system_files(const CVtAPI::API_TYPE api) const
	{
		switch( api )
		{
			case CVtAPI::PANO_API:
				return file_exists( PANO_PRESENT_FILE );
			case CVtAPI::CEPH_API:
				return file_exists( CEPH_PRESENT_FILE );
			case CVtAPI::HDS15_API:
				return file_exists( HDS15_PRESENT_FILE );
			case CVtAPI::HDS20_API:
				return file_exists( HDS20_PRESENT_FILE );
			default:
				return false;
		}

		return false;
	}

public:	
  ///
  //! Instance the system
  /*
	This function will normally be called by the global function,
	
	VTAPI_API CVtAPI& GetAPI( CVtAPI::API_TYPE api, CVtAPI::BIN_MODE bin_mode = CVtAPI::INVALID_BIN_MODE );

	As can be seen from above GetAPI is defined with a default bin_mode. Hence,in the case of the hds sensor,
	where the bin mode parameter is not applicaable, GetAPI can be called with a single parameter.

	Typically, the GetAPI with the 

	\param api			The api type, will be 
			-# Vt::CVtAPI::PANO_API
			-# Vt::CVtAPI::CEPH_API
			-# Vt::CVtAPI::HDS15_API 
			-# Vt::CVtAPI::HDS20_API
  \param bin_mode The binning mode 1x1 1x2 2x1 2x2 or blank
	*/
  static CVtAPI& Instance( const CVtAPI::API_TYPE api, const Vt::BIN_MODE bin_mode )
  {
		if (theSystem == NULL)
		{
			switch( api )
			{
			case CVtAPI::HDS15_API:
			case CVtAPI::HDS20_API:
				theSystem = new CVtSys( api ); // create sole instance
				theSystem->m_api = api;
				break;

			case CVtAPI::PANO_API:
			case CVtAPI::CEPH_API:
				theSystem = new CVtSys( api, bin_mode ); // create sole instance
				theSystem->m_api = api;
				break;

			default:
				Vt_fail( "Invalid API requested" );
			}
		}
		return *theSystem->m_API; 
  }

	///
	//! Get an instance of the last system accessed 
  /*
	This function will normally be called by the global function,
	
	VTAPI_API CVtAPI& GetAPI();

	This would be the normal way of accessing a reference to the singleton API object. However,
	a call must be first made through the parametrised version of the get API function i.e. to 
	create a single instance object the system must be told what kind of object to create. This is
	done by first calling the other version of the global function Vt::GetAPI()
	
		VTAPI_API CVtAPI& GetAPI( CVtAPI::API_TYPE api, CVtAPI::BIN_MODE bin_mode = CVtAPI::INVALID_BIN_MODE );

	\example
	main()
	{
	CVtAPI &API = GetAPI( CVtAPI::HDS20_API ); 

	API.init();
	}
		...

	void some_other_function()
	{
	CVtAPI &API = GetAPI(); // will get the single instance object created above.

	// or alternatively

	GetAPI().capture(); // for example
	}
	*/
  static CVtAPI& Instance()
  {
		Vt_precondition( theSystem != NULL, "Instance called with no args" );

		return Instance( theSystem->m_api, theSystem->m_bin_mode );
  }
  ////
  //! Kill the system off
  //
	/*
	This function is called by the global function Vt::CloseAPI()

	If the singleton system object exists this function deletes it, and thereby removes the objects
	contained within this object 
		-# The parser will be deleted, which in turn will release the data contained 
			 in the current dataset. 
		-# The pipe data object will be deleted which will release any data contained in the pipe.
		-# The current API object will be deleted
		-# The driver object (HAL) will be deleted.
	*/
  static void End()
  {
		if (theSystem != NULL)
		{
			delete theSystem;
			theSystem = NULL;
		}
  }
}; // end sys

} // end Vt namespace

#endif // __CVTSYS_H__
