#ifndef itkPerfusionMapFilter_H
#define itkPerfusionMapFilter_H

#include "itkImageToImageFilter.h"

namespace itk{


	
	template <class TInputImage, class TOutputImage>
	class ITK_EXPORT PerfusionMapFilter : public ImageToImageFilter< TInputImage, TOutputImage>
	{
		public:
			/** Standard class typedefs. */
			typedef PerfusionMapFilter                         Self;
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
			
		protected:
			PerfusionMapFilter();
			virtual ~PerfusionMapFilter() {};
			void PrintSelf(std::ostream& os, Indent indent) const;

			/** This method implements the actual reflection of the image.
			*
			* \sa ImageToImageFilter::ThreadedGenerateData(),
			*     ImageToImageFilter::GenerateData()  */
			void GenerateData(void);
	};
} //end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkperfusionmapfilter.txx"
#endif

#endif