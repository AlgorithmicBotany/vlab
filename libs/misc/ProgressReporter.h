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



#ifndef __PROGRESS_REPORTER_H__
#define __PROGRESS_REPORTER_H__

class ProgressReporter {
private:
  // disable assignment operator
  void operator=(const ProgressReporter &);
  // disable copy constructor
  ProgressReporter(const ProgressReporter &);

public:
  virtual ~ProgressReporter() {}
  ProgressReporter(void (*fnptr)(double), double minVal, double maxVal) {
    fnptr_ = fnptr;
    rep_ = NULL;
    minVal_ = minVal;
    maxVal_ = maxVal;
  }
  ProgressReporter(ProgressReporter *rep, double minVal, double maxVal) {
    fnptr_ = NULL;
    rep_ = rep;
    minVal_ = minVal;
    maxVal_ = maxVal;
  }
  virtual void set(double val) {
    if (val < 0)
      val = 0;
    if (val > 1)
      val = 1;
    if (fnptr_) {
      fnptr_((1 - val) * minVal_ + val * maxVal_);
    } else {
      rep_->set((1 - val) * minVal_ + val * maxVal_);
    }
  }

private:
  void (*fnptr_)(double);
  ProgressReporter *rep_;
  double minVal_;
  double maxVal_;
};

#endif
