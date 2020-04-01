#include "commonutility.h"
#include "config.h"

// Note:  This file must compile without QtGui
#include <QDir>
#include <QTranslator>
#include <QLibraryInfo>

namespace OCC {

Q_LOGGING_CATEGORY(lcCommonUtility, "common.utility", QtInfoMsg)

QString applicationTrPath()
{
#if defined(Q_OS_MAC)
    QString devTrPath = QCoreApplication::applicationDirPath() + QString::fromLatin1("/../../../../src/gui/");
#else
    QString devTrPath = QCoreApplication::applicationDirPath() + QString::fromLatin1("/../src/gui/");
#endif
    if (QDir(devTrPath).exists()) {
        // might miss Qt, QtKeyChain, etc.
        qWarning() << "Running from build location! Translations may be incomplete!";
        return devTrPath;
    }
#if defined(Q_OS_WIN)
    return QCoreApplication::applicationDirPath() + QLatin1String("/i18n/");
#elif defined(Q_OS_MAC)
    return QCoreApplication::applicationDirPath() + QLatin1String("/../Resources/Translations"); // path defaults to app dir.
#elif defined(Q_OS_UNIX)
    return QCoreApplication::applicationDirPath() + QString::fromLatin1("/../.." SHAREDIR "/" APPLICATION_EXECUTABLE "/i18n/");
#endif
}

QString substLang(const QString &lang)
{
    // Map the more appropriate script codes
    // to country codes as used by Qt and
    // transifex translation conventions.

    // Simplified Chinese
    if (lang == QLatin1String("zh_Hans"))
        return QLatin1String("zh_CN");
    // Traditional Chinese
    if (lang == QLatin1String("zh_Hant"))
        return QLatin1String("zh_TW");
    return lang;
}

void CommonUtility::setupTranslations(QCoreApplication *app, const QString &enforcedLocale)
{
    QStringList uiLanguages;
// uiLanguages crashes on Windows with 4.8.0 release builds
#if (QT_VERSION >= 0x040801) || (QT_VERSION >= 0x040800 && !defined(Q_OS_WIN))
    uiLanguages = QLocale::system().uiLanguages();
#else
    // older versions need to fall back to the systems locale
    uiLanguages << QLocale::system().name();
#endif

    if (!enforcedLocale.isEmpty())
        uiLanguages.prepend(enforcedLocale);

    QTranslator *translator = new QTranslator(app);
    QTranslator *qtTranslator = new QTranslator(app);
    QTranslator *qtkeychainTranslator = new QTranslator(app);

    foreach (QString lang, uiLanguages) {
        lang.replace(QLatin1Char('-'), QLatin1Char('_')); // work around QTBUG-25973
        lang = substLang(lang);
        const QString trPath = applicationTrPath();
        const QString trFile = QLatin1String("client_") + lang;
        if (translator->load(trFile, trPath) || lang.startsWith(QLatin1String("en"))) {
            // Permissive approach: Qt and keychain translations
            // may be missing, but Qt translations must be there in order
            // for us to accept the language. Otherwise, we try with the next.
            // "en" is an exception as it is the default language and may not
            // have a translation file provided.
            qInfo() << "Using" << lang << "translation";
            app->setProperty("ui_lang", lang);
            const QString qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
            const QString qtTrFile = QLatin1String("qt_") + lang;
            const QString qtBaseTrFile = QLatin1String("qtbase_") + lang;
            if (!qtTranslator->load(qtTrFile, qtTrPath)) {
                if (!qtTranslator->load(qtTrFile, trPath)) {
                    if (!qtTranslator->load(qtBaseTrFile, qtTrPath)) {
                        qtTranslator->load(qtBaseTrFile, trPath);
                    }
                }
            }
            const QString qtkeychainTrFile = QLatin1String("qtkeychain_") + lang;
            if (!qtkeychainTranslator->load(qtkeychainTrFile, qtTrPath)) {
                qtkeychainTranslator->load(qtkeychainTrFile, trPath);
            }
            if (!translator->isEmpty())
                app->installTranslator(translator);
            if (!qtTranslator->isEmpty())
                app->installTranslator(qtTranslator);
            if (!qtkeychainTranslator->isEmpty())
                app->installTranslator(qtkeychainTranslator);
            break;
        }
        if (app->property("ui_lang").isNull())
            app->setProperty("ui_lang", "C");
    }
}

} // namespace OCC
