#include "commentseries.h"

namespace Soro {
namespace MissionControl {

CommentSeries::CommentSeries(QObject *parent) : QObject(parent) { }

QString CommentSeries::getSeriesName() const {
    return "Comments";
}

void CommentSeries::onCommentEntered(QString comment) {
    update(QVariant(comment));
}

} // namespace MissionControl
} // namespace Soro
