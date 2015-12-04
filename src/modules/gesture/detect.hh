#ifndef DETECT_H
#define DETECT_H

#include <vector>

#include <image.hh>

#include "hand.hh"

using namespace tv;

class Detect {
private:
    size_t framewidth_, frameheight_;
    size_t bytesize_;
    size_t history_;     ///< Number of images for background calculation
    size_t available_;   ///< Currently available images, 0 < x <= history_
    uint8_t threshold_;  ///< If (pixel - background) > threshold, foreground.
    bool detecting_;     ///< State

    uint16_t* background_{nullptr};
    ImageData* foreground_{nullptr};
    ImageData* input_{nullptr};

public:
    Detect(void);
    ~Detect(void);
    void init(size_t width, size_t height);

    void set_fg_threshold(uint8_t value);
    bool set_history_size(size_t size);
    bool get_hand(ImageData const* data, Hand& hand, ImageData** foreground);
};

#endif
