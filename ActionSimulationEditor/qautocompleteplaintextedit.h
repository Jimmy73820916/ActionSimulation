#pragma once

#include <QPlainTextEdit>
#include "..\ActionSimulationBase\nodalpoint.h"

class QAutoCompletePlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    QAutoCompletePlainTextEdit(QWidget* parent = nullptr);
    ~QAutoCompletePlainTextEdit();

    void append(const QString& word);
    void append(const QStringList& tbl);
private slots:
    void keyPressEvent(QKeyEvent *e) override;
private:
    Jimmy::MaxMatchTree::NodalPoint* findTable_;
};

