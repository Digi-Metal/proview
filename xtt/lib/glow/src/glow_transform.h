/*
 * ProviewR   Open Source Process Control.
 * Copyright (C) 2005-2019 SSAB EMEA AB.
 *
 * This file is part of ProviewR.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ProviewR. If not, see <http://www.gnu.org/licenses/>
 *
 * Linking ProviewR statically or dynamically with other modules is
 * making a combined work based on ProviewR. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 *
 * In addition, as a special exception, the copyright holders of
 * ProviewR give you permission to, from the build function in the
 * ProviewR Configurator, combine ProviewR with modules generated by the
 * ProviewR PLC Editor to a PLC program, regardless of the license
 * terms of these modules. You may copy and distribute the resulting
 * combined work under the terms of your choice, provided that every
 * copy of the combined work is accompanied by a complete copy of
 * the source code of ProviewR (the version used to produce the
 * combined work), being distributed under the terms of the GNU
 * General Public License plus this exception.
 */

#ifndef glow_transform_h
#define glow_transform_h

#include <fstream>

#include "glow.h"

/**
 * This class represents a 3x3 matrix
 */
class Matrix {
public:
  double a11;
  double a12;
  double a13;
  double a21;
  double a22;
  double a23;

  // The rotation variable is a readonly variable used to keep track of how
  // many degrees (around the z axis) that this matrix will rotate a point
  // if applied to it.
  double rotation;

  Matrix() : a11(1), a12(0), a13(0), a21(0), a22(1), a23(0), rotation(0) {}
  Matrix(const Matrix &m) {
    set(m);
  }

  Matrix& operator=(const Matrix &m) {
    return set(m);
  }

  Matrix& set(const Matrix& m);

  double vertical_scale();
};

Matrix operator*(const Matrix &a, const Matrix &b);
glow_sPoint operator*(const Matrix &a, const glow_sPoint &b);

class GlowTransform : public Matrix {
public:
  GlowTransform() : stored(false) {}

  /**
   * Scale (sx, sy) in the direction indicated by (x0, y0)
   * Like when you drag to scale a window on your computer.
   */
  void scale(double sx, double sy, double x0, double y0);

  /**
   * Rotate "angle" degrees around a point (x0, y0)
   */
  void rotate(double angle, double x0, double y0);

  /**
   * Add a translation (x0, y0) to the translation already part of this matrix
   */
  void move(double x0, double y0);

  /**
   * Set the translation part of this matrix to (x0, y0)
   */
  void posit(double x0, double y0);

  /**
   * Apply the inverse of this matrix on a point (x, y) and return the result in (rx, ry)
   */
  glow_sPoint reverse(double x, double y);

  void save(std::ofstream& fp, glow_eSaveMode mode);
  void open(std::ifstream& fp);

  /**
   * Store a backup of this matrix, i.e. "s = this"
   */
  void store()
  {
    s.set(*this);
    stored = true;
  }

  /**
   * Restore this matrix from backup, i.e. "this = s"
   */
  void revert()
  {
    set(s);
  }

  /**
   * Same as the scale() function above, but based on s instead of this.
   */
  void scale_from_stored(double sx, double sy, double x0, double y0) {
    GlowTransform tmp;
    tmp.set(s);
    tmp.scale(sx, sy, x0, y0);
    tmp.rotation = rotation;
    set(tmp);
  }

  /**
   * Same as the rotate() function above, but based on s instead of this.
   */
  void rotate_from_stored(double angle, double x0, double y0) {
    GlowTransform tmp;
    tmp.set(s);
    tmp.rotate(angle + s.rotation, x0, y0);
    tmp.rotation = angle + s.rotation;
    set(tmp);
  }

  /**
   * Same as the move() function above, but based on s instead of this.
   */
  void move_from_stored(double x0, double y0);

  /**
   * Have we created a backup of this matrix?
   */
  bool is_stored()
  {
    return stored;
  }

  Matrix s;
  bool stored;
};

#endif
