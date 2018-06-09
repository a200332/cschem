// Group.h

#ifndef GROUP_H

#define GROUP_H

#include "Rect.h"
#include <QList>
#include <QMap>
#include <QDebug>
#include <QXmlStreamReader>
#include <QSharedData>

class Group {
  /* Invariant: A group may not be empty when placed inside a Layout.
     Empty groups are used to reflect "invalid" status.
  */
public:
  Point origin; // to be added to all contained coordinates;
  // relative to parent's origin
  QString ref;
public:
  Group();
  ~Group();
  Group(Group const &);
  Group &operator=(Group const &);
  int insert(class Object const &); // assigns ID
  void remove(int);
  bool contains(int) const;
  Object const &object(int) const;
  Object &object(int);
  QList<int> keys() const;
  bool isEmpty() const;
  Group const &subgroup(QList<int> path) const; // follows breadcrumbs
  Group &subgroup(QList<int> path);
  // Caution: Do NOT change the empty group that may be returned.
  Rect boundingRect() const; // relative to parent
  bool touches(Point p, Dim mrg=Dim()) const;
  Point originOf(QList<int> path) const; // relative to parent
  QList<int> objectsAt(Point p, Dim mrg=Dim()) const; // p is relative to parent
  // only direct children are returned
  int formSubgroup(QSet<int> const &);
  void dissolveSubgroup(int);
  void rotateCCW(Point p); // p is relative to parent
  void rotateCW(Point p); // p is relative to parent
  void flipLeftRight(Dim x); // x is relative to parent
  void flipUpDown(Dim y); // y is relative to parent
private:
  QSharedDataPointer<class GData> d;
  friend QDebug operator<<(QDebug, Group const &);  
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Group const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Group &);
};

QDebug operator<<(QDebug, Group const &);  
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Group const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Group &);

#endif
