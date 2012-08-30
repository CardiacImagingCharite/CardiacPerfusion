/*
    Copyright 2012 Christian Freye
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

#ifndef PATLAKDATA_H
#define PATLAKDATA_H

#include <qwt_data.h>


class PatlakData : public QwtData
{
  public:
    PatlakData( const QwtData &tissueSource, const QwtData &arterySource );
    virtual double y(size_t i) const;
    virtual double x(size_t i) const;
    virtual size_t size() const;
    virtual QwtData* copy() const { return new PatlakData( *this ); }
    inline void setTissueBaseline(double tb) { tissueBaseline = tb; }
    inline void setArteryBaseline(double ab) { arteryBaseline = ab; }
    inline size_t getStartIndex() const { return startIndex; }
    inline size_t getEndIndex() const { return endIndex; }
    inline void setStartIndex(size_t index) { startIndex = std::min(index,std::min(tissue.size(), artery.size())-1); }
    inline void setEndIndex(size_t index) { endIndex = std::min(index,std::min(tissue.size(), artery.size())-1);  }
  private:
    const QwtData &tissue;
    const QwtData &artery;
    double tissueBaseline, arteryBaseline;
    size_t startIndex, endIndex;
};

#endif // PATLAKDATA_H
