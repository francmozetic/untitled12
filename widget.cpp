#include "widget.h"

#include <complex>
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <math.h>

#include <QDebug>

class widget::impl {

public:
    typedef std::vector<double> v_d;
    typedef std::complex<double> c_d;
    typedef std::vector<v_d> v_v_d;
    typedef std::vector<c_d> v_c_d;
    typedef std::map<int, std::map<int, c_d>> twmap;

    const double PI = 4*atan(1.0);
    size_t winWidthSamples, frameShiftSamples, numFFTBins;
    std::vector<double> frame, prevSamples, powerSpectralCoef, lmfbCoef, hamming, mfcc;
    std::vector<std::vector<double>> vecdmfcc, fbank, dct;

    std::vector<double> vecdsimilarity;

    // Calculate cosine similarity between two vectors
    double cosine_similarity(std::vector<double> veca, std::vector<double> vecb) {
        double multiply = 0.0;
        double d_a = 0.0;
        double d_b = 0.0;

        std::vector<double>::iterator itera, iterb;

        for (itera = veca.begin(), iterb = vecb.begin(); itera != veca.end(); itera++, iterb++) {
            multiply += *itera * *iterb;
            d_a += *itera * *itera;
            d_b += *iterb * *iterb;
        }

        return multiply / (sqrt(d_a) * sqrt(d_b));
    }

    // Process each frame and return MFCCs as vector of double
    std::vector<double> processFrameTo(double* samples, size_t N) {
        // Add samples from the previous frame that overlap with the current frame to the current samples and create the frame.
        frame = prevSamples;
        for (size_t i=0; i<N; i++)
            frame.push_back(samples[i]);
        prevSamples.assign(frame.begin()+frameShiftSamples, frame.end());

        preEmphHamming();
        compPowerSpec();
        applyLogMelFilterbank();
        applyDct();

        return mfcc;
    }

    // Read samples, extract MFCCs and calculate self-similarity measures
    int processSamplesTo(std::vector<double> levels) {
        uint16_t bufferLength = winWidthSamples - frameShiftSamples;
        uint16_t position = bufferLength;

        // Read and set the initial samples
        for (int i=0; i<bufferLength; i++)
            prevSamples[i] = levels[i];

        // Initialise buffer (allocate a block of memory of type double, dynamically allocated memory is allocated on Heap^)
        bufferLength = frameShiftSamples;
        double * buffer = new double[bufferLength];

        // Allocate memory for 790 coefficients, read data and process each frame
        vecdmfcc.reserve(790);
        vecdmfcc.clear();

        for (int i=0; i<bufferLength; i++)
            buffer[i] = levels[position + i];
        position += bufferLength;

        while (position < levels.size() && vecdmfcc.size() < 790) {
            vecdmfcc.push_back(processFrameTo(buffer, bufferLength));
            for (int i=0; i<bufferLength; i++)
                buffer[i] = levels[position + i];
            position += bufferLength;
        }

        // Allocate memory for self-similarity measures
        vecdsimilarity.reserve(365 * 790);
        vecdsimilarity.clear();
        std::vector<double> veca, vecb;
        double measure;
        for (size_t j=0; j<365; j++) {
            veca = vecdmfcc[j];
            for (size_t i=j; i<790; i++) {
                vecb = vecdmfcc[i];
                measure = 1 - cosine_similarity(veca, vecb);
                vecdsimilarity.push_back(measure);
            }
        }

        delete [] buffer; // delete a block of memory
        buffer = nullptr;
        return 0;
    }

