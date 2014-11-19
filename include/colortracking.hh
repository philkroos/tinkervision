#ifndef COLORTRACKING_H
#define COLORTRACKING_H

#include "component.hh"

namespace tinkervision {
class Colortracking : public Component {
private:
    TFV_Byte min_hue_;
    TFV_Byte max_hue_;
    TFV_Callback callback_;
    TFV_Context context_;

public:
    Colortracking(TFV_Id camera_id, TFV_Byte min_hue, TFV_Byte max_hue,
                  TFV_Callback callback, TFV_Context context)
        : Component(camera_id),
          min_hue_(min_hue),
          max_hue_(max_hue),
          callback_(callback),
          context_(context) {}

    virtual ~Colortracking(void) = default;
    Colortracking(Colortracking const& other) = default;
    Colortracking(Colortracking&& other) = delete;
    Colortracking& operator=(Colortracking const& rhs) = default;
    Colortracking& operator=(Colortracking&& rhs) = delete;

    TFV_Byte min_hue(void) const { return min_hue_; }
    TFV_Byte max_hue(void) const { return max_hue_; }

    virtual void execute(TFV_ImageData* data, TFV_Int rows, TFV_Int columns);
    virtual bool valid(void) const;
};
};

#endif /* COLORTRACKING_H */
