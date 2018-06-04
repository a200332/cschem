// DimSpinner.h

#ifndef DIMSPINNER_H

#define DIMSPINNER_H

#include <QDoubleSpinBox>
#include "data/Dim.h"

class DimSpinner: public QDoubleSpinBox {
public:
  DimSpinner(QWidget *parent=0);
  Dim value() const;
  bool isMetric() const;
  bool isInch() const;
public slots:
  void setValue(Dim);
  void setMetric(bool b=true);
  void setInch();
  void setStep(Dim);
private:
  bool metric_;
  Dim step;
};

#endif