    // Read input file stream, extract MFCCs and calculate self-similarity measures
    int processTo(std::ifstream &wavFp) {
        // Read the wav header
        wavHeader hdr;
        int headerSize = sizeof(wavHeader);
        wavFp.read((char *)&hdr, headerSize); // cast the address of hdr, denoted &hdr, to a char *, i.e. a pointer to characters

        // Check audio format
        if (hdr.AudioFormat != 1 || hdr.bitsPerSample != 16) {
            qDebug() << "Unsupported audio format, use 16 bit PCM Wave";
            return 1;
        }
        // Check sampling rate
        if (hdr.SamplesPerSec != fs) {
            qDebug() << "Sampling rate mismatch: Found" << hdr.SamplesPerSec << "instead of" << fs;
            return 1;
        }

        // Initialise buffer (allocate a block of memory of type double, dynamically allocated memory is allocated on Heap^)
        uint16_t bufferLength = winWidthSamples - frameShiftSamples;
        double * buffer = new double[bufferLength];
        // Calculate bytes per sample (size of the first element in bytes)
        int bufferbps = (sizeof buffer[0]);
        qDebug() << bufferLength;

        // Read and set the initial samples
        wavFp.read((char *)buffer, bufferLength * bufferbps);   // cast the pointer of the double variable to a pointer to characters
        for (int i=0; i<bufferLength; i++) {
            prevSamples[i] = buffer[i];                         // prevSamples[i] is an ith element of std::vector<double>
            qDebug() << prevSamples[i];
        }
        delete [] buffer;

        // Recalculate buffer size
        bufferLength = frameShiftSamples;
        buffer = new double[bufferLength];

        // Allocate memory for 790 coefficients, read data and process each frame
        vecdmfcc.reserve(790);
        vecdmfcc.clear();
        wavFp.read((char *)buffer, bufferLength * bufferbps);
        while (wavFp.gcount() == bufferLength * bufferbps && !wavFp.eof() && vecdmfcc.size() < 790) {
            vecdmfcc.push_back(processFrameTo(buffer, bufferLength));
            wavFp.read((char *)buffer, bufferLength * bufferbps);
        }

        // Allocate memory for self-similarity measures
        vecdsimilarity.reserve(365 * 790);
        vecdsimilarity.clear();
        std::vector<double> veca, vecb;
        double measure;
        for (size_t j=0; j<365; j++) {
            veca = vecdmfcc[j];
            for (size_t i=j; i<790; i++) {
                vecb = vecdmfcc[i];
                measure = 1 - cosine_similarity(veca, vecb);
                vecdsimilarity.push_back(measure);
            }
        }

        delete [] buffer;
        buffer = nullptr;
        return 0;
    }





    void do_internal_work()
    {
        internal_data = 5;
    }

private:
    std::map<int, std::map<int, std::complex<double>>> twiddle;

    size_t fs = 44100;                 // Sampling rate in Hertz (default=16000)
    size_t numCepstral = 12;           // Number of output cepstra, excluding log-energy (default=12)
    size_t numFilters = 40;            // Number of Mel warped filters in filterbank (default=40)
    double preEmphCoef = 0.97;         // Pre-emphasis coefficient
    double lowFreq = 50;               // Filterbank low frequency cutoff in Hertz (default=50)
    double highFreq = 4000;            // Filterbank high freqency cutoff in Hertz (default=fs/2)
    size_t numFFT = 512;               // N-point FFT on each frame
    size_t winWidth = 25;              // Width of analysis window in milliseconds (default=25)
    size_t frameShift = 10;            // Frame shift in milliseconds (default=10)

    // Hertz to Mel conversion
    inline double Hz2Mel(double f) {
        return 2595*std::log10(1 + f/700);
    }

    // Mel to Hertz conversion
    inline double Mel2Hz(double m) {
        return 700*(std::pow(10, m/2595) - 1);
    }

    // Cooley-Tukey FFT recursive function
    std::vector<std::complex<double>> fft(std::vector<std::complex<double>> x) {
        size_t N = x.size();
        if (N==1)
            return x;

        std::vector<std::complex<double>> xe(N/2,0), xo(N/2,0), Xjo, Xjo2;

        // Construct arrays from even and odd indices
        for (size_t i=0; i<N; i+=2)
            xe[i/2] = x[i];
        for (size_t i=1; i<N; i+=2)
            xo[(i-1)/2] = x[i];

        // Compute N/2-point FFT
        Xjo = fft(xe);
        Xjo2 = fft(xo);
        Xjo.insert (Xjo.end(), Xjo2.begin(), Xjo2.end());

        // Butterfly computations
        for (size_t i=0; i<=N/2-1; i++) {
            c_d t = Xjo[i], tw = twiddle[N][i];
            Xjo[i] = t + tw * Xjo[i+N/2];
            Xjo[i+N/2] = t - tw * Xjo[i+N/2];
        }
        return Xjo;
    }

