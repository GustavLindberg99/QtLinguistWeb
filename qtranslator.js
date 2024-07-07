/*
* qtranslator.js version 1.0.3 by Gustav Lindberg
* https://github.com/GustavLindberg99/QtLinguistWeb
*/

"use strict";

import typechecked from "https://gustavlindberg99.github.io/Typecheck.js/min/typecheck-v1.min.js";

const translators = [];

class Translation{
    sourceText;
    translation;
    disambiguation;
}

export class QObject{
    static tr(sourceText /*: String */, disambiguation /*: String | null */ = null, n /*: Number | null */ = null) /*: String*/ {
        for(let translator of translators){
            const translation = translator.translate("lupdate-js", sourceText, disambiguation, n);
            if(translation !== sourceText){
                return translation;
            }
        }
        if(n !== null){
            return sourceText.replace(/%n/g, n);
        }
        return sourceText;
    }
}
QObject = typechecked(QObject);

export class QTranslator extends QObject{
    #filePath /*: String */ = "";
    #language /*: String */ = "";
    #translationsByContext /*: Map<String, Array<Translation>> */ = new Map();

    filePath() /*: String */ {
        return this.#filePath;
    }

    isEmpty() /*: Boolean */ {
        return this.#filePath === "";
    }

    language() /*: String */ {
        return this.#language;
    }

    async load(filePath /*: String */) /*: Boolean */ {
        const xhr = new XMLHttpRequest();
        xhr.open("GET", filePath, true);
        xhr.send(null);
        await new Promise(resolve => {
            xhr.onload = xhr.onerror = resolve;
        });
        if(Math.floor(xhr.status / 100) !== 2){
            console.error("Could not load file " + filePath + ": Server responded with code " + xhr.status);
            return false;
        }

        const xml = xhr.responseXML;
        if(xml == null){
            if(filePath.endsWith(".qm")){
                console.error("Could not load file " + filePath + ": QTranslator.load() expects a TS file as argument, but a QM file was given.");
            }
            else{
                console.error("Could not load file " + filePath + ": Content is not valid XML.");
            }
            return false;
        }
        if(xml.documentElement.tagName !== "TS"){
            console.error("Could not load file " + filePath + ": Unexpected tag <" + xml.documentElement.tagName + ">.");
            return false;
        }

        let translationsByContext = new Map();
        for(let contextElement of xml.documentElement.childNodes){
            if(!(contextElement instanceof Element)){
                continue;
            }
            if(contextElement.tagName !== "context"){
                console.error("Could not load file " + filePath + ": Unexpected tag <" + contextElement.tagName + ">.");
                return false;
            }
            const contextName = contextElement.querySelector("name")?.textContent ?? "";
            translationsByContext.set(contextName, []);

            for(let messageElement of contextElement.querySelectorAll("message")){
                let translation = new Translation();
                translation.sourceText = messageElement.querySelector("source")?.textContent;
                if(translation.sourceText === undefined){
                    console.error("Could not load file " + filePath + ": Source text missing.");
                    return false;
                }
                translation.disambiguation = messageElement.querySelector("comment")?.textContent ?? "";
                if(messageElement.getAttribute("numerus") === "yes"){
                    translation.translation = [...messageElement.querySelectorAll("translation > numerusform")].map(it => it.textContent);
                }
                else{
                    translation.translation = messageElement.querySelector("translation")?.textContent ?? "";
                }
                translationsByContext.get(contextName).push(translation);
            }
        }

        const language = xml.documentElement.getAttribute("language") ?? "";
        if(this.#plural(0) === null){
            console.error("Could not load file " + filePath + ": Unknown language " + language + ".");
            return false;
        }

        this.#filePath = filePath;
        this.#language = language;
        this.#translationsByContext = translationsByContext;

        return true;
    }

