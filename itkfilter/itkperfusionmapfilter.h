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

#ifndef itkPerfusionMapFilter_H
#define itkPerfusionMapFilter_H

#include "itkImageToImageFilter.h"
#include "perfusionAnalyzer.h"

namespace itk{

	class SegmentInfo;
	//class PerfusionAnalyzer;
	
	template <class TInputImage, class TOutputImage>
	class ITK_EXPORT PerfusionMapFilter : public ImageToImageFilter< TInputImage, TOutputImage>
	{
		public:
			/** Standard class typedefs. */
			typedef PerfusionMapFilter							  Self;
			typedef ImageToImageFilter<TInputImage,TOutputImage>  Superclass;
			typedef SmartPointer<Self>                            Pointer;
			typedef SmartPointer<const Self>                      ConstPointer;

			/** Method for creation through the object factory. */
			itkNewMacro(Self);
  
			/** Run-time type information (and related methods). */
			itkTypeMacro(PerfusionMapFilter, ImageToImageFilter);

			 /** Some convenient typedefs. */
			typedef TInputImage                            InputImageType;
			typedef typename    InputImageType::Pointer    InputImagePointer;
			typedef typename    InputImageType::RegionType InputImageRegionType; 
			typedef typename    InputImageType::PixelType  InputImagePixelType; 

			typedef TOutputImage                             OutputImageType;
			typedef typename     OutputImageType::Pointer    OutputImagePointer;
			typedef typename     OutputImageType::RegionType OutputImageRegionType;
			typedef typename     OutputImageType::PixelType  OutputImagePixelType;
			
			/** ImageDimension enumeration. */
			itkStaticConstMacro(ImageDimension, unsigned int,
								TInputImage::ImageDimension );
			itkStaticConstMacro(OutputImageDimension, unsigned int,
								TOutputImage::ImageDimension );
			 
			/** PerfusionMapFilter produces an image which is a different
			* resolution and with a different pixel spacing than its input
			* image.  As such, ShrinkImageFilter needs to provide an
			* implementation for GenerateOutputInformation() in order to inform
			* the pipeline execution model.  The original documentation of this
			* method is below.
			* \sa ProcessObject::GenerateOutputInformaton() */
			virtual void GenerateOutputInformation();
			
			/** PerfusionMapFilter needs a larger input requested region than the output
			* requested region.  As such, ShrinkImageFilter needs to provide an
			* implementation for GenerateInputRequestedRegion() in order to inform the
			* pipeline execution model.
			* \sa ProcessObject::GenerateInputRequestedRegion() */
//			virtual void GenerateInputRequestedRegion();

			/** Sets the artery segment, which is needed for perfusion analysis.
			*/
			void setArterySegment(const SegmentInfo *arterySegment);

			/** Sets the analyzer, which calculates the perfusion.
			*/
			void setPerfusionAnalyzer(const PerfusionAnalyzer* analyzer);

		protected:
			PerfusionMapFilter();
			virtual ~PerfusionMapFilter() {};
			void PrintSelf(std::ostream& os, Indent indent) const;

			/** This method implements the actual reflection of the image.
			*
			* \sa ImageToImageFilter::ThreadedGenerateData(),
			*     ImageToImageFilter::GenerateData()  */
			void GenerateData(void);

			/** PerfusionMapFilter can be implemented as a multithreaded filter.
			* Therefore, this implementation provides a ThreadedGenerateData()
			* routine which is called for each processing thread. The output
			* image data is allocated automatically by the superclass prior to
			* calling ThreadedGenerateData().  ThreadedGenerateData can only
			* write to the portion of the output image specified by the
			* parameter "outputRegionForThread"
			* \sa ImageToImageFilter::ThreadedGenerateData(),
			*     ImageToImageFilter::GenerateData()  */
 // void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
 //                           int threadId );

		private:
			PerfusionMapFilter(const Self&); //purposely not implemented
			void operator=(const Self&); //purposely not implemented

			SegmentInfo* m_arterySegment;
			PerfusionAnalyzer* m_analyzer;

	};
} //end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkperfusionmapfilter.txx"
#endif

#endif