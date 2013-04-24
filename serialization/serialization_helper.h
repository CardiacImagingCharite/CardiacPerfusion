/*
This file is part of perfusionkit.
Copyright 2010 Henning Meyer

perfusionkit is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

perfusionkit is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <itkMetaDataDictionary.h>
#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <QString>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <bitset>
#include "imagedefinitions.h"
#include "ctimagetreemodel.h"
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include "realimagetreeitem.h"
//#include "watershedsegmenttreeitem.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <typeinfo>



BOOST_SERIALIZATION_SPLIT_FREE(itk::MetaDataDictionary)
	BOOST_SERIALIZATION_SPLIT_FREE(QString)
	BOOST_SERIALIZATION_SPLIT_FREE(CTImageTreeItem::SegmentationValueMap)


	namespace boost {
		namespace serialization {

			template<class Archive>
			void load(Archive & ar, itk::MetaDataDictionary &d, const unsigned int version)
			{
			  uint64_t ikeyNum;
			  ar & ikeyNum;
			  std::vector< std::string >::size_type keyNum(ikeyNum);
			  while(keyNum) {
			    std::string key, value;
			    ar & key;
			    ar & value;
			    itk::EncapsulateMetaData(d, key, value);
			    --keyNum;
			  }  
			}

			template<class Archive>
			void save(Archive & ar, const itk::MetaDataDictionary &d, const unsigned int version)
			{
			  typedef std::vector< std::string > KeyContainer;
			  KeyContainer keys = d.GetKeys();
			  uint64_t ikeyNum = keys.size();
			  ar & ikeyNum;
			  for(KeyContainer::const_iterator it = keys.begin(); it != keys.end(); ++it) {
			    std::string val;
			    itk::ExposeMetaData(d, *it, val);
			    ar & *it;
			    ar & val;
			  }
			}


			template<class Archive>
			void load(Archive & ar, QString &s, const unsigned int version)
			{
				std::string string;
				ar & string;
				s = QString::fromAscii(string.c_str());
			}

			template<class Archive>
			void save(Archive & ar, const QString &s, const unsigned int version)
			{
				std::string string( s.toAscii().data() );
				ar & string;
			}


			template<class Archive, class U>
			void load(Archive & ar, boost::ptr_vector<U> &v, const unsigned int version)
			{
			        uint64_t isize;
				ar & isize;
			        typename boost::ptr_vector<U>::size_type size(isize);
				while(size) {
					TreeItem *child;
					ar & child;
					v.push_back(child);	
				--size;
				}
			}

			template<class Archive, class U>
			void save(Archive & ar, const boost::ptr_vector<U> &v, const unsigned int version)
			{
				typename boost::ptr_vector<U>::size_type size = v.size();
				uint64_t isize(size);
				ar & isize;
				for(typename boost::ptr_vector<U>::const_iterator it = v.begin(); it != v.end(); ++it) {
					const U *obj = &(*it);
					ar & obj;
				}
			}

			template<class Archive, class U>
			inline void serialize(Archive & ar, boost::ptr_vector<U> &v, const unsigned int version)
			{
				boost::serialization::split_free(ar, v, version);
			}

			typedef uint64_t BitSetStorageType;
			typedef std::bitset< 8 * sizeof( BitSetStorageType ) > BitSetType;

			typedef float RealStorageType;

			template<class Archive, unsigned Dimension>
			inline void load(Archive & ar, typename itk::SmartPointer< itk::Image<BinaryPixelType, Dimension> > &i, const unsigned int version)
			{
				typedef typename itk::Image<BinaryPixelType, Dimension> ImageType;
				i = ImageType::New();
				typename ImageType::SpacingType spacing;
				typename ImageType::SizeType size;
				typename ImageType::IndexType index;
				typename ImageType::PointType origin;
				uint64_t t;
				for(unsigned d = 0; d < Dimension; d++) {
				  ar & t;
				  size[d] = t;
				  ar & t;
				  index[d] = t;
				  ar & spacing[d];
				  ar & origin[d];
				}
				typename ImageType::RegionType region;
				region.SetIndex( index );
				region.SetSize( size );
				i->SetRegions( region );
				i->SetSpacing( spacing );
				i->SetOrigin( origin );
				i->Allocate();
				itk::ImageRegionIterator<ImageType> it(i,region);

				BitSetStorageType ulongVal = 0;
				BitSetType bitValues;
				size_t sizeCount = bitValues.size();
				for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
					if (sizeCount == bitValues.size()) {
						ar & ulongVal;
						BitSetType tb( ulongVal );
						std::swap( bitValues, tb );
						sizeCount = 0;
					}
					it.Value() = bitValues[sizeCount]? BinaryPixelOn : BinaryPixelOff;
					sizeCount++;
				}
			}



			template<class Archive, unsigned Dimension>
			inline void save(Archive & ar, const typename itk::SmartPointer< itk::Image<BinaryPixelType, Dimension> > &i, const unsigned int version)
			{
				typedef typename itk::Image<BinaryPixelType, Dimension> ImageType;
				typename ImageType::RegionType region = i->GetBufferedRegion();
				typename ImageType::SpacingType spacing = i->GetSpacing();
				typename ImageType::SizeType size = region.GetSize();
				typename ImageType::IndexType index = region.GetIndex();
				typename ImageType::PointType origin = i->GetOrigin();
				uint64_t t;
				for(unsigned d = 0; d < Dimension; d++) {
				  t = size[d];
				  ar & t;
				  t = index[d];
				  ar & t;
				  ar & spacing[d];
				  ar & origin[d];
				}
				itk::ImageRegionConstIterator<ImageType> it(i,region);
				BitSetStorageType ulongVal;
				BitSetType bitValues;
				size_t sizeCount = 0;
				for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
					bitValues[sizeCount] = (it.Value() == BinaryPixelOn);
					sizeCount++;
					if (sizeCount == bitValues.size()) {
						ulongVal = bitValues.to_ullong();
						ar & ulongVal;
						sizeCount = 0;
					}
				}
				if (sizeCount != 0) {
					ulongVal = bitValues.to_ullong();
					ar & ulongVal;
				}
			}


			template<class Archive, unsigned Dimension>
			inline void serialize(Archive & ar, typename itk::SmartPointer< itk::Image<BinaryPixelType, Dimension> > &i, const unsigned int version)
			{
				boost::serialization::split_free(ar, i, version);
			}

			template<class Archive, unsigned Dimension>
			inline void load(Archive & ar, typename itk::SmartPointer< itk::Image<RealPixelType, Dimension> > &i, const unsigned int version)
			{
				typedef typename itk::Image<RealPixelType, Dimension> ImageType;
				i = ImageType::New();
				typename ImageType::SpacingType spacing;
				typename ImageType::SizeType size;
				typename ImageType::IndexType index;
				typename ImageType::PointType origin;
				uint64_t t;
				for(unsigned d = 0; d < Dimension; d++) {
				  ar & t;
				  size[d] = t;
				  ar & t;
				  index[d] = t;
				  ar & spacing[d];
				  ar & origin[d];
				}
				typename ImageType::RegionType region;
				region.SetIndex( index );
				region.SetSize( size );
				i->SetRegions( region );
				i->SetSpacing( spacing );
				i->SetOrigin( origin );
				i->Allocate();
				itk::ImageRegionIterator<ImageType> it(i,region);

				it.GoToBegin();

				while( !it.IsAtEnd() )
				{
					RealPixelType p;
					ar & p;
					//it.Value() = p;
					it.Set(p);
					++it;
				}

				/*BitSetStorageType ulongVal = 0;
				BitSetType bitValues;
				size_t sizeCount = bitValues.size();
				for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
					if (sizeCount == bitValues.size()) {
						ar & ulongVal;
						BitSetType tb( ulongVal );
						std::swap( bitValues, tb );
						sizeCount = 0;
					}
					it.Value() = bitValues[sizeCount]? BinaryPixelOn : BinaryPixelOff;
					sizeCount++;
				}
				*/
			}



			template<class Archive, unsigned Dimension>
			inline void save(Archive & ar, const typename itk::SmartPointer< itk::Image<RealPixelType, Dimension> > &i, const unsigned int version)
			{
				typedef typename itk::Image<RealPixelType, Dimension> ImageType;
				typename ImageType::RegionType region = i->GetBufferedRegion();
				typename ImageType::SpacingType spacing = i->GetSpacing();
				typename ImageType::SizeType size = region.GetSize();
				typename ImageType::IndexType index = region.GetIndex();
				typename ImageType::PointType origin = i->GetOrigin();
				uint64_t t;
				for(unsigned d = 0; d < Dimension; d++) {
				  t = size[d];
				  ar & t;
				  t = index[d];
				  ar & t;
				  ar & spacing[d];
				  ar & origin[d];
				}
				itk::ImageRegionConstIterator<ImageType> it(i,region);
				it.GoToBegin();
				RealStorageType p;

				while( !it.IsAtEnd() )
				{
					p = it.Get();
					ar & p;
					++it;
				}

				/*BitSetStorageType ulongVal;
				BitSetType bitValues;
				size_t sizeCount = 0;
				for(it.GoToBegin(); !it.IsAtEnd(); ++it) {
				bitValues[sizeCount] = (it.Value() == BinaryPixelOn);
				sizeCount++;
				if (sizeCount == bitValues.size()) {
				ulongVal = bitValues.to_ulong();
				ar & ulongVal;
				sizeCount = 0;
				}
				}
				if (sizeCount != 0) {
				ulongVal = bitValues.to_ulong();
				ar & ulongVal;
				}
				*/
			}
			

			template<class Archive, unsigned Dimension>
			inline void serialize(Archive & ar, typename itk::SmartPointer< itk::Image<RealPixelType, Dimension> > &i, const unsigned int version)
			{
				boost::serialization::split_free(ar, i, version);
			}

			template<class Archive>
			inline void load(Archive & ar, CTImageTreeItem::SegmentationValueMap &svm, const unsigned int version)
			{
			  uint64_t s;
			  ar & s;
			  while(s) {
			    SegmentationValues sv;
			    ar & sv;
			    svm.insert(CTImageTreeItem::SegmentationValueMap::value_type(sv.segment, sv));
			    --s;
			  }
			}

			template<class Archive>
			inline void save(Archive & ar, const CTImageTreeItem::SegmentationValueMap &svm, const unsigned int version)
			{
				uint64_t s = svm.size();
				ar & s;
				BOOST_FOREACH( const CTImageTreeItem::SegmentationValueMap::value_type &svm_value, svm ) {
					const SegmentationValues &sv = svm_value.second;
					ar & sv;
				}
			}

			template<class Archive>
			void serialize(Archive & ar, QColor &color, const unsigned int version) {
				uint8_t t;
				t = color.red(); ar & t; color.setRed(t);
				t = color.green(); ar & t; color.setGreen(t);
				t = color.blue(); ar & t; color.setBlue(t);
			}



		} // namespace serialization
}

