#include <iostream.h>

#include <qspinbox.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include "cr_pack.h"
#include "cr_packfunctions.h"
#include "cr_string.h"
#include "cr_mem.h"

#ifdef Bool
#undef Bool
#endif

#include "recorderdialogimpl.h"

static const char* const pausedPPM[] = { 
"32 48 3 1",
". c None",
"a c #000000",
"# c #eaeaea",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
"................................",
".......######......######.......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
"......aaaaaaa#....aaaaaaa#......",
".......aaaaaa......aaaaaa.......",
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
"................................",
"................................",
"................................",
"................................"};

static const char* const recordingPPM[] = { 
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
 *  Constructs a crRecorderDialogImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
crRecorderDialogImpl::crRecorderDialogImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : crRecorderDialog( parent, name, modal, fl ),
			recordingPixmap( (const char**) recordingPPM ),
			pausedPixmap( (const char**) pausedPPM )
{
	setFrameNum = false ;
	setFrameSpec = false ;
	setFrameStride = false ;
	packContext = 0 ;
	saveEnabledButton->setFocus() ;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
crRecorderDialogImpl::~crRecorderDialogImpl()
{
	// no need to delete child widgets, Qt does it all for us
}

void crRecorderDialogImpl::setPackContext( CRPackContext* pc )
{
	packContext = pc ;
}

void crRecorderDialogImpl::setRecording( bool t )
{
	bool nott = ! t ;
	frameNumSpinner->setEnabled( nott ) ;
	strideSpinner->setEnabled( nott ) ;
	specLineEdit->setEnabled( nott ) ;

	if ( t ) {
		if ( setFrameNum ) {
			cout << "Setting frame number to " << frameNumSpinner->value() << endl ;
			crPackChromiumParameteriCR( GL_SAVEFRAME_FRAMENUM_CR, frameNumSpinner->value() );
		}
		if ( setFrameStride ) {
			cout << "Setting stride to " << strideSpinner->value() << endl ;
			crPackChromiumParameteriCR( GL_SAVEFRAME_STRIDE_CR, strideSpinner->value() );
		}
		if ( setFrameSpec ) {
			const char* fname = specLineEdit->text().latin1() ;
			crPackChromiumParametervCR( GL_SAVEFRAME_FILESPEC_CR, GL_BYTE,
					crStrlen( fname )+1, (void*) fname );
			cout << "Setting file spec to \"" << fname << "\"" << endl ;
		}
		crPackChromiumParameteriCR( GL_SAVEFRAME_ENABLED_CR, 1 );
		saveEnabledButton->setPixmap( recordingPixmap ) ;
		setFrameNum = false ;
		setFrameSpec = false ; 
		setFrameStride = false ; 
	} else {
		crPackChromiumParameteriCR( GL_SAVEFRAME_ENABLED_CR, 0 );
		saveEnabledButton->setPixmap( pausedPixmap ) ;
	}
	crPackSwapBuffers( 0, 0 ) ; // don't care about the window & ctx, since injectorspu intercepts
	packContext->Flush( packContext->flush_arg ) ;
}

void crRecorderDialogImpl::changeFrameNum(int)
{
	setFrameNum = true ;
}

void crRecorderDialogImpl::changeFrameSpec(const QString&)
{
	setFrameSpec = true ;
}

void crRecorderDialogImpl::changeFrameStride(int)
{
	setFrameStride = true ;
}


