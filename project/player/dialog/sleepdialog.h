#ifndef SLEEPDIALOG_H
#define SLEEPDIALOG_H

// sleepdialog.h
// 3/13/2012

#include "countdowndialog.h"

class SleepDialog : public CountdownDialog
{
  Q_OBJECT
  typedef SleepDialog Self;
  typedef CountdownDialog Base;

public:
  explicit SleepDialog(QWidget *parent = 0);

protected:
  virtual void run(); ///< \override
};

#endif // SLEEPDIALOG_H