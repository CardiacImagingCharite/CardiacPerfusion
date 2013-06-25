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
  // Image Types
  
  typedef itk::Image< CTPixelType, 2 > CTImageType2D;
  typedef itk::Image< LabelPixelType, 2 > LabelImageType2D;
  typedef itk::Image< BinaryPixelType, 2 > BinaryImageType2D;
  
  // Further type definitions
  typedef itk::LinearInterpolateImageFunction< CTImageType, double > InterpolatorType;
  typedef itk::LinearInterpolateImageFunction< LabelImageType, double > InterpolatorBWType;
  typedef itk::AffineTransform< double, 3 > AffineTransformType;
  
  typedef itk::ResampleImageFilter< CTImageType, CTImageType > ResampleFilterType;
  typedef itk::ResampleImageFilter< LabelImageType, LabelImageType > ResampleFilterBWType;

  typedef itk::BinaryBallStructuringElement< BinaryPixelType, 3 > StructuringElementType;
  typedef itk::BinaryErodeImageFilter< LabelImageType, LabelImageType, StructuringElementType > ErodeFilterType;
  typedef itk::BinaryDilateImageFilter< LabelImageType, LabelImageType, StructuringElementType > DilateFilterType;

  typedef itk::LabelMap< itk::ShapeLabelObject< itk::SizeValueType, 2> > LabelMapType2D;
  typedef itk::LabelMap< itk::ShapeLabelObject< itk::SizeValueType, 3> > LabelMapType3D;


public:
  autoAlignHeart();
  virtual ~autoAlignHeart();
  void Align();
  AffineTransformType::Pointer getTrafo(CTImageType* InputImage, int ThresholdLow = 200, int ThresholdUp = 1000, int KernelSize = 5);
  double getEllipsoidLength();
  void getCenter(double pos[3]);

protected:

  LabelImageType::Pointer resampleImage( LabelImageType::Pointer image, AffineTransformType* transform, InterpolatorBWType* interpolator, LabelImageType::SpacingType& outputSpacing, bool enlargeImage );
  CTImageType::Pointer resampleImage( CTImageType::Pointer image, AffineTransformType* transform, InterpolatorType* interpolator, CTImageType::SpacingType& outputSpacing, bool enlargeImage );
  
  BinaryImageType::Pointer binaryThreshold( CTImageType::Pointer image, CTImageType::PixelType lowerThreshold, CTImageType::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue);
  BinaryImageType2D::Pointer binaryThreshold2D( CTImageType2D::Pointer image, CTImageType2D::PixelType lowerThreshold, CTImageType2D::PixelType upperThreshold, unsigned char insideValue, unsigned char outsideValue);

  LabelMapType3D::Pointer createLabelMap( LabelImageType::Pointer BWImage );
  LabelMapType2D::Pointer createLabelMap2D( LabelImageType2D::Pointer BWImage );
  
  CTImageType::Pointer createImageFromLabelMap( LabelMapType3D::Pointer labelMap );
  LabelImageType::Pointer createBWImageFromLabelMap( LabelMapType3D::Pointer labelMap );

  double getDistanceOfLargestBlobs ( CTImageType::Pointer image, int sliceNumber );
  LabelMapType2D::Pointer getBlobs( CTImageType::Pointer image, int sliceNumber );

  double median( std::vector<double> vec );
  std::vector<double> medianFilterVector( std::vector<double> vec, int radius);
  
  double m_ellipsoidLength;
  double m_center[3];

};

#endif // autoAlignHeart_H
