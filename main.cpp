#include "mainwindow.h"
#include "common.h"

#include <QApplication>
#include <QStyle>

QSettings *mySettings = new QSettings("Midi_Controller", "Midi_Controller");

#ifdef QT_DEBUG
    const QString logFilePath = "debug.log";
    bool logToFile = false;
#else
    #ifdef __WINDOWS_MM__
        const QString logFilePath = "debug.log";
        bool logToFile = true;
    #endif
    #ifdef __MACOSX_CORE__
        const QString logFilePath = "/Users/Shared/debug.log";
        bool logToFile = true;
    #endif
#endif

void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
    QByteArray localMsg = msg.toLocal8Bit();
    QDateTime time = QDateTime::currentDateTime();
    QString formattedTime = time.toString("dd/MM/yyyy hh:mm:ss");
    QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
    QString logLevelName = msgLevelHash[type];
    QByteArray logLevelMsg = logLevelName.toLocal8Bit();

    if (logToFile) {
        QString txt = QString("%1 >> %2: %3").arg(formattedTime, logLevelName, msg);
        QFile outFile(logFilePath);
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << txt << endl;
        outFile.close();
    } else {
        fprintf(stderr, "%s\n", localMsg.constData());
        fflush(stderr);
    }

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(customMessageOutput); // custom message handler for debugging

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Midi_Controller");
    QCoreApplication::setOrganizationDomain("Midi_Controller.fr");
    QCoreApplication::setApplicationName("Midi_Controller");
    app.setApplicationName("Midi_Controller");
    app.setWindowIcon(QIcon(ICON_WINDOW));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(darkPalette);

    //app.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    qInfo() << "Lancement du programme";

    MainWindow w;
    w.show();

    return app.exec();
}
