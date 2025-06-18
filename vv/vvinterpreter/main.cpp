#include <cstdlib>
#include <iostream>
#include <string>
#include <QApplication>
//#include <qdesktopwidget.h>
#include <util/clamp.hpp>
#include "vvpapp.hpp"

#include <QString>
#include <QTextStream>
#include <QDir>
#include <QTimer>
#include <QProcess>
#include <QStyle>
#include <QScreen>

#ifdef __APPLE__
#include "CoreFoundation/CFBundle.h"
#include "CoreFoundation/CFURL.h"
#include <Carbon/Carbon.h>

QString urlToString(CFURLRef url)
{
  CFStringRef macPath = CFURLCopyFileSystemPath(url,
                                                kCFURLPOSIXPathStyle);
  const char *pathPtr = CFStringGetCStringPtr(macPath,
                                              CFStringGetSystemEncoding());
  QString result(pathPtr);
  CFRelease(macPath);
  return result;
}

QString findBundlePath()
{
  CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  QString result = urlToString(appUrlRef);
  CFRelease(appUrlRef);
  return result;
}

QString findResourcesPath()
{
  return findBundlePath() + "/Contents/Resources";
}

QString findQtPath()
{
  QString bp = findBundlePath();
  QDir bundle_path(bp);
  if(bundle_path.cd("Contents"))
  {
    if(bundle_path.cd("MacOS"))
    {
      QString prg = bundle_path.filePath("vvinterpreter");
      QProcess proc;
      QStringList args;
      args << "-L" << prg;
      proc.start("otool", args);
      proc.waitForFinished();
      QString result = QString::fromLocal8Bit(proc.readAllStandardOutput());
      QStringList lines = result.split("\n");
      QStringList qtcore = lines.filter(QRegExp(".*QtCore.*"));
      if(!qtcore.empty())
      {
        QString lib = qtcore[0].split(" ")[0].trimmed();
        QRegExp findPath = QRegExp("(.*)/QtCore\\.framework.*");
        if(findPath.exactMatch(lib))
        {
          QString pth = findPath.capturedTexts()[1];
          if(pth.startsWith("@executable_path/.."))
          {
            QDir fm_path(bp);
            fm_path.cd("Contents");
            QString qtdir = fm_path.filePath(pth.mid(QString("@executable_path/../").size()));
            return qtdir;
          }
        }
      }
    }
  }
  // Use qmake
  QProcess proc;
  QStringList args;
  args << "-query" << "QT_INSTALL_LIBS";
  proc.start("qmake", args);
  proc.waitForFinished();
  QString result = QString::fromLocal8Bit(proc.readAllStandardOutput());
  return result.trimmed();
}

void OpenHelpBook()
{
  QTextStream cerr(stderr);
  CFBundleRef myApplicationBundle = NULL;
  CFTypeRef myBookName = NULL;
  OSStatus err = noErr;

  myApplicationBundle = CFBundleGetMainBundle();
  if (myApplicationBundle == NULL)
  {
    err = fnfErr;
    cerr << "Cannot find application bundle" << Qt::endl;
    return;
  }

  myBookName = CFBundleGetValueForInfoDictionaryKey( myApplicationBundle,
                                                     CFSTR("CFBundleHelpBookName"));
  if (myBookName == NULL)
  {
    err = fnfErr;
    cerr << "Cannot find help book name" << Qt::endl;
    return;
  }

  if (CFGetTypeID(myBookName) != CFStringGetTypeID())
  {
    err = paramErr;
    cerr << "Bookname doesn't contain a string" << Qt::endl;
    return;
  }
  AHGotoPage((CFStringRef)myBookName, NULL, NULL);
}

QString getRessourceDirectory()
{
  CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFURLRef resUrlRef = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());

  QString result = QDir(urlToString(appUrlRef)).filePath(urlToString(resUrlRef));
  CFRelease(appUrlRef);
  CFRelease(resUrlRef);
  return result;
}

