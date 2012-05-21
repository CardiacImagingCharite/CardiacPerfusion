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

#include "binaryimagetreeitem.h"
#include <boost/random.hpp>
#include <itkImageRegionIterator.h>
#include <vector>
#include "ctimagetreeitem.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkCastImageFilter.h>
//#include <itkSliceBySliceImageFilter.h>

#include <QMessageBox>

// Constructor, which takes its parent, an Image and a name
BinaryImageTreeItem::BinaryImageTreeItem(TreeItem * parent, ImageType::Pointer itkImage, const QString &name)
  :BaseClass(parent, itkImage), name(name), volumeMtime(0) {
    imageKeeper = getVTKConnector();
    createRandomColor();
}

//clones an existing TreeItem
TreeItem *BinaryImageTreeItem::clone(TreeItem *clonesParent) const {
	BinaryImageTreeItem *c = new BinaryImageTreeItem(clonesParent, peekITKImage(), name );
	cloneChildren(c);
	return c;
}

//returns the number of columns (always 1, cause there is only the name of the Item)
int BinaryImageTreeItem::columnCount() const {
	return 1;
}

//returns the name of the TreeItem
QVariant BinaryImageTreeItem::do_getData_DisplayRole(int c) const {
	if (c==0) 
		return name;
	else 
		return QVariant::Invalid;
}

//returns the color of the TreeItem
QVariant BinaryImageTreeItem::do_getData_BackgroundRole(int column) const {
	return QBrush( color );
}

//returns the properties of a TreeItem
Qt::ItemFlags BinaryImageTreeItem::flags(int column) const {
    if (column != 0) 
		return Qt::NoItemFlags;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;  
}

//sets the name of the TreeItem for column 0 and return true, if succeed
bool BinaryImageTreeItem::setData(int c, const QVariant &value) {
	if (c==0 && static_cast<QMetaType::Type>(value.type()) == QMetaType::QString) {
		name = value.toString();
		return true;
	} 
	return false;
}

template <typename T>
inline T clip(T min, T val, T max) {
	return std::max<T>( std::min<T>( val, max), min );
}

//draws a sphere with a given radius at a specific position (or erases it)
void BinaryImageTreeItem::drawSphere( float radius, float x, float y, float z, bool erase ) {
	//define index and create point at position (x,y,z)
	itk::ContinuousIndex< double, ImageDimension > idx;
	ImageType::PointType point;
	point[0] = x;point[1] = y;point[2] = z;
	//get ITK image
	ImageType::Pointer itkIm = getITKImage();
	if (itkIm.IsNull()) 
		return;
	//transform point to an index
	itkIm->TransformPhysicalPointToContinuousIndex(point, idx);
	//get spacing and size of the image
	ImageType::SpacingType spacing = itkIm->GetSpacing();
	ImageType::SizeType size = itkIm->GetBufferedRegion().GetSize();
  
	long start[3],end[3];
	ImageType::SizeType itSize;
	ImageType::IndexType itIndex;
	
	//iterate over the three dimensions
	for(int i=0; i < 3; ++i) {
		//calculate radius in  dependency on spacing
		double iradius =  std::abs(radius/spacing[i]);
		//calculate start, end, size and index
		//use clip for border checking 
		start[i]	= (int)(idx[i] - iradius); start[i] 	= clip<long>(0, start[i], size[i]);
		end[i]	= (int)(idx[i] + iradius); end[i] 	= clip<long>(0, end[i], size[i]);
		itSize[i] = end[i] - start[i];
		itIndex[i] = start[i];
	}
  
	//create region for the sphere
	ImageType::RegionType DrawRegion;
	//set size and position
	DrawRegion.SetSize( itSize );
	DrawRegion.SetIndex( itIndex );
	//create iterator for the region
	typedef itk::ImageRegionIterator< ImageType > BinImageIterator;
	BinImageIterator iterator = BinImageIterator( itkIm, DrawRegion );
	iterator.GoToBegin();
	float brushRadius2 = radius * radius;

	static std::vector< float > prodx;
	prodx.resize(end[0] - start[0]);
	
	for(int lx = start[0]; lx < end[0]; ++lx) {
		float t = (lx - idx[0]) * spacing[0]; t *= t;
		prodx[lx-start[0]] = t;
	}
	//paint the sphere (or erase)
	BinaryPixelType pixelVal = BinaryPixelOn;
	if (erase) 
		pixelVal = BinaryPixelOff;

	for(int lz = start[2]; lz < end[2]; ++lz) {
		float sumz = (lz - idx[2]) * spacing[2]; sumz *= sumz;
		for(int ly = start[1]; ly < end[1]; ++ly) {
			float sumy = (ly - idx[1]) * spacing[1]; sumy *= sumy; sumy += sumz;
			for(int lx = start[0]; lx < end[0]; ++lx) {
				if ((prodx[lx-start[0]] + sumy) < brushRadius2) {
					iterator.Set(pixelVal);
				}
				++iterator;
			}
		}
	}
	//emit signal that data was modified
	itkIm->Modified();
	dynamic_cast<ConnectorData*>(getVTKConnector().get())->getConnector()->Modified();
}

