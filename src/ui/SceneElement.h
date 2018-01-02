// SceneElement.h

#ifndef SCENEELEMENT_H

#define SCENEELEMENT_H

#include <QGraphicsItemGroup>
#include <QWeakPointer>
#include <QSharedPointer>

class SceneElement: public QGraphicsItemGroup {
public:
  SceneElement(class Scene *parent, class Element const &elt);
  SceneElement(SceneElement const &) = delete;
  SceneElement &operator=(SceneElement const &) = delete;  
  ~SceneElement();
  class WeakPtr {
  public:
    WeakPtr();
    SceneElement *data() const;
    operator bool() const;
    void clear();
  private:
    SceneElement *s;
    QWeakPointer<class SceneElementData> d;
  private:
    WeakPtr(SceneElement *, QSharedPointer<SceneElementData> const &);
    friend class SceneElement;
  };
  WeakPtr weakref();
public:
  class Scene *scene();
  int id() const;
  void showName();
  void hideName();
  void showValue();
  void hideValue();
  void showLabel();
  void hideLabel();
  void rebuild();
  void hover();
  void unhover();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  QSharedPointer<class SceneElementData >d;
};

#endif
