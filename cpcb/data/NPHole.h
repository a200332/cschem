// Nphole.h

#ifndef NPHOLE_H

#define NPHOLE_H

#include "Rect.h"
#include <QXmlStreamReader>
#include <QDebug>
#include "FreeRotation.h"

class NPHole {
public:
  Point p;
  Dim d;
  Dim slotlength;
  FreeRotation rota;
public:
  NPHole();
  bool isValid() const { return !d.isNull(); }
  Rect boundingRect() const;
  void rotateCW(Point const &p0);
  void flipLeftRight(Dim const &x0);
  void flipUpDown(Dim const &y0);
};

QDebug operator<<(QDebug, NPHole const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, NPHole const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, NPHole &);

#endif