    translate(context /*: String */, sourceText /*: String */, disambiguation /*: String | null */ = null, n /*: Number | null */ = null) /*: String */ {
        if(this.isEmpty()){
            console.warn("Calling translate on an empty QTranslator. Did you forget to call translator.load() or did you call it without await?");
        }

        const translation = (this.#translationsByContext.get(context) ?? []).find(it =>
            it.sourceText === sourceText
            && it.disambiguation === (disambiguation ?? "")
            && (n === null ? typeof it.translation === "string" : it.translation instanceof Array)
        );

        if(translation === undefined || translation.translation === ""){
            if(n !== null){
                return sourceText.replace(/%n/g, n);
            }
            else{
                return sourceText;
            }
        }
        else if(n === null){
            return translation.translation;
        }
        else{
            const plural = Math.min(this.#plural(n), translation.translation.length - 1);
            const result = translation.translation[plural].replace(/%n/g, n);
            if(result === ""){
                return sourceText.replace(/%n/g, n);
            }
            else{
                return result;
            }
        }
    }

    #plural(n /*: Number */) /*: Number */ {
        switch(this.language().split("_")[0]){
        case "aa":    //Afar
        case "ab":    //Abkhazian
        case "af":    //Afrikaans
        case "am":    //Amharic
        case "as":    //Assamese
        case "az":    //Azerbaijani
        case "ba":    //Bashkir
        case "bg":    //Bulgarian
        case "bn":    //Bangla
        case "ca":    //Catalan
        case "co":    //Corsican
        case "da":    //Danish
        case "de":    //German
        case "el":    //Greek
        case "en":    //English
        case "eo":    //Esperanto
        case "es":    //Spanish
        case "et":    //Estonian
        case "eu":    //Basque
        case "fi":    //Finnish
        case "fo":    //Faroese
        case "fur":   //Friulian
        case "fy":    //Western Frisian
        case "gl":    //Galician
        case "gu":    //Gujarati
        case "ha":    //Hausa
        case "he":    //Hebrew
        case "hi":    //Hindi
        case "ia":    //Interlingua
        case "it":    //Italian
        case "ka":    //Georgian
        case "kk":    //Kazakh
        case "kl":    //Kalaallisut
        case "kn":    //Kannada
        case "ks":    //Kashmiri
        case "ku":    //Kurdish
        case "kw":    //Cornish
        case "ky":    //Kyrgyz
        case "la":    //Latin
        case "lb":    //Luxembourgish
        case "ln":    //Lingala
        case "lo":    //Lao
        case "mg":    //Malagasy
        case "ml":    //Malayalam
        case "mn":    //Mongolian
        case "mr":    //Marathi
        case "nb":    //Norwegian Bokmal
        case "ne":    //Nepali
        case "nl":    //Dutch
        case "nn":    //Norwegian Nynorsk
        case "nso":   //Northern Sotho
        case "oc":    //Occitan
        case "or":    //Odia
        case "pa":    //Punjabi
        case "ps":    //Pashto
        case "qu":    //Quechua
        case "rm":    //Romansh
        case "rn":    //Rundi
        case "rw":    //Kinyarwanda
        case "sd":    //Sindhi
        case "si":    //Sinhala
        case "sn":    //Shona
        case "so":    //Somali
        case "sq":    //Albanian
        case "ss":    //Swati
        case "st":    //Southern Sotho
        case "sv":    //Swedish
        case "sw":    //Swahili
        case "ta":    //Tamil
        case "te":    //Telugu
        case "tg":    //Tajik
        case "tk":    //Turkmen
        case "tn":    //Tswana
        case "to":    //Tongan
        case "ts":    //Tsonga
        case "ug":    //Uyghur
        case "ur":    //Urdu
        case "uz":    //Uzbek
        case "vo":    //Volapuk
        case "wo":    //Wolof
        case "xh":    //Xhosa
        case "yi":    //Yiddish
        case "zu":    //Zulu
            return n === 1 ? 1 : 0;
        case "":      //Unknown language
        case "bo":    //Tibetan
        case "dz":    //Dzongkha
        case "fa":    //Persian
        case "gn":    //Guarani
        case "hu":    //Hungarian
        case "id":    //Indonesian
        case "ja":    //Japanese
        case "jv":    //Javanese
        case "ko":    //Korean
        case "ms":    //Malay
        case "my":    //Burmese
        case "om":    //Oromo
        case "su":    //Sudanese
        case "th":    //Thai
        case "tr":    //Turkish
        case "tt":    //Tatar
        case "vi":    //Vietnamese
        case "yo":    //Yoruba
        case "zh":    //Chinese
            return 0;
        case "br":    //Breton
        case "fr":    //French
        case "fil":   //Filipino
        case "hy":    //Armenian
        case "pt":    //Portuguese
        case "ti":    //Tigrinya
        case "wa":    //Walloon
            return n > 1 ? 1 : 0;
        case "dv":    //Divehi
        case "ga":    //Irish
        case "gv":    //Manx
        case "iu":    //Inuktitut
        case "mi":    //Maori
        case "sa":    //Sanskrit
        case "se":    //Northern Sami
            switch(n){
            case 1:
                return 0;
            case 2:
                return 1;
            default:
                return 2;
            }
        case "be":    //Belarusian
        case "bs":    //Bosnian
        case "hr":    //Croatian
        case "ru":    //Russian
        case "sr":    //Serbian
        case "uk":    //Ukrainian
            if(n % 10 === 1 && n % 100 !== 11){
                return 0;
            }
            else if(n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20)){
                return 1;
            }
            else{
                return 2;
            }
        case "cs":    //Czech
        case "sk":    //Slovak
            switch(n){
            case 1:
                return 0;
            case 2:
            case 3:
            case 4:
                return 1;
            default:
                return 2;
            }
        case "ar":    //Arabic
            if(n === 0){
                return 0;
            }
            else if(n === 1){
                return 1;
            }
            else if(n === 2){
                return 2;
            }
            else if(n % 100 >= 3 && n % 100 <= 10){
                return 3;
            }
            else if(n % 100 >= 11){
                return 4;
            }
            else{
                return 5;
            }
        case "cy":    //Welsh
            switch(n){
            case 0:
                return 0;
            case 1:
                return 1;
            case 2:
            case 3:
            case 4:
            case 5:
                return 2;
            case 6:
                return 3;
            default:
                return 4;
            }
        case "gd":    //Gaelic
            if(n === 1 || n === 11){
                return 0;
            }
            else if(n === 2 || n === 12){
                return 1;
            }
            else if(n > 2 && n < 20){
                return 2;
            }
            else{
                return 3;
            }
        case "is":    //Icelandic
            if(n % 10 === 1 && n % 100 !== 11){
                return 0;
            }
            else{
                return 1;
            }
        case "lt":    //Lithuanian
            if(n % 10 === 1 && n % 100 !== 11){
                return 0;
            }
            else if(n % 10 >= 2 && (n % 100 < 10 || n % 100 >= 20)){
                return 1;
            }
            else{
                return 2;
            }
        case "lv":    //Latvian
            if(n % 10 === 1 && n % 100 !== 11){
                return 0;
            }
            else if(n !== 0){
                return 1;
            }
            else{
                return 2;
            }
        case "mk":    //Macedonian
            switch(n % 100){
            case 1:
                return 0;
            case 2:
                return 1;
            default:
                return 2;
            }
        case "mt":    //Maltese
            if(n === 1){
                return 0;
            }
            else if(n === 0 || (n % 100 >= 1 && n % 100 <= 10)){
                return 1;
            }
            else if(n % 100 >= 11 && n % 100 <= 19){
                return 2;
            }
            else{
                return 3;
            }
        case "pl":    //Polish
            if(n === 1){
                return 0;
            }
            else if(n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20)){
                return 1;
            }
            else{
                return 2;
            }
        case "ro":    //Romanian
            if(n === 1){
                return 0;
            }
            else if(n === 0 || (n % 100 > 0 && n % 100 < 20)){
                return 1;
            }
            else{
                return 2;
            }
        case "sl":    //Slovenian
            switch(n % 100){
            case 1:
                return 0;
            case 2:
                return 1;
            case 3:
            case 4:
                return 2;
            default:
                return 3;
            }
        default:
            return null;
        }
    }
}
QTranslator = typechecked(QTranslator);

export class QCoreApplication extends QObject{
    static installTranslator(translator /*: QTranslator */) /*: Boolean */ {
        if(translators.includes(translator)){
            return false;
        }
        translators.push(translator);
        return true;
    }

    static removeTranslator(translator /*: QTranslator */) /*: Boolean */ {
        const initialLength = translators.length;
        translators = translators.filter(it => it !== translator);
        return translators.length < initialLength;
    }
}
QCoreApplication = typechecked(QCoreApplication);

Object.defineProperty(String.prototype, "arg", {value: function(...args){
    return this.replace(/%([0-9])/g, (match, index) => args[index - 1] ?? match);
}});