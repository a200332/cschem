// Connection.h

#ifndef CONNECTION_H

#define CONNECTION_H

#include <QSharedData>
#include <QXmlStreamReader>
#include "PinID.h"
#include <QPolygon>
#include "Layer.h"

class Connection {
public:
  Connection();
  Connection(Connection const &);
  Connection &operator=(Connection const &);
  ~Connection();
  QString report() const;
  Connection(PinID, PinID);
public:
  int id() const;
  int fromId() const; // zero if dangling
  int toId() const; // zero if dangling
  int width() const; // only for board
  Layer layer() const;
  QString fromPin() const;
  QString toPin() const;
  QPolygon const &via() const;
  PinID from() const;
  PinID to() const;
  bool isEquivalentTo(Connection const &) const;
  bool isValid() const;
  /* Valid means either:
     (1) Two endpoints on pins that are not the same
     (2) One endpoint on a pin and at least one point in via
     (3) No endpoints on a pin but at least two points in via, _and_ via
         not starting and ending at same spot.
     Valid cannot test for zero-length in case (2). For that, use
     Geometry::isZeroLength(). */
  bool isDangling() const; // either start or end is dangling
  bool danglingStart() const; // start not connected to pin
  bool danglingEnd() const; // end not connected to pin
  Connection reversed() const; // does *not* assign a new ID
  Connection translated(QPoint delta) const;
public:
  void setFrom(PinID);
  void setTo(PinID);
  void setId(int);
  void setFromId(int);
  void setToId(int);
  void setFromPin(QString);
  void setToPin(QString);
  void setFrom(int id, QString pin="");
  void setTo(int id, QString pin="");
  void setWidth(int);
  void setLayer(Layer);
  void unsetFrom();
  void unsetTo();
  QPolygon &via();
  void setVia(QVector<QPoint> const &);
  void translate(QPoint delta);
  void reverse();
protected:
  void writeAttributes(QXmlStreamWriter &) const;
  void readAttributes(QXmlStreamReader &);
private:
  QSharedDataPointer<class ConnectionData> d;
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Connection const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Connection &);
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Connection const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Connection &);


#endif