/*
    Copyright 2012 Charit� Universit�tsmedizin Berlin, Institut f�r Radiologie
	Copyright 2010 Henning Meyer

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

    KardioPerfusion ist Freie Software: Sie k�nnen es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
    ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.

    KardioPerfusion wird in der Hoffnung, dass es n�tzlich sein wird, aber
    OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
    Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License f�r weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

#include "gammaVariate.h"


#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <boost/math/special_functions/gamma.hpp>
#include <functional>
#include <limits>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/casts.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/foreach.hpp>

#include <itkPowellOptimizer.h>

#include "linearregression.h"

using namespace boost::lambda;

const unsigned int MaximumNumberOfIterations = 1000;
const double minAlpha = 1.000001;

namespace itk {
class GammaCostFunction : public itk::SingleValuedCostFunction  {
  public:
    typedef GammaCostFunction				Self;
    typedef SingleValuedCostFunction	Superclass;
    typedef SmartPointer<Self>			Pointer;
    typedef SmartPointer<const Self>	ConstPointer;
    itkNewMacro(Self);
    itkTypeMacro( GammaCostFunction, SingleValuedCostFunction );	
    void SetData( GammaFunctions::GammaVariate::SampleSetConstPtrType sset) { 
      samples = sset;
    }
    unsigned int GetNumberOfParameters(void) const  { return 5; }
    MeasureType GetValue( const ParametersType & parameters ) const {
      GammaFunctions::GammaVariate myGamma;
      myGamma.setSamples( samples );
      myGamma.setParameters( parameters[0], parameters[1], parameters[2], parameters[3], parameters[4] );
      double dist = myGamma.distanceToSamples();

	  if (parameters[4] < minAlpha){
		  if (parameters[4] <= 1.0) dist = std::numeric_limits< double >::max();
		  else dist /= (1.0/(minAlpha-1.0))*(parameters[4]-1.0);
	  }
      return dist;
    }
    protected:
      void GetDerivative( const ParametersType & parameters, DerivativeType & derivative ) const {throw;}
      GammaCostFunction() {};
      virtual ~GammaCostFunction() {};
    private:
      GammaCostFunction(const Self&); //purposely not implemented
      void operator=(const Self&); //purposely not implemented
      GammaFunctions::GammaVariate::SampleSetConstPtrType samples;
};
}




namespace GammaFunctions {
typedef itk::PowellOptimizer Optimizer;


void GammaVariate::setParameters( double t0_, double tmax_, double y0_, double ymax_, double alpha_ ) {
  m_t0 = t0_;
  m_t0max = tmax_ - t0_;
  m_y0 = y0_;
  m_y0max = ymax_ - y0_;
  m_alpha = alpha_;
}

double GammaVariate::computeY( double t ) const {
  if (m_t0max == 0) return m_y0;
  if (t<=m_t0) return m_y0;
  double t_ = (t-m_t0)/(m_t0max);
  return  m_y0 + m_y0max * std::pow( t_, m_alpha) * std::exp( m_alpha * ( 1 - t_ ) );
}


double GammaVariate::getMaximum( void ) const {
  return m_y0max;
}

double GammaVariate::getBaseline( void ) const {
  return m_y0;
}


double GammaVariate::getMaxSlope( void ) const {
  double sqrt_alpha = std::sqrt( m_alpha );
  double slope = std::pow( (1.0 - 1.0 / sqrt_alpha) , m_alpha )
    * m_alpha * std::exp( sqrt_alpha ) / ( sqrt_alpha - 1.0 );
  slope *= m_y0max;
  slope /= m_t0max;
  return slope;
}

double GammaVariate::getCenterOfGravity() const {
  double beta = m_t0max / m_alpha;
  return beta * ( m_alpha + 1 );
}

double GammaVariate::getAUC() const {
  try {
    double beta = m_t0max / m_alpha;
    double k = std::exp( m_alpha ) * m_y0max / std::pow(m_t0max, m_alpha);
    double auc = k * std::pow( beta, m_alpha + 1) * boost::math::tgamma( m_alpha + 1 );
    return auc;
  } catch (boost::exception &) {
    return std::numeric_limits< double >::quiet_NaN();
  }
}



double GammaVariate::distanceToSamples() const {
  unsigned counter = 0;
  double cummulativeDist = 0;
  BOOST_FOREACH( const Sample current, *m_samples ) {
    double d = computeY(current.first) - current.second;
    cummulativeDist += d * d;
    ++counter;
  }
  return cummulativeDist / counter;
}


double GammaVariate::findAlpha() {

  if (m_t0max == 0) { m_alpha = minAlpha; return minAlpha; }
  std::vector<double> lnX;
  std::vector<double> lnY;

  if (m_samples->size()==0) { m_alpha = minAlpha; return minAlpha; }

  double t;
  BOOST_FOREACH( const Sample currentSample, *m_samples ) {
    if (currentSample.first > m_t0) {
      t = (currentSample.first - m_t0) / m_t0max;
      t = std::log(t) + 1.0 - t;
      lnX.push_back( t );
      
      t = std::log( currentSample.second ) - m_y0;
      lnY.push_back( t );
    }
  }
  LinearRegression( lnX.begin(), lnX.end(), lnY.begin(), t, m_alpha );
  if (m_alpha < minAlpha) m_alpha = minAlpha;
  return m_alpha;
}

void GammaVariate::findFromSamples() {
  SampleIt minYIt, maxYIt;
  minYIt = maxYIt = m_samples->begin();
  for(SampleIt it = m_samples->begin(); it != m_samples->end(); ++it) {
    if (it->second < minYIt->second) minYIt = it;
    if (it->second > maxYIt->second) maxYIt = it;
  }

  SampleIt y0It = minYIt;

  m_t0 = y0It->first;
  m_y0 = y0It->second;

  double maxT = maxYIt->first;
  
  m_t0max = maxT - m_t0;
  m_y0max = maxYIt->second - m_y0;
  optimize();
}


void GammaVariate::optimize() {
  Optimizer::Pointer optimizer = Optimizer::New();
  itk::GammaCostFunction::Pointer myCostFunction = itk::GammaCostFunction::New();
  myCostFunction->SetData( m_samples );
  optimizer->SetCostFunction( myCostFunction );
  Optimizer::ParametersType p(5);
  p[0] = m_t0;
  p[1] = m_t0 + m_t0max;
  p[2] = m_y0;
  p[3] = m_y0 + m_y0max;
  p[4] = findAlpha();
  optimizer->SetInitialPosition( p );
  optimizer->SetMaximumIteration( MaximumNumberOfIterations );
  optimizer->SetValueTolerance(1.0);
  try {
    optimizer->StartOptimization();
  } catch( itk::ExceptionObject & err ) {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
  }
  p = optimizer->GetCurrentPosition();

  setParameters( p[0], p[1], p[2], p[3], p[4]);
}

void GammaVariate::clearSamples() {
  if (m_nonConstSamples) m_nonConstSamples->clear();
}

void GammaVariate::addSample(double t, double y) {
  if (m_nonConstSamples) {
    m_nonConstSamples->insert(Sample(t,y));
  }
}



}
