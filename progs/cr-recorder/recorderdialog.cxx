/****************************************************************************
** Form implementation generated from reading ui file 'cr-recorder-dialog.ui'
**
** Created: Thu Aug 8 17:38:36 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "recorderdialog.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const image0_data[] = { 
"32 32 3 1",
". c None",
"# c #000000",
"a c #757575",
"................................",
"................................",
"................................",
"...............###..............",
"...............###..............",
"...............###..............",
".........#.....###.....#........",
"........###....###....###.......",
".......#####...###...#####......",
"......a####a...###...a####a.....",
"......#####....###....#####.....",
".....#####.....###.....#####....",
".....####......###......####....",
"....a####......###......####a...",
"....####.......###.......####...",
"....####.......###.......####...",
"....####.......###.......####...",
"....####.......###.......####...",
"....####.......###.......####...",
"....a####......###......####a...",
".....####...............####....",
".....#####.............#####....",
"......#####...........#####.....",
"......a#####.........#####a.....",
".......#######a...a#######......",
"........#################.......",
".........a#############a........",
"...........###########..........",
".............a#####a............",
"................................",
"................................",
"................................"};

static const char* const image1_data[] = { 
"32 48 2 1",
". c None",
"# c #ff0000",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
".............#######............",
"...........##########...........",
"..........#############.........",
".........##############.........",
".........###############........",
"........################........",
"........################........",
"........#################.......",
"........#################.......",
"........################........",
"........################........",
".........###############........",
".........##############.........",
"..........#############.........",
"...........##########...........",
".............#######............",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................"};


/* 
 *  Constructs a crRecorderDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
crRecorderDialog::crRecorderDialog( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    if ( !name )
	setName( "crRecorderDialog" );
    resize( 473, 134 ); 
    setCaption( tr( "Cr Recorder Dialog" ) );
    setIconText( tr( "Chromium Recorder" ) );
    crRecorderDialogLayout = new QVBoxLayout( this ); 
    crRecorderDialogLayout->setSpacing( 6 );
    crRecorderDialogLayout->setMargin( 11 );

    titleLayout = new QHBoxLayout; 
    titleLayout->setSpacing( 6 );
    titleLayout->setMargin( 0 );

    titleLabel = new QLabel( this, "titleLabel" );
    titleLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, titleLabel->sizePolicy().hasHeightForWidth() ) );
    QFont titleLabel_font(  titleLabel->font() );
    titleLabel_font.setFamily( "LuciduxSans" );
    titleLabel_font.setPointSize( 24 );
    titleLabel_font.setItalic( TRUE );
    titleLabel->setFont( titleLabel_font ); 
    titleLabel->setText( tr( "Chromium Movie Recorder" ) );
    titleLabel->setTextFormat( QLabel::AutoText );
    titleLayout->addWidget( titleLabel );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    titleLayout->addItem( spacer );

    currentFrameLabel = new QLabel( this, "currentFrameLabel" );
    QFont currentFrameLabel_font(  currentFrameLabel->font() );
    currentFrameLabel_font.setFamily( "Lucida Sans" );
    currentFrameLabel_font.setPointSize( 14 );
    currentFrameLabel->setFont( currentFrameLabel_font ); 
    currentFrameLabel->setText( tr( "0000" ) );
    currentFrameLabel->setAlignment( int( QLabel::AlignBottom | QLabel::AlignRight ) );
    titleLayout->addWidget( currentFrameLabel );
    crRecorderDialogLayout->addLayout( titleLayout );

    controlHLayout = new QHBoxLayout; 
    controlHLayout->setSpacing( 6 );
    controlHLayout->setMargin( 0 );

    exitButton = new QPushButton( this, "exitButton" );
    exitButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, exitButton->sizePolicy().hasHeightForWidth() ) );
    exitButton->setMaximumSize( QSize( 64, 64 ) );
    QFont exitButton_font(  exitButton->font() );
    exitButton_font.setFamily( "Lucida Sans" );
    exitButton_font.setPointSize( 14 );
    exitButton->setFont( exitButton_font ); 
    exitButton->setText( tr( "" ) );
    exitButton->setPixmap( image0 );
    exitButton->setFlat( TRUE );
    QToolTip::add(  exitButton, tr( "A red dot indicates the recorder is active. Two black bars indicate the recorder is paused." ) );
    controlHLayout->addWidget( exitButton );

    saveEnabledButton = new QPushButton( this, "saveEnabledButton" );
    saveEnabledButton->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)1, saveEnabledButton->sizePolicy().hasHeightForWidth() ) );
    saveEnabledButton->setMinimumSize( QSize( 48, 64 ) );
    saveEnabledButton->setMaximumSize( QSize( 64, 64 ) );
    QFont saveEnabledButton_font(  saveEnabledButton->font() );
    saveEnabledButton_font.setFamily( "Lucida Sans" );
    saveEnabledButton_font.setPointSize( 14 );
    saveEnabledButton->setFont( saveEnabledButton_font ); 
    saveEnabledButton->setText( tr( "" ) );
    saveEnabledButton->setPixmap( image1 );
    saveEnabledButton->setToggleButton( TRUE );
    saveEnabledButton->setOn( TRUE );
    saveEnabledButton->setToggleButton( TRUE );
    saveEnabledButton->setOn( TRUE );
    saveEnabledButton->setFlat( TRUE );
    QToolTip::add(  saveEnabledButton, tr( "A red dot indicates the recorder is active. Two black bars indicate the recorder is paused." ) );
    controlHLayout->addWidget( saveEnabledButton );

    paramVLayout = new QVBoxLayout; 
    paramVLayout->setSpacing( 6 );
    paramVLayout->setMargin( 0 );

    spinnerHLayout = new QHBoxLayout; 
    spinnerHLayout->setSpacing( 6 );
    spinnerHLayout->setMargin( 0 );

    framenumLabel = new QLabel( this, "framenumLabel" );
    QFont framenumLabel_font(  framenumLabel->font() );
    framenumLabel_font.setFamily( "Lucida Sans" );
    framenumLabel_font.setPointSize( 14 );
    framenumLabel->setFont( framenumLabel_font ); 
    framenumLabel->setText( tr( "Frame number" ) );
    spinnerHLayout->addWidget( framenumLabel );

    frameNumSpinner = new QSpinBox( this, "frameNumSpinner" );
    frameNumSpinner->setEnabled( FALSE );
    QFont frameNumSpinner_font(  frameNumSpinner->font() );
    frameNumSpinner_font.setFamily( "Lucida Sans" );
    frameNumSpinner_font.setPointSize( 14 );
    frameNumSpinner->setFont( frameNumSpinner_font ); 
    frameNumSpinner->setMaxValue( 999999999 );
    QToolTip::add(  frameNumSpinner, tr( "The frame number may be set to a new starting value when the recorder is paused." ) );
    spinnerHLayout->addWidget( frameNumSpinner );

    strideLabel = new QLabel( this, "strideLabel" );
    QFont strideLabel_font(  strideLabel->font() );
    strideLabel_font.setFamily( "Lucida Sans" );
    strideLabel_font.setPointSize( 14 );
    strideLabel->setFont( strideLabel_font ); 
    strideLabel->setText( tr( "Stride" ) );
    spinnerHLayout->addWidget( strideLabel );

    strideSpinner = new QSpinBox( this, "strideSpinner" );
    strideSpinner->setEnabled( FALSE );
    QFont strideSpinner_font(  strideSpinner->font() );
    strideSpinner_font.setFamily( "Lucida Sans" );
    strideSpinner_font.setPointSize( 14 );
    strideSpinner->setFont( strideSpinner_font ); 
    strideSpinner->setMaxValue( 100 );
    strideSpinner->setMinValue( 1 );
    strideSpinner->setValue( 1 );
    QToolTip::add(  strideSpinner, tr( "A stride of <i>n</i> means every <i>n</i>th frame is saved." ) );
    spinnerHLayout->addWidget( strideSpinner );
    paramVLayout->addLayout( spinnerHLayout );

    specEntryHLayout = new QHBoxLayout; 
    specEntryHLayout->setSpacing( 6 );
    specEntryHLayout->setMargin( 0 );

    specLabel = new QLabel( this, "specLabel" );
    QFont specLabel_font(  specLabel->font() );
    specLabel_font.setFamily( "Lucida Sans" );
    specLabel_font.setPointSize( 14 );
    specLabel->setFont( specLabel_font ); 
    specLabel->setText( tr( "Frame filename spec" ) );
    specEntryHLayout->addWidget( specLabel );

    specLineEdit = new QLineEdit( this, "specLineEdit" );
    specLineEdit->setEnabled( FALSE );
    QFont specLineEdit_font(  specLineEdit->font() );
    specLineEdit_font.setFamily( "Lucida Sans" );
    specLineEdit_font.setPointSize( 14 );
    specLineEdit->setFont( specLineEdit_font ); 
    specLineEdit->setText( tr( "" ) );
    QToolTip::add(  specLineEdit, tr( "The file specification describes where movie frames will be saved." ) );
    specEntryHLayout->addWidget( specLineEdit );
    paramVLayout->addLayout( specEntryHLayout );
    controlHLayout->addLayout( paramVLayout );
    crRecorderDialogLayout->addLayout( controlHLayout );

    // signals and slots connections
    connect( saveEnabledButton, SIGNAL( toggled(bool) ), this, SLOT( setRecording(bool) ) );
    connect( exitButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( frameNumSpinner, SIGNAL( valueChanged(int) ), this, SLOT( changeFrameNum(int) ) );
    connect( strideSpinner, SIGNAL( valueChanged(int) ), this, SLOT( changeFrameStride(int) ) );
    connect( strideSpinner, SIGNAL( valueChanged(int) ), this, SLOT( changeFrameStride(int) ) );
    connect( specLineEdit, SIGNAL( textChanged(const QString&) ), this, SLOT( changeFrameSpec(const QString&) ) );

    // tab order
    setTabOrder( saveEnabledButton, frameNumSpinner );
    setTabOrder( frameNumSpinner, strideSpinner );
    setTabOrder( strideSpinner, specLineEdit );
    setTabOrder( specLineEdit, exitButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
crRecorderDialog::~crRecorderDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*  
 *  Main event handler. Reimplemented to handle application
 *  font changes
 */
