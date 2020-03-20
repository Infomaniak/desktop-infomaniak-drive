#pragma once

#include "libcommon/commonlib.h"

#include <QLoggingCategory>
#include <QString>
#include <QCoreApplication>

namespace OCC {

Q_DECLARE_LOGGING_CATEGORY(lcCommonUtility)

namespace CommonUtility {
    COMMON_EXPORT void setupTranslations(QCoreApplication *app, const QString &enforcedLocale = QString());
}

} // namespace OCC
