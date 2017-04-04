#include "commentcsvseries.h"

namespace Soro {
namespace MissionControl {

CommentCsvSeries::CommentCsvSeries(QObject *parent) : QObject(parent) { }

QString CommentCsvSeries::getSeriesName() const {
    return "Comments";
}

void CommentCsvSeries::onCommentEntered(QString comment) {
    update(QVariant(comment.remove(',')));
}

bool CommentCsvSeries::shouldKeepOldValues() const {
    return false;
}

} // namespace MissionControl
} // namespace Soro
