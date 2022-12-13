/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __GILLESPIE_H__
#define __GILLESPIE_H__

class GillespieSuccessor {
public:
  const Lstring::Range &Successor() const { return _successor; }
  void Successor(const Lstring::Range &s) { _successor = s; }
  const Lstring::Range &Predecessor() const { return _predecessor; }
  void Predecessor(const Lstring::Range &p) { _predecessor = p; }
  float Propensity() const { return _propensity; }
  void Propensity(float p) { _propensity = p; }

private:
  Lstring::Range _successor;
  Lstring::Range _predecessor;
  float _propensity;
};

#endif
