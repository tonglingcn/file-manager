#include "TextPreviewer.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>

TextPreviewer::TextPreviewer(QWidget *parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    setWordWrapMode(QTextOption::NoWrap);
}

bool TextPreviewer::loadText(const QString &path) {
    QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) return false;

    // Limit large files to avoid freezing (e.g., 5 MB)
    constexpr qint64 MAX_SIZE = 5 * 1024 * 1024;
    if (fi.size() > MAX_SIZE) {
        setPlainText(QString::fromUtf8("文件过大（>%1 MB），不支持文本预览：\n%2").arg(MAX_SIZE / (1024*1024)).arg(path));
        return true;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&f);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif
    const QString content = in.readAll();
    setPlainText(content);
    document()->setDefaultFont(QFont("Monospace"));
    return true;
}
