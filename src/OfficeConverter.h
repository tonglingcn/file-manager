#ifndef OFFICECONVERTER_H
#define OFFICECONVERTER_H

#include <QString>

class OfficeConverter {
public:
    // 将办公文件转换为 PDF；成功返回 true，并输出 pdfPath；失败返回 false，并输出错误信息
    static bool convertToPdf(const QString &inputPath, QString &pdfPath, QString &errorMsg);
    
    // 检测已安装的办公软件，返回软件名称（如 "WPS Office"、"LibreOffice" 等），未检测到返回空字符串
    static QString detectInstalledOffice();

private:
    static QString cacheDir();
    static QString hashedName(const QString &path);
};

#endif // OFFICECONVERTER_H
