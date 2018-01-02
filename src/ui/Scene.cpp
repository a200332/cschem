// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverManager.h"
#include "ConnBuilder.h"
#include "svg/CircuitMod.h"
#include "svg/Geometry.h"

class SceneData {
public:
  SceneData(Scene *scene, PartLibrary const *lib):
    scene(scene), lib(lib) {
    hovermanager = 0;
    connbuilder = 0;
  }
  QPointF pinPosition(int id, QString pin) const {
    if (circ.elements().contains(id))
      return lib->upscale(Geometry(circ, lib).pinPosition(id, pin));
    else 
      return QPoint();
  }
  bool undo() {
    if (undobuffer.isEmpty())
      return false;
    redobuffer << circ;
    circ = undobuffer.takeLast();
    return true;
  }
  bool redo() {
    if (redobuffer.isEmpty())
      return false;
    undobuffer << circ;
    circ = redobuffer.takeLast();
    return true;
  }
  void preact() {
    undobuffer << circ;
    redobuffer.clear();
  }
  void deleteElement(int id);
  void deleteConnection(int id);
  void deleteConnectionSegment(int id, int seg);
  void rebuildAsNeeded(CircuitMod const &cm);
public:
  Scene *scene;
  PartLibrary const *lib;
  Circuit circ;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
  QPointF mousexy;
  HoverManager *hovermanager;
  QList<Circuit> undobuffer;
  QList<Circuit> redobuffer;
  ConnBuilder *connbuilder;
};

void SceneData::rebuildAsNeeded(CircuitMod const &cm) {
  hovermanager->unhover();
  circ = cm.circuit();
  for (int id: cm.affectedConnections()) {
    if (circ.connections().contains(id)) {
      if (conns.contains(id))
        conns[id]->rebuild();
      else
        conns[id] = new SceneConnection(scene, circ.connection(id));
    } else {
      delete conns[id];
      conns.remove(id);
    }
  }
  for (int id: cm.affectedElements()) {
    if (circ.elements().contains(id)) {
      if (elts.contains(id))
        elts[id]->rebuild();
      else
        elts[id] = new SceneElement(scene, circ.element(id));
    } else {
      delete elts[id];
      elts.remove(id);
    }
  }
}

void SceneData::deleteElement(int id) {
  CircuitMod cm(circ, lib);
  cm.deleteElement(id);
  rebuildAsNeeded(cm);
}  

void SceneData::deleteConnection(int id) {
  CircuitMod cm(circ, lib);
  cm.deleteConnection(id);
  rebuildAsNeeded(cm);
}  

void SceneData::deleteConnectionSegment(int id, int seg) {
  CircuitMod cm(circ, lib);
  cm.deleteConnectionSegment(id, seg);
  rebuildAsNeeded(cm);
}  

Scene::~Scene() {
  delete d;
}
  
Scene::Scene(PartLibrary const *lib, QObject *parent):
  QGraphicsScene(parent) {
  d = new SceneData(this, lib);
  d->hovermanager = new HoverManager(this);
}

void Scene::setCircuit(Circuit const &c) {
  for (auto i: d->elts)
    delete i;
  d->elts.clear();
  for (auto i: d->conns)
    delete i;
  d->conns.clear();
  d->circ = c;
  rebuild();
}


void Scene::rebuild() {
  d->hovermanager->unhover();
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: d->elts)
    delete i;
  d->elts.clear();
  for (auto i: d->conns)
    delete i;
  d->conns.clear();

  for (auto const &c: d->circ.elements()) 
    d->elts[c.id()] = new SceneElement(this, c);
  
  for (auto const &c: d->circ.connections())
    d->conns[c.id()] = new SceneConnection(this, c);
}

QPointF Scene::pinPosition(int partid, QString pin) const {
  return d->pinPosition(partid, pin);
}


PartLibrary const *Scene::library() const {
  return d->lib;
}

Circuit const &Scene::circuit() const {
  return d->circ;
}

Circuit &Scene::circuit() {
  return d->circ;
}

QSet<int> Scene::selectedElements() const {
  QSet<int> selection;
  for (int id: d->elts.keys())
    if (d->elts[id]->isSelected())
      selection << id;
  return selection;
}

