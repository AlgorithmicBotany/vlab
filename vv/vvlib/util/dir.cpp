#include <util/dir.hpp>

#include <QDir>

namespace util
{
  std::string absoluteDir( std::string filename )
    {
    QDir dir(QString::fromStdString(filename));
    return dir.absolutePath().toStdString();
    }
}

