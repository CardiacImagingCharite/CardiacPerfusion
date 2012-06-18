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

#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <itkMetaDataObject.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkLinearInterpolateImageFunction.h>
#include <QPalette>
#include <QInputDialog>
#include <QApplication>
#include <boost/assign.hpp>
#include <itkCastImageFilter.h>
#include "ctimagetreemodel.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <set>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/mean.hpp>

//Constructor
CTImageTreeItem::CTImageTreeItem(TreeItem * parent, DicomTagListPointer headerFields, const itk::MetaDataDictionary &_dict )
  :BaseClass(parent),HeaderFields(headerFields),dict(_dict),imageTime(-1) {
    getUIDFromDict(dict, itemUID);
}

TreeItem *CTImageTreeItem::clone(TreeItem *clonesParent) const {
	if (clonesParent==NULL) clonesParent = const_cast<TreeItem*>(parent());
	CTImageTreeItem *c = new CTImageTreeItem( clonesParent, HeaderFields, dict );
	c->fnList = fnList;
	cloneChildren(c);
	return c;
}

//
struct IndexCompareFunctor {
	typedef CTImageType::IndexType argT;

	bool operator()(const argT &x, const argT &y) const {
		if (x[0] < y[0]) return true;
		else if (x[0] > y[0]) return false;
    
		else if (x[1] < y[1]) return true;
		else if (x[1] > y[1]) return false;
    
		else if (x[2] < y[2]) return true;
		else /*if (x[2] > y[2])*/ return false;
	}
};
    
//returns the values of the segmentation
bool CTImageTreeItem::getSegmentationValues( SegmentationValues &values) const {
	//a map that contains the binary images and the segmentation values
/*	SegmentationValueMap::const_iterator it = segmentationValueCache.find( values.segment );
	// if values exist, no need to calculate again
	if (it != segmentationValueCache.end() //if segment was found
	&& it->second.mtime == values.segment->getITKMTime()
    && it->second.accuracy == values.accuracy) { //has the same accuracy
		values = it->second;
		return values.sampleCount > 0;
	}
*/	return internalGetSegmentationValues( values );
}

