#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QClipboard>
#include <QUrl>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <string>
#include <Windows.h>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
  m_isWorking( false ),
  m_title( "Chain copy" ),
  m_autoCheck( true ),
  m_manualCheck( true ),
  m_pause( false )
{
	ui->setupUi(this);

  connect( ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
           SLOT(onItemClicked(QTreeWidgetItem*,int)) );
  connect( ui->m_pCopyButton, SIGNAL(clicked()), SLOT(onCopyButtonClicked()) );
  connect( ui->m_pCheckClipboardButton, SIGNAL(clicked()), SLOT(onCheckClipBoardButtonClicked()) );
  connect( ui->m_pClearSourcesButton, SIGNAL(clicked()), SLOT(onClearSourcesButtonClicked()) );
  connect( ui->m_pClearDestinationsButton, SIGNAL(clicked()), SLOT(onClearDestinationsButtonClicked()) );

  connect( ui->m_pDestinationsListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                                                 SLOT(onDstItemClicked(QListWidgetItem*)) );

  connect( ui->m_pPauseButton, SIGNAL(clicked()), SLOT(onPauseClicked()) );

  connect( ui->m_pAutoCheckCheckBox, SIGNAL(clicked()), SLOT(onAutoCheckCheckBoxClicked()) );

  ui->m_pProgressBar->setVisible( false );
  ui->m_pPauseButton->setEnabled( false );

  QClipboard *pClip = QApplication::clipboard();

  connect( pClip, SIGNAL(dataChanged()), SLOT(onDataChanged()) );
}

MainWindow::~MainWindow()
{
	delete ui;
}

int copyFile( const std::wstring &_title, const std::wstring &_path,
							 const std::wstring &_dst, DWORD _op )
{
	SHFILEOPSTRUCT op;
	ZeroMemory( &op, sizeof( SHFILEOPSTRUCT ) );

	op.lpszProgressTitle = _title.c_str();
	op.hwnd = nullptr;

	std::vector< wchar_t > from( _path.length() + 1 );
	memcpy( &from.front(), &_path.front(), _path.length() * 2 );
	from.push_back( L'\0' );

	op.pFrom = &from.front();

	std::vector< wchar_t > to( _dst.length() + 1 );
	memcpy( &to.front(), &_dst.front(), _dst.length() * 2 );
	to.push_back( L'\0' );

	op.pTo = &to.front();

	op.wFunc = _op;
	op.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

	return ::SHFileOperationW( &op );
}