QString getHelpDirectory()
{
  QDir resource(getRessourceDirectory());
  resource.cd("VV.help");
  resource.cd("Contents");
  resource.cd("Resources");
  return resource.filePath("English.lproj");
}


#else

QString findResourcesPath()
{
  QDir libdir = QString(getenv("VVDIR"));
  if(!libdir.cd("share"))
  {
    fprintf(stderr, "Error, no directory $VVDIR/share");
    exit(-2);
  }
  if(!libdir.cd("vv"))
  {
    fprintf(stderr, "Error, no directory $VVDIR/share/vv");
    exit(-2);
  }
  return libdir.absolutePath();
}

QString getHelpDirectory()
{
  QDir resource(findResourcesPath());
  resource.cd("doc");
  return resource.filePath("VVHelp");
}

QString findQtPath()
{
  QString prg = QCoreApplication::applicationFilePath();
  QProcess proc;
  QStringList args;
  args << prg;
  proc.start("ldd", args);
  proc.waitForFinished();
  QString result = QString::fromLocal8Bit(proc.readAllStandardOutput());
  QStringList lines = result.split("\n");
  QStringList qtcore = lines.filter(QRegExp(".*QtCore.*"));
  if(!qtcore.empty())
  {
    QString lib = qtcore[0].split(" ")[0].trimmed();
    QRegExp findPath = QRegExp("=> (.*)/libQtCore.* \\(0x.*\\)");
    if(findPath.exactMatch(lib))
    {
      QString pth = findPath.capturedTexts()[1];
      QDir p(pth);
      if(p.exists())
        return pth;
    }
  }
  // Use qmake
  args.clear();
  args << "-query" << "QT_INSTALL_LIBS";
  proc.start("qmake", args);
  proc.waitForFinished();
  result = QString::fromLocal8Bit(proc.readAllStandardOutput());
  return result.trimmed();
}

#endif

#define MAKEFILE "program.mk"

int findMakefile()
{
  QString resources = findResourcesPath();
  QDir d(resources);
  if(d.exists(MAKEFILE))
  {
    QTextStream out(stdout);
    out << d.absoluteFilePath(MAKEFILE) << Qt::endl;
    return 0;
  }
  QTextStream err(stderr);
  err << "Cannot find " MAKEFILE " in '" << resources << "'" << Qt::endl;
  return 2;
}

static void usage (const std::string & progname) {
  std::cout << "Usage: " << progname << "[options] filename...\n"
    << "Options:\n"
    << "  --help         Displays this help.\n"
    << "  -d             Turns on debuggin output.\n"
    << "  -nogui         No gui.\n"
    << "  -size w h      Specifies size of the main window as a percentage of the\n"
    << "                 screen size, e.g. -size 0.5 0.25\n"
    << "  -pos x y       Specifies the left upper corner of the window in relative\n"
    << "                 coordinates w.r.t. the screen size.\n"
    << "  -glsize w h    Specifies the size of the opengl widget in pixels (useful\n"
    << "                 useful for creating animations).\n"
    << "  -max           Maximizes the window.\n"
    << "  -min           Minimizes the window.\n"
    << "  -stats         ???\n"
    << "  -animate       Start in animate mode\n"
    << "  -makefile      Output the path to the default makefile\n"
#ifdef __APPLE__
    << "  -vvhelp        Open the helkp on VV\n"
#endif
    ;
}