//calculate segmentation values
bool CTImageTreeItem::internalGetSegmentationValues( SegmentationValues &values) const {
	
	//get ITK image
	ImageType::Pointer image = getITKImage();
	if (image.IsNull()) 
		return false;
	
	//get buffered region of the image
	ImageType::RegionType ctregion = image->GetBufferedRegion();
	//define an iterator for the binary segment
	typedef itk::ImageRegionConstIteratorWithIndex< BinaryImageType > BinaryIteratorType;
	//get binary segment
	BinaryImageTreeItem::ImageType::Pointer segment = values.segment->getITKImage();
	if (segment.IsNull()) 
		return false;
	//create a binary iterator for the segment and its buffered region
	BinaryIteratorType binIter( segment, segment->GetBufferedRegion() );
	ImageType::PointType point;
  
	//The Accumulators Framework is framework for performing incremental calculations
	using namespace boost::accumulators;
	//http://boost-sandbox.sourceforge.net/libs/accumulators/doc/html/accumulators/user_s_guide.html#accumulators.user_s_guide.the_accumulators_framework
	accumulator_set<double,features<tag::count, tag::min, tag::mean, tag::max, tag::variance> > acc;  
  
	//check selected accuracy
	if (values.accuracy == SegmentationValues::SimpleAccuracy) {
		ImageType::IndexType index;
		//iterate over the pixel of the binary segment
		for(binIter.GoToBegin(); !binIter.IsAtEnd(); ++binIter) {
			//if actual value = 255
			if (binIter.Value() == BinaryPixelOn) {
				//transforms the index to a physical point in the binary segment
				segment->TransformIndexToPhysicalPoint(binIter.GetIndex(),point);
				//transform that point to an index of the CT image
				image->TransformPhysicalPointToIndex(point, index);
				//check if that index is inside the CT region
				if (ctregion.IsInside(index)) {
					//get the pixel value at the index
					int t = image->GetPixel(index);
					//check if pixel value != -2048
					if (isRealHUvalue(t)) {
						//accumulate pixel value
						acc( t );
					}
				}
			}
		}
	//check selected accuracy
	} else if (values.accuracy == SegmentationValues::PreventDoubleSamplingAccuracy) {
		ImageType::IndexType index;
		//definition for a set of indices, which can be compared
		typedef std::set< ImageType::IndexType, IndexCompareFunctor > IndexSetType;
		IndexSetType indexSet;
		//iterate over the pixel of the binary segment
		for(binIter.GoToBegin(); !binIter.IsAtEnd(); ++binIter) {
			//if actual value = 255
			if (binIter.Value() == BinaryPixelOn) {
				//transforms the index to a physical point in the binary segment
				segment->TransformIndexToPhysicalPoint(binIter.GetIndex(),point);
				//transform that point to an index of the CT image
				image->TransformPhysicalPointToIndex(point, index);
				//check if that index is inside the CT region
				if (ctregion.IsInside(index)) {

					std::pair<IndexSetType::iterator,IndexSetType::iterator> ret;
					//
					ret = indexSet.equal_range(index);
					//If x does not match any key in the container, the range returned has a length of zero, 
					//with both iterators pointing to the nearest value greater than x, if any, 
					//or to set::end if x is greater than all the elements in the container.
					if (ret.first == ret.second) {
						indexSet.insert(ret.first, index);
						//get the pixel value at the index
						int t = image->GetPixel(index);
						//check if pixel value != -2048
						if (isRealHUvalue(t)) {
							//accumulate pixel value
							acc( t );
						}
					}
				}
			}
		}
	//check selected accuracy
	} else if (values.accuracy == SegmentationValues::InterpolatedAccuracy) {
		//define an interpolate function
		typedef  itk::LinearInterpolateImageFunction< CTImageType > InterpolatorType;
		InterpolatorType::Pointer interpolator = InterpolatorType::New();
		//set input to the interpolator
		interpolator->SetInputImage( image );
		InterpolatorType::ContinuousIndexType index;
		//iterate over the pixel of the binary segment
		for(binIter.GoToBegin(); !binIter.IsAtEnd(); ++binIter) {
			//if actual value = 255
			if (binIter.Value() == BinaryPixelOn) {
				//transforms the index to a physical point in the binary segment
				segment->TransformIndexToPhysicalPoint(binIter.GetIndex(),point);
				//transform that point to an index of the CT image
				image->TransformPhysicalPointToContinuousIndex(point, index);
				//check if the index is inside the buffer of the interpolator
				if (interpolator->IsInsideBuffer(index)) {
					//Returns the linearly interpolated image intensity
					int t = interpolator->EvaluateAtContinuousIndex(index);
					//check if pixel value != -2048
					if (isRealHUvalue(t)) {
						//accumulate pixel value
						acc( t );
					}
				}
			}
		}  
	}
	//set sample count, min, mean, max and standard deviation
	//from the accumulated intensities
	values.sampleCount = count( acc );
	values.min = min( acc );
	values.mean = mean( acc );
	values.max = max( acc );
	values.stddev = std::sqrt( variance( acc ) );
	//set the image modification time
	values.mtime = segment->GetMTime();
	const_cast<CTImageTreeItem*>(this)->segmentationValueCache[ values.segment ] = values;
	return values.sampleCount > 0;
}    
    
    
bool CTImageTreeItem::setData(int column, const QVariant& value) {
	return false;
}

QVariant CTImageTreeItem::do_getData_UserRole(int column) const {
	if ((*HeaderFields)[ column ].tag == getAcquisitionDatetimeTag()) {
		return getTime();
	}
	return do_getData_DisplayRole(column);
}


QVariant CTImageTreeItem::do_getData_DisplayRole(int column) const {
	if (column < 0 || column >= int(HeaderFields->size())) 
		return QVariant::Invalid;
	if ((*HeaderFields)[ column ].tag == getNumberOfFramesTag()) 
		return getNumberOfSlices();
	if ((*HeaderFields)[ column ].tag == getAcquisitionDatetimeTag()) {
		boost::posix_time::ptime dicomTime = getPTime();
		return boost::posix_time::to_simple_string(dicomTime).c_str();
	}
	std::string val;
	itk::ExposeMetaData( dict, (*HeaderFields)[ column ].tag, val );
	return QString::fromAscii( val.c_str() );
}