    // Convert vector of double to string
    std::string v_d_to_string(v_d vec) {
        // The class template std::basic_stringstream implements operations on memory based streams.
        std::stringstream vecStream;
        for (size_t i=0; i<vec.size()-1; i++) {
            vecStream << std::scientific << vec[i];
            vecStream << ", ";
        }
        vecStream << std::scientific << vec.back();
        vecStream << "\n";
        return vecStream.str();
    }

    // Process each frame and extract MFCCs as string
    std::string processFrame(int16_t* samples, size_t N) {
        // Add samples from the previous frame that overlap with the current frame to the current samples and create the frame.
        frame = prevSamples;
        for (size_t i=0; i<N; i++)
            frame.push_back(samples[i]);
        prevSamples.assign(frame.begin() + frameShiftSamples, frame.end());

        preEmphHamming();
        compPowerSpec();
        applyLogMelFilterbank();
        applyDct();

        return v_d_to_string(mfcc);
    }

    /* Pre-emphasis and Hamming window
     * The first step is to apply a pre-emphasis filter on the signal to amplify the high frequencies.
     * A pre-emphasis filter is useful in several ways: (1) balance the frequency spectrum since high frequencies
     * usually have smaller magnitudes compared to lower frequencies, (2) avoid numerical problems during the
     * Fourier transform operation and (3) may also improve the Signal-to-Noise Ratio (SNR).
     * The pre-emphasis filter can be applied to a signal x using the first order filter in the following equation: y(t)=x(t)−αx(t−1).
     */
    void preEmphHamming(void) {
        v_d procFrame(frame.size(), hamming[0]*frame[0]);
        for (size_t i=1; i<frame.size(); i++)
            procFrame[i] = hamming[i] * (frame[i] - preEmphCoef * frame[i-1]);
        frame = procFrame;
    }

    /* Power spectrum computation
     * After pre-emphasis, we need to split the signal into short-time frames. We can safely assume that frequencies in a signal
     * are stationary over a very short period of time. Therefore, by doing a Fourier transform over this short-time frame,
     * we can obtain a good approximation of the frequency contours of the signal by concatenating adjacent frames.
     *
     * We can now do an N-point FFT on each frame to calculate the frequency spectrum, which is also called Short-Time
     * Fourier-Transform, where N is typically 256 or 512, numFFT = 512 in this case; and then compute the power spectrum (periodogram)
     * using the following equation: P=|FFT(xi)|^2 where, xi is the ith frame of signal x.
     */
    void compPowerSpec(void) {
        frame.resize(numFFT); // Pads zeros
        v_c_d framec (frame.begin(), frame.end()); // Complex frame
        v_c_d fftc = fft(framec);

        for (size_t i=0; i<numFFTBins; i++)
            powerSpectralCoef[i] = pow(abs(fftc[i]),2);
    }

    /* Applying log Mel filterbank
     * The final step to computing filter banks is applying triangular filters, typically 40 filters, numFilters = 40 on a Mel-scale
     * to the power spectrum to extract frequency bands. The Mel-scale aims to mimic the non-linear human ear perception of sound,
     * by being more discriminative at lower frequencies and less discriminative at higher frequencies.
     */
    void applyLogMelFilterbank(void) {
        lmfbCoef.assign(numFilters,0);

        for (size_t i=0; i<numFilters; i++) {
            // Multiply the filterbank matrix
            for (size_t j=0; j<fbank[i].size(); j++)
                lmfbCoef[i] += fbank[i][j] * powerSpectralCoef[j];
            // Apply Mel-flooring
            if (lmfbCoef[i] < 1.0)
                lmfbCoef[i] = 1.0;
        }

        // Applying log on amplitude
        for (size_t i=0; i<numFilters; i++)
            lmfbCoef[i] = std::log(lmfbCoef[i]);
    }

