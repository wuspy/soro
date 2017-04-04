#ifndef COMMENTCSVSERIES_H
#define COMMENTCSVSERIES_H

#include "libsoro/csvrecorder.h"

namespace Soro {
namespace MissionControl {

class CommentCsvSeries: public QObject, public CsvDataSeries
{
    Q_OBJECT
public:
    CommentCsvSeries(QObject *parent=0);
    QString getSeriesName() const;
    bool shouldKeepOldValues() const;

public slots:
    void onCommentEntered(QString comment);
};

} // namespace MissionControl
} // namespace Soro

#endif // COMMENTCSVSERIES_H
