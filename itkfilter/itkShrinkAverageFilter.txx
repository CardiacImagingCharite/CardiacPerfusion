/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie

        This file is part of KardioPerfusion.

    KardioPerfusion is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KardioPerfusion is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KardioPerfusion. If not, see <http://www.gnu.org/licenses/>.

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


#ifndef __itkShrinkAverageFilter_txx
#define __itkShrinkAverageFilter_txx

#include "itkShrinkAverageFilter.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkContinuousIndex.h>
#include <itkObjectFactory.h>
#include <itkProgressReporter.h>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>

namespace itk
{
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::ShrinkAverageFilter()
	{
		for ( unsigned int j = 0; j < ImageDimension; j++ )
		{
			m_ShrinkFactors[j] = 1;
		}
	}
	
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	void
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::PrintSelf(std::ostream & os, Indent indent) const
	{
		Superclass::PrintSelf(os, indent);
		
		os << indent << "Shrink Factor: ";
		for ( unsigned int j = 0; j < ImageDimension; j++ )
		{
			os << m_ShrinkFactors[j] << " ";
		}
		os << std::endl;
	}
	
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	void
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::SetShrinkFactors(unsigned int factor)
	{
		unsigned int j;
		
		for ( j = 0; j < ImageDimension; j++ )
		{
			if ( factor != m_ShrinkFactors[j] ) { break; }
		}
		if ( j < ImageDimension )
		{
			this->Modified();
			for ( j = 0; j < ImageDimension; j++ )
			{
				m_ShrinkFactors[j] = factor;
				if ( m_ShrinkFactors[j] < 1 )
				{
					m_ShrinkFactors[j] = 1;
				}
			}
		}
	}
	
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	void
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
						   ThreadIdType threadId)
	{
		// Get the input and output pointers
		InputImageConstPointer inputPtr = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();

		typename OutputImageType::SizeType outputSize = outputRegionForThread.GetSize();
		
		InputImageRegionType inputRegion = inputPtr->GetLargestPossibleRegion();
		typename InputImageType::SizeType inputSize;

		for ( unsigned int dim = 0; dim < TInputImage::ImageDimension; dim++ )
		{
			inputSize[dim] = outputSize[dim] * m_ShrinkFactors[dim];
		}
		inputRegion.SetSize(inputSize);
		
		// number of all pixel/voxel of the output image
		unsigned int outputPixelNumber = 1;
		for ( unsigned int dim = 0; dim < TInputImage::ImageDimension; dim++ )
		{
			outputPixelNumber *= outputSize[dim];
		}
		
		// set pixel type depending on input image
		typedef typename boost::mpl::if_< boost::is_integral<InputImagePixelType>, long int, double >::type ValType;

		// sum of all pixel values of the corresponding input values for each output pixel
		std::vector<ValType> valSum(outputPixelNumber, 0);
		
		// iterator for input image
		typedef ImageRegionConstIterator< TInputImage > InputImageIterator;
		InputImageIterator inIt(inputPtr, inputRegion);
		
		unsigned outIdx = 0;
		std::vector<unsigned> outIdxA(OutputImageType::ImageDimension, 0);

		InputIndexType indexInsideOutputBlock;

		std::vector< unsigned > outBlockSizePerDimension(OutputImageType::ImageDimension);
		unsigned osize = 1;
		for(unsigned dim = 0; dim < OutputImageType::ImageDimension; dim++) {
			outBlockSizePerDimension[dim] = osize;
			osize *= outputSize[dim];
			indexInsideOutputBlock[dim] = 0;
		}

		// walk the input image and sum all values for each corresponding output pixel
		while ( !inIt.IsAtEnd() )
		{
			valSum[outIdx] += inIt.Get();
			++inIt;
			unsigned dim = 0;
			bool nextdim = false;
			do {
				indexInsideOutputBlock[dim]++;
				nextdim = false;
				if (indexInsideOutputBlock[dim] == m_ShrinkFactors[dim])
				{
					indexInsideOutputBlock[dim] = 0;
					outIdx += outBlockSizePerDimension[dim];
					outIdxA[dim]++;
					if ( outIdxA[dim] == outputSize[dim] )
					{
						outIdxA[dim] = 0;
						outIdx -= outBlockSizePerDimension[dim+1];
						nextdim = true;
					}
				}
				dim++;
			} while (nextdim);
		}
		
		// Define/declare an iterator that will walk the output region for this
		// thread.
		typedef ImageRegionIterator< TOutputImage > OutputIterator;
		OutputIterator outIt(outputPtr, outputRegionForThread);
		
		// Number of input pixel per output pixel
		int VoxelShrinkNumber = 1;
		for ( unsigned int dim = 0; dim < TInputImage::ImageDimension; dim++ )
		{
			VoxelShrinkNumber *= m_ShrinkFactors[dim];
		}
		
		outIdx = 0;
		
		// walk the output pixel and set new value
		while ( !outIt.IsAtEnd() )
		{
			outIt.Set( valSum[outIdx] / VoxelShrinkNumber );
			++outIdx;
			++outIt;
		}

	}
	
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	void
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::GenerateInputRequestedRegion()
	{
		// Call the superclass' implementation of this method
		Superclass::GenerateInputRequestedRegion();
		
		// Get pointers to the input and output
		InputImagePointer  inputPtr = const_cast< TInputImage * >( this->GetInput() );
		OutputImagePointer outputPtr = this->GetOutput();
		
		if ( !inputPtr || !outputPtr )
		{
			return;
		}
		
		// Compute the input requested region (size and start index)
		// Use the image transformations to insure an input requested region
		// that will provide the proper range
		unsigned int i;
		const typename TOutputImage::SizeType & outputRequestedRegionSize =
		outputPtr->GetRequestedRegion().GetSize();
		const typename TOutputImage::IndexType & outputRequestedRegionStartIndex =
		outputPtr->GetRequestedRegion().GetIndex();
		
		// Convert the factor for convenient multiplication
		typename TOutputImage::SizeType factorSize;
		for ( i = 0; i < TInputImage::ImageDimension; i++ )
		{
			factorSize[i] = m_ShrinkFactors[i];
		}
		
		OutputIndexType  outputIndex;
		InputIndexType   inputIndex, inputRequestedRegionIndex;
		OutputOffsetType offsetIndex;
		
		typename TInputImage::SizeType inputRequestedRegionSize;
		typename TOutputImage::PointType tempPoint;
		
		// Use this index to compute the offset everywhere in this class
		outputIndex = outputPtr->GetLargestPossibleRegion().GetIndex();
		
		// We wish to perform the following mapping of outputIndex to
		// inputIndex on all points in our region
		outputPtr->TransformIndexToPhysicalPoint(outputIndex, tempPoint);
		inputPtr->TransformPhysicalPointToIndex(tempPoint, inputIndex);
		
		// Given that the size is scaled by a constant factor eq:
		// inputIndex = outputIndex * factorSize
		// is equivalent up to a fixed offset which we now compute
		OffsetValueType zeroOffset = 0;
		for ( i = 0; i < TInputImage::ImageDimension; i++ )
		{
			offsetIndex[i] = inputIndex[i] - outputIndex[i] * m_ShrinkFactors[i];
			// It is plausible that due to small amounts of loss of numerical
			// precision that the offset it negaive, this would cause sampling
			// out of out region, this is insurance against that possibility
			offsetIndex[i] = vnl_math_max(zeroOffset, offsetIndex[i]);
		}
		
		inputRequestedRegionIndex = outputRequestedRegionStartIndex * factorSize + offsetIndex;
		
		// Originally this was
		//  for ( i=0; i < TInputImage::ImageDimension; ++i )
		//  {
			//  inputRequestedRegionSize[i] = (outputRequestedRegionSize[i] - 1 ) *
		// factorSize[i] + 1;
		//  }
		// but with centered pixels we may sample edge to edge
		
		inputRequestedRegionSize = outputRequestedRegionSize * factorSize;
		
		typename TInputImage::RegionType inputRequestedRegion;
		inputRequestedRegion.SetIndex(inputRequestedRegionIndex);
		inputRequestedRegion.SetSize(inputRequestedRegionSize);
		inputRequestedRegion.Crop( inputPtr->GetLargestPossibleRegion() );
		
		inputPtr->SetRequestedRegion(inputRequestedRegion);
	}
	
	/**
	 * 
	 */
	template< class TInputImage, class TOutputImage >
	void
	ShrinkAverageFilter< TInputImage, TOutputImage >
	::GenerateOutputInformation()
	{
		// Call the superclass' implementation of this method
		Superclass::GenerateOutputInformation();
		
		// Get pointers to the input and output
		InputImageConstPointer inputPtr  = this->GetInput();
		OutputImagePointer     outputPtr = this->GetOutput();
		
		if ( !inputPtr || !outputPtr )
		{
			return;
		}
		
		// Compute the output spacing, the output image size, and the
		// output image start index
		unsigned int i;
		const typename TInputImage::SpacingType &
		inputSpacing = inputPtr->GetSpacing();
		const typename TInputImage::SizeType &   inputSize =
		inputPtr->GetLargestPossibleRegion().GetSize();
		const typename TInputImage::IndexType &  inputStartIndex =
		inputPtr->GetLargestPossibleRegion().GetIndex();
		
		typename TOutputImage::SpacingType outputSpacing;
		typename TOutputImage::SizeType outputSize;
		typename TOutputImage::IndexType outputStartIndex;
		
		for ( i = 0; i < TOutputImage::ImageDimension; i++ )
		{
			outputSpacing[i] = inputSpacing[i] * (double)m_ShrinkFactors[i];
			
			// Round down so that all output pixels fit input input region
			outputSize[i] = static_cast<SizeValueType>(
				vcl_floor( (double)inputSize[i] / (double)m_ShrinkFactors[i] ) );
			
			if ( outputSize[i] < 1 )
			{
				outputSize[i] = 1;
			}
			
			// Because of the later origin shift this starting index is not
			// critical
			outputStartIndex[i] = static_cast<IndexValueType>(
				vcl_ceil( (double)inputStartIndex[i] / (double)m_ShrinkFactors[i] ) );
		}
		
		outputPtr->SetSpacing(outputSpacing);
		
		// Compute origin offset
		// The physical center's of the input and output should be the same
		ContinuousIndex< double, TOutputImage::ImageDimension > inputCenterIndex;
		ContinuousIndex< double, TOutputImage::ImageDimension > outputCenterIndex;
		for ( i = 0; i < TOutputImage::ImageDimension; i++ )
		{
			inputCenterIndex[i] = inputStartIndex[i] + ( inputSize[i] - 1 ) / 2.0;
			outputCenterIndex[i] = outputStartIndex[i] + ( outputSize[i] - 1 ) / 2.0;
		}
		
		typename TOutputImage::PointType inputCenterPoint;
		typename TOutputImage::PointType outputCenterPoint;
		inputPtr->TransformContinuousIndexToPhysicalPoint(inputCenterIndex, inputCenterPoint);
		outputPtr->TransformContinuousIndexToPhysicalPoint(outputCenterIndex, outputCenterPoint);
		
		typename TOutputImage::PointType outputOrigin = outputPtr->GetOrigin();
		outputOrigin = outputOrigin + ( inputCenterPoint - outputCenterPoint );
		outputPtr->SetOrigin(outputOrigin);
		
		// Set region
		typename TOutputImage::RegionType outputLargestPossibleRegion;
		outputLargestPossibleRegion.SetSize(outputSize);
		outputLargestPossibleRegion.SetIndex(outputStartIndex);
		
		outputPtr->SetLargestPossibleRegion(outputLargestPossibleRegion);
	}
} // end namespace itk

#endif