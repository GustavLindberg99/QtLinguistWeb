#ifndef CODE_H
#define CODE_H

#include <QByteArray>
#include <QFile>
#include <lexbor/core/base.h>

extern "C" {
typedef struct lxb_html_document lxb_html_document_t;

LXB_API lxb_html_document_t *
lxb_html_document_create(void);

LXB_API lxb_html_document_t *
lxb_html_document_destroy(lxb_html_document_t *document);

LXB_API lxb_status_t
lxb_html_document_parse(lxb_html_document_t *document,
                        const lxb_char_t *html, size_t size);
}

class Code{
public:
    Code(const QByteArray &contents, const QString &filePath, int lineOffset);
    Code(QFile &file);    //The file must be opened first

    virtual ~Code() = default;

    QString contents() const;
    QString filePath() const;
    int lineNumber(int characterNumber) const;

    virtual bool parse() = 0;

protected:
    bool parseLexborDocument(lxb_html_document_t *document) const;

private:
    QByteArray _contents;
    QString _filePath;
    int _lineOffset;
};

#endif // CODE_H
