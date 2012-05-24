#ifndef __itkReflectImageFilter_txx
#define __itkReflectImageFilter_txx

#include "itkperfusionmapfilter.h"

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
	template <class TInputImage, class TOutputImage >
	void
	PerfusionMapFilter<TInputImage,TOutputImage>
	::GenerateData( void )
	{
		
	}

	template <class TInputImage, class TOutputImage >
	void
	PerfusionMapFilter<TInputImage,TOutputImage>::
	PrintSelf(std::ostream& os, Indent indent) const
	{
		Superclass::PrintSelf(os,indent);
  
		os << indent << "Nothing to print yet..." << std::endl;
	}
} //end namespace itk

#endif