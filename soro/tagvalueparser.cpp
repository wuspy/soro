#include "tagvalueparser.h"

TagValueParser::TagValueParser() {
    _contents = new QMap<QString, QString>();
}

bool TagValueParser::load(QTextStream &stream) {
    _contents->clear();

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(COMMENT)) {
            continue;
        }
        int sepIndex = line.indexOf(DELIM);
        if (sepIndex < 0) return false;
        QString tag = line.mid(0, sepIndex).trimmed().toLower();
        QString value = line.mid(sepIndex + 1).trimmed();
        if (value.contains(DELIM)) return false;
        _contents->insert(tag, value);
    }
    return true;
}

bool TagValueParser::contains(const QString &tag) {
    return _contents->contains(tag);
}

QString TagValueParser::value(const QString &tag) {
    return _contents->value(tag, "");
}

bool TagValueParser::valueAsInt(const QString &tag, int* value) {
    bool success;
    *value = this->value(tag).toInt(&success);
    return success;
}

bool TagValueParser::valueAsBool(const QString &tag, bool* value) {
    QString rawValue = this->value(tag).toLower();
    if (rawValue == "true" || rawValue == "1") {
        *value = true;
        return true;
    }
    else if (rawValue == "false" || rawValue == "0") {
        *value = false;
        return true;
    }
    return false;
}

bool TagValueParser::valueAsIP(const QString &tag, QHostAddress* value, bool allowV6) {
    QString rawValue = this->value(tag);
    if (QRegExp(IPV4_REGEX).exactMatch(rawValue) || (allowV6 && QRegExp(IPV6_REGEX).exactMatch(rawValue))) {
            value->setAddress(rawValue);
            return true;
    }
    return false;
}

int TagValueParser::count() {
    return _contents->size();
}

bool TagValueParser::remove(const QString &tag) {
    return _contents->remove(tag) > 0;
}

QList<QString> TagValueParser::tags() {
    return _contents->keys();
}

TagValueParser::~TagValueParser() {
    delete _contents;
}
