// MainWindow.cpp

#include "MainWindow.h"
#include <QGraphicsView>
#include "Scene.h"
#include "circuit/Schem.h"
#include "svg/SymbolLibrary.h"
#include "file/FileIO.h"
#include <QMenuBar>
#include <QApplication>
#include <QStatusBar>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include "Style.h"
#include <QMessageBox>
#include <QDesktopServices>
#include "LibView.h"
#include "PartListView.h"
#include "PartList.h"
#include <QDockWidget>
#include "LibView.h"
#include "ui/SignalAccumulator.h"
#include "svg/SvgExporter.h"
#include  <QClipboard>
#include <QItemSelectionModel>
#include "HoverManager.h"
#include "circuit/NumberConflicts.h"
#include "circuit/ContainerConflicts.h"
#include "svg/Paths.h"

class MWData {
public:
  MWData():
    view(0),
    scene(0),
    libview(0), libviewdock(0),
    partlistview(0), partlistviewdock(0),
    unsaved(false),
    recursedepth(0) {
  }
public:
  QGraphicsView *view;
  Scene *scene;
  QString filename;
  static QString lastdir;
  LibView *libview;
  QDockWidget *libviewdock;
  PartListView *partlistview;
  QDockWidget *partlistviewdock;
  bool unsaved;
  int recursedepth;
};

QString MWData::lastdir;

MainWindow::MainWindow(): d(new MWData()) {
  createView();
  createDocks();
  createActions();
  d->view->scale(1.5, 1.5);
  d->libview->scale(1.5);
  create(Schem());
  int w0 = 10 * d->libview->width();
  int h0 = 3 * w0 / 4;
  int w = width();
  int h = height();
  if (w < w0)
    w = w0;
  if (h < h0)
    h = h0;
  //  QPoint p0 = d->view->mapFromScene(d->scene->sceneRect().bottomRight());
  resize(w, h);
  // resize(d->libview->width() + p0.x() + 100, p0.y() + 100);
}

void MainWindow::createDocks() {
  d->libview = new LibView();
  connect(d->libview, SIGNAL(activated(QString)),
          this, SLOT(plonk(QString)));
  d->libviewdock = new QDockWidget("Library", this);
  d->libviewdock->setWidget(d->libview);
  showLibrary();

  d->partlistview = new PartListView();
  d->partlistviewdock = new QDockWidget("Parts list");
  d->partlistviewdock->setWidget(d->partlistview);
  d->partlistviewdock->hide();
}

void MainWindow::openSymbolLibraryFolder() {
  QDesktopServices::openUrl(QUrl(Paths::userSymbolRoot()));
}
 
void MainWindow::showLibrary() {
  bool vis = d->libviewdock->isVisible();
  if (vis) {
    d->libviewdock->hide();
  } else {
    d->libviewdock->show();
    addDockWidget(Qt::LeftDockWidgetArea, d->libviewdock);
  }
}

void MainWindow::showPartsList() {
  bool vis = d->partlistviewdock->isVisible();
  if (vis) {
    d->partlistviewdock->hide();
  } else {
    d->partlistviewdock->show();
    addDockWidget(Qt::RightDockWidgetArea, d->partlistviewdock);
  }
}

void MainWindow::createView() {
  d->view = new QGraphicsView(this);
  d->view->setInteractive(true);
  d->view->setMouseTracking(true);
  setCentralWidget(d->view);
}

