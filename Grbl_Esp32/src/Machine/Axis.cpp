#include "Axes.h"
#include "Axis.h"

#include <cstring>

namespace Machine {
    void Axis::group(Configuration::HandlerBase& handler) {
        handler.item("steps_per_mm", _stepsPerMm);
        handler.item("max_rate", _maxRate);
        handler.item("acceleration", _acceleration);
        handler.item("max_travel", _maxTravel);
        handler.item("soft_limits", _softLimits);

        handler.section("endstops", _endstops, _axis, -1);
        handler.section("homing", _homing);

        char tmp[6];
        tmp[0] = 0;
        strcat(tmp, "gang");

        for (size_t g = 0; g < MAX_NUMBER_GANGED; ++g) {
            tmp[4] = char(g + '0');
            tmp[5] = '\0';

            handler.section(tmp, _gangs[g], _axis, g);
        }
    }

    void Axis::afterParse() {
        bool hasMotor0 = _gangs[0] && _gangs[0]->_motor;
        bool hasMotor1 = _gangs[1] && _gangs[1]->_motor;
        bool hasLimit0 = _gangs[0] && _gangs[0]->_endstops;
        bool hasLimit1 = _gangs[1] && _gangs[1]->_endstops;

        // Fix the masks if both motors are present but there is exactly one limit setup
        bool fixLimitMasks = hasMotor0 && hasMotor1 && (hasLimit0 ^ hasLimit1);
        if (fixLimitMasks) {
            // Since there is exactly one endstop,
            // hasLimit1 is 0 if endstop0 is present, 1 if endstop 1 is present
            _gangs[hasLimit1]->_endstops->expandLimitMasks();
        }

        for (size_t i = 0; i < MAX_NUMBER_GANGED; ++i) {
            if (_gangs[i] == nullptr) {
                _gangs[i] = new Gang(_axis, i);
            }
        }
    }

    void Axis::init() {
        for (uint8_t gang_index = 0; gang_index < Axis::MAX_NUMBER_GANGED; gang_index++) {
            _gangs[gang_index]->init();
        }
        if (_homing) {
            _homing->init();
            set_bitnum(Axes::homingMask, _axis);
        }
        if (_endstops) {
            _endstops->init();
        }
    }

    // Checks if a motor matches this axis:
    bool Axis::hasMotor(const Motors::Motor* const motor) const {
        for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            if (_gangs[gang_index]->_motor == motor) {
                return true;
            }
        }
        return false;
    }

    Axis::~Axis() {
        for (uint8_t gang_index = 0; gang_index < MAX_NUMBER_GANGED; gang_index++) {
            delete _gangs[gang_index];
        }
    }
}
