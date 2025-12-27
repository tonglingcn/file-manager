#ifndef TEXTPREVIEWER_H
#define TEXTPREVIEWER_H

#include <QPlainTextEdit>

class TextPreviewer : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit TextPreviewer(QWidget *parent = nullptr);
    bool loadText(const QString &path);
};

#endif // TEXTPREVIEWER_H
