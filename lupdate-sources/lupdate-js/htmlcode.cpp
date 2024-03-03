#include <lexbor/dom/interfaces/node.h>
#include <lexbor/tag/const.h>
#include "htmlcode.h"

extern "C" {
LXB_API const lxb_char_t *
lxb_dom_element_get_attribute(lxb_dom_element_t *element,
                              const lxb_char_t *qualified_name, size_t qn_len,
                              size_t *value_len);
}

HtmlCode HtmlCode::fromPhpFile(QFile &file){
    QString phpCode = QString::fromUtf8(file.readAll());

    int openingPhpTagIndex = phpCode.indexOf("<?");
    int closingPhpTagIndex;

    QString result = phpCode.first(openingPhpTagIndex);

    while(openingPhpTagIndex != -1){
        closingPhpTagIndex = phpCode.indexOf("?>", openingPhpTagIndex + 2);
        if(closingPhpTagIndex == -1){
            //If there is no more closing PHP tag, the rest is PHP code, so just ignore it
            break;
        }
        const int numberOfNewlines = phpCode.sliced(openingPhpTagIndex, closingPhpTagIndex + 2 - openingPhpTagIndex).count('\n');
        result += QString("\n").repeated(numberOfNewlines);
        openingPhpTagIndex = phpCode.indexOf("<?", closingPhpTagIndex + 2);
        if(openingPhpTagIndex == -1){
            result += phpCode.sliced(closingPhpTagIndex + 2);
        }
        else{
            result += phpCode.sliced(closingPhpTagIndex + 2, openingPhpTagIndex - closingPhpTagIndex - 2);
        }
    }

    return HtmlCode(result.toUtf8(), file.fileName(), 1);
}

QList<JsCode> HtmlCode::inlineJsScripts() const{
    return this->_inlineJsScripts;
}

bool HtmlCode::parse(){
    this->_inlineJsScripts.clear();
    lxb_html_document_t *document = lxb_html_document_create();
    if(document == nullptr){
        return false;
    }
    const bool parseStatus = this->parseLexborDocument(document);
    if(parseStatus){
        int offset = 0;
        this->findScripts(lxb_dom_interface_node(document), offset);
    }
    lxb_html_document_destroy(document);
    return parseStatus;
}

void HtmlCode::findScripts(const lxb_dom_node_t *parent, int &offset){
    for(lxb_dom_node_t *child = parent->first_child; child != nullptr; child = child->next){
        switch(child->local_name){
        case LXB_TAG_SCRIPT:{
            //If it's a script tag, add its cotnent to this->_inlineJsScripts if it has no src tag
            const lxb_char_t *srcTagContents = lxb_dom_element_get_attribute(lxb_dom_interface_element(child), "src", 3, nullptr);
            if(srcTagContents == nullptr){
                //If the script has an src attribute, just ignore it, since we're only interested in the contents
                //Even if it has contents, the src tag gets precedence: https://stackoverflow.com/a/6528372/4284627
                //We don't need to look at the src, because if it's part of the project, it will be found when iterating over the files anyway
                const QByteArray contents = lxb_dom_node_text_content(child, nullptr);
                offset = this->contents().indexOf(contents, offset + 1);
                this->_inlineJsScripts.append(JsCode(contents, this->filePath(), this->lineNumber(offset)));
            }
            break;
        }

        case LXB_TAG__UNDEF:
        case LXB_TAG__END_OF_FILE:
        case LXB_TAG__TEXT:
        case LXB_TAG__DOCUMENT:
        case LXB_TAG__EM_COMMENT:
        case LXB_TAG__EM_DOCTYPE:
            //If it's not a tag, do nothing
            break;

        default:
            //If it's a valid HTML tag other than <script>, search for scripts among child tags
            this->findScripts(child, offset);
            break;
        }
    }
}
