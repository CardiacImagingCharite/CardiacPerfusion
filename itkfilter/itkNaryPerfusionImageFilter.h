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