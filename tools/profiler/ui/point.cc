// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "point.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <ostream>

namespace gfx {

Point::Point() : x_(0), y_(0) {
}

Point::Point(int x, int y) : x_(x), y_(y) {
}

#if defined(OS_WIN)
Point::Point(DWORD point) {
  POINTS points = MAKEPOINTS(point);
  x_ = points.x;
  y_ = points.y;
}

Point::Point(const POINT& point) : x_(point.x), y_(point.y) {
}

Point& Point::operator=(const POINT& point) {
  x_ = point.x;
  y_ = point.y;
  return *this;
}

POINT Point::ToPOINT() const {
  POINT p;
  p.x = x_;
  p.y = y_;
  return p;
}
#elif defined(OS_MACOSX)
Point::Point(const CGPoint& point) : x_(point.x), y_(point.y) {
}

CGPoint Point::ToCGPoint() const {
  return CGPointMake(x_, y_);
}
#endif

std::ostream& operator<<(std::ostream& out, const gfx::Point& p) {
  return out << p.x() << "," << p.y();
}

}  // namespace gfx
