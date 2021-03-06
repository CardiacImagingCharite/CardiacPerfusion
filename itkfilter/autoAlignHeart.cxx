/*===========================================================================
 * 
 *   This is a program to automatically detect the left ventricular myocardial
 *   long axis in cardiac CT scans and align it parallel to the coordinate axes
 *	 The program requires ITK which is available at <http://www.itk.org>
 *	 It has been tested with ITK version 4.2.0.
 * 
 *   Copyright (C) 2012 Till Huelnhagen, Marc Dewey, Henning Meyer
 * 
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * ===========================================================================*/

#include "autoAlignHeart.h"

#include "itkResampleImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkLabelMapToLabelImageFilter.h"
#include "itkShapeOpeningLabelMapFilter.h"
#include "itkTimeProbe.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryShapeKeepNObjectsImageFilter.h"
#include <fstream>
#include <vector>
#include <QProgressDialog>




template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}


//******************************
// Main program
//******************************

autoAlignHeart::autoAlignHeart() {
	
	m_ellipsoidLength = 0;
	m_center[0] = 0;
	m_center[1] = 0;
	m_center[2] = 0;
	
}

//_______________________________________________________________________________

autoAlignHeart::~autoAlignHeart() {
	
}

//_______________________________________________________________________________

autoAlignHeart::AffineTransformType::Pointer autoAlignHeart::getTrafo(CTImageType* InputImage, int ThresholdLow, int ThresholdUp, int KernelSize)
{
	// ProgressDialog with progress scale estimated from one example
	// 
	// Downsampling image...                           done    ( 0.835101s )  => 1
	// Thresholding...                                 done    ( 0.540424s )  => 1
	// Morphological opening...                        done    ( 3.76331s )   => 4
	// Creating label map...                           done    ( 3.36174s )   => 4
	// Removing all but the largest struture...        done    ( 2.6894s )    => 3
	// Creating label map...                           done    ( 2.24938s )   => 3
	// Resampling Image...                             done    ( 2.34192s )   => 3
	// Analyzing blobs in each slice...                done    ( 2.17635s )   => 3
	// ---------------------------------------------------------------------------
	// sum                                                                      22
	
	
	const int progressStepDownsampling         = 1;
	const int progressStepThresholding         = 1;
	const int progressStepMorphologicalOpening = 4;
	const int progressStepCreatingLabelMap1    = 4;
	const int progressStepRemovingStructures   = 3;
	const int progressStepCreatingLabelMap2    = 3;
	const int progressStepResampling           = 3;
	const int progressStepAnalyzingBlobs       = 3;
	
	const int progressScale = progressStepDownsampling
	+ progressStepThresholding 
	+ progressStepMorphologicalOpening 
	+ progressStepCreatingLabelMap1 
	+ progressStepRemovingStructures 
	+ progressStepCreatingLabelMap2 
	+ progressStepResampling 
	+ progressStepAnalyzingBlobs;
	
	int progress = 0;
	
	QProgressDialog alignProgress( QObject::tr("align Heart..."), QObject::tr("Abort"), progress, progressScale);
	alignProgress.setMinimumDuration(1000);
	alignProgress.setWindowModality(Qt::ApplicationModal);
	
	itk::TimeProbe clock;
	clock.Start();
	
	//**************************************
	// 1. Downsample the image to save time
	//**************************************
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	
	const CTImageType::SpacingType& inputSpacing = InputImage->GetSpacing();
	const CTImageType::RegionType& inputRegion = InputImage->GetLargestPossibleRegion();
	const CTImageType::SizeType& inputSize = inputRegion.GetSize();
	
	std::cout << "Input image Information: " << std::endl;
	std::cout << "Origin:  " << InputImage->GetOrigin() << std::endl;
	std::cout << "Size: " << inputSize << std::endl;
	std::cout << "Spacing: " << inputSpacing << std::endl;
	
	// Change image spacing to 1.5mm for downsampling
	CTImageType::SpacingType outputSpacing;
	outputSpacing[0] = 1.5 * sgn(inputSpacing[0]);
	outputSpacing[1] = 1.5 * sgn(inputSpacing[1]);
	outputSpacing[2] = 1.5 * sgn(inputSpacing[2]);
	
	std::cout << "\nDownsampling image... ";
	
	AffineTransformType::Pointer identityTransform = AffineTransformType::New();
	identityTransform->SetIdentity();
	
	CTImageType::Pointer image = resampleImage(InputImage, identityTransform, interpolator, outputSpacing, false);
	clock.Stop();
	std::cout << "\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	std::cout << "New size: " << image->GetLargestPossibleRegion().GetSize() << "\n" << std::endl;
	
	// update progress dialog
	progress += progressStepDownsampling;
	alignProgress.setValue(progress);
	
	// calculate center of the image
	CTImageType::PointType imageCenter;
	CTImageType::IndexType imageCenterIndex;
	CTImageType::SizeType imageSize = image->GetLargestPossibleRegion().GetSize();
	imageCenterIndex[0] = imageSize[0] / 2 + 0.5;
	imageCenterIndex[1] = imageSize[1] / 2 + 0.5;
	imageCenterIndex[2] = imageSize[2] / 2 + 0.5;
	image->TransformIndexToPhysicalPoint( imageCenterIndex, imageCenter );
	
	//***************************************
	// 2. Thresholding
	//***************************************
	std::cout << "Thresholding... ";
	
	BinaryImageType::PixelType outsideValue, insideValue;
	CTImageType::PixelType lowerThreshold, upperThreshold;
	outsideValue = 0;
	insideValue = 255;
	lowerThreshold = ThresholdLow;
	upperThreshold = ThresholdUp;
	
	BinaryImageType::Pointer thresholdImage = binaryThreshold(image, lowerThreshold, upperThreshold, insideValue, outsideValue);
	
	clock.Stop();
	std::cout << "\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepThresholding;
	alignProgress.setValue(progress);
	
	//***************************************
	// 3. Strong morphological opening
	//***************************************
	std::cout << "Morphological opening... ";
	
	ErodeFilterType::Pointer binaryErode = ErodeFilterType::New();
	DilateFilterType::Pointer binaryDilate = DilateFilterType::New();
	
	StructuringElementType structuringElement;
	structuringElement.SetRadius( KernelSize ); // 3x3 structuring element for radius 1
	structuringElement.CreateStructuringElement();
	
	binaryErode->SetKernel( structuringElement );
	binaryDilate->SetKernel( structuringElement );
	
	binaryErode->SetInput( thresholdImage );
	binaryDilate->SetInput( binaryErode->GetOutput());
	
	binaryErode->Update();
	
	clock.Stop();
	std::cout << "\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepMorphologicalOpening;
	alignProgress.setValue(progress);
	
	//***************************************
	// 4. Convert binary image to label map
	//***************************************
	std::cout << "Creating label map... ";
	
	LabelMapType3D::Pointer labelMap = createLabelMap( binaryDilate->GetOutput());
	
	clock.Stop();
	std::cout << "\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	std::cout << "There are " << labelMap->GetNumberOfLabelObjects() << " objects." << std::endl;
	
	// update progress dialog
	progress += progressStepCreatingLabelMap1;
	alignProgress.setValue(progress);
	
	if( labelMap->GetNumberOfLabelObjects() == 0 ) {
		std::cout << "ERROR: No objects could be found in the image. Please check thresholds and morphology kernel size. \n\nAbort" << std::endl;
		AffineTransformType::Pointer failTrafo = AffineTransformType::New();
		failTrafo->SetIdentity();
		return failTrafo;
	}
	
	//***************************************
	// 4.1 Remove all but the largest object
	//***************************************
	std::cout << "\nRemoving all but the largest struture... ";
	
	//find largest object
	int objectNumber = labelMap->GetNumberOfLabelObjects();
	double maxSize = 0;
	int maxLabel = 0;
	for( int j = 0; j < objectNumber; j++) {
		LabelMapType3D::LabelObjectType* currentLabelObject = labelMap->GetNthLabelObject(j);
		double currentSize = currentLabelObject->GetNumberOfPixels();
		if( currentSize > maxSize) {
			maxSize = currentSize;
			maxLabel = j;
		}
	}
	//std::cout << "Label with maximum size: " << maxLabel << std::endl;
	//delete the remaining objects
	while( objectNumber > 0) {
		objectNumber--;
		if( objectNumber != maxLabel) {
			labelMap->RemoveLabelObject(labelMap->GetNthLabelObject(objectNumber));
		}
	}
	maxLabel = 0;
	
	clock.Stop();
	std::cout << "\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepRemovingStructures;
	alignProgress.setValue(progress);
	
	// print out info
	std::cout << "There is(are) " << labelMap->GetNumberOfLabelObjects() << " object(s) remaining." << std::endl;
	LabelMapType3D::LabelObjectType* labelObject = labelMap->GetNthLabelObject(maxLabel);
	
	//************************************
	// 4.2 Create Label Image from Label Map
	//************************************
	std::cout << "Creating label map... ";
	
	CTImageType::Pointer labelImage = createImageFromLabelMap( labelMap );
	
	LabelMapType3D::RegionType labelRegion = labelObject->GetBoundingBox();
	std::cout << "BoxSize           = " << labelRegion.GetSize() << std::endl;
	std::cout << "EllipsoidDiameter = " << labelObject->GetEquivalentEllipsoidDiameter() << std::endl;
	std::cout << "Radius            = " << labelObject->GetEquivalentSphericalRadius() << std::endl;
	std::cout << "FeretDiameter     = " << labelObject->GetFeretDiameter() << std::endl;
	std::cout << "EllipsoidCentroid = " << labelObject->GetCentroid() << std::endl;
	
	
	m_ellipsoidLength = labelObject->GetEquivalentEllipsoidDiameter()[2];
	m_center[0] = labelObject->GetCentroid()[0];
	m_center[1] = labelObject->GetCentroid()[1];
	m_center[2] = labelObject->GetCentroid()[2];
	
	clock.Stop();
	std::cout << "\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepCreatingLabelMap1;
	alignProgress.setValue(progress);
	
	//******************************************************************************************************
	// 5. Rotate the image to make the principal axes of the main component parallel to the coordinate axes
	//******************************************************************************************************
	const CTImageType::SizeType& inputImageSize = image->GetBufferedRegion().GetSize();
	
	AffineTransformType::Pointer trafo = AffineTransformType::New();
	trafo->SetIdentity();
	
	trafo->SetCenter( imageCenter );
	trafo->SetMatrix(labelObject->GetPrincipalAxesToPhysicalAxesTransform()->GetMatrix());
	
	std::cout << "\nResampling Image... ";
	
	// Do the resampling
	ResampleFilterType::Pointer resampler2 = ResampleFilterType::New();
	resampler2->SetTransform(trafo);
	resampler2->SetInput( labelImage );  // Set the Label Image as output to be written
	resampler2->SetInterpolator( interpolator );
	resampler2->SetDefaultPixelValue( 0 );
	resampler2->SetOutputOrigin( labelImage->GetOrigin());
	resampler2->SetOutputSpacing( outputSpacing );
	resampler2->SetOutputDirection( labelImage->GetDirection());
	resampler2->SetSize( inputImageSize );
	resampler2->Update();
	
	clock.Stop();
	std::cout << "\t\t\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepResampling;
	alignProgress.setValue(progress);
	
	//************************************
	// 6. Analyse blobs in all slices
	//************************************
	std::cout << "Analyzing blobs in each slice... ";
	
	// create stringstream to save data for statisical analysis
	std::ostringstream stringStream;
	stringStream << "slice; distance; roundness0; roundness1; size0; size1; ellipsoidDiameter1_0; ellipsoidDiameter1_1; ellipsoidDiameter2_0; ellipsoidDiameter2_1; elongation0; elongation1" << std::endl; 
	
	double distance = 0;
	std::vector<double> distances;
	std::vector<int> numberOfBlobs;
	int maxDistanceSlice = 0;
	double maxDistance = 0;
	LabelImageType2D::PointType c0_old, c1_old, c0, c1;
	c0_old.Fill( 0 );
	c1_old.Fill( 0 );
	
	for( unsigned int sliceNumber = 0; sliceNumber < inputImageSize[2]; sliceNumber++ ) {
		distance = 0;
		
		// conduct blob analysis for current slice
		LabelMapType2D::Pointer labelMap = getBlobs( resampler2->GetOutput(), sliceNumber);
		int currentNumberOfBlobs = labelMap->GetNumberOfLabelObjects();
		numberOfBlobs.push_back( currentNumberOfBlobs );
		
		if( currentNumberOfBlobs == 1 ) {
			LabelMapType2D::LabelObjectType::Pointer object0 = labelMap->GetNthLabelObject( 0 );
			stringStream << sliceNumber << "; 0; " << object0->GetRoundness() << "; 0; " 
			<< object0->GetNumberOfPixels() << "; 0; " 
			<< object0->GetEquivalentEllipsoidDiameter()[0] << "; 0; " 
			<< object0->GetEquivalentEllipsoidDiameter()[1] << "; 0; "
			<< object0->GetElongation() << "; 0"
			<< std::endl;
			c0_old = object0->GetCentroid();
			c1_old.Fill ( 0 );
		}
		
		else if( currentNumberOfBlobs > 1 ) {
			LabelMapType2D::LabelObjectType::Pointer object0 = labelMap->GetNthLabelObject( 0 );
			LabelMapType2D::LabelObjectType::Pointer object1 = labelMap->GetNthLabelObject( 1 );
			
			// check assignment of labels
			LabelImageType2D::PointType c0new = object0->GetCentroid();
			LabelImageType2D::PointType c1new = object1->GetCentroid();
			double d0 = sqrt( (c0new[0] - c0_old[0]) * (c0new[0] - c0_old[0]) + (c0new[1] - c0_old[1]) * (c0new[1] - c0_old[1]) );
			double d1 = sqrt( (c1new[0] - c0_old[0]) * (c1new[0] - c0_old[0]) + (c1new[1] - c0_old[1]) * (c1new[1] - c0_old[1]) );
			if( d0 > d1 ) {
				object1 = labelMap->GetNthLabelObject( 0 );
				object0 = labelMap->GetNthLabelObject( 1 );
			}
			
			c0 = object0->GetCentroid();
			c1 = object1->GetCentroid();
			c0_old = c0;
			c1_old = c1;
			double sizeRatio = double(object0->GetNumberOfPixels()) / double(object1->GetNumberOfPixels());
			sizeRatio = (sizeRatio>1)? 1/sizeRatio:sizeRatio;
			if( sizeRatio > 0.1 ) {
				distance = sqrt( (c0[0] - c1[0]) * (c0[0] - c1[0]) + (c0[1] - c1[1]) * (c0[1] - c1[1]) );
				if (distance > maxDistance) {
					maxDistance = distance;
					maxDistanceSlice = sliceNumber;
				}
			}
			stringStream << sliceNumber << "; " << distance << "; " 
			<< object0->GetRoundness() << "; " << object1->GetRoundness() << "; " 
			<< object0->GetNumberOfPixels() << "; " << object1->GetNumberOfPixels() << "; " 
			<< object0->GetEquivalentEllipsoidDiameter()[0] << "; " << object1->GetEquivalentEllipsoidDiameter()[0] << "; "
			<< object0->GetEquivalentEllipsoidDiameter()[1] << "; " << object1->GetEquivalentEllipsoidDiameter()[1] << "; "
			<< object0->GetElongation() << "; " << object1->GetElongation() << std::endl;
		}
		
		else
			stringStream << sliceNumber << "; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0; 0" << std::endl;
		distances.push_back( distance );
	}
	
	clock.Stop();
	std::cout << "\t\tdone\t( " << clock.GetMean() << "s )\n" << std::endl;
	clock = itk::TimeProbe();
	clock.Start();
	
	// update progress dialog
	progress += progressStepAnalyzingBlobs;
	alignProgress.setValue(progress);
	
	// write resulting data to file
	std::ofstream myfile;
	myfile.open ("data.txt");
	myfile << stringStream.str();
	myfile.close();
	std::cout << "\nOutput written to data.txt and distancesMedian.txt" << std::endl;
	
	//****************************************************
	// 7. Calculate regression line of the blob distances
	//****************************************************
	// calculate mean in x and y
	double sumY = 0, meanX, meanY;
	for( unsigned int x = 0; x < distances.size(); x++) {
		sumY += distances[x];
	}
	meanX = distances.size() / 2;
	meanY = sumY / distances.size();
	
	double numerator = 0, denominator = 0;
	
	for( unsigned int x = 0; x < distances.size(); x++) {
		numerator += ( x - meanX ) * ( distances[x] - meanY );
		denominator += ( x - meanX ) * ( x - meanX );
	}
	double slope = numerator / denominator;
	std::cout << "Slope of regression line: " << slope << std::endl;
	
	std::vector<double> filteredVector = medianFilterVector( distances, 2 );
	myfile.open ("distancesMedian.txt");
	for( unsigned int i = 0; i < filteredVector.size(); i++)
		myfile << filteredVector[i] << std::endl;
	myfile.close();
	std::cout << "\nOutput written to distancesMedian.txt" << std::endl;
	
	//************************************************
	// 8. Find slice to cut to delinate the ventricle
	//************************************************
	int cutSlice = maxDistanceSlice;
	
	while( filteredVector[cutSlice] > 0) {
		cutSlice = cutSlice - sgn(slope) * 1;
	}
	std::cout << "\nCut slice: " << cutSlice << "\n" << std::endl;
	
	//extract image region with ventricle
	typedef itk::ExtractImageFilter< CTImageType, CTImageType > ExtractImageFilterType;
	
	LabelImageType::IndexType desiredStart;
	desiredStart.Fill( 0 );
	LabelImageType::SizeType desiredSize;
	
	// check which side of the image to keep
	if (slope > 0) {
		desiredSize = resampler2->GetOutput()->GetLargestPossibleRegion().GetSize();
		desiredSize[2] = cutSlice;
	}
	else {
		desiredStart[2] = cutSlice;
		desiredSize = resampler2->GetOutput()->GetLargestPossibleRegion().GetSize();
		desiredSize[2] = desiredSize[2] - cutSlice;
	}
	
	CTImageType::RegionType desiredRegion(desiredStart, desiredSize);
	
	ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
	extractImageFilter->SetInput( resampler2->GetOutput() );
	extractImageFilter->SetExtractionRegion( desiredRegion );
	extractImageFilter->SetDirectionCollapseToIdentity();
	extractImageFilter->Update();
	
	//****************************************************************
	//	9. Recalculate the principle axes of the remaining structure
	//****************************************************************
	AffineTransformType::Pointer trafo2 = AffineTransformType::New();
	trafo2->SetIdentity();
	// thresholding
	LabelImageType::Pointer croppedImage = binaryThreshold( extractImageFilter->GetOutput(), 1, 255, 255, 0);
	// create label map
	LabelMapType3D::Pointer labelMap2 = createLabelMap( croppedImage );
	
	// set Transform center to center of the image
	trafo2->SetCenter( imageCenter );
	
	//*****************************************************************
	//	10. Get the rotation and calculate the combined roation matrix
	//*****************************************************************
	itk::Matrix<double,3,3> rotationMatrix;
	rotationMatrix = trafo->GetMatrix() * labelMap2->GetNthLabelObject( 0 )->GetPrincipalAxesToPhysicalAxesTransform()->GetMatrix();
	//std::cout << "Rotation 1 :" << trafo->GetMatrix() << std::endl;
	//std::cout << "Rotation 2 :" << labelMap2->GetNthLabelObject( 0 )->GetPrincipalAxesToPhysicalAxesTransform()->GetMatrix() << std::endl;
	//std::cout << "Combined Rotation :" << rotationMatrix << std::endl;
	
	trafo2->SetMatrix( rotationMatrix );
	
	itk::Vector< double, 3 > axis;
	axis.Fill( 0 );
	axis[1] = 1;
	trafo2->Rotate3D( axis, 1.57, true );
	
	std::cout << "transformation elements: " << trafo2->GetParameters() << std::endl;
	
	return trafo2;
	
}

