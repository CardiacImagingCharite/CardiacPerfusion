/*
    This file is part of KardioPerfusion.
    Copyright 2012 Christian Freye

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
*/

#include "perfusionMapCreator.h"
#include <itkShrinkImageFilter.h>
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include <itkImageRegionIterator.h>
#include <itkCastImageFilter.h>
#include "segmentinfo.h"

#include <boost/foreach.hpp>

#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include "itkNaryPerfusionImageFilter.h"
#include <itkIdentityTransform.h>
#include <itkBSplineInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkWindowedSincInterpolateImageFunction.h>

PerfusionMapCreator::PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, SegmentInfo* artery, int factor)
	:m_analyzer(analyzer), m_arterySegment(artery), m_shrinkFactor(factor)
{
	
}

PerfusionMapCreator::~PerfusionMapCreator()
{
}

void PerfusionMapCreator::setAnalyzer(MaxSlopeAnalyzer* analyzer)
{
	m_analyzer = analyzer;
}

void PerfusionMapCreator::setShrinkFactor(int shrinkFactor)
{
	m_shrinkFactor = shrinkFactor;
}
	
void PerfusionMapCreator::setArterySegment(SegmentInfo* artery)
{
	m_arterySegment = artery;
}

RealImageTreeItem::ImageType::Pointer PerfusionMapCreator::getPerfusionMap(CTImageTreeModel* model)
{
	typedef itk::NaryPerfusionImageFilter<CTImageType, RealImageType> PerfusionFilterType;

	typedef itk::DiscreteGaussianImageFilter<CTImageType, CTImageType>
			GaussianFilterType;
	typedef itk::ShrinkImageFilter <CTImageType, CTImageType>
				ShrinkImageFilterType;

	PerfusionFilterType::Pointer perfusionFilter = PerfusionFilterType::New();

	GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
	gaussianFilter->SetVariance((m_shrinkFactor/2) * (m_shrinkFactor/2));
	gaussianFilter->SetUseImageSpacingOff();

	ShrinkImageFilterType::Pointer shrinkFilter = ShrinkImageFilterType::New();
	shrinkFilter->SetShrinkFactors(m_shrinkFactor);

	QModelIndex index = model->index(0,0);
	CTImageTreeItem *parent = dynamic_cast<CTImageTreeItem*>(&model->getItem(index));

/*	typedef itk::ImageFileWriter< CTImageType >  WriterType;
	WriterType::Pointer imageWriter = WriterType::New();
	imageWriter->SetFileName( "test.dcm" );
	*/
	
	double firstTime = parent->getTime();
	std::vector < double > times; 

	for(int i = 0; i < model->rowCount(); i++)
	{
		QModelIndex index = model->index(i,0);

		TreeItem *t = &model->getItem(index);
		CTImageTreeItem *ctitem = dynamic_cast<CTImageTreeItem*>( t->clone(parent) );
		
		times.push_back(ctitem->getTime() - firstTime);
		
		if(m_shrinkFactor > 1)
		{
			gaussianFilter->SetInput(ctitem->getITKImage());
			shrinkFilter->SetInput(gaussianFilter->GetOutput());
			shrinkFilter->Update();
			ctitem->setITKImage(shrinkFilter->GetOutput());
			ctitem->getITKImage()->DisconnectPipeline();
		}
		perfusionFilter->PushBackInput(ctitem->getITKImage());

/*		imageWriter->SetInput( shrinkFilter->GetOutput() );
		try
		{
			imageWriter->Update();
		}
		catch( itk::ExceptionObject & excep )
		{
			std::cerr << "Exception catched !" << std::endl;
			std::cerr << excep << std::endl;
		}
*/
		//m_analyzer->addImage(ctitem);

	}


	SegmentListModel* segments = m_analyzer->getSegments();

	m_arterySegment->setEnableGamma(true);
	m_arterySegment->setGammaStartIndex(0);
	m_arterySegment->setGammaEndIndex(model->rowCount());
	m_analyzer->recalculateGamma(*m_arterySegment);

	perfusionFilter->GetFunctor().setArteryGammaFunction(*m_arterySegment->getGamma());
	perfusionFilter->GetFunctor().setTimePoints(times);

	
	//Get perfusion results
	//perfusionFilter->Update();
	
/*	typedef itk::ResampleImageFilter<RealImageTreeItem::ImageType, RealImageTreeItem::ImageType> ResampleFilterType;
	ResampleFilterType::Pointer resizeFilter = ResampleFilterType::New();

	typedef itk::IdentityTransform<double> TransformFilterType;
	TransformFilterType::Pointer transformFilter = TransformFilterType::New();

	//typedef itk::WindowedSincInterpolateImageFunction<RealImageTreeItem::ImageType, 2> InterpolatorType;
	typedef itk::BSplineInterpolateImageFunction<RealImageTreeItem::ImageType> InterpolatorType;
	//typedef itk::LinearInterpolateImageFunction<RealImageTreeItem::ImageType> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();

	//transformFilter->Scale(0.10);
	transformFilter->SetIdentity();

	interpolator->SetSplineOrder(2);
	resizeFilter->SetInterpolator(interpolator);
	resizeFilter->SetDefaultPixelValue(100);
	
	resizeFilter->SetTransform(transformFilter);

	resizeFilter->SetOutputSpacing(parent->getITKImage()->GetSpacing());
	resizeFilter->SetOutputOrigin(parent->getITKImage()->GetOrigin());
	resizeFilter->SetSize(parent->getITKImage()->GetLargestPossibleRegion().GetSize());

	resizeFilter->SetInput(perfusionFilter->GetOutput());

	resizeFilter->Update();
	*/
	//assign them to an image
	RealImageTreeItem::ImageType::Pointer realImage = RealImageTreeItem::ImageType::New();
	realImage = perfusionFilter->GetOutput();

	//------------------------write image to filesystem---------------------------
	//Create output image
	CTImageType::Pointer resultImage = CTImageType::New();

	typedef itk::CastImageFilter<RealImageType, CTImageType> CastFilterType;
	CastFilterType::Pointer castfilter = CastFilterType::New();
	castfilter->SetInput( realImage );
	castfilter->Update();
	resultImage = castfilter->GetOutput();

	//fills output image with zeros
	resultImage->FillBuffer(BinaryPixelOff);

	typedef itk::ImageRegionIterator<RealImageType> RealIterator;
	typedef itk::ImageRegionIterator<CTImageType>	ResultIterator;


	RealIterator realIt(realImage, realImage->GetLargestPossibleRegion() );
	realIt.GoToBegin();

	ResultIterator resultIt(resultImage, resultImage->GetLargestPossibleRegion());
	resultIt.GoToBegin();

	while( !realIt.IsAtEnd() )
	{
		resultIt.Set((int)(realIt.Get()*1000));
		++resultIt;
		++realIt;
	}

	typedef itk::ImageFileWriter< CTImageType >  WriterType;
		WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "result.dcm" );

	writer->SetInput( resultImage );
	try 
	{
		writer->Update();
	}
	catch( itk::ExceptionObject & excep )
	{
		std::cerr << "Exception catched !" << std::endl;
		std::cerr << excep << std::endl;
	}
	//------------------------write image to filesystem---------------------------

	return realImage;
	
}