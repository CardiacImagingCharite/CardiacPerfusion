
#ifndef itkNaryPerfusionImageFilter_H
#define itkNaryPerfusionImageFilter_H

#include "itkNaryFunctorImageFilter.h"
#include "itkNumericTraits.h"

#include "gammaVariate.h"

namespace itk{

namespace Functor{

template<class TInput, class TOutput>
	class Perfusion1
{
public:
	typedef typename NumericTraits< TOutput >::ValueType OutputValueType;

	Perfusion1(){}
	~Perfusion1(){}

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
			A = static_cast< OutputValueType >( 60 * gamma.getMaxSlope() / arteryGamma.getMaximum() );
			if(!isNumber(A) || !isFiniteNumber(A))
				A = 0;
		}

		return A;
	}

	bool operator==(const Perfusion1 &) const
	{
		return true;
	}
 
	bool operator!=(const Perfusion1 &) const
	{
		return false;
	}
	
	inline void setTimePoints(const std::vector< double > &_times)
	{
		times = _times;
	}

	inline void setArteryGammaFunction(const GammaFunctions::GammaVariate &gamma)
	{
		arteryGamma = gamma;
	}

	inline bool isNumber(const TOutput x) const 
	{
		return (x == x);
	}

	inline bool isFiniteNumber(const TOutput x) const
	{
		return (x <= DBL_MAX && x >= -DBL_MAX); 
	}

private:
	std::vector< double > times;
	GammaFunctions::GammaVariate arteryGamma;


};
}

template< class TInputImage, class TOutputImage >
class ITK_EXPORT NaryPerfusionImageFilter:
	public
	NaryFunctorImageFilter< TInputImage, TOutputImage,
	Functor::Perfusion1< typename TInputImage::PixelType,
	typename TOutputImage::PixelType > >
{
public:
	typedef NaryPerfusionImageFilter Self;
	typedef NaryFunctorImageFilter<
			TInputImage, TOutputImage,
			Functor::Perfusion1< typename TInputImage::PixelType,
			typename TOutputImage::PixelType > > Superclass;
 
	typedef SmartPointer< Self > Pointer;
	typedef SmartPointer< const Self > ConstPointer;
 
	itkNewMacro(Self);
 
	itkTypeMacro(NaryPerfusionImageFilter,
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
	NaryPerfusionImageFilter() {}
	virtual ~NaryPerfusionImageFilter() {}
private:
	NaryPerfusionImageFilter(const Self &); //purposely not implemented
	void operator=(const Self &); //purposely not implemented
};

}//end namespace itk

#endif