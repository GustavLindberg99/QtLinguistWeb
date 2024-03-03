# Qt Linguist for Web

Qt Linguist for Web is a port of Qt Linguist's tool chain for JavaScript/PHP based websites. It allows you to generate translation files in Qt Linguist's TS format from JavaScript and PHP code, edit them in Qt Linguist and use them in your JavaScript/PHP website.

Note that this is only a port of translation-related methods, not of the entire Qt framework. If you want to use the entire Qt framework in web projects, you should have a look at [Qt for WebAssembly](https://doc.qt.io/qt-6/wasm.html) instead.

## Contents
- [Setup](#setup)
    - [Installing lupdate-\* and Qt Linguist on Windows](#installing-lupdate--and-qt-linguist-on-windows)
    - [Installing lupdate-\* and Qt Linguist on Ubuntu/Debian](#installing-lupdate--and-qt-linguist-on-ubuntudebian)
    - [Using the qtranslator.js JavaScript library](#using-the-qtranslatorjs-javascript-library)
    - [Using the qtranslator.php PHP library](#using-the-qtranslatorphp-php-library)
- [Translating your code](#translating-your-code)
    - [Preparing your JavaScript/PHP code for translation](#preparing-your-javascriptphp-code-for-translation)
    - [Generating TS files with lupdate-js or lupdate-php](#generating-ts-files-with-lupdate-js-or-lupdate-php)
    - [Using Qt Linguist to edit the files](#using-qt-linguist-to-edit-the-files)
    - [Loading the translation into your JavaScript/PHP code](#loading-the-translation-into-your-javascriptphp-code)
- [String formatting](#string-formatting)
    - [String formatting in PHP](#string-formatting-in-php)
    - [String formatting in JavaScript](#string-formatting-in-javascript)
- [Documentation of the `QObject`, `QTranslator` and `QCoreApplication` classes](#documentation-of-the-qobject-qtranslator-and-qcoreapplication-classes)
    - [The `QObject` class](#the-qobject-class)
    - [The `QTranslator` class](#the-qtranslator-class)
    - [The `QCoreApplication` class](#the-qcoreapplication-class)

## Setup
To use Qt Linguist for Web, you need three things:

- The lupdate-js and/or lupdate-php command line tools to generate the TS files
- Qt Linguist to edit them
- The qtranslator JavaScript/PHP library to read them.

How to install the first two depends on your operating system but has similar steps for JavaScript and PHP, and the last one works the same on all operating systems but works differently in JavaScript and PHP.

### Installing lupdate-\* and Qt Linguist on Windows
To install these tools on Windows, you can use the [installation program](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/qt-linguist-web-install.exe). It contains three components, of which you can select one or more:
- **lupdate-js**: Needed to generate TS files from JavaScript code. Select this one if your website uses client-side JavaScript.
- **lupdate-php**: Needed to generate TS files from PHP code. Select this one if your website uses PHP.
- **Qt Linguist**: Needed to translate the TS files. You will always need Qt Linguist, but Qt Linguist is also installed by default if you install [Qt](https://www.qt.io/download-open-source), and the version provided here is the same as the one provided by Qt (but the one provided by Qt might be more up to date). This is provided as a convenience if you don't have Qt already installed so that you don't have to install the entire Qt C++ framework. So select this one if you don't have the Qt C++ framework already installed.

If you don't want to or can't use the installer, you can also download portable versions for [lupdate-js and lupdate-php](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/lupdate-web-portable.zip) and for  [Qt Linguist](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/qt-linguist-portable.zip). Again, the portable Qt Linguist version is the same one as the one that's installed by default together with Qt, so if you already have the Qt C++ framework installed, you don't need to install Qt Linguist separately.

### Installing lupdate-\* and Qt Linguist on Ubuntu/Debian
To install lupdate-js on Ubuntu/Debian, download [the lupdate-js deb file](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/lupdate-js.deb), open a terminal in the folder where you downloaded it and run the following command:

```bash
sudo apt install ./lupdate-js.deb
```

Similarly, to install lupdate-php, download [the lupdate-php deb file](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/lupdate-php.deb), open a terminal in the folder where you downloaded it and run the following command:

```bash
sudo apt install ./lupdate-php.deb
```

If you don't have the Qt C++ framework installed, you can install Qt Linguist by installing the Qt dev tools:

```bash
sudo apt install qttools5-dev-tools
```

Then you can run Qt Linguist with the `linguist` command. However, if you already have the Qt C++ framework installed, Qt Linguist is installed together with it by default, so you can use that version instead.

### Using the qtranslator.js JavaScript library
To read the TS files in your JavaScript code, you need the qtranslator.js library. This libarary can only be used in [ES6 modules](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Modules, and needs to be imported in files where you use it:

```javascript
import {QCoreApplication, QObject, QTranslator} from "https://gustavlindberg99.github.io/QtLinguistWeb/qtranslator.js";
```

Note that since qtranslator.js uses AJAX, it can only be used in browser-based JavaScript (so it's not compatible with Node.js) and can't be run on the `file://` protocol.

### Using the qtranslator.php PHP library
To read the TS files in your JavaScript code, you need the qtranslator.php library. To do so, download the [qtranslator.php](https://raw.githubusercontent.com/GustavLindberg99/QtLinguistWeb/main/qtranslator.php) file into your PHP project folder, and include it as follows:

```php
require_once('qtranslator.php');
```

## Translating your code
### Preparing your JavaScript/PHP code for translation
To make strings in your JavaScript/PHP code translateable, you need to pass them as an argument to the `QObject.tr` function (in JavaScript) or the `QObject::tr` function (in PHP). Here are examples of the most basic useage of this function in both JavaScript and PHP:

```javascript
import {QObject} from "https://gustavlindberg99.github.io/QtLinguistWeb/qtranslator.js";

console.log(QObject.tr("Hello World!"));
```

```php
require_once('qtranslator.php');

echo QObject::tr("Hello World!");
```

If you run this code, you won't notice much. This is because you haven't translated anything yet, so `QObject.tr` will just return its input. But when you've translated it, it will return the translated string instead.

### Generating TS files with lupdate-js or lupdate-php
To start translating your website, you need to start by generating the TS files that will contain the translations. This is done by the `lupdate-js` or `lupdate-php` command line tools.

To do this, the first thing you need to do is specify which languages you want to translate to, which is done by specifying the file names of the TS files you want to generate. The name of a TS file has the structure `<your app name>_<language abbreviation>.ts`, for example `HelloWorld_fr.ts`. The first part of the name (the app name) is not parsed and can be whatever you want, but the second part is parsed when determining which language the file is for, so it must be a valid language abbreviation. Qt Linguist for Web currently supports 135 languages, a complete list of their abbreviations can be found by running `lupdate-js -list-languages` or `lupdate-php -list-languages`.

There are three ways of specifying the names of the TS files you want to generate:
- If your website has a .htaccess file, the easiest way is to specify it in a comment at the beginning of your .htaccess file. The first line of that comment must be `# LUPDATE_JS_OPTIONS` for lupdate-js or `# LUPDATE_PHP_OPTIONS` for lupdate-php. You can then specify the names of the .ts files using `# TRANSLATIONS = name1.ts name2.ts ...`. You can also have line breaks if you end the line with a backslash. For example, to translate a PHP project to English, French and Swedish, you can put the following code at the beginning of your .htaccess file:

    ```bash
    # LUPDATE_PHP_OPTIONS
    # TRANSLATIONS = \
    #     HelloWorld_en.ts \
    #     HelloWorld_fr.ts \
    #     HelloWorld_sv.ts \
    ```

    Note that this will place the TS files in the root folder of your project, if you want them in a subfolder, you can do `TRANSLATIONS = subfolder/HelloWorld_en.ts ...`.

    If you do it this way, just running `lupdate-js` or `lupdate-php` in the root folder of your project is sufficient.

- You can also put this comment at the beginning of your index.html or index.php file. This is similar to how you would do in .htaccess, but the syntax for comments is different:

    ```html
    <!-- LUPDATE_PHP_OPTIONS
    TRANSLATIONS = \
        HelloWorld_en.ts \
        HelloWorld_fr.ts \
        HelloWorld_sv.ts \
    -->
    ```

    Again, if you do it this way, just running `lupdate-js` or `lupdate-php` in the root folder of your project is sufficient.

- If you don't want to change your .htaccess or index files, or if you want to override what you specified in them, you can pass the `-ts` argument to `lupdate-js` or `lupdate-php`. For example, the following will only generate a French translation, regardless of what's specified in the .htaccess or index files (if anything):

    ```bash
    lupdate-php -ts HelloWorld_fr.ts
    ```

    You still need to run it in the root folder of your project for it to be able to find all the files.

If you specified which TS files to generate in your .htaccess or index file, it's often enough to just run `lupdate-js` or `lupdate-php` in the root folder of your project, without any arguments. However, there are some options that you might find useful, run `lupdate-js -help` or `lupdate-php -help` for a complete list of those options.

### Using Qt Linguist to edit the files
The files generated by `lupdate-js` and `lupdate-php` are of the same format as the ones generated by Qt's `lupdate` for C++ files, so Qt Linguist can open them without any problems. You can find a guide for using Qt Linguist [here](https://doc.qt.io/qt-6/linguist-translators.html).

### Loading the translation into your JavaScript/PHP code
To load the translation, you need to use a `QTranslator` object. First you need to call the `load` method on it, then you need to install it using `QCoreApplication.installTranslator` (in JavaScript) or `QCoreApplication::installTranslator` (in PHP). Here is the same example as above, but this time it actually loads the translations (and should return the translated strings if you've translated them):

```javascript
import {QCoreApplication, QObject, QTranslator} from "https://gustavlindberg99.github.io/QtLinguistWeb/qtranslator.js";

const translator = new QTranslator();
translator.load("/HelloWorld_fr.ts");
QCoreApplication.installTranslator(translator);

console.log(QObject.tr("Hello World!"));
```

```php
require_once('qtranslator.php');

$translator = new QTranslator();
$translator->load($_SERVER['DOCUMENT_ROOT'] . '/HelloWorld_fr.ts');
QCoreApplication::installTranslator($translator);

echo QObject::tr("Hello World!");
```

## String formatting
Sometimes you might want to translate a string that contains an unknown substring (such as user input). This is done differently in JavaScript and PHP, since the two languages have different features.

Note that in both languages you should *not* use the built-in string formatting features (double quoted strings in PHP, template literals in JavaScript), since the `QObject.tr` function expects the unformatted string as input in order to be able to determine which string corresponds to which translation.

### String formatting in PHP
To format translated strings in PHP, use the built-in [`sprintf`](https://www.php.net/sprintf) function. You should pass the result of `QObject::tr` to `sprintf`, not the other way around. Example:

```php
echo htmlspecialchars(sprintf(QObject::tr('Hello %s!'), $_POST['username']));
```

If you have multiple format arguments, you should use `%1$s`, `%2$s`, etc, in case the placeholders are in a different order in the translated string:

```php
echo htmlspecialchars(sprintf(QObject::tr('The %2$s contains a %1$s.'), $_POST['animal'], $_POST['location']));
```

Note that you can't use `QObject::htr` here instead of `htmlspecialchars` and `QObject::tr`, since that wouldn't escape the user input.

If one of the inputs is a number that can be singular or plural, you can use `%n` and the third argument of the `QObject::tr` method. When you do this, Qt Linguist will allow you to create different translations for singular and plural.

```php
echo QObject::htr('There are %n monkeys in the tree.', null, $_POST['number']);
```

Note that it's OK to use `QObject::htr` here since the only argument was passed to `QObject::htr` itself, not `sprintf`.

This can be combined with regular formatting:

```php
echo htmlspecialchars(sprintf(QObject::tr('The %s contains %n monkeys.', null, $_POST['number']), $_POST['location']));
```

Here the number (`%n`) is passed as an argument to `QObject::tr` and the string (`%s`) is passed as an argument to `sprintf`.

### String formatting in JavaScript
JavaScript doesn't have a built-in string formatting function, so qtranslator.js adds an `arg` method to the built-in String class that can be used for formatting. The `arg` method can take up to nine arguments, and replaces any occurrence of `%1` with the first argument, `%2` with the second argument, etc. Example:

```javascript
const name = prompt(QObject.tr("What's your name?"));
alert(QObject.tr("Hello %1!").arg(name));
```

If one of the inputs is a number that can be singular or plural, you can use `%n` and the third argument of the `QObject.tr` method. When you do this, Qt Linguist will allow you to create different translations for singular and plural.

```javascript
const number = parseInt(prompt(QObject.tr("How many monkeys are there in the tree?")));
alert(QObject.tr("There are %n monkeys in the tree.", null, number));
```

This can be combined with regular formatting:

```javascript
const number = parseInt(prompt(QObject.tr("How many monkeys are there?")));
const location = prompt(QObject.tr("Where are they?"));
alert(QObject.tr("The %1 contains %n monkeys.", null, number).arg(location));
```

## Documentation of the `QObject`, `QTranslator` and `QCoreApplication` classes
### The `QObject` class
The JavaScript version of the `QObject` class only has one method and the PHP version has only two methods, all of which are static (so there's not much point in creating `QObject` instances):
- `tr`, almost an exact port of [Qt's `QObject::tr` C++ method](https://doc.qt.io/qt-6/qobject.html#tr). The only difference is that in JavaScript and PHP the third parameter is allowed to be null, indicating that there are no plurals (if the third parameter is non-null, all occurrences of `%n` will be replaced with the value of that parameter, see [here](https://doc.qt.io/qt-6/i18n-source-translation.html#handle-plural-forms) for more details). In case you don't know C/C++ that well, the first two parameters are strings (`const char*` is a string in C). In JavaScript and PHP, the first parameter is non-nullable and the second parameter is nullable.
- In PHP, there is a `QObject::htr` static method which is a shorthand for `htmlspecialchars(QObject::tr(...))`. It's good practice to use `QObject::htr` instead of `QObject::tr` whenever you print the output directly even if the source string doesn't have any special characters, in case the translated string does (for example if the source string contains smart quotes and the translated string contains regular quotes).

### The `QTranslator` class
The `QTranslator` JavaScript and PHP classes are almost exact ports of Qt's [`QTranslator` C++ class](https://doc.qt.io/qt-6/qtranslator.html), with the following differences:

- The constructor can't take a `QObject` parent as a parameter.
- There is only one version of the `load` method which takes one parameter, which is the path to the TS file (*not* the QM file as in C++, Qt Linguist for Web doesn't use QM files).
- The `context` parameter of the `translate` method should always be set to `lupdate-js` in JavaScript and `lupdate-php` in PHP. This allows you to have your JavaScript and PHP translations all in the same file if you want. The way the context is used in C++ is not very useful in JavaScript or PHP since you don't have the entire Qt framework so there's not much point in creating subclasses of the `QObject` class (in fact, don't, because if you do, lupdate-js and lupdate-php won't recognize the calls to the `tr` method). Also, the `n` parameter of the `translate` method treats -1 as any other number in JavaScript and PHP and uses null instead to indicate that there are no plurals.

### The `QCoreApplication` class
The JavaScript and PHP versions of the `QCoreApplication` class only have only two methods, all of which are static (so there's not much point in creating `QCoreApplication` instances):
- `installTranslator`, a port of [Qt's `QCoreApplication::installTranslator` C++ method](https://doc.qt.io/qt-6/qcoreapplication.html#installTranslator).
- `removeTranslator`, a port of [Qt's `QCoreApplication::removeTranslator` C++ method](https://doc.qt.io/qt-6/qcoreapplication.html#removeTranslator).