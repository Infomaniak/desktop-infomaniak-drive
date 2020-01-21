namespace SettingsDialogCommon
{

/** display name with two lines that is displayed in the settings
 * If width is bigger than 0, the string will be ellided so it does not exceed that width
 */
QString shortDisplayNameForSettings(Account* account, int width)
{
    QString drive = account->driveName();
    if (width > 0) {
        QFont f;
        QFontMetrics fm(f);
        drive = fm.elidedText(drive, Qt::ElideMiddle, width);
    }
    return drive;
}

}
