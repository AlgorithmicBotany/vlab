/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __PERFORMANCEMONITOR_H__
#define __PERFORMANCEMONITOR_H__

#ifdef WIN32
#include <Windows.h>

#define TIME_THIS(group) PerformanceMonitor::TimeThis timeThis(group)
//#define TIME_THIS(group) void(group)

class PerformanceMonitor {
public:
  static void Start(int iGroup);
  static void Stop(int iGroup);

  static void Report(std::ostream &target);

  static double TimeGroup(int iGroup);

  class Init {
  public:
    Init();
  };

  struct GroupData {
    GroupData() { mTotalTime.QuadPart = 0L; }
    LARGE_INTEGER mTotalTime;
    LARGE_INTEGER mLastStartTime;
  };

  class TimeThis {
  public:
    TimeThis(int iGroup) : miGroup(iGroup) {
      PerformanceMonitor::Start(miGroup);
    }
    ~TimeThis() { PerformanceMonitor::Stop(miGroup); }

  private:
    int miGroup;
  };

private:
  static LARGE_INTEGER sFrequency;

  static Init sInit;
  static const int kMaxGroups = 10;

  static GroupData groupData[kMaxGroups];
};

#else // WIN32

#define TIME_THIS(group) void(group)

class PerformanceMonitor {
public:
  static void Start(int iGroup) { (void)iGroup; }
  static void Stop(int iGroup) { (void)iGroup; }
  static void Report(std::ostream &target) { (void)target; }
  static double TimeGroup(int iGroup) {
    (void)iGroup;
    return 0;
  }
};

#endif

#endif
