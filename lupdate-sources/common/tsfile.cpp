#include <QDir>
#include <QDomDocument>
#include <QFileInfo>
#include <QRegularExpression>
#include <algorithm>
#include "args.h"
#include "languages.h"
#include "tsfile.h"
#include "version.h"

void TsFile::mkpath() const{
    QFileInfo(*this).dir().mkpath(".");
}

bool TsFile::loadTranslations(){
    //Clear the previous translations in case it's already been parsed
    this->_translationsByContext.clear();
    this->_language.clear();
    this->_sourceLanguage.clear();

    //Get the language based on the file name
    const QStringList splitFileName = QFileInfo(*this).completeBaseName().split("_");
    const QStringList::const_iterator language = std::find_if(splitFileName.begin() + 1, splitFileName.end(), [](const QString &str){return languageIsValid(str);});
    if(language != splitFileName.end()){
        this->_language = *language;
    }

    //If the file is empty or doesn't exist, don't read, just write
    if(this->peek(1).isEmpty()){
        return true;
    }

    //Open the document
    QDomDocument xmlDocument;
    const QDomDocument::ParseResult parseResult = xmlDocument.setContent(this);
    if(!parseResult){
        this->setErrorString("Parse error at " + this->fileName() + ":" + QString::number(parseResult.errorLine) + ":" + QString::number(parseResult.errorColumn) + ": " + parseResult.errorMessage);
        return false;
    }

    const QDomElement tsElement = xmlDocument.documentElement();
    if(tsElement.tagName() != "TS"){
        this->setErrorString("Unexpected tag <" + tsElement.tagName() + "> at " + this->fileName() + ":" + QString::number(tsElement.lineNumber()) + ":" + QString::number(tsElement.columnNumber()));
        return false;
    }

    //If the language is specified in the file, use that one instead
    if(tsElement.hasAttribute("language")){
        this->_language = tsElement.attribute("language");
        if(!languageIsValid(this->_language)){
            this->_language.clear();
            this->setErrorString("File " + this->fileName() + " won't be updated: it contains translation but the target language is not recognized");
            return false;
        }
    }

    //Get the translations
    const auto tsElementChildren = this->childTags(tsElement, {"context"});
    if(tsElementChildren == std::nullopt){
        return false;
    }
    for(const QDomElement &context: tsElementChildren->value("context")){
        const std::optional<QString> contextName = this->childText(context.firstChildElement("name"));
        if(contextName == std::nullopt){
            return false;
        }
        const auto contextElementChildren = this->childTags(context, {"name", "message"});
        if(contextElementChildren == std::nullopt){
            return false;
        }
        for(const QDomElement &message: contextElementChildren->value("message")){
            const auto messageElementChildren = this->childTags(message, {"location", "source", "oldsource", "translation", "comment"});
            const std::optional<QString> sourceText = this->childText(message.firstChildElement("source"));
            const std::optional<QString> disambiguation = this->childText(message.firstChildElement("comment"));
            if(messageElementChildren == std::nullopt || sourceText == std::nullopt || disambiguation == std::nullopt){
                return false;
            }
            const QDomElement translationElement = message.firstChildElement("translation");
            Translation t;
            t.sourceText = *sourceText;
            t.disambiguation = *disambiguation;
            t.unfinished = translationElement.attribute("type") == "unfinished";
            t.obsolete = translationElement.attribute("type") == "obsolete";
            for(const QDomElement &location: messageElementChildren->value("location")){
                const QString fileName = location.attribute("filename");
                const int line = location.attribute("line").toInt();
                t.locations.append({fileName, line});
            }
            const std::optional<QString> translation = this->childText(translationElement, true);
            if(translation == std::nullopt){
                const auto translationElementChildren = this->childTags(translationElement, {"numerusform"});
                if(translationElementChildren == std::nullopt){
                    return false;
                }
                for(const QDomElement &numerusForm: translationElementChildren->value("numerusform")){
                    const std::optional<QString> numerusFormText = this->childText(numerusForm);
                    if(numerusFormText == std::nullopt){
                        return false;
                    }
                    t.translation.append(*numerusFormText);
                }
                t.hasPlural = true;
            }
            else{
                t.translation = {*translation};
                t.hasPlural = false;
            }
            this->_translationsByContext[*contextName].append(t);
        }
    }

    //Get the source language (if the attribute doesn't exist it will just be an empty string)
    this->_sourceLanguage = tsElement.attribute("sourcelanguage");

    return true;
}

