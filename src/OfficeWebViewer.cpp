#include "OfficeWebViewer.h"
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QTimer>

OfficeWebViewer::OfficeWebViewer(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

OfficeWebViewer::~OfficeWebViewer() = default;

void OfficeWebViewer::setupUI() {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // 状态标签
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #E3F2FD; color: #1976D2; }");
    m_statusLabel->setVisible(false);
    m_layout->addWidget(m_statusLabel);

    // WebEngine 视图
    m_webView = new QWebEngineView(this);
    m_layout->addWidget(m_webView);
}

bool OfficeWebViewer::loadDocument(const QString &filePath) {
    QFileInfo fi(filePath);
    if (!fi.exists()) {
        m_webView->setHtml(generateErrorHtml(tr("文件不存在: %1").arg(filePath)));
        return false;
    }

    m_currentFile = filePath;
    QString ext = fi.suffix().toLower();
    QString html;

    m_statusLabel->setText(tr("正在加载文档..."));
    m_statusLabel->setVisible(true);

    // 根据文件类型转换
    if (ext == "docx") {
        html = convertDocxToHtml(filePath);
    } else if (ext == "xlsx" || ext == "xls") {
        html = convertXlsxToHtml(filePath);
    } else if (ext == "doc" || ext == "ppt" || ext == "pptx") {
        // 这些格式需要更复杂的处理
        html = generateErrorHtml(tr("暂不支持 %1 格式的直接预览\n\n建议：\n1. 安装 LibreOffice 使用 PDF 转换预览\n2. 或使用对应的办公软件打开").arg(ext.toUpper()));
    } else {
        html = generateErrorHtml(tr("不支持的文件格式: %1").arg(ext));
    }

    if (html.isEmpty()) {
        m_statusLabel->setVisible(false);
        return false;
    }

    m_webView->setHtml(html);
    m_statusLabel->setText(tr("文档加载完成: %1").arg(fi.fileName()));
    
    // 3秒后隐藏状态标签
    QTimer::singleShot(3000, this, [this]() {
        m_statusLabel->setVisible(false);
    });

    return true;
}

QString OfficeWebViewer::convertDocxToHtml(const QString &filePath) {
    // 方案1：使用 pandoc 转换
    QString pandoc = QStandardPaths::findExecutable("pandoc");
    if (!pandoc.isEmpty()) {
        QProcess proc;
        proc.setProgram(pandoc);
        proc.setArguments({filePath, "-t", "html", "--standalone"});
        proc.start();
        
        if (proc.waitForFinished(30000)) {
            QString html = QString::fromUtf8(proc.readAllStandardOutput());
            if (!html.isEmpty()) {
                return html;
            }
        }
    }

    // 方案2：使用 python-docx 提取文本
    QString python = QStandardPaths::findExecutable("python3");
    if (python.isEmpty()) {
        python = QStandardPaths::findExecutable("python");
    }
    
    if (!python.isEmpty()) {
        // 创建临时 Python 脚本
        QString script = R"(
import sys
try:
    from docx import Document
    doc = Document(sys.argv[1])
    print('<html><head><meta charset="utf-8"><style>')
    print('body { font-family: Arial, sans-serif; padding: 20px; max-width: 800px; margin: 0 auto; }')
    print('p { margin: 10px 0; line-height: 1.6; }')
    print('h1, h2, h3 { color: #333; margin-top: 20px; }')
    print('</style></head><body>')
    for para in doc.paragraphs:
        text = para.text.strip()
        if text:
            style = para.style.name.lower()
            if 'heading 1' in style:
                print(f'<h1>{text}</h1>')
            elif 'heading 2' in style:
                print(f'<h2>{text}</h2>')
            elif 'heading 3' in style:
                print(f'<h3>{text}</h3>')
            else:
                print(f'<p>{text}</p>')
    print('</body></html>')
except ImportError:
    print('<html><body><h2>需要安装 python-docx</h2><p>运行: pip3 install python-docx</p></body></html>')
except Exception as e:
    print(f'<html><body><h2>错误</h2><p>{str(e)}</p></body></html>')
)";

        QProcess proc;
        proc.setProgram(python);
        proc.setArguments({"-c", script, filePath});
        proc.start();
        
        if (proc.waitForFinished(30000)) {
            QString html = QString::fromUtf8(proc.readAllStandardOutput());
            if (!html.isEmpty() && html.contains("<html>")) {
                return html;
            }
        }
    }

    // 方案3：使用 docx2txt
    QString docx2txt = QStandardPaths::findExecutable("docx2txt");
    if (!docx2txt.isEmpty()) {
        QProcess proc;
        proc.setProgram(docx2txt);
        proc.setArguments({filePath, "-"});
        proc.start();
        
        if (proc.waitForFinished(30000)) {
            QString text = QString::fromUtf8(proc.readAllStandardOutput());
            if (!text.isEmpty()) {
                text = text.replace("<", "&lt;").replace(">", "&gt;");
                text = text.replace("\n", "<br>");
                return QString("<html><head><meta charset='utf-8'><style>body{font-family:Arial;padding:20px;max-width:800px;margin:0 auto;}</style></head><body><pre style='white-space:pre-wrap;'>%1</pre></body></html>").arg(text);
            }
        }
    }

    return generateErrorHtml(tr("无法转换 DOCX 文件\n\n请安装以下工具之一：\n• pandoc: sudo apt install pandoc\n• python-docx: pip3 install python-docx\n• docx2txt: sudo apt install docx2txt"));
}

