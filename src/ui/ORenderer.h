// ORenderer.h

#ifndef ORENDERER_H

#define ORENDERER_H

#include "data/Object.h"
#include "data/NodeID.h"
#include <QPainter>
#include <QTransform>

class ORenderer {
public:
  enum class Override {
    None,
    WronglyIn,
    Missing
  };
public:
  ORenderer(QPainter *painter, Point const &origin=Point());
  ~ORenderer();
  void setMoving(Point const &movingdelta);
  void setSelPoints(QMap<Layer, QSet<Point> > const &);
  void setPurePoints(QMap<Layer, QSet<Point> > const &);
  void setStuckPoints(QMap<Layer, QSet<Point> > const &);
  void setLayer(Layer l);
  void setOverride(Override ovr);
  void pushOrigin(Point const &origin);
  void popOrigin();
  void drawObject(Object const &o, bool selected=false,
		  QSet<NodeID> const &subnet=QSet<NodeID>());
  void drawGroup(Group const &g, bool selected=false,
		  QSet<NodeID> const &subnet=QSet<NodeID>());
  void drawText(Text const &t, bool selected=false);
  void drawTrace(Trace const &t, bool selected=false, bool innet=false);
  void drawArc(Arc const &g, bool selected=false);
  void drawPad(Pad const &g, bool selected=false, bool innet=false);	       		 
  void drawHole(Hole const &g, bool selected=false, bool innet=false);
public:
  static QByteArray objectToSvg(Object const &,
				  Dim margin=Dim(), Dim minSize=Dim());
  static void render(Group const &, QPainter *); // all layers
  static void render(Object const &, QPainter *); // all layers
private:
  QColor overrideColor(QColor const &) const;
private:
  QPainter *p;
  Layer layer;
  QList<Point> originstack;
  Point origin;
  Point movingdelta;
  QMap<Layer, QSet<Point> > stuckpts;
  QMap<Layer, QSet<Point> > selpts;
  QMap<Layer, QSet<Point> > purepts;
  bool toplevel;
  Override overr;
};

#endif