//_______________________________________________________________________________

double autoAlignHeart::getEllipsoidLength()
{
	return m_ellipsoidLength;
}

//_______________________________________________________________________________

void autoAlignHeart::getCenter(double pos[3])
{
	pos[0] = m_center[0];
	pos[1] = m_center[1];
	pos[2] = m_center[2];
}

//_______________________________________________________________________________

LabelImageType::Pointer autoAlignHeart::resampleImage( LabelImageType::Pointer image, AffineTransformType* transform, InterpolatorBWType* interpolator, LabelImageType::SpacingType& outputSpacing, bool enlargeImage)
{
	//calculate the output size according to the desired output spacing
	const LabelImageType::SizeType& inputSize = image->GetLargestPossibleRegion().GetSize();
	const LabelImageType::SpacingType& inputSpacing = image->GetSpacing();
	
	LabelImageType::SizeType   outputSize;
	typedef LabelImageType::SizeType::SizeValueType SizeValueType;
	if( !enlargeImage ) {
		outputSize[0] = static_cast<SizeValueType>(inputSize[0] * inputSpacing[0] / outputSpacing[0] + .5);
		outputSize[1] = static_cast<SizeValueType>(inputSize[1] * inputSpacing[1] / outputSpacing[1] + .5);
		outputSize[2] = static_cast<SizeValueType>(inputSize[2] * inputSpacing[2] / outputSpacing[2] + .5);
	}
	else {
		outputSize[0] = static_cast<SizeValueType>(inputSize[0] * inputSpacing[0] / outputSpacing[0] + .5);
		outputSize[1] = static_cast<SizeValueType>(inputSize[1] * inputSpacing[1] / outputSpacing[1] + .5);
		outputSize[2] = static_cast<SizeValueType>(inputSize[2] * inputSpacing[2] / outputSpacing[2] + .5);
		SizeValueType diagonal = static_cast<SizeValueType>( sqrt( double(outputSize[0]*outputSize[0] + outputSize[1]*outputSize[1] + outputSize[2]*outputSize[2]) ) );
		
		AffineTransformType::OutputVectorType translation;
		translation[0] = -outputSpacing[0] * ( diagonal - outputSize[0] ) / 2;
		translation[1] = -outputSpacing[1] * ( diagonal - outputSize[1] ) / 2;
		translation[2] = -outputSpacing[2] * ( diagonal - outputSize[2] ) / 2;
		transform->Translate( translation );
		
		outputSize[0] = static_cast<SizeValueType>( diagonal );
		outputSize[1] = static_cast<SizeValueType>( diagonal );
		outputSize[2] = static_cast<SizeValueType>( diagonal );
	}
	
	ResampleFilterBWType::Pointer resampler = ResampleFilterBWType::New();
	resampler->SetInput(image);
	resampler->SetTransform(transform);
	resampler->SetInterpolator(interpolator);
	resampler->SetOutputOrigin ( image->GetOrigin());
	resampler->SetOutputSpacing ( outputSpacing );
	resampler->SetOutputDirection ( image->GetDirection());
	resampler->SetSize ( outputSize );
	resampler->SetDefaultPixelValue( 0 );
	resampler->Update ();
	
	return resampler->GetOutput();
}

