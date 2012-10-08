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

#ifndef __itkReflectImageFilter_txx
#define __itkReflectImageFilter_txx

#include "itkperfusionmapfilter.h"
#include "itkProgressReporter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageDuplicator.h"
#include "itkExtractImageFilter.h"

#include "perfusionAnalyzer.h"

namespace itk
{
	/**
	* Constructor
	*/
	template <class TInputImage, class TOutputImage >
	PerfusionMapFilter<TInputImage,TOutputImage >
	::PerfusionMapFilter()
	{
		this->SetNumberOfRequiredInputs( 1 );
	}


	template <class TInputImage, class TOutputImage >
	void
	PerfusionMapFilter<TInputImage,TOutputImage>::
	PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
  
		os << indent << "Nothing to print yet..." << std::endl;
	}

/*	template <class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage,TOutputImage>
	::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
					int threadId)
	{
		itkDebugMacro(<<"Actually executing");

		// Get the input and output pointers
		typename Superclass::InputImageConstPointer  inputPtr = this->GetInput();
		typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

		// support progress methods/callbacks
		ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  
		// Define the portion of the input to walk for this thread
		InputImageRegionType inputRegionForThread;
  
		// Define the iterators.
		typedef ImageRegionIterator<TOutputImage>     OutputIterator;
		typedef ImageRegionConstIterator<TInputImage> InputIterator;

		OutputIterator outIt(outputPtr,outputRegionForThread );
		InputIterator inIt(inputPtr, inputRegionForThread );
		outIt.GoToBegin();
		inIt.GoToBegin();

		// walk the output region, and copy the input image
		while( !outIt.IsAtEnd() )
		{
			// copy the input pixel to the output
			//replace that by calculation of perfusion values
			outIt.Set( static_cast<OutputImagePixelType>(inIt.Get()) );
			++outIt; 
			++inIt; 
			progress.CompletedPixel();
		}
	}
	/*
		// Define the iterators.
	typedef ImageRegionConstIterator<TInputImage>  InputIterator;
    typedef ImageRegionIterator<TOutputImage>      OutputIterator;

	OutputIterator outIt(outputPtr, outputPtr->GetLargestPossibleRegion() );
	InputIterator inIt(inputPtr, inputPtr->GetLargestPossibleRegion() );
	outIt.GoToBegin();
	inIt.GoToBegin();

	int maxTimePoints = inputPtr->GetLargestPossibleRegion().GetSize()[3];
	int volumeSize = 1;
	for(int i = 0; i < 3; i++)
	{
		volumeSize *= inputPtr->GetLargestPossibleRegion().GetSize()[i];
	}

	// walk the output region, and copy the input image
	while( !outIt.IsAtEnd() )
	{
		InputIterator tempIt(inputPtr, inputPtr->GetLargestPossibleRegion() );
		tempIt = inIt;

		// copy the input pixel to the output
		//replace that by calculation of perfusion 
		
		for(int time = 0; time < maxTimePoints; time++)
		{
//			tempIt = tempIt + (time * volumeSize);
			
		}
		
		outIt.Set( static_cast<OutputImagePixelType>(inIt.Get()) );

		++outIt; 
		++inIt; 
		progress.CompletedPixel();
	}
	*/


/**
 * GenerateData Performs the reflection
 */
