
#include <QtDebug>

#include <QApplication>
#include <QBoxLayout>
#include <QDialog>
#include <QDockWidget>
#include <QDir>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QUrl>
#include <QWheelEvent>

#include <KAboutData>
#include <KMessageBox>
#include <KIOFileWidgets/KDirSortFilterProxyModel>
#include <KIOFileWidgets/KFilePlacesModel>
#include <KIOFileWidgets/KImageFilePreview>
#include <KIOFileWidgets/KPreviewWidgetBase>
#include <KIOFileWidgets/KUrlNavigator>
#include <KIOWidgets/KDirLister>
#include <KIOWidgets/KDirModel>
#include <KIOWidgets/KFile>
#include <KWidgetsAddons/KSeparator>
#include <KXmlGui/KToolBar>
#include <KXmlGui/KXmlGuiWindow>


/**
 * @brief Tree view of files with zoom signals when using mouse wheel
 */
class ZoomTreeView : public QTreeView {
  Q_OBJECT

  public:
    ZoomTreeView(QWidget *parent = 0)
      : QTreeView(parent){
    }

  signals:
    void zoomInEvent(QPoint position);
    void zoomOutEvent();

  protected:
    void wheelEvent(QWheelEvent *event) override {
      if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        event->ignore();
        return;
      }

      if (event->delta() > 0) {
        emit zoomInEvent(event->pos());
      } else {
        emit zoomOutEvent();
      }
      event->accept();
    }
};


class MainWindow : public KXmlGuiWindow {
  Q_OBJECT

  public:
    MainWindow()
      : KXmlGuiWindow(0),
        sortColumnLogicalIndex(0),
        sortOrderIsAscending(true) {

      // navigator bar (directory path)
      urlNavigator = new KUrlNavigator(new KFilePlacesModel(this), QUrl(), this);
      connect(urlNavigator, SIGNAL(urlChanged(QUrl)), SLOT(changeTreeURL(QUrl)));

      // tree model
      model = new KDirModel(this);
      // proxy model for sorting
      proxyModel = new KDirSortFilterProxyModel(this);
      proxyModel->setSourceModel(model);
      connect(model->dirLister(), SIGNAL(completed()), SLOT(adjustColumnWidth()));

      // tree view
      treeView = new ZoomTreeView(this);
      treeView->setModel(proxyModel);
      treeView->setAnimated(true);
      // treeView->header()->setClickable(true);
      treeView->setMouseTracking(true);
      // selection
      QItemSelectionModel *itemSelectionModel = new QItemSelectionModel(proxyModel);
      treeView->setSelectionModel(itemSelectionModel);

      connect(treeView, SIGNAL(entered(QModelIndex)), SLOT(setPreviewItem(QModelIndex)));
      connect(treeView, SIGNAL(activated(QModelIndex)), SLOT(setNavigatorURL(QModelIndex)));
      connect(treeView->header(), SIGNAL(sectionClicked(int)), SLOT(setModelSorting(int)));

      // file preview
      filePreview = new KImageFilePreview(this);

      // layout
      QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

      QWidget *browserWidget = new QWidget(this);
      QVBoxLayout *browserLayout = new QVBoxLayout(this);
      browserLayout->addWidget(urlNavigator);
      browserLayout->addWidget(treeView);
      browserWidget->setLayout(browserLayout);
      splitter->addWidget(browserWidget);

      QFrame *sideFrame = new QFrame(this);
      sideFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
      sideFrame->setWindowTitle("Information");
      QVBoxLayout *frameLayout = new QVBoxLayout(sideFrame);
      frameLayout->addWidget(filePreview);
      splitter->addWidget(sideFrame);

      setCentralWidget(splitter);

      // set window dimensions
      int height = 800;
      int width = 1.618 * height;
      resize(width, height);

      // kde magic
      setupGUI();

      // initialize
      const QUrl startUrl = QUrl::fromUserInput(QDir::homePath());
      urlNavigator->setLocationUrl(startUrl);
    }

  private slots:
    /**
     * @brief Change URL in dir model
     * @param url
     */
    void changeTreeURL(QUrl url) {
      qDebug() << "changed url to: " << url;
      model->dirLister()->openUrl(url);
      treeView->setRootIndex(model->indexForUrl(url));
    }

    void adjustColumnWidth() {
      treeView->resizeColumnToContents(0);
    }

    /**
     * @brief Set navigator URL to item URL of model index
     * @param index
     */
    void setNavigatorURL(QModelIndex index) {
      qDebug() << "item activated";
      KFileItem item = model->itemForIndex(proxyModel->mapToSource(index));
      if (item.isDir()) {
        urlNavigator->setLocationUrl(item.url());
      }
    }

    void setModelSorting(int logicalIndex) {
      qDebug() << "set model sorting, sort index: " << logicalIndex;
      if (sortColumnLogicalIndex == logicalIndex) {
        sortOrderIsAscending = !sortOrderIsAscending;
      } else {
        sortColumnLogicalIndex = logicalIndex;
        sortOrderIsAscending = true;
      }
      Qt::SortOrder order =
          sortOrderIsAscending ? Qt::AscendingOrder : Qt::DescendingOrder;
      model->sort(logicalIndex, order);
    }

    void changedSelection(QItemSelection selected,
                          QItemSelection deselected) {
      qDebug() << "changed selection to " << selected.indexes();
      qDebug() << "selection size: " << selected.size();
    }

    void setPreviewItem(QModelIndex index) {
      KFileItem item = model->itemForIndex(proxyModel->mapToSource(index));
      QUrl url = item.url();
      filePreview->showPreview(url);
    }

    void setURL(QPoint position) {
      QModelIndex itemIndex = treeView->indexAt(position);
      if (itemIndex.isValid()) {
        setNavigatorURL(itemIndex);
      }
    }

    void goUp() {
      urlNavigator->goUp();
    }

  private:
    void resizeEvent(QResizeEvent *event) override {
      // TODO: use event!?
      qDebug() << "widget width " << treeView->width() << " c0 "
               << treeView->columnWidth(0) << " c1 " << treeView->columnWidth(1)
               << " c2 " << treeView->columnWidth(2);
      treeView->resizeColumnToContents(0);
    }

    KUrlNavigator *urlNavigator;
    KDirModel *model;
    KDirSortFilterProxyModel *proxyModel;
    QTreeView *treeView;
    KImageFilePreview *filePreview;
    int sortColumnLogicalIndex;
    bool sortOrderIsAscending;
};

int main(int argc, char *argv[]) {
  qDebug() << "Hello, world! QT version: " << qVersion();

  QApplication app(argc, argv);

  // KMainWindows must be "new"
  MainWindow *window = new MainWindow();
  window->show();

  return app.exec();
}

// needed by the Q_OBJECT macro
#include "main.moc"
