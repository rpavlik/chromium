#ifndef CRRECORDERDIALOGIMPL_H
#define CRRECORDERDIALOGIMPL_H

#include <qpixmap.h>

#include "recorderdialog.h"

struct CRPackContext ;

class crRecorderDialogImpl
	: public crRecorderDialog
{ 
	Q_OBJECT
	protected:
		bool setFrameNum ;
		bool setFrameSpec ;
		bool setFrameStride ;

		int curFrameNum ;

		QPixmap recordingPixmap ;
		QPixmap pausedPixmap ;

		CRPackContext* packContext ;

	public:
		crRecorderDialogImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
		~crRecorderDialogImpl();

		void setPackContext( CRPackContext* ) ;

	public slots:
		virtual void setRecording(bool);
		virtual void changeFrameNum(int);
		virtual void changeFrameSpec(const QString&);
		virtual void changeFrameStride(int);

} ;

#endif // CRRECORDERDIALOGIMPL_H
