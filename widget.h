#ifndef WIDGET
#define WIDGET

#include <QCoreApplication>

#include <memory>

/* "Pointer to implementation" or "pImpl" is a C++ programming technique that removes implementation details of a class
 * from its object representation by placing them in a separate class, accessed through unique-ownership opaque pointer.
 * This technique is used to construct C++ library interfaces with stable ABI and to reduce compile-time dependencies.
 */

class widget : public QObject {
    Q_OBJECT

public:
    widget();
    ~widget();

    widget(widget&&) = default;
    widget& operator=(widget&&) = default;

    int processTo(std::ifstream &wavFp);
    void do_internal_work();

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

struct wavHeader {
    /* RIFF Chunk Descriptor */
    uint8_t         RIFF[4];            // RIFF Header Magic header
    uint32_t        ChunkSize;          // RIFF Chunk Size
    uint8_t         WAVE[4];            // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t         fmt[4];             // FMT header
    uint32_t        Subchunk1Size;      // Size of the fmt chunk
    uint16_t        AudioFormat;        // Audio format 1=PCM,6=mulaw,7=alaw,257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t        NumOfChan;          // Number of channels 1=Mono 2=Stereo
    uint32_t        SamplesPerSec;      // Sampling Frequency in Hz
    uint32_t        bytesPerSec;        // bytes per second
    uint16_t        blockAlign;         // 2=16-bit mono, 4=16-bit stereo
    uint16_t        bitsPerSample;      // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t         Subchunk2ID[4];     // "data" string
    uint32_t        Subchunk2Size;      // Sampled data length
};

#endif // WIDGET
