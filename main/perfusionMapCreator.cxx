
#include "perfusionMapCreator.h"
#include "itkShrinkImageFilter.h"
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"

PerfusionMapCreator::PerfusionMapCreator(PerfusionAnalyzer* analyzer, int factor)
	:m_analyzer(analyzer), m_shrinkFactor(factor)
{
	
}

PerfusionMapCreator::~PerfusionMapCreator()
{
}

void PerfusionMapCreator::setAnalyzer(PerfusionAnalyzer* analyzer)
{
	m_analyzer = analyzer;
}

void PerfusionMapCreator::setShrinkFactor(int shrinkFactor)
{
	m_shrinkFactor = shrinkFactor;
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
		//ctitem->setITKImage(shrinkFilter->GetOutput());
		m_analyzer->addImage(ctitem);
	}

	//create image for segmenting
	BinaryImageTreeItem::ImageType::Pointer segmentImage = BinaryImageTreeItem::ImageType::New();
		
	BinaryImageTreeItem::ImageType::RegionType region;
	BinaryImageTreeItem::ImageType::IndexType start;
	start.Fill(0);
 
	region.SetSize(shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize());
	region.SetIndex(start);
 
	segmentImage->SetRegions(region);
	segmentImage->Allocate();

	//define iterator for the image
	typedef itk::ImageRegionIterator<BinaryImageTreeItem::ImageType>      SegmentIterator;
	
	SegmentIterator segIt(segmentImage, segmentImage->GetLargestPossibleRegion() );
	segIt.GoToBegin();

	//define duplicator
	typedef itk::ImageDuplicator< BinaryImageTreeItem::ImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();

	while( !segIt.IsAtEnd() )
	{
		//set pixel and duplicate segment
		segIt.Set(255);
		//duplicator->SetInputImage(segmentImage);
		//duplicator->Update();

		//create binaryImageItem and add it to the analyzer
		BinaryImageTreeItem::ImageType::Pointer clonedImage = duplicator->GetOutput();
		BinaryImageTreeItem seg(NULL, clonedImage, "test");
		m_analyzer->addSegment(&seg);

		segIt.Set(0);

		++segIt;
	}

	m_analyzer->calculateTacValues();

	
	return NULL;
	
}