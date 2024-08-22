/** \file VthdsCalib.h

	The calibration class for the hds sensor.

	This class contains the main functions for performing capturing data, calculating the calibration 
	coefficients and storing the calibration coefficients for the hds sensor.

	Currently the calibration process for the hds sensor consists of two polynomial fits.

* Copyright (c) 2013 by
* All Rights Reserved
* REVISIONS: 
* $Log:  $
*
*/

#ifndef __CVTHDSCALIB_H__
#define __CVTHDSCALIB_H__

namespace Vt {
	
/**
* Big endian swapping
*/
template<typename T>
	swap_bytes(T *inbuf, vt_ulong n1)
{
	int i;
	vt_char *buf;
	vt_char apu1,apu2,apu3,apu4;
	
	buf = (vt_char *)inbuf;
	for(i=0;i<n1;i++)
	{
		apu1 = *buf++;
		apu2 = *buf++;
		apu3 = *buf++;
		apu4 = *buf--;
		buf--;
		buf--;
		*buf++ = apu4;
		*buf++ = apu3;
		*buf++ = apu2;
		*buf++ = apu1;
	}
} 

	
template<typename SumType, typename ImageType>
void sum(CVtImage<SumType>& out, const CVtImage<ImageType>& in )
{
	for(vt_ulong row = 0; row < in.height(); row++)
	{
		for(vt_ulong col = 0; col < in.width(); col++)
		{
			out[row][col] += in[row][col];
		}
	}
}

template<typename ImageType>
void divide(CVtImage<ImageType>& im, const vt_double divisor )
{
	for(vt_ulong row = 0; row < im.height(); row++)
	{
		for(vt_ulong col = 0; col < im.width(); col++)
		{
			im[row][col] /= divisor;
		}
	}
}
	

/**
	When the calibration dark frames are acquired the reset voltage is varied. This defines the initial
	reset voltage.
*/
#define START_CALIB_VOLTAGE			"VR_RESET_VOLTAGES_1_9V"

/**
	When the calibration dark frames are acquired the reset voltage is varied. 
	This defines the final reset voltage.
*/

#define END_CALIB_VOLTAGE				"VR_RESET_VOLTAGES_3_9V"

class CVthdsImpAPI;

/**
  \image html vatech.jpg

	\brief 	The calibration class for the hds sensor.

  \par REQUIREMENTS: 

	This class contains the main functions for performing capturing data, calculating the calibration 
	coefficients and storing the calibration coefficients for the hds sensor.

	Currently the calibration process for the hds sensor consists of two polynomial fits.
  
	This is class which implements the hds calibration functions. It is a templatized class. The 
	respective template parameters are
		-# ImageType which holds the type for the images which are to be calibrated-
		-# CoefType  this holds the type for the calibration coefficients.
		-# MaskType  this holds the type for the bad pixel mask.
*/

template<typename ImageType
					, typename CoefType
					, typename MaskType	>
class CVthdsCalib
{
public:
	SENSOR_INFO m_hw_info;

	typedef CVthdsLineParser::DATASET_ENTRY_TYPE DATASET_ENTRY_TYPE;

	typedef std::multimap<std::string, std::string>	REFE_FNAMES;
	/**
	\brief The filenames for the intermediate data files.

	This structure is initialised on construction by the routine init_fnames();

	@see init_fnames();
	*/
	REFE_FNAMES m_refe_fnames;

	typedef std::multimap<vt_ulong, vt_ulong>	FILTER_AVES;
	/**
	\typedef The collection class which holds the association between the filter number and the
	number of bright frames acquire. e.g. the for the first filter type, filter number 0, requres 
	4 images say.
	*/
	FILTER_AVES m_filt_nums;

	/**
	\typedef bright_names
	The file names for the acquire bright images are stored in triples, two 
	bright frame file names and one dark frame filename.
	*/
	typedef struct {
		std::string data1;
		std::string data2;
		std::string refe;
	} bright_names;

	/**
	\typedef BRIGHT_NAMES
	This defines collection class where the bright filenames are stored.
	*/
	typedef std::multimap<vt_ulong, bright_names>	BRIGHT_FNAMES;
	
	/**
	\var m_bright_names
	This is the variable which holds the names for bright files.
	*/

	BRIGHT_FNAMES m_bright_fnames;

	enum {
		DARK_IMAGES_PER_AVE			= 20 //!< How many dark frames are averaged for each frame actually written out.
		, BRIGHT_FILTERS				= 5  //!< How many bright filters are used to acquire the bright images.
	};
	/**
	\brief The type for the 5th order polynomial coefficient.
	*/
	typedef vt_double POLY5COEF[6]; 
	/**
	\brief The type for the 3rd order polynomial coefficient.
	*/
	typedef vt_double POLY3COEF[3];

