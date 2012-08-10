
#include "perfusionMapCreator.h"
#include "itkShrinkImageFilter.h"
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"
#include "itkCastImageFilter.h"
#include "segmentinfo.h"

#include <boost/foreach.hpp>

#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkNaryPerfusionImageFilter.h"

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

	//create image for segmenting

/*	BinaryImageTreeItem::ImageType::Pointer segmentImage;

	//create caster, that transforme the CT image to a binary image
	typedef itk::CastImageFilter< CTImageType, BinaryImageType> BinaryCastFilterType;
	BinaryCastFilterType::Pointer binaryCaster = BinaryCastFilterType::New();
	binaryCaster->SetInput( shrinkFilter->GetOutput() );
	binaryCaster->Update();
	segmentImage = binaryCaster->GetOutput();
	//fills the segment with zeros
	segmentImage->FillBuffer(BinaryPixelOff);

	//define caster, that transforms a CT image to a real image
	RealImageType::Pointer resultImage;

	//create caster, that transforme the CT image to a binary image
	typedef itk::CastImageFilter< CTImageType, RealImageType> RealCastFilterType;
	RealCastFilterType::Pointer realCaster = RealCastFilterType::New();
	realCaster->SetInput( shrinkFilter->GetOutput() );
	realCaster->Update();
	resultImage = realCaster->GetOutput();
	//fills the segment with zeros
	resultImage->FillBuffer(BinaryPixelOff);


	//define iterator for the images
	typedef itk::ImageRegionIterator<BinaryImageTreeItem::ImageType>    SegmentIterator;
	typedef itk::ImageRegionIterator<RealImageType>						ResultIterator;


	SegmentIterator segIt(segmentImage, segmentImage->GetLargestPossibleRegion() );
	segIt.GoToBegin();

	ResultIterator resultIt(resultImage, resultImage->GetLargestPossibleRegion());
	resultIt.GoToBegin();

	//define duplicator for binary image
	typedef itk::ImageDuplicator< BinaryImageTreeItem::ImageType > BinaryDuplicatorType;
	BinaryDuplicatorType::Pointer binaryDuplicator = BinaryDuplicatorType::New();

	//prepare segments for each voxel
	while( !segIt.IsAtEnd() )
	{
		//set pixel and duplicate segment
		segIt.Set(BinaryPixelOn);
		binaryDuplicator->SetInputImage(segmentImage);
		binaryDuplicator->Update();

		//create binaryImageItem and add it to the analyzer
		BinaryImageTreeItem* seg = new BinaryImageTreeItem(parent, binaryDuplicator->GetOutput(), "test");

		m_analyzer->addSegment(seg);
		segmentImage->DisconnectPipeline();
		segIt.Set(BinaryPixelOff);

		++segIt;
	}
*/

/*	typedef itk::ImageFileWriter< BinaryImageTreeItem::ImageType >  WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "test.dcm" );

	SegmentListModel* testSegments = m_analyzer->getSegments();
	BOOST_FOREACH( SegmentInfo &currentSegment, *testSegments) {
	
		writer->SetInput( currentSegment.getSegment()->getITKImage() );
		try 
		{
			writer->Update();
		}
		catch( itk::ExceptionObject & excep )
		{
			std::cerr << "Exception catched !" << std::endl;
			std::cerr << excep << std::endl;
		}
	}
	*/
//	m_analyzer->calculateTacValues();


	SegmentListModel* segments = m_analyzer->getSegments();

	m_arterySegment->setEnableGamma(true);
	m_arterySegment->setGammaStartIndex(0);
	m_arterySegment->setGammaEndIndex(model->rowCount());
	m_analyzer->recalculateGamma(*m_arterySegment);

	perfusionFilter->GetFunctor().setArteryGammaFunction(*m_arterySegment->getGamma());
	perfusionFilter->GetFunctor().setTimePoints(times);

	/*
	for(int i = 1; i < segments->rowCount();i++)
	{
		SegmentInfo &seg = segments->getSegment(i);
		seg.setArterySegment(m_arterySegment);

		seg.setEnableGamma(true);
		seg.setGammaStartIndex(0);
		seg.setGammaEndIndex(model->rowCount());

		m_analyzer->recalculateGamma(seg);

		double gammaMax = seg.getArterySegment()->getGammaMaximum();
		double maxSlope = seg.getGammaMaxSlope();
		double perfusion = 0;
		if(gammaMax != 0)
		{
			perfusion = 60 * maxSlope / gammaMax;
			if(perfusion > 0)
				resultIt.Set(perfusion);
		}

		++resultIt;
	}
	*/
	//Get perfusion results
	perfusionFilter->Update();
	//assign them to an image
	RealImageTreeItem::ImageType::Pointer realImage = RealImageTreeItem::ImageType::New();
	realImage = perfusionFilter->GetOutput();
	
	//Create output image
	CTImageType::Pointer resultImage = CTImageType::New();

	typedef itk::CastImageFilter<RealImageType, CTImageType> CastFilterType;
	CastFilterType::Pointer castfilter = CastFilterType::New();
	castfilter->SetInput( perfusionFilter->GetOutput() );
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

	/*typedef itk::RescaleIntensityImageFilter< RealImageType, CTImageType > RescaleFilterType;
	RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	rescaleFilter->SetInput(perfusionFilter->GetOutput());
	rescaleFilter->SetOutputMinimum(0);
	rescaleFilter->SetOutputMaximum(32767);
	*/
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

	return realImage ;
	
}