//applies a regionGrow algorithm from a given seed with a specific threshold to the parent image
void BinaryImageTreeItem::regionGrow( float x, float y, float z, int threshold, boost::function<void()> clearAction) {
	
	//create index and point at position (x,y,z)
	ImageType::IndexType idx;

	ImageType::PointType point;
	point[0] = x;point[1] = y;point[2] = z;
	//ImageType::Pointer itkIm = getITKImage();
	//get ITK CT image
	CTImageTreeItem::ImageType::Pointer parentImagePointer = dynamic_cast<CTImageTreeItem*>(parent())->getITKImage();
	if (parentImagePointer.IsNull()) 
		return;

	ImageType::SizeType size = parentImagePointer->GetBufferedRegion().GetSize();

	//transform point to index
	bool outOfImage = parentImagePointer->TransformPhysicalPointToIndex(point, idx);
	int test = clip<long>(0, idx[2], size[2]);
	idx[2] = test;
	//get pixel at index
	ImageType::PixelType p = parentImagePointer->GetPixel(idx);
	//if (p == BinaryPixelOn) {

	//define gaussian smoothing filter
	typedef itk::RecursiveGaussianImageFilter< CTImageType, CTImageType > denoiseFilterType;
	denoiseFilterType::Pointer denoiseFilter = denoiseFilterType::New();
	denoiseFilter->SetSigma(4.0);
	denoiseFilter->SetInput(parentImagePointer);

	//define region grow filter
	typedef itk::ConnectedThresholdImageFilter< CTImageType, BinaryImageType > RegionGrowFilterType;
	RegionGrowFilterType::Pointer filter = RegionGrowFilterType::New();
	//connect filter to the output of the smoothing filter
	filter->SetInput( denoiseFilter->GetOutput() );
	//set borders in dependence of the given threshold

	float lower = p-(threshold/2);
	float upper = p+(threshold/2);

	filter->SetLower(lower);
	filter->SetUpper(upper);
	//set replace value to 255
	filter->SetReplaceValue(BinaryPixelOn);
	filter->SetSeed(idx);
	filter->Update();
	//create image pointer for the result
	ImageType::Pointer result = filter->GetOutput();
	//set the result as actual segment
	setITKImage( result );
	clearAction();
	//} else {
	//  QMessageBox::information(0,QObject::tr("Region Grow Error"),QObject::tr("Click on a part of the segmentation"));
	//}
	
}

void BinaryImageTreeItem::setPixel(float x, float y, float z)
{
	//get ITK image
	ImageType::Pointer itkIm = getITKImage();
	if (itkIm.IsNull()) 
		return;

	CTImageType::IndexType pixelIndex;
    pixelIndex[0] = x;
    pixelIndex[1] = y;
	pixelIndex[2] = z;
 
	itkIm->SetPixel(pixelIndex, BinaryPixelOn);
}

