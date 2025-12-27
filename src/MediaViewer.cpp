#include "MediaViewer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFileInfo>
#include <QStyle>

MediaViewer::MediaViewer(QWidget *parent) : QWidget(parent) {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 视频显示区域
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("QVideoWidget { background-color: black; }");
    mainLayout->addWidget(m_videoWidget, 1);

    // 音频信息面板（当播放音频时显示）
    m_audioPanel = new QWidget(this);
    m_audioPanel->setStyleSheet(
        "QWidget { "
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "    stop:0 #667eea, stop:1 #764ba2);"
        "}"
    );
    auto *audioLayout = new QVBoxLayout(m_audioPanel);
    audioLayout->setContentsMargins(40, 40, 40, 40);
    audioLayout->setSpacing(20);
    audioLayout->setAlignment(Qt::AlignCenter);
    
    // 音乐图标
    m_iconLabel = new QLabel(m_audioPanel);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setStyleSheet(
        "QLabel { "
        "  background-color: rgba(255, 255, 255, 0.15); "
        "  border-radius: 80px; "
        "  padding: 30px; "
        "}"
    );
    m_iconLabel->setFixedSize(160, 160);
    
    // 加载 SVG 图标
    QPixmap musicIcon(":/icons/icons/music-note.svg");
    if (!musicIcon.isNull()) {
        m_iconLabel->setPixmap(musicIcon.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    audioLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    
    // 文件名
    m_titleLabel = new QLabel(m_audioPanel);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "QLabel { "
        "  color: white; "
        "  font-size: 24px; "
        "  font-weight: bold; "
        "  background: transparent; "
        "}"
    );
    m_titleLabel->setWordWrap(true);
    audioLayout->addWidget(m_titleLabel);
    
    // 文件信息
    m_infoLabel = new QLabel(m_audioPanel);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet(
        "QLabel { "
        "  color: rgba(255, 255, 255, 0.8); "
        "  font-size: 14px; "
        "  background: transparent; "
        "}"
    );
    audioLayout->addWidget(m_infoLabel);
    
    audioLayout->addStretch();
    m_audioPanel->hide();
    mainLayout->addWidget(m_audioPanel, 1);

    // 控制面板
    m_controlPanel = new QWidget(this);
    m_controlPanel->setStyleSheet(
        "QWidget { "
        "  background-color: rgba(52, 73, 94, 0.95); "
        "  border-top: 1px solid rgba(255, 255, 255, 0.1); "
        "}"
    );
    auto *controlLayout = new QVBoxLayout(m_controlPanel);
    controlLayout->setContentsMargins(15, 12, 15, 12);
    controlLayout->setSpacing(10);

    // 进度条
    m_positionSlider = new QSlider(Qt::Horizontal, m_controlPanel);
    m_positionSlider->setRange(0, 0);
    m_positionSlider->setStyleSheet(
        "QSlider::groove:horizontal { "
        "  height: 6px; "
        "  background: rgba(255, 255, 255, 0.2); "
        "  border-radius: 3px; "
        "} "
        "QSlider::handle:horizontal { "
        "  background: white; "
        "  width: 16px; "
        "  height: 16px; "
        "  margin: -5px 0; "
        "  border-radius: 8px; "
        "} "
        "QSlider::sub-page:horizontal { "
        "  background: #3498DB; "
        "  border-radius: 3px; "
        "}"
    );
    controlLayout->addWidget(m_positionSlider);

    // 底部控制栏
    auto *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);

    // 播放/暂停按钮
    m_btnPlayPause = new QPushButton(m_controlPanel);
    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_btnPlayPause->setFixedSize(44, 44);
    m_btnPlayPause->setStyleSheet(
        "QPushButton { "
        "  background-color: #3498DB; "
        "  border: none; "
        "  border-radius: 22px; "
        "  padding: 8px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #2980B9; "
        "} "
        "QPushButton:pressed { "
        "  background-color: #21618C; "
        "}"
    );
    bottomLayout->addWidget(m_btnPlayPause);

    // 时间标签
    m_timeLabel = new QLabel("00:00 / 00:00", m_controlPanel);
    m_timeLabel->setStyleSheet("QLabel { color: white; font-size: 14px; font-weight: 500; }");
    m_timeLabel->setMinimumWidth(130);
    bottomLayout->addWidget(m_timeLabel);

    bottomLayout->addStretch();

    // 音量图标
    auto *volumeIcon = new QLabel(m_controlPanel);
    volumeIcon->setPixmap(style()->standardIcon(QStyle::SP_MediaVolume).pixmap(20, 20));
    volumeIcon->setStyleSheet("QLabel { padding: 0 5px; }");
    bottomLayout->addWidget(volumeIcon);

    // 音量滑块
    m_volumeSlider = new QSlider(Qt::Horizontal, m_controlPanel);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setMaximumWidth(100);
    m_volumeSlider->setStyleSheet(
        "QSlider::groove:horizontal { "
        "  height: 4px; "
        "  background: rgba(255, 255, 255, 0.2); "
        "  border-radius: 2px; "
        "} "
        "QSlider::handle:horizontal { "
        "  background: white; "
        "  width: 12px; "
        "  height: 12px; "
        "  margin: -4px 0; "
        "  border-radius: 6px; "
        "} "
        "QSlider::sub-page:horizontal { "
        "  background: white; "
        "  border-radius: 2px; "
        "}"
    );
    bottomLayout->addWidget(m_volumeSlider);

    controlLayout->addLayout(bottomLayout);
    mainLayout->addWidget(m_controlPanel);

    // 创建媒体播放器
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_player->setVideoOutput(m_videoWidget);

    // 连接信号
    connect(m_btnPlayPause, &QPushButton::clicked, this, &MediaViewer::onPlayPauseClicked);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MediaViewer::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MediaViewer::onDurationChanged);
    connect(m_positionSlider, &QSlider::sliderMoved, this, &MediaViewer::onSliderMoved);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &MediaViewer::onVolumeChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaViewer::onMediaStatusChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this, &MediaViewer::onErrorOccurred);

    // 设置初始音量
    m_audioOutput->setVolume(0.7);
}

