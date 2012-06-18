
#include "perfusionMapCreator.h"
#include "itkShrinkImageFilter.h"
#include "ctimagetreeitem.h"
#include "binaryimagetreeitem.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"
#include "itkCastImageFilter.h"
#include "segmentinfo.h"

#include "itkImageFileWriter.h"

PerfusionMapCreator::PerfusionMapCreator(MaxSlopeAnalyzer* analyzer, int factor)
	:m_analyzer(analyzer), m_shrinkFactor(factor)
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
	typedef itk::CastImageFilter< CTImageType, BinaryImageType> CastFilterType;
	CastFilterType::Pointer caster = CastFilterType::New();
	caster->SetInput( shrinkFilter->GetOutput() );
	caster->Update();
	segmentImage = caster->GetOutput();
	//fills the segment with zeros
	segmentImage->FillBuffer(BinaryPixelOff);

	//BinaryImageTreeItem *result = new BinaryImageTreeItem(parent, segmentImage, "test");
		
/*	BinaryImageTreeItem::ImageType::RegionType region;
	BinaryImageTreeItem::ImageType::IndexType start;
	start.Fill(0);
 
	region.SetSize(shrinkFilter->GetOutput()->GetLargestPossibleRegion().GetSize());
	region.SetIndex(start);
 
	segmentImage->SetRegions(region);
	segmentImage->Allocate();
	*/
	//define iterator for the image
	typedef itk::ImageRegionIterator<BinaryImageTreeItem::ImageType>      SegmentIterator;
	
	SegmentIterator segIt(segmentImage, segmentImage->GetLargestPossibleRegion() );
	segIt.GoToBegin();

	//define duplicator
	typedef itk::ImageDuplicator< BinaryImageTreeItem::ImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();


	typedef itk::ImageFileWriter< BinaryImageTreeItem::ImageType >  WriterType;
	WriterType::Pointer writer = WriterType::New();
	writer->SetFileName( "test.dcm" );


	//prepare segments for each voxel
	while( !segIt.IsAtEnd() )
	{
		//set pixel and duplicate segment
		segIt.Set(255);
		duplicator->SetInputImage(segmentImage);
		duplicator->Update();

		//create binaryImageItem and add it to the analyzer
		BinaryImageTreeItem seg(parent, duplicator->GetOutput(), "test");
		
		writer->SetInput( duplicator->GetOutput() );
		try 
		{
			writer->Update();
		}
		catch( itk::ExceptionObject & excep )
		{
			std::cerr << "Exception catched !" << std::endl;
			std::cerr << excep << std::endl;
		}


		m_analyzer->addSegment(&seg);
		segmentImage->DisconnectPipeline();
		segIt.Set(0);

		++segIt;
	}

	m_analyzer->calculateTacValues();

	SegmentListModel* segments = m_analyzer->getSegments();
	
	for(int i = 0; i < segments->columnCount();i++)
	{
		SegmentInfo &seg = segments->getSegment(i);
		seg.setEnableGamma(true);
		
		seg.setGammaStartIndex(0);
		seg.setGammaEndIndex(model->rowCount());

		m_analyzer->recalculateGamma(seg);
	}
	
	
	
	return NULL;
	
}