	/**
	\brief A 5th order polynomial coefficient image.
	Each pixel of the m_cal5 image is a POLY5COEF type. Hence accessing the (row, col) pixel of such an image
	actually references a vector or polynomial coefficients. Hence the following progressive dereferences 
	are possible.
	
	\code
	CoefType rowvec* m_cal[row];

	CoefType coefs[] = rowvec[col];
	\endcode

	or alternatively

	\code
	poly( data, m_cal5[row][col], 5 );
	\endcode
	
	@see poly.
	*/
	CVtImage<POLY5COEF>	m_cal5;

	/**
	\brief A 3rd order polynomial coefficient image.
	Each pixel of the m_cal3 image is a POLY3COEF type. Hence accessing the (row, col) pixel of such an image
	actually references a vector or polynomial coefficients. Hence the following progressive dereferences 
	are possible.
	
	\code
	CoefType rowvec* m_cal[row];

	CoefType coefs[] = rowvec[col];
	\endcode

	or alternatively

	\code
	poly( data, m_cal3[row][col], 3 )
	\endcode
	
	@see poly.
	*/
	CVtImage<POLY3COEF>	m_cal3;


	CVtImage<ImageType>						 &m_dark;
	CVtImage<MaskType>						 &m_mask;
	CVtDataset<DATASET_ENTRY_TYPE> &m_data;

	/**
		\brief Hds calibration destructor

		Currently just a place holder.
	*/
	virtual ~CVthdsCalib()	{}
	
	/**
	 \brief hds calibration constructor
	
		\param data A reference to the system dataset. This object is actually owned by the system parser.
		\param dark A reference to the most recent dark frame, this is currently owned by the api object.
		\param mask A reference to the good pixel mask, this is currently owned by the api object.
	*/
	CVthdsCalib(CVtDataset<DATASET_ENTRY_TYPE> &data
												, CVtImage<ImageType> &dark
												, CVtImage<MaskType> &mask ) 
												: m_data( data )
												, m_dark( dark )
												, m_mask( mask ) 
	{
		init_fnames();
	}
	/**
	\brief Main calibration calculation routine.

	The hds calibration process consists of the application of two polynomial fits. This function uses the information
	stored in various intermediate files to calculate these polynomial coefficients.
	*/
	void recalc()
	{
	}

	/**
	\brief Add all the images together in the associated dataset

	Because this routine has a reference the current system dataset object. It is possible 
	to iterate through the dataset adding together all the images.

	\param out the summed image.
	*/

	void sum(CVtImage<CoefType>& out )
	{
		for( CVtDataset<DATASET_ENTRY_TYPE>::iterator it = m_data.begin();
				  it != m_data.end(); it++ 
				)
		{
			CVtImage<vt_acq_im_type> *im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im == NULL)
				continue; // image incorrect type

			Vt::sum( out, *im );
		}
	}

	/**
	\brief Capture bright frames

	 During the calculation of the calibration coefficients a number of bright frames are 
	 acquired. This code acquires each of the bright frame data sets and saves the data after
	 each acquisition.

	 Currently the code assumes that each bright dataset consists of three images. Two bright images 
	 and one dark. The number of bright frames acquired various depending on the filter used.
	 The number of bright frame acquire for a particular filter is stored tin the variable m_filt_nums.

	 \sa m_filt_nums
	*/
	void bright_frames(CVthdsImpAPI& API)
	{
		printf( "BRIGHT FRAMES\n" );

		vt_ulong image_height = API.image_height();
		vt_ulong image_width  = API.image_width();

		vt_ulong imno = 0;
		API.m_dataset_size = 2;
		for(vt_ulong filtno=0; filtno < BRIGHT_FILTERS; filtno++)
		{
			FILTER_AVES::iterator it = m_filt_nums.find( filtno );

			vt_ulong  num_bright_aves = (*it).second;
			printf( "Place Filter Number %d in place\n", filtno );
			getchar();

			for(vt_ulong idx = 0; idx < num_bright_aves; idx++)
			{
				// capture
				API.capture();

				// save the current data set
				save_bright( imno++, API );
			}
		}
		API.set_api_params(); // return dataset size to default
	}
 

