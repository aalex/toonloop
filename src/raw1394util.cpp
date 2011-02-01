/* raw1394util.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//#include "util.h"
#include "log.h"

#include <cerrno>
#include <cstring>
#include <libavc1394/avc1394.h>
#include <string>
#include <sstream>
#include <libavc1394/rom1394.h>
#include "raw1394util.h"

#define MOTDCT_SPEC_ID    0x00005068


/** Open the raw1394 device and get a handle.
 *  
 * \return number of ports found
 */

namespace {
int raw1394_get_num_ports()
{
    int n_ports;
    struct raw1394_portinfo pinf[ 16 ];
    raw1394handle_t handle;

    /* get a raw1394 handle */
    if (!(handle = raw1394_new_handle()))
        THROW_ERROR("raw1394 - failed to get handle: " << strerror(errno));

    n_ports = raw1394_get_port_info(handle, pinf, 16);
    raw1394_destroy_handle(handle);

    if (n_ports  < 0)
        THROW_ERROR("raw1394 - failed to get port info: " << strerror(errno));

    return n_ports;
}


/** Open the raw1394 device and get a handle.
 *  
 * \param port A 0-based number indicating which host adapter to use.
 * \return a raw1394 handle.
 */

#ifdef RAW1394_V_0_8
    raw1394handle_t (* const rawHandle)(void) = raw1394_get_handle;
#else
    raw1394handle_t (* const rawHandle)(void) = raw1394_new_handle;
#endif

raw1394handle_t raw1394_open(int port)
{
    struct raw1394_portinfo pinf[ 16 ];
    /* get a raw1394 handle */
    raw1394handle_t handle = rawHandle();


    if (!handle)
        THROW_ERROR("raw1394 - failed to get handle: " << strerror(errno) );

    if (raw1394_get_port_info( handle, pinf, 16 ) < 0 )
    {
        raw1394_destroy_handle(handle);
        THROW_ERROR("raw1394 - failed to get port info: " <<  strerror(errno));
    }

    /* tell raw1394 which host adapter to use */
    if (raw1394_set_port(handle, port) < 0)
    {
        raw1394_destroy_handle(handle);
        THROW_ERROR("raw1394 - failed to set set port: " <<  strerror(errno) );
    }

    return handle;
}


/// FIXME: test with multiple devices
std::vector<std::string> discoverAVC(int* port, octlet_t* guid)
{
    rom1394_directory rom_dir;
    raw1394handle_t handle;
    int device = -1;
    int i, j = 0;
    int m = raw1394_get_num_ports();
    std::vector<std::string> results;
    std::ostringstream stream;

    if (*port >= 0)
    {
        /* search on explicit port */
        j = *port;
        m = *port + 1;
    }

    for (; j < m and device == -1; j++)
    {
        handle = raw1394_open(j);
        for (i = 0; i < raw1394_get_nodecount(handle); ++i)
        {
            if (*guid > 1)
            {
                /* select explicitly by GUID */
                if (*guid == rom1394_get_guid(handle, i))
                {
                    device = i;
                    *port = j;
                    break;
                }
            }
            else
            {
                /* select first AV/C Tape Reccorder Player node */
                if (rom1394_get_directory(handle, i, &rom_dir) < 0)
                {
                    rom1394_free_directory(&rom_dir);
                    LOG_WARNING("error reading config rom directory for node " << i);
                }
                if (((rom1394_get_node_type(&rom_dir) == ROM1394_NODE_TYPE_AVC) and 
                         avc1394_check_subunit_type(handle, i, AVC1394_SUBUNIT_TYPE_VCR)) or 
                       (rom_dir.unit_spec_id == MOTDCT_SPEC_ID))
                {
                    octlet_t my_guid, *pguid = (*guid == 1)? guid : &my_guid;
                    *pguid = rom1394_get_guid( handle, i );
                    stream << rom_dir.label << ": GUID 0x" << 
                        (quadlet_t) (*pguid>>32) << (quadlet_t) (*pguid & 0xffffffff) << std::endl;
                    results.push_back(stream.str());
                    LOG_DEBUG(stream.str());
                    device = i;
                    *port = j;
                    rom1394_free_directory(&rom_dir);
                    break;
                }
                rom1394_free_directory(&rom_dir);
            }
        }
        raw1394_destroy_handle(handle);
    }

    return results;
}
}

std::vector<std::string> Raw1394::getDeviceList()
{
    raw1394handle_t handle;
    int port, device;
    octlet_t guid = 0;
    port = device = -1;

    if (!(handle = raw1394_new_handle()))
        THROW_ERROR("raw1394 cannot get handle");

    raw1394_destroy_handle(handle);
    std::vector<std::string> devices(discoverAVC(&port, &guid));
    return devices;
}


bool Raw1394::cameraIsReady() 
{
    std::vector<std::string> devices(getDeviceList());

    if (devices.empty())
    {
        LOG_WARNING("Dv source is not ready, no device available");
        return false;
    }

    return true;
}

/// Returns true if one or more cameras were found
bool Raw1394::listCameras() 
{
    using std::vector;
    using std::string;
    vector<string> devices(getDeviceList());
    if (not devices.empty())
    {
        LOG_PRINT("\nDV1394 devices:\n");
        for (vector<string>::const_iterator iter = devices.begin(); iter != devices.end(); ++iter)
            LOG_PRINT("    " << *iter << std::endl);
    }
    return not devices.empty();
}