/*void BinaryImageTreeItem::regionGrow( float x, float y, float z, boost::function<void()> clearAction) {
  ImageType::IndexType idx;
  ImageType::PointType point;
  point[0] = x;point[1] = y;point[2] = z;
  ImageType::Pointer itkIm = getITKImage();
  if (itkIm.IsNull()) return;
  itkIm->TransformPhysicalPointToIndex(point, idx);
  ImageType::PixelType p = itkIm->GetPixel(idx);
  if (p == BinaryPixelOn) {
    typedef itk::ConnectedThresholdImageFilter< ImageType, ImageType > RegionGrowFilterType;
    RegionGrowFilterType::Pointer filter = RegionGrowFilterType::New();
    filter->SetInput( itkIm );
    filter->SetLower(BinaryPixelOn);
    filter->SetUpper(BinaryPixelOn);
    filter->SetReplaceValue(BinaryPixelOn);
    filter->SetSeed(idx);
    filter->Update();
    ImageType::Pointer result = filter->GetOutput();
    setITKImage( result );
    clearAction();
  } else {
    QMessageBox::information(0,QObject::tr("Region Grow Error"),QObject::tr("Click on a part of the segmentation"));
  }
}
*/

//applies a canny edge filter to the parent image
/*void BinaryImageTreeItem::extractEdges()
{
	//get ITK CT image
	CTImageTreeItem::ImageType::Pointer parentImagePointer = dynamic_cast<CTImageTreeItem*>(parent())->getITKImage();
	//define image conversion filter
	typedef itk::CastImageFilter<
               CTImageType, RealImageType >  CT2Real;
	typedef itk::CastImageFilter<
			   RealImageType, LabelImageType>  Real2Labeled;

	//convert CT image to real image
	CT2Real::Pointer ct2realFilter = CT2Real::New();
	Real2Labeled::Pointer real2LabeledFilter = Real2Labeled::New();
	ct2realFilter->SetInput(parentImagePointer);

	//define and create canny edge filter
	typedef itk::CannyEdgeDetectionImageFilter< RealImageType, RealImageType> EdgeFilterType;
	EdgeFilterType::Pointer filter = EdgeFilterType::New();
	filter->SetInput(ct2realFilter->GetOutput());
	filter->SetVariance(2.0);
	filter->SetLowerThreshold(30.0);
	filter->SetUpperThreshold(50.0);

	//convert filter output to labeled image
	real2LabeledFilter->SetInput(filter->GetOutput());
	ImageType::Pointer result = real2LabeledFilter->GetOutput();
	//set the result as actual segment
	setITKImage(result);
}
*/
//applies a threshold filter with given borders to the parent image
void BinaryImageTreeItem::thresholdParent(double lower, double upper) {
	//get ITK CT image
	CTImageTreeItem::ImageType::Pointer parentImagePointer = dynamic_cast<CTImageTreeItem*>(parent())->getITKImage();
	//define and create threshold filter
	typedef itk::BinaryThresholdImageFilter< CTImageType, BinaryImageType > ThresholdFilterType;
	ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
	filter->SetInput( parentImagePointer );
	filter->SetLowerThreshold( lower );
	filter->SetUpperThreshold( upper );
	filter->SetInsideValue( BinaryPixelOn );
	filter->SetOutsideValue( BinaryPixelOff );
	filter->Update();
	//get result and set binary ITK image
	ImageType::Pointer result = filter->GetOutput();
	setITKImage( result );
}

