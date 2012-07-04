
#ifndef itkNaryMaximumSlopeImageFilter_H
#define itkNaryMaximumSlopeImageFilter_H

#include "itkNaryFunctorImageFilter.h"
#include "gammaVariate.h"

#include "itkNumericTraits.h"

namespace itk{

namespace Functor{

template<class TInput, class TOutput>
	class MaximumSlope1
{
public:
	typedef typename NumericTraits< TOutput >::ValueType OutputValueType;

	MaximumSlope1(){}
	~MaximumSlope1(){}

	inline TOutput operator()(const std::vector< TInput > & B) const
	{
		GammaFunctions::GammaVariate gamma;
		OutputValueType A = NumericTraits< TOutput >::NonpositiveMin();
		if(B.size() == times.size())
		{
			for(int i = 0; i < B.size(); i++)
			{
				gamma.addSample(times[i], B[i]);
			}

			gamma.findFromSamples();
			A = static_cast< OutputValueType >( gamma.getMaxSlope() );
		}

		return A;
	}

	bool operator==(const MaximumSlope1 &) const
	{
		return true;
	}
 
	bool operator!=(const MaximumSlope1 &) const
	{
		return false;
	}

	inline void setTimePoints(const std::vector< double > _times)
	{
		times = _times;
	}

private:
	std::vector< double > times;
};
}

template< class TInputImage, class TOutputImage >
class ITK_EXPORT NaryMaximumSlopeImageFilter:
	public
	NaryFunctorImageFilter< TInputImage, TOutputImage,
	Functor::MaximumSlope1< typename TInputImage::PixelType,
	typename TInputImage::PixelType > >
{
public:
	typedef NaryMaximumSlopeImageFilter Self;
	typedef NaryFunctorImageFilter<
			TInputImage, TOutputImage,
			Functor::MaximumSlope1< typename TInputImage::PixelType,
			typename TInputImage::PixelType > > Superclass;
 
	typedef SmartPointer< Self > Pointer;
	typedef SmartPointer< const Self > ConstPointer;
 
	itkNewMacro(Self);
 
	itkTypeMacro(NaryMaximumSlopeImageFilter,
			NaryFunctorImageFilter);
 
#ifdef ITK_USE_CONCEPT_CHECKING
 
	itkConceptMacro( InputConvertibleToOutputCheck,
			( Concept::Convertible< typename TInputImage::PixelType,
			typename TOutputImage::PixelType > ) );
	itkConceptMacro( InputLessThanComparableCheck,
			( Concept::LessThanComparable< typename TInputImage::PixelType > ) );
	itkConceptMacro( InputHasNumericTraitsCheck,
			( Concept::HasNumericTraits< typename TInputImage::PixelType > ) );
 
#endif

protected:
	NaryMaximumSlopeImageFilter() {}
	virtual ~NaryMaximumSlopeImageFilter() {}
private:
	NaryMaximumSlopeImageFilter(const Self &); //purposely not implemented
	void operator=(const Self &); //purposely not implemented
};

}//end namespace itk

#endif