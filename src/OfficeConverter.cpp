#include "OfficeConverter.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

enum class OfficeType {
    None,
    LibreOffice,
    WPS,
    OnlyOffice,
    Unoconv,      // 基于 LibreOffice 的转换工具
    Pandoc,       // 通用文档转换工具
    TextExtract   // 纯文本提取（降级方案）
};

static OfficeType detectOfficeApp(QString *exePathOut = nullptr, QString *typeNameOut = nullptr) {
    // 1. 优先检测 unoconv（最稳定的转换工具）
    QString unoconv = QStandardPaths::findExecutable("unoconv");
    if (!unoconv.isEmpty()) {
        if (exePathOut) *exePathOut = unoconv;
        if (typeNameOut) *typeNameOut = "Unoconv";
        return OfficeType::Unoconv;
    }
    
    // 2. 检测 LibreOffice（开源免费，功能完整，已验证转换命令可用）
    QString soffice = QStandardPaths::findExecutable("soffice");
    if (soffice.isEmpty()) {
        soffice = QStandardPaths::findExecutable("libreoffice");
    }
    // 尝试常见的安装路径
    if (soffice.isEmpty()) {
        QStringList possiblePaths = {
            "/usr/bin/libreoffice",
            "/usr/bin/soffice",
            "/opt/libreoffice/program/soffice",
            "/snap/bin/libreoffice"
        };
        for (const QString& path : possiblePaths) {
            if (QFile::exists(path)) {
                soffice = path;
                break;
            }
        }
    }
    if (!soffice.isEmpty()) {
        if (exePathOut) *exePathOut = soffice;
        if (typeNameOut) *typeNameOut = "LibreOffice";
        return OfficeType::LibreOffice;
    }
    
    // 3. 检测 WPS（中国市场常用）
    QString wps = QStandardPaths::findExecutable("wps");
    if (wps.isEmpty()) {
        // Deepin/UOS 的 WPS 可能安装在 /opt/apps 目录
        wps = "/opt/apps/cn.wps.wps-office/files/bin/wps";
        if (!QFile::exists(wps)) {
            wps.clear();
        }
    }
    if (!wps.isEmpty()) {
        if (exePathOut) *exePathOut = wps;
        if (typeNameOut) *typeNameOut = "WPS Office";
        return OfficeType::WPS;
    }
    
    // 4. 检测 pandoc（通用文档转换）
    QString pandoc = QStandardPaths::findExecutable("pandoc");
    if (!pandoc.isEmpty()) {
        if (exePathOut) *exePathOut = pandoc;
        if (typeNameOut) *typeNameOut = "Pandoc";
        return OfficeType::Pandoc;
    }
    
    // 5. 检测 OnlyOffice
    QString onlyoffice = QStandardPaths::findExecutable("onlyoffice-desktopeditors");
    if (onlyoffice.isEmpty()) {
        onlyoffice = QStandardPaths::findExecutable("desktopeditors");
    }
    if (!onlyoffice.isEmpty()) {
        if (exePathOut) *exePathOut = onlyoffice;
        if (typeNameOut) *typeNameOut = "OnlyOffice";
        return OfficeType::OnlyOffice;
    }
    
    // 6. 检测文本提取工具（降级方案）
    QString docx2txt = QStandardPaths::findExecutable("docx2txt");
    if (!docx2txt.isEmpty()) {
        if (exePathOut) *exePathOut = docx2txt;
        if (typeNameOut) *typeNameOut = "Text Extract";
        return OfficeType::TextExtract;
    }
    
    return OfficeType::None;
}

QString OfficeConverter::cacheDir() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir d(base);
    if (!d.exists()) d.mkpath(".");
    const QString cache = base + "/office_cache";
    QDir c(cache);
    if (!c.exists()) c.mkpath(".");
    return cache;
}

QString OfficeConverter::hashedName(const QString &path) {
    QByteArray key = QFileInfo(path).absoluteFilePath().toUtf8();
    QByteArray hash = QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex();
    return QString::fromUtf8(hash);
}