MediaViewer::~MediaViewer() {
    if (m_player) {
        m_player->stop();
    }
}

bool MediaViewer::isAudioFile(const QString &path) {
    static const QStringList exts = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "ape", "opus"};
    const QString ext = QFileInfo(path).suffix().toLower();
    return exts.contains(ext);
}

bool MediaViewer::isVideoFile(const QString &path) {
    static const QStringList exts = {"mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "m4v", "mpg", "mpeg", "3gp"};
    const QString ext = QFileInfo(path).suffix().toLower();
    return exts.contains(ext);
}

bool MediaViewer::loadMedia(const QString &path) {
    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return false;
    }

    m_player->stop();
    m_player->setSource(QUrl::fromLocalFile(path));

    m_isAudio = isAudioFile(path);

    if (m_isAudio) {
        // 音频文件：隐藏视频窗口，显示音频面板
        m_videoWidget->hide();
        m_audioPanel->show();
        
        // 设置文件名
        QString fileName = info.fileName();
        m_titleLabel->setText(fileName);
        
        // 设置文件信息
        QString fileSize = formatFileSize(info.size());
        QString ext = info.suffix().toUpper();
        m_infoLabel->setText(QString("%1  •  %2").arg(ext).arg(fileSize));
    } else {
        // 视频文件：显示视频窗口
        m_videoWidget->show();
        m_audioPanel->hide();
    }

    m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_timeLabel->setText("00:00 / 00:00");
    m_positionSlider->setValue(0);

    // 自动播放
    m_player->play();

    return true;
}

void MediaViewer::stop() {
    if (m_player) {
        m_player->stop();
    }
}

void MediaViewer::onPlayPauseClicked() {
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        m_player->play();
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void MediaViewer::onPositionChanged(qint64 position) {
    if (!m_positionSlider->isSliderDown()) {
        m_positionSlider->setValue(position);
    }
    
    const qint64 duration = m_player->duration();
    m_timeLabel->setText(QString("%1 / %2")
        .arg(formatTime(position))
        .arg(formatTime(duration)));
}

void MediaViewer::onDurationChanged(qint64 duration) {
    m_positionSlider->setRange(0, duration);
}

void MediaViewer::onSliderMoved(int position) {
    m_player->setPosition(position);
}

void MediaViewer::onVolumeChanged(int value) {
    m_audioOutput->setVolume(value / 100.0);
}

void MediaViewer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::LoadedMedia) {
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else if (status == QMediaPlayer::EndOfMedia) {
        m_btnPlayPause->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_player->setPosition(0);
    }
}

void MediaViewer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString) {
    Q_UNUSED(error);
    if (m_isAudio) {
        m_titleLabel->setText(tr("播放失败"));
        m_infoLabel->setText(errorString);
        
        // 显示错误图标
        QPixmap errorIcon(":/icons/icons/error-icon.svg");
        if (!errorIcon.isNull()) {
            m_iconLabel->setPixmap(errorIcon.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

QString MediaViewer::formatFileSize(qint64 size) {
    if (size < 1024) return QString::number(size) + " B";
    if (size < 1024 * 1024) return QString::number(size / 1024.0, 'f', 1) + " KB";
    if (size < 1024 * 1024 * 1024) return QString::number(size / (1024.0 * 1024.0), 'f', 1) + " MB";
    return QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
}

QString MediaViewer::formatTime(qint64 milliseconds) {
    const qint64 seconds = milliseconds / 1000;
    const qint64 minutes = seconds / 60;
    const qint64 hours = minutes / 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes % 60, 2, 10, QChar('0'))
            .arg(seconds % 60, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes)
            .arg(seconds % 60, 2, 10, QChar('0'));
    }
}