QVariant CTImageTreeItem::do_getData_ForegroundRole(int column) const {
	if (peekITKImage().IsNull()) 
		return QApplication::palette().midlight();
	return QVariant::Invalid;
}

//get the acquisition time from meta data
boost::posix_time::ptime CTImageTreeItem::getPTime() const {
	std::string dicomTimeString;
	itk::ExposeMetaData( dict, getAcquisitionDatetimeTag(), dicomTimeString );
	if(dicomTimeString.empty())
	{
		std::string dicomContentDateString;
		std::string dicomAquistionTimeString;
		itk::ExposeMetaData( dict, getContentDateTag(), dicomContentDateString );
		itk::ExposeMetaData( dict, getAquisitionTimeTag(), dicomAquistionTimeString );
		dicomTimeString = dicomContentDateString + dicomAquistionTimeString;
	}
	using namespace boost::posix_time;
	time_input_facet *dicomTimeFacet = new time_input_facet("%Y%m%d%H%M%S%F");
	std::stringstream dicomTimeStream;
	dicomTimeStream.imbue( std::locale(std::locale::classic(), dicomTimeFacet) );
	dicomTimeStream.str( dicomTimeString );
	ptime dicomTime;
	dicomTimeStream >> dicomTime;
	return dicomTime;
}

//get image acquisition time
double CTImageTreeItem::getTime() const {
	if (imageTime > 0) 
		return imageTime;
	using namespace boost::posix_time;
	ptime dicomTime = getPTime();
	time_duration since1900 = dicomTime - ptime(boost::gregorian::date(1900,1,1));
	double secSince1900 = double(since1900.ticks()) / time_duration::ticks_per_second();
	return secSince1900;
}


Qt::ItemFlags CTImageTreeItem::flags(int column) const {
    if (column < 0 || column >= int(HeaderFields->size())) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;  
}

//return number of columns
int CTImageTreeItem::columnCount() const {
    return HeaderFields->size();
}

//get number of slices
int CTImageTreeItem::getNumberOfSlices() const {
	ImageType::Pointer itkImage = peekITKImage();
	if (itkImage.IsNotNull()) {
		ImageType::RegionType imageRegion = itkImage->GetLargestPossibleRegion();
		return static_cast<uint>(imageRegion.GetSize(2));
	} else {
		int num = fnList.size();
		if (num > 1) 
			return num;
		else {
			std::string t;
			itk::ExposeMetaData( dict, getNumberOfFramesTag(), t );
			std::istringstream buffer(t);
			buffer >> num;
			return num;
		}  
	}
}

//class for the progress dialog while reading images
class CTImageTreeItem::ReaderProgress : public itk::Command {
public: 
    
	typedef ReaderProgress Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::ImageSeriesReader< ImageType > ReaderType;
    itkNewMacro( Self );
    
	void setDialog( QProgressDialog *progress, int progressScale, int progressBase ) {
		if (progressDialog_scpd) progressDialog_scpd.reset(NULL);
		progressDialog = progress;
		this->progressScale = progressScale;
		this->progressBase = progressBase;
    }

protected:
    const static int defaultProgressScale = 10000;
    ReaderProgress():progressScale(defaultProgressScale), progressBase(0), progressDialog(NULL) {};
    
	void Execute( itk::Object *caller, const itk::EventObject &event) {
		Execute((const itk::Object*)caller, event);
    }

    void Execute( const itk::Object *caller, const itk::EventObject &event) {
		const ReaderType *reader = dynamic_cast<const ReaderType *>(caller);
		getDialog()->setValue( progressBase + reader->GetProgress() * progressScale );
    }

