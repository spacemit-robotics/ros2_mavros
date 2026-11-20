// Copyright 2026
//
// Compatibility declarations for MAVLink distributions that omit selected
// legacy C++ message wrappers while retaining their wire-level definitions.

#pragma once

#include <mavlink/v2.0/common/common.hpp>

#if !__has_include(<mavlink/v2.0/common/mavlink_msg_global_position_int.hpp>)
namespace mavlink
{
namespace common
{
namespace msg
{

/**
 * @brief GLOBAL_POSITION_INT message wrapper.
 *
 * MAVLink message 33 remains part of the Common dialect and is emitted by
 * current flight controllers.  Some newer generated C++ dialect packages omit
 * its wrapper, so provide the standard wire-compatible definition here.
 */
struct GLOBAL_POSITION_INT : mavlink::Message
{
    static constexpr msgid_t MSG_ID = 33;
    static constexpr size_t LENGTH = 28;
    static constexpr size_t MIN_LENGTH = 28;
    static constexpr uint8_t CRC_EXTRA = 104;
    static constexpr auto NAME = "GLOBAL_POSITION_INT";

    uint32_t time_boot_ms;
    int32_t lat;
    int32_t lon;
    int32_t alt;
    int32_t relative_alt;
    int16_t vx;
    int16_t vy;
    int16_t vz;
    uint16_t hdg;

    inline std::string get_name() const override
    {
        return NAME;
    }

    inline Info get_message_info() const override
    {
        return {MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA};
    }

    inline std::string to_yaml() const override
    {
        return NAME;
    }

    inline void serialize(mavlink::MsgMap & map) const override
    {
        map.reset(MSG_ID, LENGTH);
        map << time_boot_ms;
        map << lat;
        map << lon;
        map << alt;
        map << relative_alt;
        map << vx;
        map << vy;
        map << vz;
        map << hdg;
    }

    inline void deserialize(mavlink::MsgMap & map) override
    {
        map >> time_boot_ms;
        map >> lat;
        map >> lon;
        map >> alt;
        map >> relative_alt;
        map >> vx;
        map >> vy;
        map >> vz;
        map >> hdg;
    }
};

}  // namespace msg
}  // namespace common
}  // namespace mavlink
#endif