//_______________________________________________________________________________

// Image resampling function
CTImageType::Pointer autoAlignHeart::resampleImage( CTImageType::Pointer image, AffineTransformType* transform, InterpolatorType* interpolator, CTImageType::SpacingType& outputSpacing, bool enlargeImage) 
{
	//calculate the output size according to the desired output spacing
	const CTImageType::SizeType& inputSize = image->GetLargestPossibleRegion().GetSize();
	const CTImageType::SpacingType& inputSpacing = image->GetSpacing();
	
	CTImageType::SizeType outputSize;
	typedef CTImageType::SizeType::SizeValueType SizeValueType;
	if( !enlargeImage ) {
		outputSize[0] = static_cast<SizeValueType>(inputSize[0] * inputSpacing[0] / outputSpacing[0] + .5);
		outputSize[1] = static_cast<SizeValueType>(inputSize[1] * inputSpacing[1] / outputSpacing[1] + .5);
		outputSize[2] = static_cast<SizeValueType>(inputSize[2] * inputSpacing[2] / outputSpacing[2] + .5);
	}
	else {
		outputSize[0] = static_cast<SizeValueType>(inputSize[0] * inputSpacing[0] / outputSpacing[0] + .5);
		outputSize[1] = static_cast<SizeValueType>(inputSize[1] * inputSpacing[1] / outputSpacing[1] + .5);
		outputSize[2] = static_cast<SizeValueType>(inputSize[2] * inputSpacing[2] / outputSpacing[2] + .5);
		SizeValueType diagonal = static_cast<SizeValueType>( sqrt( double(outputSize[0]*outputSize[0] + outputSize[1]*outputSize[1] + outputSize[2]*outputSize[2]) ) );
		
		AffineTransformType::OutputVectorType translation;
		translation[0] = -outputSpacing[0] * ( diagonal - outputSize[0] ) / 2;
		translation[1] = -outputSpacing[1] * ( diagonal - outputSize[1] ) / 2;
		translation[2] = -outputSpacing[2] * ( diagonal - outputSize[2] ) / 2;
		transform->Translate( translation );
		
		outputSize[0] = static_cast<SizeValueType>( diagonal );
		outputSize[1] = static_cast<SizeValueType>( diagonal );
		outputSize[2] = static_cast<SizeValueType>( diagonal );
	}
	
	ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	resampler->SetInput(image);
	resampler->SetTransform(transform);
	resampler->SetInterpolator(interpolator);
	resampler->SetOutputOrigin ( image->GetOrigin());
	resampler->SetOutputSpacing ( outputSpacing );
	resampler->SetOutputDirection ( image->GetDirection());
	resampler->SetSize ( outputSize );
	resampler->SetDefaultPixelValue( 100 );
	resampler->Update ();
	
	return resampler->GetOutput();
}

