#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <boost/filesystem.hpp>
#include <set>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:

  void onCheckClipBoardButtonClicked();

	void onItemClicked(QTreeWidgetItem*,int);

  void onAutoCheckCheckBoxClicked();

  void onDataChanged();

  void onCopyButtonClicked();

  void onClearSourcesButtonClicked();
  void onClearDestinationsButtonClicked();

  void onDstItemClicked(QListWidgetItem*);

  void onPauseClicked();

private:
  // Window title.
  const QString m_title;

	Ui::MainWindow *ui;

	bool m_isWorking;
  bool m_autoCheck;
  bool m_manualCheck;
  bool m_pause;
};

#endif // MAINWINDOW_H