void MainWindow::createActions() {
  QMenu *menu = 0;
  QAction *act = 0;

  menu = menuBar()->addMenu(tr("&File"));

  act = new QAction(tr("&New"), this);
  act->setShortcuts(QKeySequence::New);
  act->setStatusTip(tr("Create a new file"));
  connect(act, &QAction::triggered, this, &MainWindow::newAction);
  menu->addAction(act);

  act = new QAction(tr("&Open…"), this);
  act->setShortcuts(QKeySequence::Open);
  act->setStatusTip(tr("Open an existing file"));
  connect(act, &QAction::triggered, this, &MainWindow::openAction);
  menu->addAction(act);

  act = new QAction(tr("&Save"), this);
  act->setShortcuts(QKeySequence::Save);
  connect(act, &QAction::triggered, this, &MainWindow::saveAction);
  menu->addAction(act);

  act = new QAction(tr("Save &as…"), this);
  act->setShortcuts(QKeySequence::SaveAs);
  connect(act, &QAction::triggered, this, &MainWindow::saveAsAction);
  menu->addAction(act);

  act = new QAction(tr("&Export circuit as svg…"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
  connect(act, &QAction::triggered, this, &MainWindow::exportCircuitAction);
  menu->addAction(act);

  act = new QAction(tr("Export &parts list as csv…"), this);
  connect(act, &QAction::triggered, this, &MainWindow::exportPartListAction);
  menu->addAction(act);

  act = new QAction(tr("Copy circuit &image to clipboard"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
  connect(act, &QAction::triggered, this, &MainWindow::circuitToClipboardAction);
  menu->addAction(act);

  act = new QAction(tr("Copy parts lis&t to clipboard"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
  connect(act, &QAction::triggered,
	  this, &MainWindow::partListToClipboardAction);
  menu->addAction(act);
  
  act = new QAction(tr("&Quit"), this);
  act->setShortcuts(QKeySequence::Quit);
  act->setStatusTip(tr("Quit the program"));
  connect(act, &QAction::triggered, this, &MainWindow::quitAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&Edit"));

  act = new QAction(tr("&Copy"), this);
  act->setShortcuts(QKeySequence::Copy);
  act->setStatusTip(tr("Copy selection to clipboard"));
  connect(act, &QAction::triggered, this, &MainWindow::copyAction);
  menu->addAction(act);
  
  act = new QAction(tr("Cu&t"), this);
  act->setShortcuts(QKeySequence::Cut);
  act->setStatusTip(tr("Cut selection to clipboard"));
  connect(act, &QAction::triggered, this, &MainWindow::cutAction);
  menu->addAction(act);
  
  act = new QAction(tr("&Paste"), this);
  act->setShortcuts(QKeySequence::Paste);
  act->setStatusTip(tr("Paste clipboard into circuit"));
  connect(act, &QAction::triggered, this, &MainWindow::pasteAction);
  menu->addAction(act);

  act = new QAction(tr("&Undo"), this);
  act->setShortcuts(QKeySequence::Undo);
  act->setStatusTip(tr("Undo"));
  connect(act, &QAction::triggered, this, &MainWindow::undoAction);
  menu->addAction(act);

  act = new QAction(tr("Re&do"), this);
  act->setShortcuts(QKeySequence::Redo);
  act->setStatusTip(tr("Redo"));
  connect(act, &QAction::triggered, this, &MainWindow::redoAction);
  menu->addAction(act);

  act = new QAction(tr("&Rotate clockwise"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(Qt::CTRL + Qt::Key_R));
  connect(act, &QAction::triggered, this, &MainWindow::rotateCWAction);
  menu->addAction(act);

  act = new QAction(tr("Rotate &anticlockwise"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
  connect(act, &QAction::triggered, this, &MainWindow::rotateCCWAction);
  menu->addAction(act);

  act = new QAction(tr("&Flip"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(Qt::CTRL + Qt::Key_F));
  act->setStatusTip(tr("Flip horizontally"));
  connect(act, &QAction::triggered, this, &MainWindow::flipAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&Tools"));

  
  act = new QAction(tr("&Open external symbol library"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
  act->setStatusTip(tr("Open folder view onto external symbol library"));
  connect(act, &QAction::triggered, this, &MainWindow::openSymbolLibraryFolder);
  menu->addAction(act);
  
  act = new QAction(tr("Remove &dangling connections"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  act->setStatusTip(tr("Cleanup circuit by removing dangling connections"));
  connect(act, &QAction::triggered, this, &MainWindow::removeDanglingAction);
  menu->addAction(act);

  act = new QAction(tr("Check for &numbering conflicts"), this);
  act->setStatusTip(tr("Check whether any circuit elements improperly share "
		       "the same names, and if so, offer to renumber them"));
  connect(act, &QAction::triggered, this, &MainWindow::resolveConflictsAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&View"));

  act = new QAction(tr("&Zoom in"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(QKeySequence::ZoomIn)
		    << QKeySequence(Qt::CTRL + Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Plus));
  act->setStatusTip(tr("Zoom in"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomIn);
  menu->addAction(act);

  act = new QAction(tr("&Zoom out"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence::ZoomOut
                    << QKeySequence(Qt::Key_Minus));
  act->setStatusTip(tr("Zoom out"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomOut);
  menu->addAction(act);

  act = new QAction(tr("&Library"), this);
  act->setStatusTip(tr("Show/hide library pane"));
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
  connect(act, &QAction::triggered, this, &MainWindow::showLibrary);
  menu->addAction(act);

  act = new QAction(tr("&Parts List"), this);
  act->setStatusTip(tr("Show/hide parts list"));
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
  connect(act, &QAction::triggered, this, &MainWindow::showPartsList);
  menu->addAction(act);

  menuBar()->addSeparator();
  menu = menuBar()->addMenu(tr("&Help"));

  act = new QAction(tr("&About…"), this);
  act->setStatusTip(tr("Information about this program"));
  connect(act, &QAction::triggered, this, &MainWindow::aboutAction);
  menu->addAction(act);

  act = new QAction(tr("Delete element or trace"), this);
  act->setShortcut(QKeySequence(Qt::Key_Delete));
  addAction(act);
  connect(act, &QAction::triggered, [this]() { d->scene->key_delete(); });

  act = new QAction(tr("Delete annotation"), this);
  act->setShortcut(QKeySequence(Qt::Key_Backspace));
  addAction(act);
  connect(act, &QAction::triggered, [this]() { d->scene->key_backspace(); });
}

void MainWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
  delete d;
}

void MainWindow::newAction() {
  auto *mw = d->scene->schem().isEmpty() ? this : new MainWindow();
  mw->create(Schem());
  mw->show();
}

void MainWindow::openAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = Paths::defaultLocation();
  QString fn = QFileDialog::getOpenFileName(0,
					    "Open schematic file",
					    d->lastdir,
					    tr("Schematics (*.schem)"));
  if (!fn.isEmpty()) {
    auto *mw = d->scene->schem().isEmpty() ? this : new MainWindow();
    mw->load(fn);
    mw->show();
  }
}

bool MainWindow::saveAction() {
  if (d->filename.isEmpty())
    return saveAsAction();
  else
    return saveAs(d->filename);
}

bool MainWindow::saveAsAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = Paths::defaultLocation();
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Save schematic as…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("schem");
  dlg.setDirectory(d->lastdir);
  dlg.setNameFilter(tr("Schematics (*.schem)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".schem");
  }
  if (!dlg.exec())
    return false;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return false;

  d->lastdir = dlg.directory().absolutePath();
  
  return saveAs(fns.first());
}

void MainWindow::quitAction() {
  QApplication::quit();
}



void MainWindow::create(Schem const &schem) {
  if (d->scene) {
    d->partlistview->setModel(0);
    d->view->setScene(0);
    delete d->scene;
  }
  d->scene = new Scene(schem);
  connect(d->scene->hoverManager(), &HoverManager::hoverChanged,
          this, &MainWindow::setStatusMessage);
  d->view->setScene(d->scene);
  setWindowTitle(Style::programName());
  d->filename = "";

  d->libview->setLibrary(&d->scene->library());
  
  d->partlistview->setModel(d->scene->partlist());

  connect(d->scene, &Scene::libraryChanged,
	  d->libview, &LibView::rebuild);

  connect(d->scene, &Scene::selectionChanged,
	  this, &MainWindow::selectionToPartList);

  connect(d->partlistview->selectionModel(),
	  &QItemSelectionModel::selectionChanged,
	  this, &MainWindow::selectionFromPartList);

  connect(d->scene, &Scene::libraryChanged,
	  this, &MainWindow::markChanged);
  connect(d->scene, &Scene::circuitChanged,
	  this, &MainWindow::markChanged);
  setStatusMessage("Created new circuit");
}

void MainWindow::load(QString fn) {
  create(FileIO::loadSchematic(fn));

  d->libview->rebuild();
  setWindowTitle(fn);
  d->filename = fn;
  d->lastdir = QFileInfo(fn).dir().absolutePath();
  setStatusMessage(tr("Loaded “%1”").arg(fn));
}

bool MainWindow::saveAs(QString fn) {
  Schem schem = d->scene->schem();
  schem.circuit().renumber(1);
  if (FileIO::saveSchematic(fn, schem)) {
    setWindowTitle(fn);
    d->filename = fn;
    d->unsaved = false;
    setStatusMessage(tr("Saved “%1”").arg(fn));
    return true;
  } else {
    QMessageBox::warning(this, Style::programName(),
			 tr("Could not save schematic as “")
			 + fn + tr("”"),
			 QMessageBox::Ok);
    return false;
  }
}

void MainWindow::markChanged() {
  d->unsaved = true;
  QString fn = d->filename;
  if (fn.isEmpty())
    fn = "(Untitled)";
  setWindowTitle(fn + " *");
}

void MainWindow::zoomIn() {
  d->view->scale(1.5, 1.5);
}

void MainWindow::zoomOut() {
  d->view->scale(1/1.5, 1/1.5);
}

void MainWindow::setStatusMessage(QString msg) {
  statusBar()->showMessage(msg);
}

void MainWindow::aboutAction() {
  QString me = "<b>" + Style::programName() + "</b>";
  QString vsn = Style::versionName();
  QMessageBox::about(0, "About " + me,
                     me + " " + vsn
                     + "<p>" + "(C) 2018–2019 Daniel A. Wagenaar\n"
                     + "<p>" + me + " is a program for electronic circuit layout with high-quality SVG export. More information is available at <a href=\"http://www.danielwagenaar.net/cschem\">www.danielwagenaar.net/cschem</a>.\n"
                     + "<p>" + me + " is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
                     + "<p>" + me + " is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
                     + "<p>" + "You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">www.gnu.org/licenses/gpl-3.0.en.html</a>.");
}

void MainWindow::copyAction() {
  d->scene->copyToClipboard();
}

void MainWindow::cutAction() {
  d->scene->copyToClipboard(true);
}

void MainWindow::pasteAction() {
  d->scene->pasteFromClipboard();
}

void MainWindow::undoAction() {
  d->scene->undo();
}

void MainWindow::redoAction() {
  d->scene->redo();
}

void MainWindow::removeDanglingAction() {
  d->scene->removeDangling();
}

void MainWindow::plonk(QString sym) {
  d->scene->plonk(sym, d->view->mapToScene(QPoint(d->view->width()/2,
                                                  d->view->height()/2)));
}

void MainWindow::closeEvent(QCloseEvent *e) {
  if (d->unsaved) {
    auto ret
      = QMessageBox::warning(this, Style::programName(),
			     tr("The schematic has been modified.\n"
				"Do you want to save your changes?"),
			     QMessageBox::Save
			     | QMessageBox::Discard
			     | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
      if (saveAction())
	e->accept();
      else
	e->ignore();
      break;
    case QMessageBox::Cancel:
      e->ignore();
      break;
    case QMessageBox::Discard:
      e->accept();
      break;
    default:
      e->accept();
      break;
    }
  } else {
    e->accept();
  }
}

void MainWindow::resizeEvent(QResizeEvent *e) {
  QMainWindow::resizeEvent(e);
  d->partlistview->resetWidth();
}

void MainWindow::flipAction() {
  d->scene->flipx();
}

void MainWindow::rotateCCWAction() {
  d->scene->rotate(1);
}

void MainWindow::rotateCWAction() {
  d->scene->rotate(-1);
}

void MainWindow::exportCircuitAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = Paths::defaultLocation();
  
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Export schematic as svg…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("svg");
  dlg.setDirectory(d->lastdir);
  //dlg.setFilter(QDir::AllFiles);
  dlg.setNameFilter(tr("Scalable vector graphics (*.svg)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".svg");
  }
  if (!dlg.exec())
    return;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return;

  d->lastdir = dlg.directory().absolutePath();

  QString fn = fns.first();

  SvgExporter xp(d->scene->schem());
  if (!xp.exportSvg(fn)) {
    qDebug() << "Failed to export svg";
  }
}

void MainWindow::exportPartListAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = Paths::defaultLocation();
  
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Export parts list as csv…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("schem");
  dlg.setDirectory(d->lastdir);
  //dlg.setFilter(QDir::AllFiles);
  dlg.setNameFilter(tr("Comma-separated value file (*.csv)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".csv");
  }
  if (!dlg.exec())
    return;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return;

  d->lastdir = dlg.directory().absolutePath();

  QString fn = fns.first();

  QFile file(fn);
  if (file.open(QFile::WriteOnly)) {
    QTextStream ts(&file);
    for (QStringList  line: d->partlistview->model()->asTable()) {
      for (QString &str: line) {
	str.replace("\"", tr("”"));
	str = "\"" + str + "\"";
      }
      ts << line.join(",");
      ts << "\n";
    }
  } else {
    qDebug() << "Failed to export parts list";
    // should show error box
  }
}

void MainWindow::circuitToClipboardAction() {
  QRectF rr = d->scene->itemsBoundingRect().adjusted(-2, -2, 2, 2);
  QSizeF ss = rr.size();
  QSizeF sdest = 2*ss;
  QRectF rdest = QRectF(QPointF(), sdest);
  QPixmap img(sdest.toSize());
  img.fill();
  { QPainter ptr(&img);
    d->scene->unhover();
    d->scene->render(&ptr, rdest, rr);
    d->scene->rehover();
  }
  QApplication::clipboard()->setPixmap(img);
}
 
void MainWindow::partListToClipboardAction() {
  QList<QStringList> symbols = d->partlistview->model()->asTable();
  QString text;
  for (QStringList const &line: symbols)
    text += line.join("\t") + "\n";
  QApplication::clipboard()->setText(text);
  
} 

void MainWindow::selectionToPartList() {
  qDebug() << "selection to partlist" << d->scene->selectedElements();
  d->recursedepth ++;
  if (d->recursedepth == 1)
    d->partlistview->selectElements(d->scene->selectedElements());
  d->recursedepth --;
}

void MainWindow::selectionFromPartList() {
  d->recursedepth ++;
  QSet<int> sel = d->partlistview->selectedElements();
  if (d->recursedepth == 1)
    d->scene->selectElements(sel);
  d->recursedepth --;
}

void MainWindow::resolveConflictsAction() {
  ContainerConflicts cc(d->scene->circuit(), d->scene->library());
  NumberConflicts nc(d->scene->circuit());
  if (cc.conflicts().isEmpty() && nc.conflictingNames().isEmpty()) {
    QMessageBox::information(this, Style::programName(),
			     "No part numbering conflicts found.");
    return;
  }

  if (!cc.conflicts().isEmpty()) {
    QMessageBox
      ::information(this, Style::programName(),
		    "The following part numbering conflicts were found:\n\n"
		    + cc.conflicts().join(";\n") + ".\n\n"
		    "Unfortunately, this cannot be automatically resolved.");
    return;
  }

  if (!nc.canResolve()) {
    QMessageBox
      ::information(this, Style::programName(),
		    "The following part numbers occur more than once:\n\n"
		    + nc.conflictingNames().join(", ") + "\n\n"
		    "Unfortunately, this cannot be automatically resolved.");
    // Really, of course, we should report exactly what must be manually
    // resolved.
    return;
  }

  auto b = QMessageBox
    ::question(this, Style::programName(),
	       "The following part numbers occur more than once:\n\n"
	       + nc.conflictingNames().join(", ") + "\n\n"
	       "Would you like to automatically renumber elements as needed"
	       " to resolve the conflict?",
	       QMessageBox::Yes | QMessageBox::No);
  if (b==QMessageBox::Yes) {
    d->scene->renumber(nc.newNames());
  }
}

 
