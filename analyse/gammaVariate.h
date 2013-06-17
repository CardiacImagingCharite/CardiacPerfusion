/*
    Copyright 2012 Charité Universitätsmedizin Berlin, Institut für Radiologie
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

#ifndef GAMMAVARIATE_H
#define GAMMAVARIATE_H



#include <set>
#include <boost/shared_ptr.hpp>

namespace itk {
class GammaCostFunction;
}

namespace GammaFunctions {


/*! \class GammaVariate GammaVariate.h "GammaVariate.h"
 *  \brief This is the representation of a gamma variate.
 */
class GammaVariate {
  public:
    typedef std::pair< double, double > Sample;
    typedef std::set< Sample > SampleSetType;
    typedef SampleSetType::const_iterator SampleIt;
    typedef boost::shared_ptr< const SampleSetType > SampleSetConstPtrType;
    GammaVariate():m_nonConstSamples(new SampleSetType), m_samples(m_nonConstSamples) {};
	///Sets a sample set.
	/*!
	\param sset Sample set.
	*/
    void setSamples(SampleSetConstPtrType sset) { m_samples = sset; m_nonConstSamples.reset();}
	///Adds a sample to the set. 
	/*!
	\param t time
	\param y value
	*/
    void addSample(double t, double y);
	///Clears the sample set.
    void clearSamples();
	///Cómputes y value.
	/*!
	\param t time

	\return Returns the y value.
	*/
    double computeY( double t ) const;
	///Computes the maximum slope.
	/*!
	\return Returns the maximum slope.
	*/
    double getMaxSlope( void ) const;
	///Returns the calculated baseline. 
	/*!
	\return Returns the baseline.
	*/
    double getBaseline( void ) const;
	///Returns the center of gravity.
	/*!
	\return Returns the CoG.
	*/
    double getCenterOfGravity() const;
	///Returns the area under the curve.
	/*!
	\return Returns the AuC.
	*/
    double getAUC() const;
	///Returns the maximum.
	/*!
	\return Returns the maximum.
	*/
    double getMaximum( void ) const;
    ///Finds the gamma variate from the given samples.
    void findFromSamples();
	///Getter for all relevant parameter.
    void getParameters( double &t0_, double &tmax_, double &y0_, double &ymax_, double &alpha_ ) {
      t0_ = m_t0; tmax_ = m_t0max; y0_ = m_y0; ymax_ = m_y0max; alpha_ = m_alpha;
    }
	///Setter for all relevant parameter.
    void setParameters( double t0_, double tmax_, double y0_, double ymax_, double alpha_ );
	///Tries to find the alpha of the gamma variate.
    double findAlpha();

  protected:
	  ///Calculates the distance to the samples.
	  /*!
	\return Returns the distance.
	*/
    double distanceToSamples() const;
    friend class itk::GammaCostFunction;

  private:
    void optimize();
    double m_t0;
    double m_t0max;
    double m_y0;
    double m_y0max;
    double m_alpha;
    boost::shared_ptr< SampleSetType > m_nonConstSamples;
    SampleSetConstPtrType m_samples;
};

}
#endif //GAMMAVARIATE_H