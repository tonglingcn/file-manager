#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    // 必须在创建 QApplication 之前设置 HighDPI 策略
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    QApplication::setApplicationName("FileManagerPreview");
    QApplication::setOrganizationName("Local");
    
    // 设置应用图标
    QIcon appIcon(":/icons/icons/app-logo.svg");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }

    MainWindow w;
    w.resize(1200, 720);
    w.setWindowTitle("文件管理器预览");
    w.show();

    return app.exec();
}