bool OfficeConverter::convertToPdf(const QString &inputPath, QString &pdfPath, QString &errorMsg) {
    errorMsg.clear();
    pdfPath.clear();

    QFileInfo fi(inputPath);
    if (!fi.exists() || !fi.isFile()) {
        errorMsg = QObject::tr("输入文件不存在: %1").arg(inputPath);
        return false;
    }

    QString officeExe;
    QString officeName;
    OfficeType officeType = detectOfficeApp(&officeExe, &officeName);
    
    if (officeType == OfficeType::None) {
        errorMsg = QObject::tr("未检测到办公软件或转换工具，请安装以下任一软件：\n\n"
                              "【推荐方案】\n"
                              "• unoconv (最稳定): sudo apt install unoconv\n"
                              "• LibreOffice (开源): sudo apt install libreoffice\n\n"
                              "【其他方案】\n"
                              "• WPS Office (中国常用)\n"
                              "• Pandoc (通用转换): sudo apt install pandoc\n"
                              "• OnlyOffice (现代界面)");
        return false;
    }

    const QString outDir = cacheDir();
    const QString baseName = hashedName(inputPath) + ".pdf";
    const QString outPdf = outDir + "/" + baseName;

    // 若缓存存在且比源文件新，直接复用
    QFileInfo outInfo(outPdf);
    if (outInfo.exists() && outInfo.lastModified() >= fi.lastModified()) {
        pdfPath = outPdf;
        return true;
    }

    // 根据不同的办公软件使用不同的转换命令
    QProcess proc;
    QStringList args;
    
    // 对于 LibreOffice，添加环境变量优化
    if (officeType == OfficeType::LibreOffice) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        // 设置 LibreOffice 使用的显示为虚拟显示，避免窗口弹出
        env.insert("DISPLAY", ":99");
        proc.setProcessEnvironment(env);
    }
    
    switch (officeType) {
        case OfficeType::Unoconv:
            // unoconv 是最稳定的转换工具
            args << "-f" << "pdf" << "-o" << outPdf << fi.absoluteFilePath();
            break;
            
        case OfficeType::LibreOffice:
            // LibreOffice 标准转换命令（已验证可用）
            args << "--headless" << "--convert-to" << "pdf" 
                 << "--outdir" << outDir << fi.absoluteFilePath();
            break;
            
        case OfficeType::WPS:
            // WPS 命令行转换（如果支持）
            // 注意：WPS 的命令行转换功能可能需要专业版
            args << "--export-pdf" << fi.absoluteFilePath() << outPdf;
            break;
            
        case OfficeType::Pandoc:
            // Pandoc 通用文档转换
            args << fi.absoluteFilePath() << "-o" << outPdf;
            break;
            
        case OfficeType::OnlyOffice:
            // OnlyOffice 转换命令（如果支持命令行）
            args << "--convert-to" << "pdf" 
                 << "--output" << outPdf << fi.absoluteFilePath();
            break;
            
        case OfficeType::TextExtract:
            // 纯文本提取（降级方案）
            // 这里只是占位，实际需要特殊处理
            errorMsg = QObject::tr("仅支持文本提取，无法生成 PDF");
            return false;
            
        default:
            errorMsg = QObject::tr("不支持的办公软件类型");
            return false;
    }
    
    proc.setProgram(officeExe);
    proc.setArguments(args);
    proc.setProcessChannelMode(QProcess::MergedChannels);

    // 设定超时，避免卡死
    proc.start();
    if (!proc.waitForStarted(15000)) {
        errorMsg = QObject::tr("%1 启动失败").arg(officeName);
        return false;
    }
    if (!proc.waitForFinished(120000)) { // 最多等 120s
        proc.kill();
        errorMsg = QObject::tr("%1 转换超时").arg(officeName);
        return false;
    }

    const QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    
    // WPS 可能生成的文件名不同，尝试查找
    if (officeType == OfficeType::WPS && !QFile::exists(outPdf)) {
        // WPS 可能直接在当前目录生成，尝试移动
        QString wpsOutput = fi.absolutePath() + "/" + fi.completeBaseName() + ".pdf";
        if (QFile::exists(wpsOutput)) {
            QFile::rename(wpsOutput, outPdf);
        }
    }

    if (!QFile::exists(outPdf)) {
        errorMsg = QObject::tr("未生成 PDF，%1 可能不支持此文件格式: %2\n输出: %3")
                      .arg(officeName, fi.suffix(), output);
        return false;
    }

    pdfPath = outPdf;
    return true;
}

