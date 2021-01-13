#pragma once

#include "libcommon/commonlib.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QString>

namespace OCC {

Q_DECLARE_LOGGING_CATEGORY(lcCommonUtility)

namespace CommonUtility {
    COMMON_EXPORT void setupTranslations(QCoreApplication *app, const QString &enforcedLocale = QString());

    // Color threshold check
    COMMON_EXPORT bool colorThresholdCheck(int red, int green, int blue);
}

} // namespace OCC
