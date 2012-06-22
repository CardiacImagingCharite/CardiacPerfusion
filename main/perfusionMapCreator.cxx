
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

PerfusionMapCreator::PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, const SegmentInfo* artery, int factor)
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
	
void PerfusionMapCreator::setArterySegment(const SegmentInfo* artery)
{
	m_arterySegment = artery;
}

RealImageType* PerfusionMapCreator::getPerfusionMap(CTImageTreeModel* model)
{
	typedef itk::ShrinkImageFilter <CTImageType, CTImageType>
				ShrinkImageFilterType;
	ShrinkImageFilterType::Pointer shrinkFilter
				= ShrinkImageFilterType::New();
	shrinkFilter->SetShrinkFactors(m_shrinkFactor);

	for(int i = 0; i < model->rowCount(); i++)
	{
		QModelIndex index = model->index(i,0);
		CTImageTreeItem *ctitem = dynamic_cast<CTImageTreeItem*>(&model->getItem(index));

		shrinkFilter->SetInput(ctitem->getITKImage());
		shrinkFilter->Update();
		ctitem->setITKImage(shrinkFilter->GetOutput());

		m_analyzer->addImage(ctitem);
	}

	//create image for segmenting
	QModelIndex index = model->index(0,0);
	CTImageTreeItem *parent = dynamic_cast<CTImageTreeItem*>(&model->getItem(index));

	BinaryImageTreeItem::ImageType::Pointer segmentImage;

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
	m_analyzer->calculateTacValues();

	SegmentListModel* segments = m_analyzer->getSegments();

	for(int i = 0; i < segments->rowCount();i++)
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
			resultIt.Set(perfusion);
		}

		++resultIt;
	}
	
	return resultImage;
	
}