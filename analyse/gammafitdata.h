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

#ifndef GAMMAFITDATA_H
#define GAMMAFITDATA_H

#include <qwt_data.h>
#include <limits>
#include "gammaVariate.h"

const unsigned GammaSamples = 500;
class GammaFitData: public QwtData {
public:
  GammaFitData():
    xstart(std::numeric_limits<double>::max()), xend(std::numeric_limits<double>::min()) { }
    virtual QwtData *copy() const {
        return new GammaFitData(*this);
    }
    virtual size_t size() const {
        return GammaSamples;
    }
    virtual double x(size_t i) const {
        return xstart + (xend - xstart) / GammaSamples * i;
    }
    virtual double y(size_t i) const {
      return gamma.computeY(x(i));
    }
    GammaFunctions::GammaVariate &getGammaVariate() { return gamma; }
    const GammaFunctions::GammaVariate &getGammaVariate() const { return gamma; }
    void includeTime(double t) {
      if (t>xend) xend = t;
      if (t<xstart) xstart = t;
    }
private:
  double xstart, xend;
  GammaFunctions::GammaVariate gamma;
};


#endif // GAMMAFITDATA_H
