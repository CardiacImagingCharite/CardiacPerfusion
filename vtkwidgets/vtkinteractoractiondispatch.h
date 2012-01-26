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

#ifndef VTKINTERACTORACTIONDISPATCH_H
#define VTKINTERACTORACTIONDISPATCH_H

#include "signalslib.hpp"
#include <boost/shared_ptr.hpp>

typedef boost::signal<void (int, int, float , float, float)> ActionSignal; // dx, dy, posx, posy, posz
typedef ActionSignal::slot_type ActionSlotType;
struct ActionDispatch {
  enum ActionType {
    MovingAction,
    ClickingAction
  };
  enum RestrictionType {
    Restricted,
    UnRestricted
  };
  boost::shared_ptr< ActionSignal > sig;
  std::string label;
  ActionType atype;
  RestrictionType restrict;
  bool valid;
  ActionDispatch(const std::string &label_, const ActionSignal::slot_type &slot, ActionType atype_, RestrictionType restrict_ )
    :sig( new ActionSignal ), label(label_), atype(atype_), restrict(restrict_), valid( true ) { sig->connect(slot); }
  ActionDispatch():valid(false) {}
};


#endif // VTKINTERACTORACTIONDISPATCH_H