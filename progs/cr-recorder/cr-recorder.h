#ifndef cr_recorder_h
#define cr_recorder_h

#include <qobject.h>

class QNetRecv : public QObject
{
	Q_OBJECT
	public:
		QNetRecv( QObject* parent, const char* name ) ;
	protected:
		virtual void timerEvent( QTimerEvent* tev ) ;
} ;

#endif // cr_recorder_h