//_______________________________________________________________________________

// Thresholding function
BinaryImageType::Pointer autoAlignHeart::binaryThreshold( CTImageType::Pointer image, CTImageType::PixelType lowerThreshold, CTImageType::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue)
{
	typedef itk::BinaryThresholdImageFilter<CTImageType, BinaryImageType> BinaryThresholdFilterType;
	
	BinaryThresholdFilterType::Pointer binaryThresholdFilter = BinaryThresholdFilterType::New();
	binaryThresholdFilter->SetInput(image);
	binaryThresholdFilter->SetOutsideValue( outsideValue );
	binaryThresholdFilter->SetInsideValue( insideValue );
	binaryThresholdFilter->SetUpperThreshold(upperThreshold);
	binaryThresholdFilter->SetLowerThreshold(lowerThreshold);
	binaryThresholdFilter->Update();
	
	return binaryThresholdFilter->GetOutput();
}

//_______________________________________________________________________________

autoAlignHeart::BinaryImageType2D::Pointer autoAlignHeart::binaryThreshold2D( CTImageType2D::Pointer image, CTImageType2D::PixelType lowerThreshold, CTImageType2D::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue)
{
	typedef itk::BinaryThresholdImageFilter<CTImageType2D, BinaryImageType2D> BinaryThresholdFilterType;
	
	BinaryThresholdFilterType::Pointer binaryThresholdFilter = BinaryThresholdFilterType::New();
	binaryThresholdFilter->SetInput(image);
	binaryThresholdFilter->SetOutsideValue( outsideValue );
	binaryThresholdFilter->SetInsideValue( insideValue );
	binaryThresholdFilter->SetUpperThreshold(upperThreshold);
	binaryThresholdFilter->SetLowerThreshold(lowerThreshold);
	binaryThresholdFilter->Update();
	
	return binaryThresholdFilter->GetOutput();
}

