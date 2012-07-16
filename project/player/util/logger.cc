// logger.cc
// 10/16/2011
#include "logger.h"
#include "osdconsole.h"
#include "module/qtext/htmltag.h"
#include <QtCore>

enum { LEFT_LENGTH = 100, RIGHT_LENGTH = 0 };

void
Logger::log(const QString &message)
{
  if (!OsdConsole::globalInstance())
    return;
  if (message.size() < LEFT_LENGTH + RIGHT_LENGTH)
    gConsole().append(
      //HTML_STYLE_OPEN(color:blue)
      ": "
      + message +
      //HTML_STYLE_CLOSE()
      HTML_BR()
    );
  else
    gConsole().append(
      //HTML_STYLE_OPEN(color:blue)
      ": "
      + message.left(LEFT_LENGTH) +
      HTML_STYLE_OPEN(color:orange) " ..." HTML_STYLE_CLOSE()
      //+ message.right(RIGHT_LENGTH) +
      //HTML_STYLE_CLOSE()
      HTML_BR()
    );
  LoggerSignals::globalInstance()->showMessage(
    //HTML_STYLE_OPEN(color:blue) ": " +
    message +
    //HTML_STYLE_CLOSE()
    HTML_BR()
  );
}

void
Logger::notify(const QString &message)
{
  if (!OsdConsole::globalInstance())
    return;
  if (message.size() < LEFT_LENGTH + RIGHT_LENGTH)
    gConsole().append(
      HTML_STYLE_OPEN(color:cyan) ": "
      + message +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  else
    gConsole().append(
      HTML_STYLE_OPEN(color:cyan) ": "
      + message.left(LEFT_LENGTH) +
      HTML_STYLE_OPEN(color:orange) " ..." HTML_STYLE_CLOSE()
      //+ message.right(RIGHT_LENGTH) +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  LoggerSignals::globalInstance()->showNotification(
    HTML_STYLE_OPEN(color:cyan) ": "
    + message +
    HTML_STYLE_CLOSE() HTML_BR()
  );
}

void
Logger::warn(const QString &message)
{
  if (!OsdConsole::globalInstance())
    return;
  if (message.size() < LEFT_LENGTH + RIGHT_LENGTH)
    gConsole().append(
      HTML_STYLE_OPEN(color:cyan)
      ": " + message +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  else
    gConsole().append(
      HTML_STYLE_OPEN(color:cyan) ": "
      + message.left(LEFT_LENGTH) +
      HTML_STYLE_OPEN(color:orange) " ..." HTML_STYLE_CLOSE()
      //+ message.right(RIGHT_LENGTH) +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  LoggerSignals::globalInstance()->showWarning(
    HTML_STYLE_OPEN(color:cyan) ": "
    + message +
    HTML_STYLE_CLOSE() HTML_BR()
  );
}

void
Logger::error(const QString &message)
{
  if (!OsdConsole::globalInstance())
    return;
  if (message.size() < LEFT_LENGTH + RIGHT_LENGTH)
    gConsole().append(
      HTML_STYLE_OPEN(color:red) ": "
      + message +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  else
    gConsole().append(
      HTML_STYLE_OPEN(color:red) ": "
      + message.left(LEFT_LENGTH) +
      HTML_STYLE_OPEN(color:orange) " ..." HTML_STYLE_CLOSE()
      //+ message.right(RIGHT_LENGTH) +
      HTML_STYLE_CLOSE() HTML_BR()
    );
  LoggerSignals::globalInstance()->showError(
    HTML_STYLE_OPEN(color:red) ": "
    + message +
    HTML_STYLE_CLOSE() HTML_BR()
  );
}

// EOF