template <class TInputImage, class TOutputImage >
void
PerfusionMapFilter<TInputImage,TOutputImage>
::GenerateData( void )
{
/*	itkDebugMacro(<<"Actually executing");

	
	if(m_arterySegment == NULL || m_analyzer == NULL)
		return;

	// Get the input and output pointers
	typename Superclass::InputImageConstPointer  inputPtr = this->GetInput();
	typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

	// support progress methods/callbacks
	ProgressReporter progress(this, 0,  inputPtr->GetRequestedRegion().GetNumberOfPixels() );
	
	//define and create filter for extracting volumes from 4D data
	typedef itk::ExtractImageFilter<TInputImage,CTImageType> ExtractFilterType;

	ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();

	//create region for extracting the images
	TInputImage::IndexType::Pointer desiredIndex;
	desiredIndex->Fill(0);
 
    TInputImage::SizeType desiredSize = inputPtr->GetLargestPossibleRegion().GetSize();
	int timeSamples = desiredSize[3];
	desiredSize[3] = 0;

	TInputImage::RegionType::Pointer desiredRegion(desiredIndex, desiredSize);

	extractFilter->SetInput(inputPtr);
	extractFilter->SetExtractionRegion(desiredRegion);

//	for(int i = 0; i < timeSamples; i++)
//	{
//		m_analyzer->addImage( dynamic_cast<CTImageTreeItem*>(item) )
//	}
	

	//create image for segmenting
	BinaryImageTreeItem::ImageType::Pointer segmentImage = BinaryImageTreeItem::ImageType::New();
		
	BinaryImageTreeItem::ImageType::RegionType region;
	BinaryImageTreeItem::ImageType::IndexType start;
	start.Fill(0);
 
	region.SetSize(outputPtr->GetLargestPossibleRegion().GetSize());
	region.SetIndex(start);
 
	segmentImage->SetRegions(region);
	segmentImage->Allocate();


	//define iterator for the image
	typedef ImageRegionIterator<BinaryImageTreeItem::ImageType>      SegmentIterator;
	
	SegmentIterator segIt(segmentImage, segmentImage->GetLargestPossibleRegion() );
	segIt.GoToBegin();

	//define duplicator
	typedef itk::ImageDuplicator< BinaryImageTreeItem::ImageType > DuplicatorType;
	DuplicatorType::Pointer duplicator = DuplicatorType::New();

	while( !segIt.IsAtEnd() )
	{
		//set pixel and duplicate segment
		segIt.Set(255);
		duplicator->SetInputImage(segmentImage);
		duplicator->Update();

		//create binaryImageItem and add it to the analyzer
		BinaryImageTreeItem::ImageType::Pointer clonedImage = duplicator->GetOutput();
		BinaryImageTreeItem seg(NULL, clonedImage, "test");
		m_analyzer->addSegment(&seg);

		segIt.Set(0);

		++segIt;
	}

*/
}

	template <class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage,TOutputImage>
	::GenerateOutputInformation()
	{
		// do not call the superclass' implementation of this method since
	    // this filter allows the input and the output to be of different dimensions
		//Superclass::GenerateOutputInformation();
  
		// Get pointers to the input and output
		InputImageConstPointer  inputPtr  = this->GetInput();
		OutputImagePointer      outputPtr = this->GetOutput();

		if ( !inputPtr || !outputPtr )
		{
			return;
		}
  
		// Compute the output spacing, the output image size, and the
		// output image start index
		unsigned int i;
		const typename TInputImage::SpacingType&
				inputSpacing	= inputPtr->GetSpacing();
		const typename TInputImage::SizeType&   
				inputSize		= inputPtr->GetLargestPossibleRegion().GetSize();
		const typename TInputImage::IndexType&  
				inputStartIndex = inputPtr->GetLargestPossibleRegion().GetIndex();
		const typename TInputImage::PointType&  
				inputOrigin = inputPtr->GetOrigin();


		typename TOutputImage::SpacingType  outputSpacing;
		typename TOutputImage::SizeType     outputSize;
		typename TOutputImage::IndexType    outputStartIndex;
		typename TOutputImage::PointType    outputOrigin;



		for (i = 0; i < TOutputImage::ImageDimension; i++)
		{
			outputSpacing[i]	= inputSpacing[i];
			outputSize[i]		= inputSize[i];
			outputStartIndex[i] = inputStartIndex[i];
			outputOrigin[i]		= inputOrigin[i];

			std::cout << "Spacing_" << i << ": " << outputSpacing[i] << std::endl;
			std::cout << "Size_" << i << ": " << outputSize[i] << std::endl;
			std::cout << "Index_" << i << ": " << outputStartIndex[i] << std::endl;
			std::cout << "Origin_" << i << ": " << outputOrigin[i] << std::endl;
		}
		
		outputPtr->SetSpacing(outputSpacing);

		// Set region
		typename TOutputImage::RegionType outputRegion;
		outputRegion.SetSize( outputSize );
		outputRegion.SetIndex( outputStartIndex );

		outputPtr->SetLargestPossibleRegion( outputRegion );
		outputPtr->SetRequestedRegion(outputRegion);
		outputPtr->SetBufferedRegion(outputRegion);
		outputPtr->Allocate();

		//Set origin
		outputPtr->SetOrigin(outputOrigin);

	}

	template<class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage, TOutputImage>::setArterySegment(const SegmentInfo *arterySegment)
	{
		m_arterySegment = arterySegment;
	}

	template<class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage, TOutputImage>::setPerfusionAnalyzer(const PerfusionAnalyzer* analyzer)
	{
		m_analyzer = analyzer;
	}

/*	template <class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage,TOutputImage>
	::GenerateInputRequestedRegion()
	{

	}
*/
} //end namespace itk

#endif