    QProgressDialog *getDialog(void) {
		if (progressDialog == NULL) {
			progressDialog_scpd.reset(new QProgressDialog(QObject::tr("Loading Volume..."), QObject::tr("Abort"), 0, progressScale));
			progressDialog = progressDialog_scpd.get();
			progressDialog->setCancelButton(0);
			progressDialog->setMinimumDuration(1000);
			progressDialog->setWindowModality(Qt::ApplicationModal);
		}
		return progressDialog;
    }

    int progressScale;
    int progressBase;
    boost::scoped_ptr< QProgressDialog > progressDialog_scpd;
    QProgressDialog *progressDialog;
};

//read CT image
void CTImageTreeItem::retrieveITKImage(QProgressDialog *progress, int progressScale, int progressBase) {
	//define and create file reader
	typedef ReaderProgress::ReaderType ReaderType;
	ReaderType::Pointer imageReader = ReaderType::New();
	ReaderType::FileNamesContainer fc;
	//assign filenames
	fc.assign(fnList.begin(), fnList.end());
	//set ImageIO and filenames
	itk::GDCMImageIO::Pointer gdcmImageIO = itk::GDCMImageIO::New();
	imageReader->SetImageIO( gdcmImageIO );
	imageReader->SetFileNames(fc);
	//create reader progress
	ReaderProgress::Pointer progressor = ReaderProgress::New();
	if (progress) 
		progressor->setDialog( progress, progressScale, progressBase );
	imageReader->AddObserver(itk::AnyEvent(), progressor);
	//try to read the images
	try {
		imageReader->Update();
	}catch( itk::ExceptionObject & excep ) {
		std::cerr << "Exception caught !" << std::endl;
		std::cerr << excep << std::endl;
	}
	//assign the reader output to an image pointer
	ImageType::Pointer imagePtr =  imageReader->GetOutput();
	setITKImage(imagePtr);
	//emit signal, that data has changed
	model->dataChanged(model->createIndex(childNumber(),0,parent()),model->createIndex(childNumber(),columnCount()-1,parent()));
}

//get the unique identifier from the meta data
void CTImageTreeItem::getUIDFromDict(const itk::MetaDataDictionary &dict, std::string &iUID) {
	std::string nFrames;
	itk::ExposeMetaData( dict, getNumberOfFramesTag(), nFrames );
//	if (nFrames.empty() || nFrames == "0") 
//		itk::ExposeMetaData( dict, getSeriesInstanceUIDTag(), iUID );
//	else  
		itk::ExposeMetaData( dict, getSOPInstanceUIDTag(), iUID );
}

//DICOM tags
const std::string &CTImageTreeItem::getNumberOfFramesTag() {
	const static std::string NumberOfFramesTag("0028|0008");
	return NumberOfFramesTag;
}
const std::string &CTImageTreeItem::getSOPInstanceUIDTag() {
	const static std::string SOPInstanceUIDTag("0008|0018");
	return SOPInstanceUIDTag;
}
const std::string &CTImageTreeItem::getSeriesInstanceUIDTag() {
	const static std::string SeriesInstanceUIDTag("0020|000e");
	return SeriesInstanceUIDTag;
}
const std::string &CTImageTreeItem::getAcquisitionDatetimeTag() {
	const static std::string SOPInstanceUIDTag("0008|002a");
	return SOPInstanceUIDTag;
}
const std::string &CTImageTreeItem::getContentDateTag() {
	const static std::string SOPInstanceUIDTag("0008|0023");
	return SOPInstanceUIDTag;
}
const std::string &CTImageTreeItem::getAquisitionTimeTag() {
	const static std::string SOPInstanceUIDTag("0008|0032");
	return SOPInstanceUIDTag;
}

