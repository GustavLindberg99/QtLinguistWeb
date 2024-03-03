#ifndef HTMLCODE_H
#define HTMLCODE_H

#include <QList>
#include "code.h"
#include "jscode.h"

typedef struct lxb_dom_node lxb_dom_node_t;

class HtmlCode : public Code{
public:
    using Code::Code;
    static HtmlCode fromPhpFile(QFile &file);

    QList<JsCode> inlineJsScripts() const;
    bool parse() override;

private:
    void findScripts(const lxb_dom_node_t *parent, int &offset);

    QList<JsCode> _inlineJsScripts;
};

#endif // HTMLCODE_H
