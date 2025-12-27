#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVideoWidget>

class QSlider;
class QLabel;
class QPushButton;
class QVBoxLayout;

class MediaViewer : public QWidget {
    Q_OBJECT
public:
    explicit MediaViewer(QWidget *parent = nullptr);
    ~MediaViewer() override;

    bool loadMedia(const QString &path);
    void stop();

private slots:
    void onPlayPauseClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onSliderMoved(int position);
    void onVolumeChanged(int value);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);

private:
    QString formatTime(qint64 milliseconds);
    QString formatFileSize(qint64 size);
    bool isAudioFile(const QString &path);
    bool isVideoFile(const QString &path);

    QMediaPlayer *m_player {nullptr};
    QAudioOutput *m_audioOutput {nullptr};
    QVideoWidget *m_videoWidget {nullptr};
    
    QWidget *m_audioPanel {nullptr};
    QLabel *m_iconLabel {nullptr};
    QLabel *m_titleLabel {nullptr};
    QLabel *m_infoLabel {nullptr};
    
    QWidget *m_controlPanel {nullptr};
    QPushButton *m_btnPlayPause {nullptr};
    QSlider *m_positionSlider {nullptr};
    QSlider *m_volumeSlider {nullptr};
    QLabel *m_timeLabel {nullptr};
    
    bool m_isAudio {false};
};

#endif // MEDIAVIEWER_H