bool crRecorderDialog::event( QEvent* ev )
{
    bool ret = QDialog::event( ev ); 
    if ( ev->type() == QEvent::ApplicationFontChange ) {
	QFont titleLabel_font(  titleLabel->font() );
	titleLabel_font.setFamily( "LuciduxSans" );
	titleLabel_font.setPointSize( 24 );
	titleLabel_font.setItalic( TRUE );
	titleLabel->setFont( titleLabel_font ); 
	QFont currentFrameLabel_font(  currentFrameLabel->font() );
	currentFrameLabel_font.setFamily( "Lucida Sans" );
	currentFrameLabel_font.setPointSize( 14 );
	currentFrameLabel->setFont( currentFrameLabel_font ); 
	QFont exitButton_font(  exitButton->font() );
	exitButton_font.setFamily( "Lucida Sans" );
	exitButton_font.setPointSize( 14 );
	exitButton->setFont( exitButton_font ); 
	QFont saveEnabledButton_font(  saveEnabledButton->font() );
	saveEnabledButton_font.setFamily( "Lucida Sans" );
	saveEnabledButton_font.setPointSize( 14 );
	saveEnabledButton->setFont( saveEnabledButton_font ); 
	QFont framenumLabel_font(  framenumLabel->font() );
	framenumLabel_font.setFamily( "Lucida Sans" );
	framenumLabel_font.setPointSize( 14 );
	framenumLabel->setFont( framenumLabel_font ); 
	QFont frameNumSpinner_font(  frameNumSpinner->font() );
	frameNumSpinner_font.setFamily( "Lucida Sans" );
	frameNumSpinner_font.setPointSize( 14 );
	frameNumSpinner->setFont( frameNumSpinner_font ); 
	QFont strideLabel_font(  strideLabel->font() );
	strideLabel_font.setFamily( "Lucida Sans" );
	strideLabel_font.setPointSize( 14 );
	strideLabel->setFont( strideLabel_font ); 
	QFont strideSpinner_font(  strideSpinner->font() );
	strideSpinner_font.setFamily( "Lucida Sans" );
	strideSpinner_font.setPointSize( 14 );
	strideSpinner->setFont( strideSpinner_font ); 
	QFont specLabel_font(  specLabel->font() );
	specLabel_font.setFamily( "Lucida Sans" );
	specLabel_font.setPointSize( 14 );
	specLabel->setFont( specLabel_font ); 
	QFont specLineEdit_font(  specLineEdit->font() );
	specLineEdit_font.setFamily( "Lucida Sans" );
	specLineEdit_font.setPointSize( 14 );
	specLineEdit->setFont( specLineEdit_font ); 
    }
    return ret;
}

void crRecorderDialog::changeFrameNum(int)
{
    qWarning( "crRecorderDialog::changeFrameNum(int): Not implemented yet!" );
}

void crRecorderDialog::changeFrameSpec(const QString&)
{
    qWarning( "crRecorderDialog::changeFrameSpec(const QString&): Not implemented yet!" );
}

void crRecorderDialog::changeFrameStride(int)
{
    qWarning( "crRecorderDialog::changeFrameStride(int): Not implemented yet!" );
}

void crRecorderDialog::setRecording(bool)
{
    qWarning( "crRecorderDialog::setRecording(bool): Not implemented yet!" );
}

