/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
	
	This file is part of KardioPerfusion.

    KardioPerfusion is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KardioPerfusion is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KardioPerfusion.  If not, see <http://www.gnu.org/licenses/>.

    Diese Datei ist Teil von KardioPerfusion.

    KardioPerfusion ist Freie Software: Sie können es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License für weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#ifndef autoAlignHeart_H
#define autoAlignHeart_H

#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "imagedefinitions.h"


class autoAlignHeart {
  
public:
  //******************************
  // Global type definitions
  //******************************
  // Image Types
  
  // const unsigned int InputDimension = 3;
  
  typedef itk::Image< signed short, 2 > SignedShortImageType2D;
  typedef itk::Image< signed short, 3 > SignedShortImageType3D;
  typedef itk::Image< unsigned char, 2 > UnsignedCharImageType2D;
  typedef itk::Image< unsigned char, 3 > UnsignedCharImageType3D;
  typedef itk::Image< itk::Vector< unsigned char , 3>, 2> ColorImageType2D;
  typedef itk::Image< itk::Vector< unsigned char , 3>, 3> ColorImageType3D;
  
  // Further type definitions
  typedef itk::LinearInterpolateImageFunction< SignedShortImageType3D, double > InterpolatorType;
  typedef itk::LinearInterpolateImageFunction< UnsignedCharImageType3D, double > InterpolatorBWType;
  typedef itk::AffineTransform< double, 3 > AffineTransformType;
  
  typedef itk::ResampleImageFilter< SignedShortImageType3D, SignedShortImageType3D > ResampleFilterType;
  typedef itk::ResampleImageFilter< UnsignedCharImageType3D, UnsignedCharImageType3D > ResampleFilterBWType;

  typedef itk::BinaryBallStructuringElement< unsigned char, 3 > StructuringElementType;
  typedef itk::BinaryErodeImageFilter< UnsignedCharImageType3D, UnsignedCharImageType3D, StructuringElementType > ErodeFilterType;
  typedef itk::BinaryDilateImageFilter< UnsignedCharImageType3D,	UnsignedCharImageType3D, StructuringElementType > DilateFilterType;

  typedef itk::LabelMap< itk::ShapeLabelObject< itk::SizeValueType, 2> > LabelMapType2D;
  typedef itk::LabelMap< itk::ShapeLabelObject< itk::SizeValueType, 3> > LabelMapType3D;


public:
  autoAlignHeart();
  virtual ~autoAlignHeart();
  void Align();
  AffineTransformType::Pointer getTrafo(CTImageType* InputImage, int ThresholdLow = 200, int ThresholdUp = 1000, int KernelSize = 5, int OutputSelector = 0);

protected:

  UnsignedCharImageType3D::Pointer resampleImage( UnsignedCharImageType3D::Pointer image, AffineTransformType* transform, InterpolatorBWType* interpolator, UnsignedCharImageType3D::SpacingType& outputSpacing, bool enlargeImage );
  SignedShortImageType3D::Pointer resampleImage( SignedShortImageType3D::Pointer image, AffineTransformType* transform, InterpolatorType* interpolator, SignedShortImageType3D::SpacingType& outputSpacing, bool enlargeImage );
  
  UnsignedCharImageType3D::Pointer binaryThreshold( SignedShortImageType3D::Pointer image, SignedShortImageType3D::PixelType lowerThreshold, SignedShortImageType3D::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue);
  UnsignedCharImageType2D::Pointer binaryThreshold2D( SignedShortImageType2D::Pointer image, SignedShortImageType2D::PixelType lowerThreshold, SignedShortImageType2D::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue);

  LabelMapType3D::Pointer createLabelMap( UnsignedCharImageType3D::Pointer BWImage );
  LabelMapType2D::Pointer createLabelMap2D( UnsignedCharImageType2D::Pointer BWImage );
  
  SignedShortImageType3D::Pointer createImageFromLabelMap( LabelMapType3D::Pointer labelMap );
  UnsignedCharImageType3D::Pointer createBWImageFromLabelMap( LabelMapType3D::Pointer labelMap );

  double getDistanceOfLargestBlobs ( SignedShortImageType3D::Pointer image, int sliceNumber );
  LabelMapType2D::Pointer getBlobs( SignedShortImageType3D::Pointer image, int sliceNumber );

  double median( std::vector<double> vec );
  std::vector<double> medianFilterVector( std::vector<double> vec, int radius);

};

#endif // autoAlignHeart_H
