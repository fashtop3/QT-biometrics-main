#ifndef FPTPATH_H
#define FPTPATH_H
#include <QDir>
#include <QCoreApplication>

//this path was hidden with this command
// attrib +s +h "./.fpt"
//to unhide use
//attrib -s -h "./.fpt"


#ifndef TEMP_PATH
static QString _FPT_PATH_()
{
    return QString(QCoreApplication::applicationDirPath() + "/.fpt");
}

static QString getFPTFilePath(const QString &str)
{
    return QCoreApplication::applicationDirPath() + "/.fpt/" + str;
}

static QString getFPTTempFilePath(const QString &str)
{
    return QCoreApplication::applicationDirPath() + "/.fpt/temp/" + str;
}
#endif

#endif // FPTPATH_H
