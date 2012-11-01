#pragma once

#include <string>

namespace services
{
    struct event
    {
        enum event_type
        {
            null = 0,
            access,
            attrib,
            close_write,
            close_nowrite,
            modify,
            delete_self,
            move_self,
            open,
        };

        event_type type;
        std::string filename;

        event ( const std::string& f = "", event_type t = event::null ) :
            filename(f), type(t)
        {
        }
    };
}