//generate binary segment at actual CT image
BinaryImageTreeItem *CTImageTreeItem::generateSegment(void) {
	typedef itk::CastImageFilter< CTImageType, BinaryImageType> CastFilterType;
  
	bool ok;
	//show dialog for segment name
	QString segName = QInputDialog::getText(NULL, QObject::tr("Segment Name"),
						QObject::tr("Name:"), QLineEdit::Normal,
						QObject::tr("Unnamed Segment"), &ok);
	BinaryImageTreeItem::ImageType::Pointer seg;
	//if name is valid and dialog was closed with OK
	if (ok && !segName.isEmpty()) {
	    //create caster, that transforme the CT image to a binary image
		CastFilterType::Pointer caster = CastFilterType::New();
		caster->SetInput( getITKImage() );
		caster->Update();
		seg = caster->GetOutput();
		//fills the segment with zeros
		seg->FillBuffer(BinaryPixelOff);
		//create a binary tree item as child of this CT image
		BinaryImageTreeItem *result = new BinaryImageTreeItem(this, seg, segName);
		//insert child into the hierarchy
		insertChild(result);
		return result;
	}
	return NULL;
}

//generate binary segment at actual CT image
BinaryImageTreeItem *CTImageTreeItem::generateSegment(QString name) {
	typedef itk::CastImageFilter< CTImageType, BinaryImageType> CastFilterType;
	
	BinaryImageTreeItem::ImageType::Pointer seg;
	//if name is valid and dialog was closed with OK
	if (!name.isEmpty()) {
	    //create caster, that transforme the CT image to a binary image
		CastFilterType::Pointer caster = CastFilterType::New();
		caster->SetInput( getITKImage() );
		caster->Update();
		seg = caster->GetOutput();
		//fills the segment with zeros
		seg->FillBuffer(BinaryPixelOff);
		//create a binary tree item as child of this CT image
		BinaryImageTreeItem *result = new BinaryImageTreeItem(this, seg, name);
		//insert child into the hierarchy
		insertChild(result);
		return result;
	}
	return NULL;
}
/*

bool BinaryImageTreeItem::watershedParent(const BinaryImageTreeItem *includedSegment, const BinaryImageTreeItem *excludedSegment) {
  bool ok;
  double gaussSigma = QInputDialog::getDouble(NULL, QObject::tr("Watershed"),
    QObject::tr("Sigma value for Gaussian Smoothing:"), 2.0, 1.0, 10.0, .5, &ok);  
  if (!ok) return false;
  
  LabelImageType::Pointer labelImage;
  {
    typedef itk::BinaryThresholdImageFilter< BinaryImageType, LabelImageType > ThresholdFilterType;
    ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
    filter->SetInput( excludedSegment->getITKImage() );
    filter->SetLowerThreshold( BinaryPixelOn );
    filter->SetUpperThreshold( BinaryPixelOn );
    filter->SetInsideValue( 1 );
    filter->SetOutsideValue( 0 );
    filter->Update();
    labelImage = filter->GetOutput();
    typedef itk::ImageRegionConstIterator< ImageType > BinImageIterator;
    typedef itk::ImageRegionIterator< LabelImageType > LabelImageIterator;
    BinImageIterator incIt = BinImageIterator( includedSegment->getITKImage(), includedSegment->getITKImage()->GetBufferedRegion() );
    LabelImageIterator labIt = LabelImageIterator( labelImage, labelImage->GetBufferedRegion() );
    for(incIt.GoToBegin(), labIt.GoToBegin(); !incIt.IsAtEnd(); ++incIt, ++labIt) {
      if (incIt.Get() == BinaryPixelOn) labIt.Set(2);
    }    
  }
  
  CTImageTreeItem::ImageType::Pointer parentImagePointer = dynamic_cast<CTImageTreeItem*>(parent())->getITKImage();
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<CTImageTreeItem::ImageType, CTImageTreeItem::ImageType> GaussGradMagFilterType;
  GaussGradMagFilterType::Pointer gaussGradMag = GaussGradMagFilterType::New();
  gaussGradMag->SetInput(parentImagePointer);
  gaussGradMag->SetSigma( gaussSigma );
  
  typedef itk::MorphologicalWatershedFromMarkersImageFilter<CTImageTreeItem::ImageType, LabelImageType> WatershedFilterType;
  WatershedFilterType::Pointer watershed = WatershedFilterType::New();
  watershed->SetInput1(gaussGradMag->GetOutput());
  watershed->SetInput2(labelImage);
  watershed->Update();
  ImageType::Pointer result = watershed->GetOutput();
  setITKImage( result );  
  return true;
}

*/