void MainWindow::onCopyButtonClicked()
{
  // copy

	const int items = ui->treeWidget->topLevelItemCount();

  if ( !items )
  {
    QMessageBox::information( this, m_title, "No items?" );

    return;
  }

	m_isWorking = true;
	bool abortRes( false );

  ui->m_pClearDestinationsButton->setEnabled( false );
  ui->m_pClearSourcesButton->setEnabled( false );
  ui->m_pCheckClipboardButton->setEnabled( false );
  ui->m_pDestinationsListWidget->setEnabled( false );
  ui->m_pCopyButton->setEnabled( false );
  ui->treeWidget->setEnabled( false );
  ui->m_pPauseButton->setEnabled( true );
  ui->groupBox_2->setEnabled( false );
  ui->m_pAutoCheckCheckBox->setEnabled( false );

  ui->m_pIdleRadioButton->setChecked( true );

  ui->m_pProgressBar->setVisible( true );
  ui->m_pProgressBar->setMaximum( items );

  for ( int i( 0 ); i < items; ++i )
	{
		QTreeWidgetItem *pItem( ui->treeWidget->topLevelItem( i ) );

		if ( abortRes )
		{
			pItem->setText( 2, "Aborted" );
			continue;
		}

		const QString status = pItem->text( 2 );

		if ( status == "Not copied" ||
				 status == "Aborted" ||
				 status == "Destination empty" ||
				 status == "Src not found" )
		{
			std::wstring src = pItem->text( 0 ).toStdWString();

			if ( !boost::filesystem::exists( src ) )
			{
				pItem->setText( 2, "Src not found" );

				continue;
			}

      while ( m_pause )
      {
        ::Sleep( 250 );
      }

			std::wstring dst = pItem->text( 1 ).toStdWString();

			if ( status == "Aborted" )
			{
				if ( QMessageBox::question( this, "Question", "Copy again?",
																	QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
					continue;
			}

			if ( dst.empty() )
			{
				pItem->setText( 2, "Destination empty" );

				continue;
			}

      while ( m_pause )
      {
        ::Sleep( 250 );
      }

			std::wstring title = std::wstring( L"Chain copy " ) + ( boost::filesystem::wpath( src ) ).filename().wstring();

			int result = copyFile( title, src, dst, FO_COPY );

      while ( m_pause )
      {
        ::Sleep( 250 );
      }

      setWindowTitle( m_title + " - Copied " + QString::number( i + 1 ) +
                      " from " + QString::number( items ) );

			switch ( result )
			{
			case 0:
				// ok
				pItem->setText( 2, "Ok" );
				break;
			case 1223:
				// aborted
				pItem->setText( 2, "Aborted" );

				if ( QMessageBox::question( this, "Question", "Abort rest?",
                                  QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
				{
					abortRes = true;

					continue;
				}

				break;
			default:
				pItem->setText( 2, "Error: " + QString::number( result ) );
			}
		}

    qApp->processEvents();

    ui->m_pProgressBar->setValue( i+1 );
	}

	// aborted: 1223
	// finished: 0

	m_isWorking = false;

  ui->m_pClearDestinationsButton->setEnabled( true );
  ui->m_pClearSourcesButton->setEnabled( true );
  ui->m_pCheckClipboardButton->setEnabled( true );
  ui->m_pDestinationsListWidget->setEnabled( true );
  ui->m_pCopyButton->setEnabled( true );
  ui->treeWidget->setEnabled( true );
  ui->m_pPauseButton->setEnabled( false );
  ui->m_pPauseButton->setText( "Pause" );
  ui->groupBox_2->setEnabled( true );
  ui->m_pAutoCheckCheckBox->setEnabled( true );

  ui->m_pProgressBar->setVisible( false );

  setWindowTitle( m_title + " - Done" );

  if ( ui->checkBox->isChecked() )
    close();
}

void MainWindow::onClearSourcesButtonClicked()
{
  ui->treeWidget->clear();
}

void MainWindow::onClearDestinationsButtonClicked()
{
  ui->m_pDestinationsListWidget->clear();
}

void MainWindow::onDstItemClicked( QListWidgetItem *_pItem )
{
  if ( m_isWorking )
    return;

  const QString selected( _pItem->text() );

  foreach ( QTreeWidgetItem *pSrcItem, ui->treeWidget->selectedItems() )
  {
    pSrcItem->setText( 1, selected );
    pSrcItem->setText( 2, "Not copied" );
  }

  ui->m_pDestinationsListWidget->blockSignals( true );
  for ( int i( 0 ); i < ui->m_pDestinationsListWidget->count(); ++i )
  {
    QListWidgetItem *pItem( ui->m_pDestinationsListWidget->item( i ) );

    if ( pItem->text() == selected )
      continue;

    pItem->setCheckState( Qt::Unchecked );
  }
  ui->m_pDestinationsListWidget->blockSignals( false );
}

void MainWindow::onPauseClicked()
{
  if ( m_isWorking )
  {
    m_pause = !m_pause;

    if ( m_pause )
    {
      ui->m_pPauseButton->setText( "Resume" );
    }
    else
    {
      ui->m_pPauseButton->setText( "Pause" );
    }
  }
}

void MainWindow::onCheckClipBoardButtonClicked()
{
  // check clipboard

  bool collect( ui->m_pCollectSourcesRadioButton->isChecked() ||
            ui->m_pCollectDestinationsRadioButton->isChecked() );

  if ( !collect )
    return;

	QClipboard *pClip = QApplication::clipboard();
	const QMimeData *pMimeData = pClip->mimeData();

  std::vector< std::wstring > items;

	if ( pMimeData->hasUrls() )
	{
		foreach ( QUrl url, pMimeData->urls() )
		{
			boost::filesystem::wpath newItem( url.toLocalFile().toStdWString() );

      std::wstring path( newItem.make_preferred().wstring() );

      if ( boost::filesystem::is_directory( newItem ) )
        path += L"\\";

      items.push_back( path );
		}
	}
  else if ( pMimeData->hasText() )
  {
    std::wstring text = pMimeData->text().toStdWString();

    if ( boost::filesystem::exists( text ) )
    {
      if ( boost::filesystem::is_directory( text ) )
        text += L"\\";

      items.push_back( text );
    }
  }

  if ( ui->m_pCollectSourcesRadioButton->isChecked() )
  {
    for ( size_t idx( 0 ); idx < items.size(); ++idx )
    {
      QTreeWidgetItem *pItem( new QTreeWidgetItem() );

      pItem->setText( 0, QString::fromStdWString( items.at( idx ) ) );
      pItem->setText( 2, "Not copied" );

      ui->treeWidget->addTopLevelItem( pItem );
    }
  }
  else if ( ui->m_pCollectDestinationsRadioButton->isChecked() )
  {
    for ( size_t idx( 0 ); idx < items.size(); ++idx )
    {
      QListWidgetItem *pItem( new QListWidgetItem( QString::fromStdWString( items.at( idx ) ) ) );

      pItem->setCheckState( Qt::Unchecked );

      ui->m_pDestinationsListWidget->addItem( pItem );
    }
  }
}

void MainWindow::onItemClicked(QTreeWidgetItem *_pItem, int _column)
{
  Q_UNUSED( _column )

  if ( m_isWorking )
    return;

  // mark destination

  const QString dst = _pItem->text( 1 );

  for ( int i( 0 ); i < ui->m_pDestinationsListWidget->count(); ++i )
  {
    QListWidgetItem *pItem( ui->m_pDestinationsListWidget->item( i ) );

    if ( pItem->text() == dst )
      pItem->setCheckState( Qt::Checked );
    else
      pItem->setCheckState( Qt::Unchecked );
  }
}

/*
void MainWindow::on_pushButton_4_clicked()
{
  //if ( m_currentItem.empty() )
  //	return;

	const int items = ui->treeWidget->topLevelItemCount();

	for ( int i( 0 ); i < items; ++i )
	{
		QTreeWidgetItem *pItem( ui->treeWidget->topLevelItem( i ) );

    if ( pItem->text( 0 ).toStdWString() == m_currentItem )
		{
			pItem->setText( 2, "Not copied" );

			return;
		}
	}
}
*/

void MainWindow::onAutoCheckCheckBoxClicked()
{
  m_autoCheck = !m_autoCheck;
}

void MainWindow::onDataChanged()
{
  if ( m_autoCheck && !m_isWorking )
  {    
    m_manualCheck = false;
    onCheckClipBoardButtonClicked();
    m_manualCheck = true;
  }
}
/*
void MainWindow::on_pushButton_7_clicked()
{
  // source browse

  QFileDialog dlg( this );

  dlg.setFileMode( QFileDialog::Directory );

  dlg.exec();

  foreach ( QString fileName, dlg.selectedFiles() )
  {
    //ui->lineEdit_2->setText( fileName );
  }
}

void MainWindow::on_pushButton_6_clicked()
{
  // destination browse

  QFileDialog dlg( this );

  dlg.setFileMode( QFileDialog::Directory );

  dlg.exec();

  foreach ( QString fileName, dlg.selectedFiles() )
  {
    //ui->lineEdit->setText( fileName );
  }
}
*/
