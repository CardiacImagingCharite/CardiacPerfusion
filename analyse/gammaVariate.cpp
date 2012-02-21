/*
    This file is part of perfusionkit.
    Copyright 2010 Henning Meyer

    perfusionkit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    perfusionkit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with perfusionkit.  If not, see <http://www.gnu.org/licenses/>.
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
  t0 = t0_;
  t0max = tmax_ - t0_;
  y0 = y0_;
  y0max = ymax_ - y0_;
  alpha = alpha_;
}

double GammaVariate::computeY( double t ) const {
  if (t0max == 0) return y0;
  if (t<=t0) return y0;
  double t_ = (t-t0)/(t0max);
  return  y0 + y0max * std::pow( t_, alpha) * std::exp( alpha * ( 1 - t_ ) );
}


double GammaVariate::getMaximum( void ) const {
  return y0max;
}

double GammaVariate::getBaseline( void ) const {
  return y0;
}


double GammaVariate::getMaxSlope( void ) const {
  double sqrt_alpha = std::sqrt( alpha );
  double slope = std::pow( (1.0 - 1.0 / sqrt_alpha) , alpha )
    * alpha * std::exp( sqrt_alpha ) / ( sqrt_alpha - 1.0 );
  slope *= y0max;
  slope /= t0max;
  return slope;
}

double GammaVariate::getCenterOfGravity() const {
  double beta = t0max / alpha;
  return beta * ( alpha + 1 );
}

double GammaVariate::getAUC() const {
  try {
    double beta = t0max / alpha;
    double k = std::exp( alpha ) * y0max / std::pow(t0max, alpha);
    double auc = k * std::pow( beta, alpha + 1) * boost::math::tgamma( alpha + 1 );
    return auc;
  } catch (boost::exception &) {
    return std::numeric_limits< double >::quiet_NaN();
  }
}



double GammaVariate::distanceToSamples() const {
  unsigned counter = 0;
  double cummulativeDist = 0;
  BOOST_FOREACH( const Sample current, *samples ) {
    double d = computeY(current.first) - current.second;
    cummulativeDist += d * d;
    ++counter;
  }
  return cummulativeDist / counter;
}


double GammaVariate::findAlpha() {
  const double minAlpha = 1.000001;
  if (t0max == 0) { alpha = minAlpha; return minAlpha; }
  std::vector<double> lnX;
  std::vector<double> lnY;

  if (samples->size()==0) { alpha = minAlpha; return minAlpha; }

  double t;
  BOOST_FOREACH( const Sample currentSample, *samples ) {
    if (currentSample.first > t0) {
      t = (currentSample.first - t0) / t0max;
      t = std::log(t) + 1.0 - t;
      lnX.push_back( t );
      
      t = std::log( currentSample.second ) - y0;
      lnY.push_back( t );
    }
  }
  LinearRegression( lnX.begin(), lnX.end(), lnY.begin(), t, alpha );
  if (alpha < minAlpha) alpha = minAlpha;
  return alpha;
}

void GammaVariate::findFromSamples() {
  SampleIt minYIt, maxYIt;
  minYIt = maxYIt = samples->begin();
  for(SampleIt it = samples->begin(); it != samples->end(); ++it) {
    if (it->second < minYIt->second) minYIt = it;
    if (it->second > maxYIt->second) maxYIt = it;
  }

  SampleIt y0It = minYIt;

  t0 = y0It->first;
  y0 = y0It->second;

  double maxT = maxYIt->first;
  
  t0max = maxT - t0;
  y0max = maxYIt->second - y0;
  optimize();
}


void GammaVariate::optimize() {
  Optimizer::Pointer optimizer = Optimizer::New();
  itk::GammaCostFunction::Pointer myCostFunction = itk::GammaCostFunction::New();
  myCostFunction->SetData( samples );
  optimizer->SetCostFunction( myCostFunction );
  Optimizer::ParametersType p(5);
  p[0] = t0;
  p[1] = t0 + t0max;
  p[2] = y0;
  p[3] = y0 + y0max;
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
  if (nonConstSamples) nonConstSamples->clear();
}

void GammaVariate::addSample(double t, double y) {
  if (nonConstSamples) {
    nonConstSamples->insert(Sample(t,y));
  }
}



}
