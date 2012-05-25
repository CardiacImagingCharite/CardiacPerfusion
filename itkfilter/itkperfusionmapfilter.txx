#ifndef __itkReflectImageFilter_txx
#define __itkReflectImageFilter_txx

#include "itkperfusionmapfilter.h"
#include "itkProgressReporter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"


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

	/**
	* GenerateData Creates the perfusion map
	*/
/*	template <class TInputImage, class TOutputImage >
	void
	PerfusionMapFilter<TInputImage,TOutputImage>
	::GenerateData( void )
	{
		typename Superclass::InputImageConstPointer  inputPtr = this->GetInput();
		typename Superclass::OutputImagePointer outputPtr = this->GetOutput(0);
		
		InputImageType::RegionType inputLargestPossibleRegion;
		inputLargestPossibleRegion = inputPtr->GetLargestPossibleRegion();

		InputImageType::RegionType inputBufferedRegion;
		inputBufferedRegion = inputPtr->GetBufferedRegion();

		InputImageType::RegionType inputRequestedRegion;
		inputRequestedRegion = inputPtr->GetRequestedRegion();


	}
*/
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

		OutputIterator outIt(outputPtr, outputRegionForThread);
		InputIterator inIt(inputPtr, inputRegionForThread);
		int i = 0;
		// walk the output region, and sample the input image
		while( !outIt.IsAtEnd() )
		{
			OutputImagePixelType p = static_cast<OutputImagePixelType>(inIt.Get());
			std::cout << "inputIterator at position" << i << ": " << p << std::endl;
			++i;
			// copy the input pixel to the output
			outIt.Set( p );
			++outIt; 
			++inIt; 
			progress.CompletedPixel();
		}
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
	itkDebugMacro(<<"Actually executing");

	// Get the input and output pointers
	typename Superclass::InputImageConstPointer  inputPtr = this->GetInput();
	typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

	// support progress methods/callbacks
	ProgressReporter progress(this, 0,  inputPtr->GetRequestedRegion().GetNumberOfPixels() );
  
	// Define the iterators.
	typedef ImageRegionConstIterator<TInputImage>  InputIterator;
    typedef ImageRegionIterator<TOutputImage>      OutputIterator;

	OutputIterator outIt(outputPtr, outputPtr->GetLargestPossibleRegion() );
	InputIterator inIt(inputPtr, inputPtr->GetLargestPossibleRegion() );
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

/*	template <class TInputImage, class TOutputImage>
	void 
	PerfusionMapFilter<TInputImage,TOutputImage>
	::GenerateInputRequestedRegion()
	{

	}
*/
} //end namespace itk

#endif