    /* Computing discrete cosine transform
     * It turns out that filter bank coefficients computed in the previous step are highly correlated, which could be
     * problematic in some machine learning algorithms. Therefore, we can apply Discrete Cosine Transform (DCT)
     * to decorrelate the filter bank coefficients and yield a compressed representation of the filter banks. Typically,
     * for Automatic Speech Recognition (ASR), the resulting cepstral coefficients 2-13 are retained and the rest are discarded.
     */
    void applyDct(void) {
        mfcc.assign(numCepstral+1,0);
        for (size_t i=0; i<=numCepstral; i++) {
            for (size_t j=0; j<numFilters; j++)
                mfcc[i] += dct[i][j] * lmfbCoef[j];
        }
    }

    // Precompute filterbank
    void initFilterbank(void) {
        // Convert low and high frequencies to Mel scale
        double lowFreqMel = Hz2Mel(lowFreq);
        double highFreqMel = Hz2Mel(highFreq);

        // Calculate filter centre-frequencies
        v_d filterCentreFreq;
        filterCentreFreq.reserve(numFilters+2);
        for (size_t i=0; i<numFilters+2; i++)
            filterCentreFreq.push_back(Mel2Hz(lowFreqMel + (highFreqMel-lowFreqMel)/(numFilters+1)*i));

        // Calculate FFT bin frequencies
        v_d fftBinFreq;
        fftBinFreq.reserve(numFFTBins);
        for (size_t i=0; i<numFFTBins; i++)
            fftBinFreq.push_back(fs/2.0/(numFFTBins-1)*i);

        // Allocate memory for the filterbank
        fbank.reserve(numFilters*numFFTBins);

        // Populate the filterbank matrix
        for (size_t filt=1; filt<=numFilters; filt++) {
            v_d ftemp;
            for (size_t bin=0; bin<numFFTBins; bin++) {
                double weight;
                if (fftBinFreq[bin] < filterCentreFreq[filt-1])
                    weight = 0;
                else if (fftBinFreq[bin] <= filterCentreFreq[filt])
                    weight = (fftBinFreq[bin] - filterCentreFreq[filt-1]) / (filterCentreFreq[filt] - filterCentreFreq[filt-1]);
                else if (fftBinFreq[bin] <= filterCentreFreq[filt+1])
                    weight = (filterCentreFreq[filt+1] - fftBinFreq[bin]) / (filterCentreFreq[filt+1] - filterCentreFreq[filt]);
                else
                    weight = 0;
                ftemp.push_back(weight);
            }
            fbank.push_back(ftemp);
        }
    }

    // Precompute Hamming window and dct matrix
    void initHammingDct(void) {
        size_t i, j;

        // After slicing the signal into frames, we apply a window function such as the Hamming window to each frame.
        hamming.assign(winWidthSamples, 0);
        for (i=0; i<winWidthSamples; i++)
            hamming[i] = 0.54 - 0.46 * cos(2 * PI * i / (winWidthSamples-1));

        v_d v1(numCepstral+1,0), v2(numFilters,0);
        for (i=0; i <= numCepstral; i++)
            v1[i] = i;
        for (i=0; i < numFilters; i++)
            v2[i] = i + 0.5;

        dct.reserve(numFilters*(numCepstral+1));
        double c = sqrt(2.0/numFilters);
        for (i=0; i<=numCepstral; i++) {
            v_d dtemp;
            for (j=0; j<numFilters; j++)
                dtemp.push_back(c * cos(PI / numFilters * v1[i] * v2[j]));
            dct.push_back(dtemp);
        }
    }

    // Twiddle factor computation
    void compTwiddle(void) {
        const std::complex<double> J(0,1);
        for (size_t n=2; n<=numFFT; n*=2)
            for (size_t k=0; k<=n/2-1; k++)
                twiddle[n][k] = exp(-2*PI*k/n*J);
    }





    int internal_data = 0;
};

widget::widget() : pimpl(std::make_unique<impl>()) {
    pimpl->do_internal_work();



}
