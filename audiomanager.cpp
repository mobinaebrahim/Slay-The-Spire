#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVector>

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager& instance();

    void playMusic(const QString &path);
    void stopMusic();

    void playEffect(const QString &path);

    void setMusicVolume(int volume);   // 0 to 100
    void setEffectsVolume(int volume); // 0 to 100

    int musicVolume() const;
    int effectsVolume() const;

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager();
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void loadSettings();

    QMediaPlayer *m_musicPlayer;
    QAudioOutput *m_musicOutput;

    static const int EffectPoolSize = 4; // lets 4 short sounds overlap without cutting each other off
    QVector<QMediaPlayer*> m_effectPlayers;
    QVector<QAudioOutput*> m_effectOutputs;
    int m_nextEffectPlayer = 0;

    int m_musicVolume = 50;
    int m_effectsVolume = 50;
};

#endif // AUDIOMANAGER_H