template<class Archive>
void CTImageTreeModel::serialize(Archive & ar, const unsigned int version) {
	ar.register_type(static_cast<TreeItem*>(NULL));
	ar.register_type(static_cast<ITKVTKTreeItem<BinaryImageType>*>(NULL));
	ar.register_type(static_cast<ITKVTKTreeItem<CTImageType>*>(NULL));
	ar.register_type(static_cast<CTImageTreeItem*>(NULL));
	ar.register_type(static_cast<BinaryImageTreeItem*>(NULL));
	ar.register_type(static_cast<RealImageTreeItem*>(NULL));

	beginResetModel(); 
	ar & HeaderFields;
	ar & rootItem;
	endResetModel(); 
}

template<class Archive>
void TreeItem::serialize(Archive & ar, const unsigned int version) {
	ar & model;
	ar & parentItem;
	ar & childItems;
}

template<class TImage>
template<class Archive>
void ITKVTKTreeItem<TImage>::serialize(Archive & ar, const unsigned int version) {
	ar & boost::serialization::base_object<BaseClass>(*this);
}

template<class Archive>
void BinaryImageTreeItem::serialize(Archive & ar, const unsigned int version) {
	ar & name;
	ar & color;
	ImageType::Pointer binIm = peekITKImage();
	ar & binIm;
	ar & boost::serialization::base_object<BaseClass>(*this);
	setITKImage( binIm );
	imageKeeper = getVTKConnector();
}