int TsFile::setTranslations(const QList<Translation> &newTranslations, const Options &options){
    int numberOfNewTranslations = 0;

    //Remove the old locations
    QList<Translation> &oldTranslations = this->_translationsByContext[PROGRAMNAME];
    for(Translation &oldTranslation: oldTranslations){
        oldTranslation.locations.clear();
    }

    //Add the new translations
    for(const Translation &newTranslation: newTranslations){
        const QList<Translation>::iterator oldTranslation = std::find(oldTranslations.begin(), oldTranslations.end(), newTranslation);
        if(oldTranslation == oldTranslations.end()){
            numberOfNewTranslations++;
            oldTranslations.append(newTranslation);
            if(newTranslation.hasPlural){
                oldTranslations.last().translation.resize(qMax(1, numberOfPlurals(this->_language)));
            }
        }
        else{
            oldTranslation->locations += newTranslation.locations;
            oldTranslation->obsolete = false;
        }
    }

    //Handle obsolete translations
    for(int i = 0; i < oldTranslations.size(); i++){
        if(oldTranslations[i].locations.isEmpty()){
            if(options.noObsolete || oldTranslations[i].translation.join("").isEmpty()){
                oldTranslations.remove(i);
                i--;
            }
            else{
                oldTranslations[i].obsolete = true;
            }
        }
    }

    return numberOfNewTranslations;
}

void TsFile::writeTranslationsToFile(){
    QDomDocument document;

    QDomElement tsElement = document.createElement("TS");
    tsElement.setAttribute("version", "2.1");
    if(!this->_language.isEmpty()){
        tsElement.setAttribute("language", this->_language);
    }
    if(!this->_sourceLanguage.isEmpty()){
        tsElement.setAttribute("sourcelanguage", this->_sourceLanguage);
    }
    document.appendChild(tsElement);

    for(const QString &contextName: this->_translationsByContext.keys()){
        QDomElement contextElement = document.createElement("context");
        tsElement.appendChild(contextElement);

        QDomElement nameElement = document.createElement("name");
        nameElement.appendChild(document.createTextNode(contextName));
        contextElement.appendChild(nameElement);

        for(const Translation &translation: this->_translationsByContext[contextName]){
            QDomElement messageElement = document.createElement("message");
            if(translation.hasPlural){
                messageElement.setAttribute("numerus", "yes");
            }
            contextElement.appendChild(messageElement);

            for(const QPair<QString, int> &location: translation.locations){
                QDomElement locationElement = document.createElement("location");
                locationElement.setAttribute("filename", QFileInfo(this->fileName()).absoluteDir().relativeFilePath(QFileInfo(location.first).absoluteFilePath()));
                locationElement.setAttribute("line", location.second);
                messageElement.appendChild(locationElement);
            }

            QDomElement sourceElement = document.createElement("source");
            sourceElement.appendChild(document.createTextNode(translation.sourceText));
            messageElement.appendChild(sourceElement);

            if(!translation.disambiguation.isEmpty()){
                QDomElement commentElement = document.createElement("comment");
                commentElement.appendChild(document.createTextNode(translation.disambiguation));
                messageElement.appendChild(commentElement);
            }

            QDomElement translationElement = document.createElement("translation");
            if(translation.obsolete){
                translationElement.setAttribute("type", "obsolete");
            }
            else if(translation.unfinished){
                translationElement.setAttribute("type", "unfinished");
            }
            if(translation.hasPlural){
                for(const QString &numerusForm: translation.translation){
                    QDomElement numerusFormElement = document.createElement("numerusform");
                    numerusFormElement.appendChild(document.createTextNode(numerusForm));
                    translationElement.appendChild(numerusFormElement);
                }
            }
            else if(!translation.translation.isEmpty()){
                translationElement.appendChild(document.createTextNode(translation.translation[0]));
            }
            messageElement.appendChild(translationElement);
        }
    }

    this->resize(0);
    this->write(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<!DOCTYPE TS>\n"
    );
    this->write(document.toString(4).toUtf8());
}

