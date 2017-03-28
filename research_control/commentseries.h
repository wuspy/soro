#ifndef COMMENTSERIES_H
#define COMMENTSERIES_H

#include "libsoro/csvrecorder.h"

namespace Soro {
namespace MissionControl {

class CommentSeries: public QObject, public CsvDataSeries
{
    Q_OBJECT
public:
    CommentSeries(QObject *parent=0);
    QString getSeriesName() const;

public slots:
    void onCommentEntered(QString comment);
};

} // namespace MissionControl
} // namespace Soro

#endif // COMMENTSERIES_H