//_______________________________________________________________________________

// Function to create a label map from a binary image
autoAlignHeart::LabelMapType3D::Pointer autoAlignHeart::createLabelMap( LabelImageType::Pointer BWImage )
{
	typedef itk::BinaryImageToShapeLabelMapFilter< BinaryImageType > BinaryImageToShapeLabelMapFilterType;
	BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
	binaryImageToShapeLabelMapFilter->SetInput(BWImage);
	binaryImageToShapeLabelMapFilter->Update();
	
	return binaryImageToShapeLabelMapFilter->GetOutput();
}

//_______________________________________________________________________________

// Function to create a label map from a binary image
autoAlignHeart::LabelMapType2D::Pointer autoAlignHeart::createLabelMap2D( LabelImageType2D::Pointer BWImage )
{
	typedef itk::BinaryImageToShapeLabelMapFilter< BinaryImageType2D > BinaryImageToShapeLabelMapFilterType;
	BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
	binaryImageToShapeLabelMapFilter->SetInput(BWImage);
	binaryImageToShapeLabelMapFilter->Update();
	
	return binaryImageToShapeLabelMapFilter->GetOutput();
}

//_______________________________________________________________________________

CTImageType::Pointer autoAlignHeart::createImageFromLabelMap( LabelMapType3D::Pointer labelMap )
{
	typedef itk::LabelMapToLabelImageFilter<LabelMapType3D, CTImageType> LabelMapToLabelImageFilterType;
	LabelMapToLabelImageFilterType::Pointer labelMapToLabelImageFilter = LabelMapToLabelImageFilterType::New();
	labelMapToLabelImageFilter->SetInput( labelMap );
	labelMapToLabelImageFilter->Update();
	
	return labelMapToLabelImageFilter->GetOutput();
}

