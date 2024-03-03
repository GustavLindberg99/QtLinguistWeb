#ifndef TSFILE_H
#define TSFILE_H

#include <QDomElement>
#include <QFile>
#include <QMap>
#include <QString>
#include <optional>

struct Options;

class TsFile : public QFile{
public:
    struct Translation{
        QString sourceText;
        QStringList translation;    //This can have more than one element in the case of plurals
        QString disambiguation;
        QList<QPair<QString, int>> locations;
        bool unfinished = true;
        bool obsolete = false;
        bool hasPlural;
    };

    using QFile::QFile;

    void mkpath() const;

    bool loadTranslations();
    int setTranslations(const QList<Translation> &translations, const Options &options);
    void writeTranslationsToFile();

    int numberOfTranslations() const;
    int numberOfObsoleteTranslations() const;

private:
    std::optional<QMap<QString, QList<QDomElement>>> childTags(const QDomElement &parent, const QStringList &allowedTagNames);
    std::optional<QString> childText(const QDomElement &parent, bool allowErrors = false);

    static bool languageIsValid(const QString &language);
    static int numberOfPlurals(const QString &language);

    QMap<QString, QList<Translation>> _translationsByContext;
    QString _language;
    QString _sourceLanguage;
};

bool operator==(const TsFile::Translation &translation1, const TsFile::Translation &translation2);

#endif // TSFILE_H
