/****************************************************************************
** Form interface generated from reading ui file 'cr-recorder-dialog.ui'
**
** Created: Thu Aug 8 17:38:35 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef CRRECORDERDIALOG_H
#define CRRECORDERDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class crRecorderDialog : public QDialog
{ 
    Q_OBJECT

public:
    crRecorderDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~crRecorderDialog();

    QLabel* titleLabel;
    QLabel* currentFrameLabel;
    QPushButton* exitButton;
    QPushButton* saveEnabledButton;
    QLabel* framenumLabel;
    QSpinBox* frameNumSpinner;
    QLabel* strideLabel;
    QSpinBox* strideSpinner;
    QLabel* specLabel;
    QLineEdit* specLineEdit;

public slots:
    virtual void changeFrameNum(int);
    virtual void changeFrameSpec(const QString&);
    virtual void changeFrameStride(int);
    virtual void setRecording(bool);

protected:
    QVBoxLayout* crRecorderDialogLayout;
    QHBoxLayout* titleLayout;
    QHBoxLayout* controlHLayout;
    QVBoxLayout* paramVLayout;
    QHBoxLayout* spinnerHLayout;
    QHBoxLayout* specEntryHLayout;
    bool event( QEvent* );
};

#endif // CRRECORDERDIALOG_H