//_______________________________________________________________________________

LabelImageType::Pointer autoAlignHeart::createBWImageFromLabelMap( LabelMapType3D::Pointer labelMap )
{
	typedef itk::LabelMapToLabelImageFilter<LabelMapType3D, LabelImageType> LabelMapToLabelImageFilterType;
	LabelMapToLabelImageFilterType::Pointer labelMapToLabelImageFilter = LabelMapToLabelImageFilterType::New();
	labelMapToLabelImageFilter->SetInput( labelMap );
	labelMapToLabelImageFilter->Update();
	
	return labelMapToLabelImageFilter->GetOutput();
}

//_______________________________________________________________________________

double autoAlignHeart::getDistanceOfLargestBlobs ( CTImageType::Pointer image, int sliceNumber )
{
	typedef itk::ExtractImageFilter< CTImageType, CTImageType2D > ExtractImageFilterType;
	LabelImageType::IndexType desiredStart;
	desiredStart[0] = 0;
	desiredStart[1] = 0;
	desiredStart[2] = sliceNumber;
	
	LabelImageType::SizeType desiredSize;
	desiredSize = image->GetLargestPossibleRegion().GetSize();
	desiredSize[2] = 0;
	
	CTImageType::RegionType desiredRegion(desiredStart, desiredSize);
	
	ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
	
	extractImageFilter->SetInput( image );
	extractImageFilter->SetExtractionRegion( desiredRegion );
	extractImageFilter->SetDirectionCollapseToIdentity();
	extractImageFilter->Update();
	
	// Thresholding
	LabelImageType2D::Pointer bwImage = binaryThreshold2D( extractImageFilter->GetOutput(), 1, 255, 255, 0);
	
	//keep only the largest two blobs
	typedef itk::BinaryShapeKeepNObjectsImageFilter<LabelImageType2D> BinaryShapeKeepNObjectsImageFilterType;
	BinaryShapeKeepNObjectsImageFilterType::Pointer binaryShapeKeepNObjectsImageFilter = BinaryShapeKeepNObjectsImageFilterType::New();
	binaryShapeKeepNObjectsImageFilter->SetInput(bwImage);
	binaryShapeKeepNObjectsImageFilter->SetNumberOfObjects( 2 );
	binaryShapeKeepNObjectsImageFilter->SetBackgroundValue( 0 );
	binaryShapeKeepNObjectsImageFilter->SetReverseOrdering( false );
	binaryShapeKeepNObjectsImageFilter->SetAttribute( "NumberOfPixels" );
	binaryShapeKeepNObjectsImageFilter->Update();
	
	// Create label map
	LabelMapType2D::Pointer labelMap = createLabelMap2D( binaryShapeKeepNObjectsImageFilter->GetOutput() );
	
	std::cout << "Slice number: " << sliceNumber << std::endl;
	double distance = 0;
	if( labelMap->GetNumberOfLabelObjects() > 1) {
		// check size ratio of the blobs > 10%
		double sizeRatio = double(labelMap->GetNthLabelObject(0)->GetNumberOfPixels()) / double(labelMap->GetNthLabelObject(1)->GetNumberOfPixels());
		sizeRatio = (sizeRatio>1)? 1/sizeRatio:sizeRatio;
		std::cout << "Size ratio: " << sizeRatio << std::endl;
		if( sizeRatio > 0.1 ) {
			LabelImageType2D::PointType c0 = labelMap->GetNthLabelObject(0)->GetCentroid();
			LabelImageType2D::PointType c1 = labelMap->GetNthLabelObject(1)->GetCentroid();
			distance = sqrt( (c0[0] - c1[0]) * (c0[0] - c1[0]) + (c0[1] - c1[1]) * (c0[1] - c1[1]) );
			std::cout << "Distance between centroids: " << distance << std::endl;
			std::cout << "Size0: " << labelMap->GetNthLabelObject(0)->GetNumberOfPixels() << "\nSize1: " << labelMap->GetNthLabelObject(1)->GetNumberOfPixels() << std::endl;
		}
	}
	
	return distance;
}