	/**
	\brief Save bright frames
	
	 During the calculation of the calibration coefficients a number of bright frames are 
	 acquired. This code saves the data set associated with each bright frame.
	 The code assumes that the bright frame data sets contain three images, two bright frames and
	 a single dark frame.

	*/
	void save_bright( const vt_ulong imageno, CVthdsImpAPI& API )
	{
	
		CVtDataset<DATASET_ENTRY_TYPE>::iterator it = m_data.begin();
		CVtImage<vt_acq_im_type> *refe  = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second); it++;
		CVtImage<vt_acq_im_type> *data1 = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second); it++;
		CVtImage<vt_acq_im_type> *data2 = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second); 
		
		if (refe == NULL  || data1 == NULL  || data2 == NULL  )
			// should we throw and exception here?
			return;

		BRIGHT_FNAMES::iterator bfnames_it = m_bright_fnames.find( imageno );
		
		std::string fname_base( HDS_CALIB_BRIGHT_FNAME_BASE );
		std::string fname( fname_base );

		fname.append( (*bfnames_it++).second.refe );
		API.save_imfile( refe->lines(), sizeof( vt_acq_im_type )
											, refe->width()
											, refe->height()
											, fname
											, false );			

		fname = fname_base;
		fname.append( (*bfnames_it++).second.data1 );
		API.save_imfile( data1->lines(), sizeof( vt_acq_im_type )
											, data1->width()
											, data1->height()
											, fname
											, false );			

		fname = fname_base;
		fname.append( (*bfnames_it).second.data2 );
		API.save_imfile( data2->lines(), sizeof( vt_acq_im_type )
											, data2->width()
											, data2->height()
											, fname
											, false );			

	}

	/**
		\brief Capture dark frames
		This routine can be used to calculate the 
	*/
	void dark_frames(CVthdsImpAPI& API)
	{
		printf( "DARK FRAMES\n" );
		API.send_command( std::string( HDS_DEFAULT_RESET_VOLTAGE ) );		

		vt_ulong image_height = API.image_height();
		vt_ulong image_width  = API.image_width();
		
    CVthdsImpAPI::CODE_PAIRS::iterator start = API.m_codes.find( START_CALIB_VOLTAGE );
    CVthdsImpAPI::CODE_PAIRS::iterator end   = API.m_codes.find( END_CALIB_VOLTAGE );

		CVtImage<CoefType> ave( image_width, image_height );    
		for( CVthdsImpAPI::CODE_PAIRS::iterator it = start;
				 it != end; 
				 it++
			 )
		{
			std::string command_str( (*it).first );

			// prepare to capture data
			API.arm( DARK_IMAGES_PER_AVE );

			// software trigger
			API.soft_trigger();

			for(vt_ulong idx = 0; idx < DARK_IMAGES_PER_AVE; idx++)
			{
				// set reset voltage
				API.send_command( command_str );

				printf( "\n" );

				// capture
				API.m_driver.read_pipe();

				// calculate average
				sum( ave );

				// remove currently acquired images
				API.delete_dataset();
			}
			// divide image
			divide( ave, DARK_IMAGES_PER_AVE );

			// OK read the info
			API.reset();

			// save image
			REFE_FNAMES::iterator it = m_refe_fnames.find( (*it).first );

			if (it != m_refe_fnames.end() )
			{
				API.save_imfile( ave.lines(), sizeof( CoefType ), image_width, image_height, (*it).second, false );
			}
		}
	}


	/**
	\brief Main calibration run

	The calibration class contains functions to calculate the calibration coefficient and contains functions 
	to apply the calibration coefficients. This the main function for calculation of the calibration coefficients.
	
	Currently the algorithm for this is;-
		-# Acquire dark frames,
	\code 
			for start_reset_voltage to end_reset_voltage,
			{
				for number of dark frame averages
				{
					Acquire dark frames
					Sum dark frame 
				}

				Save averaged dark frame for this reset volatage.
			}
	\endcode			
		-# Acquire bright frames
	\code 
			for start_filter to end_filter
			{
				externally change filter 
				for number of frame bright averages
				{
					Acquire bright frames
					Sum dark frame 
				}

				Save averaged bright frame for this filter setting
			}
	\endcode			
		-# Calculate calibration coefficients

	*/

	void calibration_run(CVthdsImpAPI& API)
	{
		// capture dark frames
		dark_frames( API );

		// capture bright frames
		bright_frames( API );
		
		// calculate calibration coefficients
		recalc(); 
	}

	/**
	\brief polynomial expansion

	The application of the calibration process to the acquire data involves the use of various calibration
	coefficients.
	*/
	template<typename T>
	vt_double poly(const T data, const CoefType *coefs, const vt_ulong order )
	{
		vt_double val = coefs[1] + coefs[0]*data;
		for(vt_ulong coefno=2; coefno < order; coefno++)
		{
			val = coefs[coefno] + data*val;
		}
		val *= data;

		return val;
	}

	
	/**
	  \brief Main application of calibration routine

		This is the routine that is called after acquiring a dataset to produce a calibrated image.

		\param out The calibrated image.
		\param num_images The calibration process operates on a number of input images to produce an output image. This
		parameter determines how many input images are used.
	*/
	void operator()( CVtImage<ImageType>& out, const vt_ulong num_images ) // how many images to use
	{
		Vt_precondition( m_data.size() == 0, "No images present" );
		
		// extract pointer from the various datasets
		
		CoefType *val3  = new CoefType [num_images-1];
		CoefType *val5  = new CoefType [num_images-1];

		for(vt_ulong row=0; row< m_mask.height();row++)
		{
			for(vt_ulong col=0; col< m_mask.width();col++)
			{
				CoefType dark = poly( m_dark[row][col], m_cal5[row][col], 5 );
				CoefType sum  = 0.0;

				CVtDataset<DATASET_ENTRY_TYPE>::iterator it = m_data.begin();
				for(vt_ulong idx=0; it != m_data.end(); it++, idx++)
				{
					CVtImage<ImageType> *imp = dynamic_cast<CVtImage<ImageType>*>((*it).second);			
					CVtImage<ImageType>& im  = *imp;
					
					ImageType data = im[row][col];
								
					CoefType I =  
					sum += poly( data, m_cal5[row][col], 5 ) - dark;
				}
				CoefType ave = sum/num_images;
				//
				// calculate final image value
				//
				out[row][col] = (ImageType) poly( ave, m_cal3[row][col], 3 );
			}
		}
		// tidy
		if (val3 != NULL)
			delete val3;
		if (val5 != NULL)
			delete val5;

	}

	/**
	\brief Save calibration coefficients

	Once calculated the calibration´coefficients can be saved out to disk. The name of the calibration
	coefficient file is saved in the hds API definition file VthdsAPI.h.

	The calibration coefficients 
	*/
	void save( std::string fname )
	{
		FILE *fpout = fopen( fname.c_str(), "wb" );
		
		Vt_precondition(fpout != NULL, "CVthdsCalib::save failed to open output file\n" );
		
		// Read the header data
		vt_ulong num_pix = GetAPI().image_height()*GetAPI().image_width();

		fwrite( (const char *) m_hw_info.begin(), sizeof( vt_byte ), m_hw_info.length(), fpout );
		fwrite( (const char *) m_cal5.begin(), sizeof( CoefType ), num_pix, fpout );
		fwrite( (const char *) m_cal3.begin(), sizeof( CoefType ), num_pix, fpout );
		fwrite( (const char *) m_mask.begin(), sizeof( MaskType ), num_pix, fpout );
		
		fclose(fpout);
	}
	
	/**
	\brief istream operator for the hds Calibration object.

	This function allows the hds calibration object to tbe streamed in from disk.
	Defined merely for convenience. This will normally be called from the hds implementation api
	object (Vt::CVthdsImpAPI) init function.

	\sa Vt::CVthdsImpAPI
	*/

	friend std::istream& operator >> (std::istream& IS
																		, CVthdsCalib<ImageType, CoefType, MaskType>& Object)
	{
		CVtAPI &API = GetAPI();
		
		Vt_precondition(IS.good() ? true : false, "CVthdsCalib::operator >> - Failed to open input file");
		
		// Read the header data
		vt_ulong width	 = API.image_width();
		vt_ulong height  = API.image_height();
		vt_ulong num_pix = width*height;
		
		//
		// read in information about the current hardware
		//
		vt_ulong hwinfo_len = Object.m_hw_info.length();

		vt_byte *hw_info_buf = new vt_byte[ hwinfo_len ];
		IS.read( (char *) hw_info_buf, hwinfo_len );
		Object.m_hw_info = hw_info_buf;

		//
		// read in 5th order coefficients
		//
		CoefType *ptr = (CoefType *) new vt_byte[ 6*num_pix*sizeof(CoefType) ];
		IS.read( (char *) ptr, num_pix*sizeof(CoefType) );
		
		Object.m_cal5.resizeCopy( width, height, (POLY5COEF *)ptr );
		
		//
		// read in 3rd order coefficients
		//
		ptr = (CoefType *) new vt_byte[ 3*num_pix*sizeof(CoefType) ];
		IS.read( (char *) ptr, num_pix*sizeof(CoefType) );
    
		Object.m_cal3.resizeCopy( width, height, (POLY3COEF *)ptr );
		
		//
		// read mask
		//
		MaskType *mask = new vt_byte[ num_pix*sizeof(MaskType) ];
		IS.read( (char *) mask, num_pix*sizeof(MaskType) );
		
		Object.m_mask.resizeCopy( width, height, (MaskType *) mask );
		
		return IS;
	}

	/**
	\brief Initialise the filenames used for 

	The calibration process involves saving a number of intermediate files. This function initialises the 
	file names that will be used for these intermediate files. The names are currently chosen to be consistent with
	the labview implementation.
	*/
	void init_fnames()
	{
		std::pair<std::string, std::string>  refe_fname;

		// dark image filenames
		refe_fname.first  = "VR_RESET_VOLTAGES_1_8V";
		refe_fname.second = "refe180.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_1_9V";
		refe_fname.second = "refe190.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_0V";
		refe_fname.second = "refe200.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_1V";
		refe_fname.second = "refe210.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_2V";
		refe_fname.second = "refe220.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_3V";
		refe_fname.second = "refe230.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_4V";
		refe_fname.second = "refe240.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_5V";
		refe_fname.second = "refe250.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_6V";
		refe_fname.second = "refe260.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_7V";
		refe_fname.second = "refe270.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_8V";
		refe_fname.second = "refe280.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_2_9V";
		refe_fname.second = "refe290.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_0V";
		refe_fname.second = "refe300.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_1V";
		refe_fname.second = "refe310.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_2V";
		refe_fname.second = "refe320.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_3V";
		refe_fname.second = "refe330.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_4V";
		refe_fname.second = "refe340.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_5V";
		refe_fname.second = "refe350.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_6V";
		refe_fname.second = "refe360.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_7V";
		refe_fname.second = "refe370.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_8V";
		refe_fname.second = "refe380.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_3_9V";
		refe_fname.second = "refe390.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_0V";
		refe_fname.second = "refe400.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_1V";
		refe_fname.second = "refe410.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_2V";
		refe_fname.second = "refe420.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_3V";
		refe_fname.second = "refe430.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_4V";
		refe_fname.second = "refe440.raw";
		m_refe_fnames.insert(refe_fname);

		refe_fname.first  = "VR_RESET_VOLTAGES_4_5V";
		refe_fname.second = "refe450.raw";
		m_refe_fnames.insert(refe_fname);

		// number of images used for each filter setting
		std::pair<vt_ulong, vt_ulong>  filts;
		filts.first  = 0; // filter number
		filts.second = 4; // number of files bright images
		m_filt_nums.insert(filts);

		filts.first  = 1;
		filts.second = 6;
		m_filt_nums.insert(filts);

		filts.first  = 2;
		filts.second = 8;
		m_filt_nums.insert(filts);

		filts.first  = 3;
		filts.second = 12;
		m_filt_nums.insert(filts);

		filts.first  = 4;
		filts.second = 20;
		m_filt_nums.insert(filts);

		// filename for bright images
		std::vector<std::string> suffix;
		suffix.push_back( "r" );
		suffix.push_back( "s" );
		suffix.push_back( "t" );
		suffix.push_back( "u" );
		suffix.push_back( "v" );

		vt_ulong cnt = 0;
		vt_ulong idx = 0;

		bright_names bfnames;

		for(std::vector<std::string>::iterator sfx = suffix.begin(); sfx != suffix.end(); sfx++ )
		{
			FILTER_AVES::iterator num_filt_it = m_filt_nums.find( idx++ );
			vt_ulong filt_num = (*num_filt_it).second;
		
			std::string data1( "data1." ); 
			data1.append( (*sfx) );

			std::string data2( "data2." ); 
			data2.append( *sfx );
			
			std::string refe(  "refe." ) ;		
			refe.append( *sfx );

			for( vt_ulong filtno = 0; filtno < filt_num; filtno++, cnt++ )
			{
				char num_str[256];
				std::string data1_fname( data1 ); data1_fname.append( itoa( filtno, num_str, 10 ) );
				std::string data2_fname( data2 ); data2_fname.append( itoa( filtno, num_str, 10 ) );
				std::string refe_fname( refe );		refe_fname.append( itoa( filtno, num_str, 10 ) );

				bfnames.data1 = data1_fname;
				bfnames.data2 = data2_fname;
				bfnames.refe  = refe_fname;

				m_bright_fnames.insert( std::pair<vt_ulong, bright_names>( cnt, bfnames)  );
			}
		}
	}
};

} // Vt namespace
#endif // __CVTHDSCALIB_H__
