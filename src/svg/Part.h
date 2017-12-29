// Part.h

#ifndef PART_H

#define PART_H

#include "XmlElement.h"
#include <QRect>
#include <QSharedDataPointer>

class Part {
public:
  Part(XmlElement const &elt);
  Part();
  ~Part();
  Part(Part const &);
  Part &operator=(Part const &);
  XmlElement const &element() const;
  QString name() const;
  QPoint pinPosition(QString pinname) const; // relative to bbox
  QStringList pinNames() const; // sorted
  QPoint origin() const; // position of first pin relative to bbox
  bool isValid() const;
  void setBBox(QRect);
  QRect bbox() const; // in original svg
  QRect shiftedBBox() const; // bbox as if first pin were at (0,0)
private:
  void scanPins(XmlElement const &elt);
private:
  QSharedDataPointer<class PartData> d;
};

#endif
