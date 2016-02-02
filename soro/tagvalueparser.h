#ifndef TAGVALUEPARSER_H
#define TAGVALUEPARSER_H

//This char/string separates tags from values in a line
#define DELIM '='
//This char/string denotes a line as a comment
#define COMMENT '#'

//lol thanks stackoverflow
#define IPV4_REGEX "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
#define IPV6_REGEX "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"

#include "soro_global.h"

/* A simple class to parse a tag-value formatted configuration file (INI format)
 * from a QTextStream
 *
 * - Call load(QTextStream) to read in a file
 * - Call value(QString&), valueAsInt(QString&), etc to get a tag's value
 */
class SOROSHARED_EXPORT TagValueParser {

private:
    QMap<QString, QString> *_contents;

public:
    TagValueParser();
   ~TagValueParser();

    /* Loads a configuration file from a QTextStream.
     *
     * QTextStreams can be created from any QIODevice, such as
     * QFile and QNetworkResponse.
     *
     * Returns true if the file was read in successfully, false otherwise.
     *
     * As soon as this completes, it is safe to close the original file/network
     * resources as everything will be loaded locally.
     */
    bool load(QTextStream &fileStream);

    /* Gets a tag's value from the last file read in.
     */
    QString value(const QString &tag);

    /* Gets a tag's value from the last file read in as an integer,
     * and returns true if the conversion was successful.
     */
    bool valueAsInt(const QString &tag, int *value);

    /* Gets a tag's value from the last file read in as an boolean,
     * and returns true if the conversion was successful.
     */
    bool valueAsBool(const QString &tag, bool *value);

    /* Gets a tag's value from the last file read in as an QHostAddress,
     * and returns true if the conversion was successful.
     */
    bool valueAsIP(const QString &tag, QHostAddress *value, bool allowV6);

    /* Gets the list of tags from the last file read in.
     */
    QList<QString> tags();

    /* Returns true if the tag was contained in the last file read in, false otherwise.
     */
    bool contains(const QString &tag);

    /* Gets the number of tags/value pairs read in.
     */
    int count();

    /* Removes a loaded tag/value pair. Does not modify the original file.
     */
    bool remove(const QString &tag);
};

#endif // TAGVALUEPARSER_H
