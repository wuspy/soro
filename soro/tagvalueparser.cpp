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

bool TagValueParser::contains(const QString &key) {
    return _contents->contains(key);
}

QString TagValueParser::value(const QString &key) {
    return _contents->value(key, "");
}

bool TagValueParser::valueAsInt(const QString &key, int* value) {
    bool success;
    *value = this->value(key).toInt(&success);
    return success;
}

bool TagValueParser::valueAsBool(const QString &key, bool* value) {
    QString rawValue = this->value(key).toLower();
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

bool TagValueParser::valueAsIP(const QString &key, QHostAddress* value, bool allowV6) {
    QString rawValue = this->value(key);
    if (QRegExp(IPV4_REGEX).exactMatch(rawValue) || (allowV6 && QRegExp(IPV6_REGEX).exactMatch(rawValue))) {
            value->setAddress(rawValue);
            return true;
    }
    return false;
}

int TagValueParser::count() {
    return _contents->size();
}

bool TagValueParser::remove(const QString &key) {
    return _contents->remove(key) > 0;
}

QList<QString> TagValueParser::keys() {
    return _contents->keys();
}

TagValueParser::~TagValueParser() {
    delete _contents;
}