void Scene::tentativelyMoveSelection(QPointF delta) {
  QSet<int> selection = selectedElements();
  QSet<int> internalcons = d->circ.connectionsIn(selection);
  QSet<int> fromcons = d->circ.connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ.connectionsTo(selection) - internalcons;

  for (int id: internalcons)
    d->conns[id]->temporaryTranslate(delta);
  for (int id: fromcons)
    d->conns[id]->temporaryTranslateFrom(delta);
  for (int id: tocons)
    d->conns[id]->temporaryTranslateTo(delta);
}

void Scene::moveSelection(QPointF delta) {
  QPoint dd = (delta/d->lib->scale()).toPoint();

  QSet<int> selection = selectedElements();
  QSet<int> internalcons = d->circ.connectionsIn(selection);
  QSet<int> fromcons = d->circ.connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ.connectionsTo(selection) - internalcons;
  
  if (!dd.isNull()) {
    // must actually change circuit
    d->preact();
    Circuit origCirc(d->circ);
    CircuitMod cm(d->circ, d->lib);
    for (int id: selection)
      cm.translateElement(id, dd);
    for (int id: internalcons)
      cm.translateConnection(id, dd);

    for (int id: fromcons)
      cm.reroute(id, origCirc);
    for (int id: tocons)
      cm.reroute(id, origCirc);

    for (int id: selection)
      cm.removeOverlappingJunctions(id);
    for (int id: fromcons)
      cm.adjustOverlappingConnections(id);

    // Just a little sanity check
    Circuit newcirc = cm.circuit();
    for (int id: selection)
      if (!newcirc.elements().contains(id))
	qDebug() << "Selected element" << origCirc.element(id).report()
		 << "got deleted in move. This might be bad.";
    // End of sanity check
    
    d->rebuildAsNeeded(cm);
  } else {
    // restore stuff
    for (int id: selection)
      d->elts[id]->rebuild();
    for (int id: internalcons + fromcons + tocons)
      d->conns[id]->rebuild();
  }
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "Scene: mousePress" << e->scenePos();
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());

  if (d->connbuilder) {
    d->connbuilder->mousePress(e);
    update();
  } else {
    if (d->hovermanager->onPin()) {
      d->connbuilder = new ConnBuilder(this);
      addItem(d->connbuilder);
      d->connbuilder->start(e->scenePos(),
                            d->hovermanager->element(),
                            d->hovermanager->pin());
    } else {
      QGraphicsScene::mousePressEvent(e);
    }
  }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : e->buttons()
                                     ? HoverManager::Purpose::None
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseMove(e);
    update();
  } else {
    QGraphicsScene::mouseMoveEvent(e);
  }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "Scene: mouseRelease" << e->scenePos();
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseRelease(e);
    update();
    if (d->connbuilder->isComplete())
      finalizeConnection();
    d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                        e->modifiers() & Qt::ShiftModifier)
                                       ? HoverManager::Purpose::Connecting
                                       : HoverManager::Purpose::Moving);
  } else {
    QGraphicsScene::mouseReleaseEvent(e);
  }
}

void Scene::keyReleaseEvent(QKeyEvent *e) {
  // for whatever reason, releasing shift produces a key event with key==0
  // and modifiers() still containing shift. Useless.
  if (e->key()==Qt::Key_Shift
      && d->hovermanager->primaryPurpose() != HoverManager::Purpose::None)      
    d->hovermanager->setPrimaryPurpose(d->connbuilder
                                       ? HoverManager::Purpose::Connecting
                                       : HoverManager::Purpose::Moving);
  QGraphicsScene::keyReleaseEvent(e);
}

void Scene::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Shift
      && d->hovermanager->primaryPurpose() != HoverManager::Purpose::None)
    d->hovermanager->setPrimaryPurpose(HoverManager::Purpose::Connecting);

  if (d->connbuilder) {
    d->connbuilder->keyPress(e);
    if (d->connbuilder->isComplete())
      finalizeConnection();
    return;
  }
  
  if (focusItem()==0) {
    QGraphicsItem *item = itemAt(d->mousexy, QTransform());
    while (item && item->parentItem())
      item = item->parentItem();

    SceneElement *elt = dynamic_cast<SceneElement *>(item);
    SceneConnection *con = dynamic_cast<SceneConnection *>(item);

    if (elt)
      keyPressOnElement(elt, e);
    else if (con)
      keyPressOnConnection(con, con->segmentAt(d->mousexy), e);

    keyPressAnywhere(e);
  } else {
    QGraphicsScene::keyPressEvent(e);
  }
}