template<class Archive>
void RealImageTreeItem::serialize(Archive & ar, const unsigned int version) {
	ar & name;
	ImageType::Pointer realIm = peekITKImage();
	ar & realIm;
	ar & boost::serialization::base_object<BaseClass>(*this);
	setITKImage( realIm );
	imageKeeper = getVTKConnector();

	m_colorMap = vtkLookupTable::New();
	
	//Set the shape of the colorMap to linear
	m_colorMap->SetRampToLinear();
	//Set number of created colors
	m_colorMap->SetNumberOfTableValues(256);
	//Set the range of the image values
	m_colorMap->SetTableRange( 0, 10);
	//Set the range of the available colors
	m_colorMap->SetHueRange(0.0,0.667);
	//Set the saturation range of the colors
	//m_colorMap->SetSaturationRange( 0.7, 1 );
	//set the brightness of the colors
	m_colorMap->SetValueRange( 0.8, 1 );
	//generate LUT
	m_colorMap->Build();
	//in order to hide the maximum and minimum values, 
	//set the alpha of the borders to zero
	
	//create alpha ramp at the borders 
	for(int i = 0; i < 16; i++)
	{
		double rgba[4];
		m_colorMap->GetTableValue(i,rgba);
		m_colorMap->SetTableValue(i,rgba[0],rgba[1],rgba[2],1/16*i);

		m_colorMap->GetTableValue(255-i,rgba);
		m_colorMap->SetTableValue(255-i,rgba[0],rgba[1],rgba[2],1/16*i);
	}

	m_colorMap->SetAlpha(0.6);

	/*  ar & boost::serialization::base_object<BaseClass>(*this);
	ar & inside;
	thresholdInside->SetInput( inside->getITKImage() );
	ar & outside;
	thresholdOutside->SetInput( outside->getITKImage() );

	double gaussSigma = gaussGradMag->GetSigma();
	ar & gaussSigma;
	gaussGradMag->SetSigma( gaussSigma );
	gaussGradMag->SetInput( dynamic_cast<CTImageTreeItem*>(parent())->getITKImage() );
	*/
}

