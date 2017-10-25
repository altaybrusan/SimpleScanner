#include "utils.h"
#include "utils.h"
#include <QString>
#include <QDebug>
#include <QFileInfo>

namespace SimpleScanner
{
    Utils::Utils()
    {

    }

    // check if path exists and if so,
    // is it really a file or not?
    bool Utils::FileExists(const QString &path)
    {
        QFileInfo check_file(path);
        return check_file.exists() && check_file.isFile();
    }

}