int main(int argc, char** argv) {
  bool gui = true;

  unsigned int model_index = 0;

  double height = 0.33, width = 0.25;
  double x = 0.3, y = 0.3;
  bool maximise = false, minimise = false;
  bool relsize = false, glsize = false;
  bool debug = false;
  bool stats = false;
  bool animate = false;
  bool position = false;

  for(int i = 0 ; i < argc ; ++i)
  {
    if(QString("-b") == argv[i] or QString("--batch") == argv[i]
       or QString("-makefile") == argv[i] or QString("-m") == argv[i]
       or QString("--help") == argv[i] or QString("-h") == argv[i] or QString("-help") == argv[i])
    {
      gui = false;
      break;
    }
  }

  QCoreApplication *qapp;
  if(!gui)
    qapp = new QCoreApplication(argc, argv);
  else
    qapp = new QApplication(argc, argv);

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "-help" || arg == "--help") {
      usage (argv[0]);
      return 0;
    }
    else if (arg == "-d") {
      debug = true;
    }
    else if (arg == "-nogui") {
      gui = false;
    }
    else if (arg == "-size") {
      if (i + 2 < argc) {
        relsize = true;
        width = util::clamp(std::atof(argv[i + 1]), 0.0, 1.0);
        height = util::clamp(std::atof(argv[i + 2]), 0.0, 1.0);
        i += 2;
      }
    }
    else if (arg == "-glsize") {
      if (i + 2 < argc) {
        glsize = true;
        width = std::atof(argv[i + 1]);
        height = std::atof(argv[i + 2]);
        i += 2;
      }
    }
    else if (arg == "-pos") {
      if (i + 2 < argc) {
        position = true;
        x = util::clamp(std::atof(argv[i + 1]), 0.0, 1.0);
        y = util::clamp(std::atof(argv[i + 2]), 0.0, 1.0);
        i += 2;
      }
    }
    else if (arg == "-max") {
      maximise = true;
    }
    else if (arg == "-min") {
      minimise = true;
    }
    else if (arg == "-stats") {
      stats = true;
    }
    else if (arg == "-animate") {
      animate = true;
    }
    else if (arg == "-makefile") {
      return findMakefile();
    }
    else if (arg == "-resources") {
      QTextStream out(stdout);
      out << findResourcesPath() << Qt::endl;
      return 0;
    }
    else if (arg == "-qtdir") {
      QTextStream out(stdout);
      out << findQtPath() << Qt::endl;
      return 0;
    }
#ifdef __APPLE__
    else if(arg == "-vvhelp") {
      OpenHelpBook();
      return 0;
    }
#endif
    else {
      //fprintf(stderr, "Argument %d: '%s'\n", i, argv[i]);
      model_index = i;
    }
  }

  // First
  //QApplication qapp(argc, argv);

  VVPApp* vvpapp = new VVPApp();

  if(debug)
    vvpapp->debug();

  if(!gui)
    vvpapp->nogui();

  if(stats)
    vvpapp->stats();

  if(animate)
    vvpapp->startanimate();


#ifdef VVSTATIC
  vvpapp->open( "" );
#else
  if (model_index and argv[model_index]) {
    vvpapp->open(argv[model_index]);
  }
#endif

  if (!gui) {
    std::exit(0);
  }

  QScreen *screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  int screenHeight = screenGeometry.height();
  int screenWidth = screenGeometry.width();

  if (glsize && relsize) {
    std::cerr << "Warning: both -size and -glsize specified. Using -glsize\n";
  }
  //QSize size(640,480); // default window size
  //if (relsize) {
  //  size.setWidth(int(width * screenWidth));
  //  size.setHeight(int(height * screenHeight));
  //}
  // default window size is half the screen
  QSize size(int(width * screenWidth),int(height * screenHeight)); 
  if (glsize) {
    size.setWidth(int(width));
    size.setHeight(int(height));
  }
  // center the window on the screen
  vvpapp->setGeometry(
    QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size, screen->availableGeometry())
  );

  if (position) {
    int xp = int(x * screenWidth);
    int yp = int(y * screenHeight);
    vvpapp->move(xp,yp);
  }
    
  vvpapp->show();
  QTimer::singleShot(0, vvpapp, SLOT(raise()));
  if (maximise) vvpapp->showMaximized();
  if (minimise) vvpapp->showMinimized();
  

  int result = qapp->exec();
  delete qapp;
  return result;
}

