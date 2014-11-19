#ifndef COMPONENT_H
#define COMPONENT_H

#include "tinkervision_defines.h"

namespace tinkervision {

class Component {
private:
    TFV_Id camera_id_;
    bool active_;

public:
    Component(TFV_Id camera_id) : camera_id_(camera_id), active_(false) {}
    virtual ~Component(void) = default;

    Component(Component const& other) = default;
    Component(Component&& other) = default;
    Component& operator=(Component const& rhs) = default;
    Component& operator=(Component&& rhs) = default;

    bool active(void) const { return active_; }
    TFV_Id camera_id(void) const { return camera_id_; }

    void activate(void) { active_ = true; }
    void deactivate(void) { active_ = false; }

    virtual void execute(TFV_ImageData* data, TFV_Int rows,
                         TFV_Int columns) = 0;
    virtual bool valid(void) const = 0;
};
};
#endif /* COMPONENT_H */