boost::filesystem::path normalize( const boost::filesystem::path &p_) {
	boost::filesystem::path p = p_;
	p = p.normalize();
	if (p.root_directory().empty()) return p;
	boost::filesystem::path result;
	bool lastWasRoot = false;
	for(boost::filesystem::path::const_iterator i = p.begin(); i != p.end(); ++i) {
		if (result.empty()) {
			if (*i == p.root_directory()) {
				lastWasRoot = true;
				result /= *i;
			}
		} else {
			if (lastWasRoot) {
				if (*i != ".." && *i != ".") {
					result /= *i;
					lastWasRoot = false;
				}
			} else result /= *i;
		}
	}
	return result;
}

boost::filesystem::path absoluteDirectory( const boost::filesystem::path &p_) {
	boost::filesystem::path p = p_;
	if (!p.is_complete()) p = boost::filesystem::current_path() / p;
	return normalize(p).branch_path();
}

// TEST
boost::filesystem::path relativeDirectory( const boost::filesystem::path &p_) {
  boost::filesystem::path p = p_;
  if (!p.is_complete()) p = boost::filesystem::current_path() / p;
  return normalize(p).branch_path();
}

boost::filesystem::path fromAtoB( const boost::filesystem::path &a, const boost::filesystem::path &b) {
	if (a.root_path() != b.root_path()) return boost::filesystem::path();
	boost::filesystem::path::const_iterator ia = a.begin();  
	boost::filesystem::path::const_iterator ib = b.begin();  
	boost::filesystem::path result;
	while(ia != a.end() && ib != b.end() && *ia == *ib) {
		ia++;
		ib++;
	}
	while(ia != a.end()) {
		result /= "..";
		ia++;
	}
	while(ib != b.end()) {
		result /= *ib;
		ib++;
	}
	return result;
}

template<class Archive>
void CTImageTreeItem::load(Archive & ar, const unsigned int version) {
	ar & itemUID;
	uint64_t fnListLength;
	ar & fnListLength;
	std::string fn;
	std::string serPathString;
	ar & serPathString;
	boost::filesystem::path serPath( serPathString );
	for(;fnListLength != 0; --fnListLength) {
		ar & fn;
		boost::filesystem::path fnPath( fn );
		if (!fnPath.is_complete()) {
			fnPath = normalize( serPath / fnPath );
		}
		fnList.insert(fnPath.string());
	}
	ar & HeaderFields;
	ar & dict;
	ar & boost::serialization::base_object<BaseClass>(*this);
	ar & segmentationValueCache;
}

template<class Archive>
void CTImageTreeItem::save(Archive & ar, const unsigned int version) const {
	ar & itemUID;
	const uint64_t fnListLength = fnList.size();
	ar & fnListLength;
	boost::filesystem::path serPath( absoluteDirectory( model->getSerializationPath() ) );
	std::string serPathString = serPath.string();
	ar & serPathString;
	BOOST_FOREACH( const std::string &name, fnList ) {
		boost::filesystem::path fnPath( name );
		boost::filesystem::path newFnPath = fromAtoB( serPath, fnPath );
		newFnPath = normalize(newFnPath);
		if (newFnPath.empty()) newFnPath = fnPath;
		ar & newFnPath.string();
	}
	ar & HeaderFields;
	ar & dict;
	ar & boost::serialization::base_object<BaseClass>(*this);
	ar & segmentationValueCache;
}


template<class Archive>
void SegmentationValues::serialize(Archive & ar, const unsigned int version) {
	ITKVTKTreeItem<BinaryImageType> *nonconstseg = const_cast<ITKVTKTreeItem<BinaryImageType> *>(segment);
	ar & nonconstseg;
	segment = nonconstseg;
	uint64_t imin(min), imax(max), isampleCount(sampleCount);
        uint64_t iaccuracy = static_cast<uint64_t>(accuracy);
	ar & mean; ar & stddev; ar & imin; ar & imax; ar & isampleCount; ar & iaccuracy;
	min = imin; max = imax; sampleCount = isampleCount; accuracy = static_cast<Accuracy>(iaccuracy);
	bool matchingMtime = mtime == segment->getITKMTime(); 
	ar & matchingMtime;
	if (matchingMtime)
		mtime = segment->getITKMTime();
	else mtime = 0; 
}

template<class Archive>
void DicomTagType::serialize(Archive & ar, const unsigned int version) {
	ar & name;
	ar & tag;
}



#endif