//_______________________________________________________________________________

autoAlignHeart::LabelMapType2D::Pointer autoAlignHeart::getBlobs( CTImageType::Pointer image, int sliceNumber ) 
{
	typedef itk::ExtractImageFilter< CTImageType, CTImageType2D > ExtractImageFilterType;
	LabelImageType::IndexType desiredStart;
	desiredStart[0] = 0;
	desiredStart[1] = 0;
	desiredStart[2] = sliceNumber;
	
	LabelImageType::SizeType desiredSize;
	desiredSize = image->GetLargestPossibleRegion().GetSize();
	desiredSize[2] = 0;
	
	CTImageType::RegionType desiredRegion(desiredStart, desiredSize);
	
	ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
	
	extractImageFilter->SetInput( image );
	extractImageFilter->SetExtractionRegion( desiredRegion );
	extractImageFilter->SetDirectionCollapseToIdentity();
	extractImageFilter->Update();
	
	// Thresholding
	LabelImageType2D::Pointer bwImage = binaryThreshold2D( extractImageFilter->GetOutput(), 1, 255, 255, 0);
	
	//keep only the largest two blobs
	typedef itk::BinaryShapeKeepNObjectsImageFilter<BinaryImageType2D> BinaryShapeKeepNObjectsImageFilterType;
	BinaryShapeKeepNObjectsImageFilterType::Pointer binaryShapeKeepNObjectsImageFilter = BinaryShapeKeepNObjectsImageFilterType::New();
	binaryShapeKeepNObjectsImageFilter->SetInput(bwImage);
	binaryShapeKeepNObjectsImageFilter->SetNumberOfObjects( 2 );
	binaryShapeKeepNObjectsImageFilter->SetBackgroundValue( 0 );
	binaryShapeKeepNObjectsImageFilter->SetReverseOrdering( false );
	binaryShapeKeepNObjectsImageFilter->SetAttribute( "NumberOfPixels" );
	binaryShapeKeepNObjectsImageFilter->Update();
	
	// Create label map
	LabelMapType2D::Pointer labelMap = createLabelMap2D( binaryShapeKeepNObjectsImageFilter->GetOutput() );
	
	return labelMap;
}