QString OfficeWebViewer::convertXlsxToHtml(const QString &filePath) {
    // 使用 Python openpyxl 或 pandas 读取 Excel
    QString python = QStandardPaths::findExecutable("python3");
    if (python.isEmpty()) {
        python = QStandardPaths::findExecutable("python");
    }
    
    if (!python.isEmpty()) {
        QString script = R"(
import sys
try:
    import openpyxl
    wb = openpyxl.load_workbook(sys.argv[1], data_only=True)
    print('<html><head><meta charset="utf-8"><style>')
    print('body { font-family: Arial, sans-serif; padding: 20px; }')
    print('table { border-collapse: collapse; margin: 20px 0; }')
    print('th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }')
    print('th { background-color: #4CAF50; color: white; }')
    print('tr:nth-child(even) { background-color: #f2f2f2; }')
    print('.sheet-name { font-size: 18px; font-weight: bold; margin: 20px 0 10px 0; color: #333; }')
    print('</style></head><body>')
    
    for sheet_name in wb.sheetnames:
        sheet = wb[sheet_name]
        print(f'<div class="sheet-name">工作表: {sheet_name}</div>')
        print('<table>')
        
        for i, row in enumerate(sheet.iter_rows(values_only=True)):
            if i == 0:
                print('<tr>')
                for cell in row:
                    print(f'<th>{cell if cell is not None else ""}</th>')
                print('</tr>')
            else:
                print('<tr>')
                for cell in row:
                    print(f'<td>{cell if cell is not None else ""}</td>')
                print('</tr>')
            
            if i > 100:  # 限制显示行数
                print('<tr><td colspan="100">... (仅显示前100行)</td></tr>')
                break
        
        print('</table>')
    
    print('</body></html>')
except ImportError:
    print('<html><body><h2>需要安装 openpyxl</h2><p>运行: pip3 install openpyxl</p></body></html>')
except Exception as e:
    print(f'<html><body><h2>错误</h2><p>{str(e)}</p></body></html>')
)";

        QProcess proc;
        proc.setProgram(python);
        proc.setArguments({"-c", script, filePath});
        proc.start();
        
        if (proc.waitForFinished(30000)) {
            QString html = QString::fromUtf8(proc.readAllStandardOutput());
            if (!html.isEmpty() && html.contains("<html>")) {
                return html;
            }
        }
    }

    return generateErrorHtml(tr("无法转换 Excel 文件\n\n请安装：\npip3 install openpyxl"));
}

QString OfficeWebViewer::generateErrorHtml(const QString &message) {
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        }
        .error-box {
            background: white;
            padding: 40px;
            border-radius: 10px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            max-width: 500px;
            text-align: center;
        }
        .error-icon {
            font-size: 64px;
            margin-bottom: 20px;
        }
        h2 {
            color: #333;
            margin: 0 0 20px 0;
        }
        p {
            color: #666;
            line-height: 1.6;
            white-space: pre-line;
        }
    </style>
</head>
<body>
    <div class="error-box">
        <div class="error-icon">📄</div>
        <h2>无法预览文档</h2>
        <p>%1</p>
    </div>
</body>
</html>
)";
    return html.arg(message);
}
