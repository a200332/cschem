// Point.h

#ifndef POINT_H

#define POINT_H

#include <QDebug>
#include "Dim.h"
#include <QPointF>

struct Point {
  Dim x;
  Dim y;
public:
  explicit Point(Dim x=Dim(), Dim y=Dim()): x(x), y(y) { }
  QString toString() const { return x.toString() + " " + y.toString(); }
  static Point fromString(QString s, bool *ok=0);
  QPointF toMils() const { return QPointF(x.toMils(), y.toMils()); }
  static Point fromMils(QPointF const &p) {
    return Point(Dim::fromMils(p.x()), Dim::fromMils(p.y()));
  }
  bool operator==(Point const &o) const { return x==o.x && y==o.y; }
};

QDebug operator<<(QDebug, Point const &);
inline uint qHash(Point const &p) {
  return qHash(QPair<double,double>(p.x.toMils(),p.y.toMils()));
}


#endif
