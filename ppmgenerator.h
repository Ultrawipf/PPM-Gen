#ifndef PPMGENERATOR_H
#define PPMGENERATOR_H
#include <QByteArray>
#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <QThread>
#include <QByteArray>
#include <QVector>

class Generator : public QIODevice
{
    Q_OBJECT

public:
    Generator(const QAudioFormat &format, qint64 durationUs,int channels, QObject *parent);
    ~Generator();
    QVector<int> data;
    void start();
    void stop();
    int channels;
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    void setData(QVector<int> data);
    void update();
    int dataOffset=0;

private:
    void generateData(const QAudioFormat &format, qint64 durationUs,int channels);
    void appendToBuffer(int channelBytes, unsigned char *ptr, int sampleIndex, qreal value);
    int sampleIndex=0;
    int channelCount=1;
    //unsigned char *buffer_ptr;

private:
    qint64 m_pos;
    QByteArray m_buffer;
    QAudioFormat format;
    qint64 durationUs;
};

class PPMGenerator : public QObject
{
    Q_OBJECT
private:

    QAudioFormat format;
    QByteArray m_buffer;
public:
    QAudioOutput* audio;
    PPMGenerator(int channels);
    void start();
    void stop();
    void forceUpdate();
    void setChannelData(int channel, int data);
    int getChannelData(int channel);
    Generator *generator;
    void setOffset(int offset);

public slots:
    void audioNotify();

};

#endif // PPMGENERATOR_H