//_______________________________________________________________________________

// returns the median of a vector
double autoAlignHeart::median( std::vector<double> vec )
{
	typedef std::vector<double>::size_type vec_sz;
	
	vec_sz size = vec.size();
	
	std::nth_element(vec.begin(), vec.begin()+size, vec.end());
	
	vec_sz mid = size/2;
	
	if( size % 2 == 0) {
		std::nth_element(vec.begin(), vec.begin() + mid - 1, vec.end());
		double mid1 = vec[mid - 1];
		std::nth_element(vec.begin(), vec.begin() + mid, vec.end());
		double mid2 = vec[mid]; 
		return ( mid1 + mid2 ) / 2;
	}
	else {
		std::nth_element(vec.begin(), vec.begin() + mid, vec.end());
		return vec[ mid ];
	}
}

//_______________________________________________________________________________

std::vector<double> autoAlignHeart::medianFilterVector( std::vector<double> vec, int radius)
{
	std::vector<double> filteredVector( vec.size() );
	// copy entries at end and beginning which are not filtered
	for( int i = 0; i < radius; i++)
		filteredVector[i] = vec[i];
	for( unsigned int i = vec.size() - 1; i > vec.size() - radius; i--)
		filteredVector[i] = vec[i];
	// fill middle values
	for( unsigned int i = radius; i < vec.size() - radius; i++) {
		std::vector<double> tempVector( 2*radius + 1 );
		for( int j = 0; j < 2*radius + 1; j++) {
			tempVector[j] = vec[ j - radius + i];
			filteredVector[i] = median( tempVector );
		}
	}
	return filteredVector;
}