//dilates the binary overlay image with a given number of iteration
void BinaryImageTreeItem::binaryDilate(int iterations) {
	//get ITK binary image
	ImageType::Pointer itkIm = getITKImage();
	if (itkIm.IsNull()) 
		return;
	//define structuring element and dilate filter
	typedef itk::BinaryBallStructuringElement< BinaryPixelType, ImageDimension > KernelType;
	typedef itk::BinaryDilateImageFilter< BinaryImageType, BinaryImageType, KernelType > FilterType;
	//create and configure filter
	FilterType::Pointer filter = FilterType::New();
	filter->SetInput( itkIm );
	KernelType kernel;
	kernel.SetRadius( iterations );
	kernel.CreateStructuringElement();
	filter->SetBackgroundValue(BinaryPixelOff);
	filter->SetForegroundValue(BinaryPixelOn);
	filter->SetKernel( kernel );
	filter->Update();
	//get result and set binary ITK image
	ImageType::Pointer result = filter->GetOutput();
	setITKImage( result );
}

//erodes the binary overlay image with a given number of iteration
void BinaryImageTreeItem::binaryErode(int iterations) {
	//get binary ITK image
	ImageType::Pointer itkIm = getITKImage();
	if (itkIm.IsNull()) 
		return;
	//define structuring element and erode filter
	typedef itk::BinaryBallStructuringElement< BinaryPixelType, ImageDimension > KernelType;
	typedef itk::BinaryErodeImageFilter< BinaryImageType, BinaryImageType, KernelType > FilterType;
	//create and configure filter
	FilterType::Pointer filter = FilterType::New();
	filter->SetInput( itkIm );
	KernelType kernel;
	kernel.SetRadius( iterations );
	kernel.CreateStructuringElement();
	filter->SetBackgroundValue(BinaryPixelOff);
	filter->SetForegroundValue(BinaryPixelOn);
	filter->SetKernel( kernel );
	filter->Update();
	//get result and set binary ITK image
	ImageType::Pointer result = filter->GetOutput();
	setITKImage( result );
}


double BinaryImageTreeItem::getVolumeInML(void) const {
  ImageType::Pointer itkIm = getITKImage();
  if (itkIm.IsNull()) return -1;
  if (volumeMtime != itkIm->GetMTime()) {
    unsigned long voxelCount = 0;
    typedef itk::ImageRegionConstIterator< ImageType > BinImageIterator;
    BinImageIterator iterator = BinImageIterator( itkIm, itkIm->GetBufferedRegion() );
    for(iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator) {
      if (iterator.Get() == BinaryPixelOn) voxelCount++;
    }
    ImageType::SpacingType spacing = itkIm->GetSpacing();
    const_cast<BinaryImageTreeItem*>(this)->volumeInML = std::abs(spacing[0] * spacing[1] * spacing[2] * voxelCount / 1000.0);
    const_cast<BinaryImageTreeItem*>(this)->volumeMtime = itkIm->GetMTime();
  }
  return volumeInML;
}

//creates a random color for the overlay
void BinaryImageTreeItem::createRandomColor() {
  static boost::mt19937 rng;
  static boost::uniform_int<> rainbow(0,256*6);
  static boost::variate_generator<boost::mt19937&, boost::uniform_int<> > rainbowcolor(rng, rainbow);
  int hue = rainbowcolor();
  color = Qt::black;
  if (hue<256) {
    int ascend = hue;
      color.setRed(255);
      color.setGreen(ascend);
  } else if (hue<512) {
    int descend = 511-hue;
      color.setRed(descend);
      color.setGreen(255);
  } else if (hue<768) {
    int ascend = hue-512;
      color.setGreen(255);
      color.setBlue(ascend);
  } else if (hue<1024) {
    int descend = 1023-hue;
      color.setGreen(descend);
      color.setBlue(255);
  } else if (hue<1280) {
    int ascend = hue-1024;
      color.setRed(ascend);
      color.setBlue(255);
  } else { //if (color<1536) 
    int descend = 1535-hue;
      color.setRed(255);
      color.setBlue(descend);
  }
}

/*void BinaryImageTreeItem::test(float x, float y, float z, int r)
{
	std::cout << x << "\t" << y << "\t" << z << "\t" << r << std::endl;
}
*/