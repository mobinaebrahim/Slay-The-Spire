#include "audiomanager.h"
#include <QSettings>
#include <QUrl>

AudioManager& AudioManager::instance()
{
    static AudioManager inst;
    return inst;
}

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
    m_musicPlayer = new QMediaPlayer(this);
    m_musicOutput = new QAudioOutput(this);
    m_musicPlayer->setAudioOutput(m_musicOutput);
    m_musicPlayer->setLoops(QMediaPlayer::Infinite);

    for (int i = 0; i < EffectPoolSize; ++i) {
        QMediaPlayer *player = new QMediaPlayer(this);
        QAudioOutput *output = new QAudioOutput(this);
        player->setAudioOutput(output);
        m_effectPlayers.append(player);
        m_effectOutputs.append(output);
    }

    loadSettings();
}

AudioManager::~AudioManager()
{
}

void AudioManager::loadSettings()
{
    QSettings settings;
    m_musicVolume = settings.value("audio/musicVolume", 50).toInt();
    m_effectsVolume = settings.value("audio/effectsVolume", 50).toInt();

    m_musicOutput->setVolume(m_musicVolume / 100.0);
    for (QAudioOutput *output : m_effectOutputs) {
        output->setVolume(m_effectsVolume / 100.0);
    }
}

void AudioManager::playMusic(const QString &path)
{
    m_musicPlayer->setSource(QUrl(path));
    m_musicPlayer->play();
}

void AudioManager::stopMusic()
{
    m_musicPlayer->stop();
}

void AudioManager::playEffect(const QString &path)
{
    QMediaPlayer *player = m_effectPlayers[m_nextEffectPlayer];
    player->setSource(QUrl(path));
    player->play();

    m_nextEffectPlayer = (m_nextEffectPlayer + 1) % EffectPoolSize;
}

void AudioManager::setMusicVolume(int volume)
{
    m_musicVolume = volume;
    m_musicOutput->setVolume(volume / 100.0);

    QSettings settings;
    settings.setValue("audio/musicVolume", volume);
}

void AudioManager::setEffectsVolume(int volume)
{
    m_effectsVolume = volume;
    for (QAudioOutput *output : m_effectOutputs) {
        output->setVolume(volume / 100.0);
    }

    QSettings settings;
    settings.setValue("audio/effectsVolume", volume);
}

int AudioManager::musicVolume() const { return m_musicVolume; }
int AudioManager::effectsVolume() const { return m_effectsVolume; }

