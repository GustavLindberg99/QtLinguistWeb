#include <QRegularExpression>
#include <lexbor/dom/interfaces/node.h>
#include "optionscode.h"
#include "version.h"

QStringList OptionsCode::tsFiles() const{
    return this->_tsFiles;
}

bool OptionsCode::parse(){
    this->_tsFiles.clear();

    const QString contents = this->contents();
    static const QRegularExpression htaccessRegex(
        //Allow for other comments before to allow specifying different LUPDATE_JS_OPTIONS and LUPDATE_PHP_OPTIONS
        "^(?:\\s*#.*\n)*?"
        //LUPDATE_OPTIONS in a .htaccess comment
        "\\s*#[^\\S\n]*(?:" LUPDATE_OPTIONS "|LUPDATE_OPTIONS)[^\\S\n]*"
        //Newline followed by a continuation of the comment
        "\n[^\\S\n]*#"
        //Contents of the options
        "([\\S\\s]+?)"
        //The options are finished when we find a line that does not start with a comment (including an empty line)
        "\n[^\\S\n]*(?:[^#\\s]|\n|$)"
    );

    //.htaccess syntax
    const QRegularExpressionMatch htaccessMatch = htaccessRegex.match(contents);
    if(htaccessMatch.hasMatch()){
        static const QRegularExpression newlineCommentRegex("\n[^\\S\n]*#");
        const QString options = htaccessMatch.captured(1).replace(newlineCommentRegex, "\n");
        this->parseOptionsString(options);
        return true;
    }

    //HTML syntax
    lxb_html_document_t *document = lxb_html_document_create();
    if(document != nullptr && this->parseLexborDocument(document)){
        //This will be the first tag in the entire document, often a doctype.
        //If the document is empty, it's nullptr.
        lxb_dom_node_t *child = lxb_dom_interface_node(document)->first_child;

        //We're not interested in the doctype here, so if it's a doctype, skip it
        if(child != nullptr && child->local_name == LXB_TAG__EM_DOCTYPE){
            child = child->next;
        }

        //Find LUPDATE_OPTIONS comment among the first non-doctype tags.
        //Look at more than just the first comment to allow specifying separate LUPDATE_JS_OPTIONS and LUPDATE_PHP_OPTIONS
        while(child != nullptr && child->local_name == LXB_TAG__EM_COMMENT){
            QString commentContent = QString::fromUtf8(lxb_dom_node_text_content(child, nullptr)).trimmed();
            if(commentContent.startsWith(LUPDATE_OPTIONS) || commentContent.startsWith("LUPDATE_OPTIONS")){
                this->parseOptionsString(commentContent);
                lxb_html_document_destroy(document);
                return true;
            }
            else{
                child = child->next;
            }
        }
    }

    lxb_html_document_destroy(document);
    return false;
}

void OptionsCode::parseOptionsString(QString options){
    static const QRegularExpression backslashNewline("\\\\\n");
    static const QRegularExpression translations("(?:^|\n)\\s*TRANSLATIONS\\s*=(.+)");
    const QRegularExpressionMatch match = translations.match(options.replace(backslashNewline, ""));
    if(match.hasMatch()){
        const QString matchesString = match.captured(1).trimmed();
        static const QRegularExpression spaces("\\s+");
        this->_tsFiles = matchesString.split(spaces);
    }
}