void Scene::keyPressOnElement(class SceneElement *elt, QKeyEvent *e) {
  int id = elt->id();
  switch (e->key()) {
  case Qt::Key_Delete:
    d->preact();
    d->deleteElement(id);
    break;
  }
}

void Scene::keyPressOnConnection(class SceneConnection *con, int seg,
				 QKeyEvent *e) {
  int id = con->id();
  switch (e->key()) {
  case Qt::Key_Delete: {
    d->preact();
    d->deleteConnectionSegment(id, seg);
  } break;
  }
}

void Scene::keyPressAnywhere(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Z:
    if (e->modifiers() & Qt::ControlModifier) {
      if (e->modifiers() & Qt::ShiftModifier) {
	if (d->redo())
	  rebuild();
      } else {
	if (d->undo())
	  rebuild();
      }
    }
    break;
  }
}

int Scene::elementAt(QPointF scenepos) const {
  if (!itemAt(scenepos, QTransform()))
    return -1; // shortcut in case nothing there at all
  
  for (auto e: d->elts)
    if (e->boundingRect().contains(e->mapFromScene(scenepos)))
      return e->id();

  return -1;
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

QString Scene::pinAt(QPointF scenepos, int elementId) const {
  if (!d->elts.contains(elementId))
    return "-";
  QString sym = d->circ.element(elementId).symbol();
  Part const &part = library()->part(sym);
  double r = library()->scale();
  for (auto p: part.pinNames())
    if (L2(scenepos - pinPosition(elementId, p)) <= 1.1*r*r)
      return p;
  return "-";
}

int Scene::connectionAt(QPointF scenepos, int *segp) const {
  if (segp)
    *segp = -1;
  
  if (!itemAt(scenepos, QTransform()))
    return -1; // shortcut in case nothing there at all
  
 for (auto c: d->conns) {
   int seg = c->segmentAt(scenepos);
   if (seg>=0) {
     if (segp)
       *segp = seg;
     return c->id();
   }
 }
 
 return -1;
}

void Scene::finalizeConnection() {
  d->hovermanager->unhover();
  if (!d->connbuilder->isAbandoned()) {
    d->preact();
    QList<int> cc;
    for (auto c: d->connbuilder->junctions()) {
      d->circ.insert(c);
      if (d->elts.contains(c.id()))
        delete d->elts[c.id()];
      d->elts[c.id()] = new SceneElement(this, c);
    }
    for (auto c: d->connbuilder->connections()) {
      d->circ.insert(c);
      if (d->conns.contains(c.id()))
        delete d->conns[c.id()];
      d->conns[c.id()] = new SceneConnection(this, c);
      cc << c.id();
    }

    CircuitMod cm(d->circ, d->lib);
    for (int c: cc) 
      cm.simplifyConnection(c);
    for (int c: cc)
      qDebug() << "Connection" << d->circ.connection(c).report();
    for (int c: cc)
      cm.removeConnectionsEquivalentTo(c);
    for (int c: cc)
      cm.adjustOverlappingConnections(c);
    for (auto c: d->connbuilder->junctions())
      cm.removePointlessJunction(c.id());
    d->rebuildAsNeeded(cm);
  }

  delete d->connbuilder;
  d->connbuilder = 0;
}

QMap<int, class SceneElement *> const &Scene::elements() const {
  return d->elts;
}

QMap<int, class SceneConnection *> const &Scene::connections() const {
  return d->conns;
}

void Scene::modifyConnection(int id, QPolygonF newpath) {
  qDebug() << newpath << id;
  if (!connections().contains(id))
    return;

  d->preact();
  
  Connection con(d->circ.connection(id));
  if (con.fromId() > 0)
    newpath.removeFirst();
  if (con.toId() > 0)
    newpath.removeLast();
  QPolygon pp;
  for (auto p: newpath)
    pp << d->lib->downscale(p);
  con.setVia(pp);
  d->circ.insert(con);
  d->conns[id]->rebuild();
  qDebug() << "rebuilt";

  CircuitMod cm(d->circ, d->lib);
  if (cm.adjustOverlappingConnections(id))
    d->rebuildAsNeeded(cm);
}
