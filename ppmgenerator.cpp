#include "ppmgenerator.h"
#include <qmath.h>
#include <qendian.h>
#include <qdebug.h>
#include <QTime>
#define CONSTRAIN(data,min, max) (data>min?(data<max?data:max):min);

//Length of a ppm frame in microseconds.
#define LENGTH 22500

PPMGenerator::PPMGenerator(int channels)
{
    format.setSampleRate(48000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    audio = new QAudioOutput(format, this);
   //connect(audio,SIGNAL(notify()),this,SLOT(audioNotify()));
    generator=new Generator(format, LENGTH,channels,this);
}

#define CHAN_OFFSET 20 //channel offset in µs

void PPMGenerator::setChannelData(int channel, int data){
    if(channel>generator->channels)
        return;
    data = CONSTRAIN(data,1000,2000);
    generator->data[channel]=data;
}
int PPMGenerator::getChannelData(int channel){
    if(channel>generator->channels)
        return 0;
    return generator->data[channel];
}

void PPMGenerator::setOffset(int offset){
    generator->dataOffset=offset;
}

void PPMGenerator::start(){
    generator->start();
    audio->setBufferSize(2 * LENGTH * ((format.sampleRate()* (format.sampleSize() / 8)) / 1000000.0));
    audio->start(generator);
    qDebug()<<"Buffer Size: "<<audio->bufferSize();
}
void PPMGenerator::stop(){

    audio->stop();
    generator->stop();

}

void Generator::setData(QVector<int> data){
    this->data=data;
}
void PPMGenerator::audioNotify(){
    qDebug()<<"Notify";
}

void PPMGenerator::forceUpdate(){
    generator->update();
}


void Generator::update(){
    generateData(format, durationUs,channels);
}

Generator::Generator(const QAudioFormat &format,
                     qint64 durationUs, int channels,
                     QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0),channels(channels),format(format),durationUs(durationUs)
{
    data.fill(1500,channels);
    if (format.isValid())
        generateData(format, durationUs,channels);
}

Generator::~Generator()
{

}

void Generator::start()
{
    open(QIODevice::ReadOnly);
}

void Generator::stop()
{
    m_pos = 0;
    close();
}

void Generator::appendToBuffer(int channelBytes,unsigned char *ptr,int sampleIndex, qreal value){
    for (int i=0; i<channelCount; ++i) {
        const quint8 tmp = static_cast<quint8>((1.0 + value) / 2 * 255);
        *reinterpret_cast<quint8*>(ptr+(sampleIndex*channelBytes)) = tmp;
    }
}

void Generator::generateData(const QAudioFormat &format, qint64 durationUs, int channels)
{

    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;
    channelCount=format.channelCount();



    qint64 length = (format.sampleRate() * channelCount * (format.sampleSize() / 8)) * durationUs / 1000000;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes) // suppress warning in release builds
    //m_buffer.clear();
    m_buffer.resize(length);

    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());
    float us = ((format.sampleRate()* (format.sampleSize() / 8)) / 1000000.0);


    sampleIndex=0;


    //-----------generate ppm frame--------------------------------
    for(int i=0;i<(2000 * us);i++)
        appendToBuffer(channelBytes,ptr,sampleIndex++,qreal(1)); //Start marker

    for(int ch=0;ch<channels;ch++){
        for(int i=0;i<((data[ch] - (300-dataOffset))*us);i++)
            appendToBuffer(channelBytes,ptr,sampleIndex++,qreal(0)); //Channel Data

        for(int i=0;i<((300.0)*us);i++)
            appendToBuffer(channelBytes,ptr,sampleIndex++,qreal(1)); //Seperator
    }
    while(sampleIndex<length)
        appendToBuffer(channelBytes,ptr,sampleIndex++,qreal(0)); //Fill remaining time
    //--------------------------------------------------------------

}

qint64 Generator::readData(char *data, qint64 len)
{
    len=this->durationUs * ((format.sampleRate()* (format.sampleSize() / 8)) / 1000000.0); //force the length to our frame size

    qint64 total = 0;
    update();

    if (!m_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((m_buffer.size() - m_pos), len - total);
            memcpy(data + total, m_buffer.constData() + m_pos, chunk);
            m_pos = (m_pos + chunk) % m_buffer.size();
            total += chunk;
        }
    }

    return total;
}

qint64 Generator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 Generator::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}
