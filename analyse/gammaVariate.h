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



class GammaVariate {
  public:
    typedef std::pair< double, double > Sample;
    typedef std::set< Sample > SampleSetType;
    typedef SampleSetType::const_iterator SampleIt;
    typedef boost::shared_ptr< const SampleSetType > SampleSetConstPtrType;
    GammaVariate():nonConstSamples(new SampleSetType), samples(nonConstSamples) {};
    void setSamples(SampleSetConstPtrType sset) { samples = sset; nonConstSamples.reset();}
    void addSample(double t, double y);
    void clearSamples();
    double computeY( double t ) const;
    double getMaxSlope( void ) const;
    double getBaseline( void ) const;
    double getCenterOfGravity() const;
    double getAUC() const;
    double getMaximum( void ) const;
    
    void findFromSamples();
    void getParameters( double &t0_, double &tmax_, double &y0_, double &ymax_, double &alpha_ ) {
      t0_ = t0; tmax_ = t0max; y0_ = y0; ymax_ = y0max; alpha_ = alpha;
    }
    void setParameters( double t0_, double tmax_, double y0_, double ymax_, double alpha_ );

    double findAlpha();

  protected:
    double distanceToSamples() const;
    friend class itk::GammaCostFunction;

  private:
    void optimize();
    double t0;
    double t0max;
    double y0;
    double y0max;
    double alpha;
    boost::shared_ptr< SampleSetType > nonConstSamples;
    SampleSetConstPtrType samples;
};

}
#endif //GAMMAVARIATE_H