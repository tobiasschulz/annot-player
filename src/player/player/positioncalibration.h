#ifndef POSITIONCALIBRATION_H
#define POSITIONCALIBRATION_H

// positioncalibration.h
// 4/8/2012

#include <QtGui/QWidget>

QT_FORWARD_DECLARE_CLASS(QPainter)

class DataManager;
class SignalHub;
class Player;

class PositionCalibration : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(PositionCalibration)
  typedef PositionCalibration Self;
  typedef QWidget Base;

public:
  PositionCalibration(SignalHub *hub, Player *player, QWidget *parent = nullptr);

signals:
  void visibleChanged(bool visible);

public:
  bool needsDisplay() const;

public slots:
  void invalidatePaint() { if (isVisible()) repaint(); }

  void updateVisible();

  // - Events -
public slots:
  void setVisible(bool visible) override;
protected:
  void paintEvent(QPaintEvent *event) override;

  // - Paint -
protected:
  void paintCoordinate(QPainter &painter, const QRect &view);

private:
  SignalHub *hub_;
  Player *player_;
};

#endif // POSITIONCALIBRATION_H