QString OfficeConverter::detectInstalledOffice() {
    QString officeName;
    detectOfficeApp(nullptr, &officeName);
    return officeName;
}

bool OfficeConverter::isOfficeDocument(const QString &filePath) {
    QString suffix = QFileInfo(filePath).suffix().toLower();
    QStringList officeExtensions = {
        // Microsoft Office
        "doc", "docx", "dot", "dotx",
        "xls", "xlsx", "xlt", "xltx",
        "ppt", "pptx", "pot", "potx",
        // LibreOffice/OpenOffice
        "odt", "ott", "ods", "ots", "odp", "otp",
        // 其他常见格式
        "rtf", "wps", "et", "dps"
    };
    return officeExtensions.contains(suffix);
}

bool OfficeConverter::extractTextContent(const QString &inputPath, QString &textContent, QString &errorMsg) {
    errorMsg.clear();
    textContent.clear();
    
    QFileInfo fi(inputPath);
    if (!fi.exists() || !fi.isFile()) {
        errorMsg = QObject::tr("输入文件不存在: %1").arg(inputPath);
        return false;
    }
    
    QString suffix = fi.suffix().toLower();
    
    // 对于 docx/xlsx/pptx 等基于 ZIP 的文档，尝试提取文本
    QStringList zipBasedFormats = {"docx", "xlsx", "pptx", "odt", "ods", "odp"};
    if (zipBasedFormats.contains(suffix)) {
        // 尝试使用 unzip 提取文档中的文本内容
        QProcess proc;
        QStringList args;
        args << "-p" << fi.absoluteFilePath() << "word/document.xml";
        
        if (suffix == "docx" || suffix == "odt") {
            args[1] = (suffix == "docx") ? "word/document.xml" : "content.xml";
        } else if (suffix == "xlsx" || suffix == "ods") {
            args[1] = (suffix == "xlsx") ? "xl/sharedStrings.xml" : "content.xml";
        } else if (suffix == "pptx" || suffix == "odp") {
            args[1] = (suffix == "pptx") ? "ppt/slides/slide1.xml" : "content.xml";
        }
        
        proc.start("unzip", args);
        if (proc.waitForFinished(10000)) {
            QString xmlContent = QString::fromUtf8(proc.readAllStandardOutput());
            // 简单的 XML 文本提取（移除标签）
            textContent = xmlContent;
            textContent.remove(QRegularExpression("<[^>]*>"));
            textContent = textContent.simplified();
            
            if (!textContent.isEmpty() && textContent.length() > 10) {
                return true;
            }
        }
    }
    
    // 尝试使用 strings 命令提取可读文本
    QProcess stringsProc;
    stringsProc.start("strings", {fi.absoluteFilePath()});
    if (stringsProc.waitForFinished(10000)) {
        QString output = QString::fromLocal8Bit(stringsProc.readAllStandardOutput());
        QStringList lines = output.split('\n');
        
        // 过滤出有意义的内容
        QStringList meaningfulLines;
        for (const QString& line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.length() > 3 && !trimmed.contains(QRegularExpression("[\\x00-\\x1F]"))) {
                meaningfulLines.append(trimmed);
            }
        }
        
        if (!meaningfulLines.isEmpty()) {
            textContent = meaningfulLines.join('\n');
            return true;
        }
    }
    
    errorMsg = QObject::tr("无法提取文件文本内容");
    return false;
}

void OfficeConverter::clearCache() {
    QString cache = cacheDir();
    QDir cacheDir(cache);
    if (cacheDir.exists()) {
        cacheDir.removeRecursively();
        cacheDir.mkpath(".");
    }
}