int TsFile::numberOfTranslations() const{
    return this->_translationsByContext[PROGRAMNAME].size();
}

int TsFile::numberOfObsoleteTranslations() const{
    const auto translations = this->_translationsByContext[PROGRAMNAME];
    return std::count_if(translations.begin(), translations.end(), [](const Translation &translation){return translation.obsolete;});
}

std::optional<QMap<QString, QList<QDomElement>>> TsFile::childTags(const QDomElement &parent, const QStringList &allowedTagNames){
    const QDomNodeList children = parent.childNodes();
    QMap<QString, QList<QDomElement>> result;
    for(int i = 0; i < children.size(); i++){
        const QDomNode child = children.at(i);
        switch(child.nodeType()){
        case QDomNode::CommentNode:
        case QDomNode::AttributeNode:
            continue;
        case QDomNode::ElementNode:
            if(!allowedTagNames.contains(child.nodeName())){
                this->_language.clear();
                this->setErrorString("Unexpected tag <" + child.nodeName() + "> at " + this->fileName() + ":" + QString::number(child.lineNumber()) + ":" + QString::number(child.columnNumber()));
                return std::nullopt;
            }
            result[child.nodeName()].append(child.toElement());
            break;
        default:
            this->_language.clear();
            this->setErrorString("Expected character data at " + this->fileName() + ":" + QString::number(child.lineNumber()) + ":" + QString::number(child.columnNumber()));
            return std::nullopt;
        }
    }
    return result;
}

std::optional<QString> TsFile::childText(const QDomElement &parent, bool allowErrors){
    const QDomNodeList children = parent.childNodes();
    QString result;
    for(int i = 0; i < children.size(); i++){
        const QDomNode child = children.at(i);
        switch(child.nodeType()){
        case QDomNode::CommentNode:
        case QDomNode::AttributeNode:
            continue;
        case QDomNode::TextNode:
        case QDomNode::CDATASectionNode:
            result += child.nodeValue();
            break;
        case QDomNode::ElementNode:
            if(child.nodeName() == "byte"){
                const QString characterCode = child.toElement().attribute("value");
                if(characterCode.startsWith("x")){
                    result += static_cast<char>(characterCode.sliced(1).toInt(nullptr, 16));
                }
                else{
                    result += static_cast<char>(characterCode.toInt());
                }
                break;
            }
            //fall through
        default:
            if(!allowErrors){
                this->_language.clear();
                this->setErrorString("Expected character data at " + this->fileName() + ":" + QString::number(child.lineNumber()) + ":" + QString::number(child.columnNumber()));
            }
            return std::nullopt;
        }
    }
    return result;
}

bool TsFile::languageIsValid(const QString &language){
    return numberOfPlurals(language) != 0;
}

int TsFile::numberOfPlurals(const QString &language){
    const auto foundLanguage = std::find_if(
        std::begin(languages),
        std::end(languages),
        [&language](const std::tuple<const char*, int, const char*> &validLanguage){
            return QRegularExpression("^" + QRegularExpression::escape(std::get<0>(validLanguage)) + "([^a-z]|$)", QRegularExpression::CaseInsensitiveOption).match(language).hasMatch();
        }
    );
    if(foundLanguage != std::end(languages)){
        return std::get<1>(*foundLanguage);
    }
    return 0;
}

bool operator==(const TsFile::Translation &translation1, const TsFile::Translation &translation2){
    return translation1.sourceText == translation2.sourceText && translation1.disambiguation == translation2.disambiguation && translation1.hasPlural == translation2.hasPlural;
}
