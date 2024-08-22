/** \file VtDataset.h contains CVtDataset class.

	Defines the class which holds a collection of related image data.

	Typically a dataset will contain all the input and intermediate images require to produce
	the final displayed image.

* Copyright (c) 2013 by
* All Rights Reserved
*
* REVISIONS: 
* $Log: VtDataset.h $
*
*/

#ifndef __CVTDATASET_H__
#define __CVTDATASET_H__

namespace Vt 
{

/**
\class CVtDataset 
*/


template<class T>
class CVtDataset 
{
public:
	typedef std::pair<T, CVtImageBaseClass*>	DATASET_ENTRY;
	typedef	std::vector<DATASET_ENTRY >	DATASET;

	typedef DATASET::iterator iterator;
	typedef DATASET::const_iterator const_iterator;

protected:
	DATASET		m_dataset;

public:
	//
	// Initialise reconstruction and globals in base class
	//
	CVtDataset() {}
  //
  // Destructor
  //
  virtual ~CVtDataset() 
	{
		delete_dataset();
	}

	virtual iterator begin()
	{
		return m_dataset.begin();
	}
	virtual iterator end()
	{
		return m_dataset.end();
	}

	virtual vt_bool delete_dataset()
	{
		// get rid of any data sets hanging around
		for(;!m_dataset.empty(); m_dataset.pop_back())
		{
			DATASET_ENTRY entry = m_dataset.back();
			delete entry.second; // delete the image
		}

		return (_CrtCheckMemory() == TRUE);
	}

	virtual vt_bool delete_image(CVtAPI::IM_TYPE im_type)
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			if ( (*it).first.type == im_type)
			{
				CVtImageBaseClass* im = (*it).second;

				if (im != NULL)
					delete im;

				m_dataset.erase( it );

				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return true;
			}
		}

		return false;
	}

	///
	// image data accessors
	//
	virtual vt_ushort * image_ptr()
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im != NULL && ((*it).first.type == CVtAPI::OUTPUT_IM))
			{
				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return im->begin();
			}
		}
		return NULL;
	}
	virtual vt_ushort * image_ptr(CVtAPI::IM_TYPE im_type)
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im != NULL && ((*it).first.type == im_type))
			{
				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return im->begin();
			}
		}
		return NULL;
	}
	virtual vt_ushort ** image_ptrs(CVtAPI::IM_TYPE im_type)
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im != NULL && ((*it).first.type == im_type))
			{
				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return im->lines();
			}
		}

		return NULL;
	}


	///
	// get image height - returns image height
	// currently all the images are the same height
	//
	virtual vt_ulong image_width(CVtAPI::IM_TYPE im_type)
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im != NULL && ((*it).first.type == im_type))
			{
				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return im->width();
			}
		}
		return 0;
	}
	virtual vt_ulong image_height(CVtAPI::IM_TYPE im_type) 
	{
		for(DATASET::iterator it = m_dataset.begin(); it != m_dataset.end(); it++)
		{
			CVtImage<vt_acq_im_type>* im = dynamic_cast<CVtImage<vt_acq_im_type>*>((*it).second);
			if (im != NULL && ((*it).first.type == im_type))
			{
				Vt_precondition( _CrtCheckMemory() == TRUE, "image_ptr::Memory problem detected\n" );
				return im->height();
			}
		}
		return 0;
	}

	//
	// operator[]
	// 
	vt_ulong size()
	{
		return m_dataset.size();
	}

	//
	// operator[]
	// 
	CVtImageBaseClass* operator [](const vt_ulong idx)
	{
		return m_dataset[idx].second;
	}
	///
	// dataset manipulation
	//
	// Datasets can be any 2d set of data. If we are in non-sync
	// mode then Dataset 0 is usually the raw input data buffers.
	//
	void add_dataset( T ent_type, CVtImageBaseClass *pdata )
	{
		m_dataset.push_back( std::pair< T, CVtImageBaseClass* >(ent_type, pdata ) );
	}

	///
	// pop_back
	//
	DATASET_ENTRY pop_back( const CVtAPI::IM_TYPE im_type )
	{
		DATASET_ENTRY entry = m_dataset.back();

		if ( im_type != entry.first.type )
			Vt_fail( "VtSys::pop_back::Unexpected entry type" );

		// remove entry from vector
		m_dataset.pop_back();
		return entry;
	}

	DATASET_ENTRY get_back( const CVtAPI::IM_TYPE im_type )
	{
		DATASET_ENTRY entry = m_dataset.back();

		if ( im_type != entry.first.type )
			Vt_fail( "VtSys::pop_back::Unexpected entry type" );

		return entry;
	}
};

} // end Vt namespace
#endif // __CVTHDSIMPAPI_H__
