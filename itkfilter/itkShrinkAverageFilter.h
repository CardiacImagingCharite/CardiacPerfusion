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


#ifndef __itkShrinkAverageFilter_h
#define __itkShrinkAverageFilter_h

#include "itkImageToImageFilter.h"

namespace itk
{
	/** \class ShrinkAverageFilter
	 * \brief Reduce the size of an image by an integer factor in each
	 * dimension and calculate the average value.
	 *
	 * ShrinkAverageFilter reduces the size of an image by an integer factor
	 * in each dimension. In contrast to itkShrinkImageFilter, this filter 
	 * calculates the average pixel value from each corresponding input 
	 * pixel. The algorithm implemented is a simple subsample.
	 */
	template< class TInputImage, class TOutputImage >
	class ITK_EXPORT ShrinkAverageFilter:
	public ImageToImageFilter< TInputImage, TOutputImage >
	{
	public:
		/** Standard class typedefs. */
		typedef ShrinkAverageFilter                               Self;
		typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
		typedef SmartPointer< Self >                            Pointer;
		typedef SmartPointer< const Self >                      ConstPointer;
		
		/** Method for creation through the object factory. */
		itkNewMacro(Self);
		
		/** Run-time type information (and related methods). */
		itkTypeMacro(ShrinkAverageFilter, ImageToImageFilter);
		
		/** Typedef to images */
		typedef TOutputImage                          OutputImageType;
		typedef TInputImage                           InputImageType;
		typedef typename OutputImageType::Pointer     OutputImagePointer;
		typedef typename InputImageType::Pointer      InputImagePointer;
		typedef typename InputImageType::ConstPointer InputImageConstPointer;
		
		typedef typename TOutputImage::IndexType  OutputIndexType;
		typedef typename TInputImage::IndexType   InputIndexType;
		typedef typename TOutputImage::OffsetType OutputOffsetType;
		typedef typename TInputImage::PixelType   InputImagePixelType; 
		typedef typename TOutputImage::PixelType  OutputImagePixelType; 
		
		/** Typedef to describe the output image region type. */
		typedef typename TInputImage::RegionType InputImageRegionType;
		
		/** Typedef to describe the output image region type. */
		typedef typename TOutputImage::RegionType OutputImageRegionType;
				
		/** ImageDimension enumeration. */
		itkStaticConstMacro(ImageDimension, unsigned int,
							TInputImage::ImageDimension);
		itkStaticConstMacro(OutputImageDimension, unsigned int,
							TOutputImage::ImageDimension);
		
		typedef FixedArray< unsigned int, ImageDimension > ShrinkFactorsType;
		
		/** Set the shrink factors. Values are clamped to
		 * a minimum value of 1. Default is 1 for all dimensions. */
		itkSetMacro(ShrinkFactors, ShrinkFactorsType);
		void SetShrinkFactors(unsigned int factor);
		void SetShrinkFactor(unsigned int i, unsigned int factor)
		{
			m_ShrinkFactors[i] = factor;
		}
		
		/** Get the shrink factors. */
		itkGetConstReferenceMacro(ShrinkFactors, ShrinkFactorsType);
		
		/** ShrinkAverageFilter produces an image which is a different
		 * resolution and with a different pixel spacing than its input
		 * image.  As such, ShrinkAverageFilter needs to provide an
		 * implementation for GenerateOutputInformation() in order to inform
		 * the pipeline execution model.  The original documentation of this
		 * method is below.
		 * \sa ProcessObject::GenerateOutputInformaton() */
		virtual void GenerateOutputInformation();
		
		/** ShrinkAverageFilter needs a larger input requested region than the output
		 * requested region.  As such, ShrinkAverageFilter needs to provide an
		 * implementation for GenerateInputRequestedRegion() in order to inform the
		 * pipeline execution model.
		 * \sa ProcessObject::GenerateInputRequestedRegion() */
		virtual void GenerateInputRequestedRegion();
		
		#ifdef ITK_USE_CONCEPT_CHECKING
		/** Begin concept checking */
		itkConceptMacro( InputConvertibleToOutputCheck,
						 ( Concept::Convertible< typename TInputImage::PixelType, typename TOutputImage::PixelType > ) );
		itkConceptMacro( SameDimensionCheck,
						 ( Concept::SameDimension< ImageDimension, OutputImageDimension > ) );
		/** End concept checking */
		#endif
	protected:
		ShrinkAverageFilter();
		~ShrinkAverageFilter() {}
		void PrintSelf(std::ostream & os, Indent indent) const;
		
		/** ShrinkAverageFilter can be implemented as a multithreaded filter.
		 * Therefore, this implementation provides a ThreadedGenerateData() routine
		 * which is called for each processing thread. The output image data is
		 * allocated automatically by the superclass prior to calling
		 * ThreadedGenerateData().  ThreadedGenerateData can only write to the
		 * portion of the output image specified by the parameter
		 * "outputRegionForThread"
		 *
		 * \sa ImageToImageFilter::ThreadedGenerateData(),
		 *     ImageToImageFilter::GenerateData() */
		void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
								  ThreadIdType threadId);
		
	private:
		ShrinkAverageFilter(const Self &); //purposely not implemented
		void operator=(const Self &);    //purposely not implemented
		
		ShrinkFactorsType m_ShrinkFactors;
	};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShrinkAverageFilter.txx"
#endif

#endif