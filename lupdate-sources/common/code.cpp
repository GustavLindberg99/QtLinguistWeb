#include <QRegularExpression>
#include "code.h"

Code::Code(const QByteArray &contents, const QString &filePath, int lineOffset):
    _contents(contents),
    _filePath(filePath),
    _lineOffset(lineOffset)
{}

Code::Code(QFile &file):
    _contents(file.readAll()),
    _filePath(file.fileName()),
    _lineOffset(1)
{}

QString Code::contents() const{
    return QString::fromUtf8(this->_contents).replace("\r\n", "\n").replace("\r", "\n");
}

QString Code::filePath() const{
    return this->_filePath;
}

int Code::lineNumber(int characterNumber) const{
    const QString contentsBefore = this->contents().first(characterNumber);
    return contentsBefore.count('\n') + this->_lineOffset;
}

bool Code::parseLexborDocument(lxb_html_document_t *document) const{
    return lxb_html_document_parse(document, this->_contents.constData(), this->_contents.size()) == LXB_STATUS_OK;
}
