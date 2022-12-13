/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <fstream>

#include <Windows.h>

#include "PerformanceMonitor.h"

LARGE_INTEGER PerformanceMonitor::sFrequency;
PerformanceMonitor::GroupData PerformanceMonitor::groupData[];

PerformanceMonitor::Init::Init() {
  ::QueryPerformanceFrequency(&(PerformanceMonitor::sFrequency));
}

PerformanceMonitor::Init sInit;

void PerformanceMonitor::Start(int iGroup) {
  ::QueryPerformanceCounter(&(groupData[iGroup].mLastStartTime));
}

void PerformanceMonitor::Stop(int iGroup) {
  LARGE_INTEGER stopTime;
  ::QueryPerformanceCounter(&stopTime);
  LARGE_INTEGER delta;
  delta.QuadPart =
      stopTime.QuadPart - groupData[iGroup].mLastStartTime.QuadPart;
  groupData[iGroup].mTotalTime.QuadPart += delta.QuadPart;
}

void PerformanceMonitor::Report(std::ostream &target) {
  for (int iGroup = 0; iGroup < kMaxGroups; ++iGroup) {
    double fTime = TimeGroup(iGroup);
    target << "Group: " << iGroup << " --> " << fTime << " s\n";
  }
}

double PerformanceMonitor::TimeGroup(int iGroup) {
  return static_cast<double>(groupData[iGroup].mTotalTime.QuadPart) /
         static_cast<double>(sFrequency.QuadPart);
}
