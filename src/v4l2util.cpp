// v4l2util.cpp
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

// see v4l2-ctl.cpp for reference

#include "config.h"

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <fcntl.h>  // for O_RDWR
#include <cerrno>
#include <map>
#include <cstdio>
#include <vector>
#include <string>
#include <boost/assign.hpp>

// for filesystem ops
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "log.h" //#include "util.h"
#include "v4l2util.h"

#include <boost/lexical_cast.hpp>

static int doioctl(int fd, long request, void *data, const std::string &name)
{
    int result = ioctl(fd, request, data);
    if (result < 0)
        LOG_DEBUG("IOCTL " << name << " failed: " << strerror(errno) << std::endl);

    return result;
}

static v4l2_format getCaptureFormat(int fd)
{
    v4l2_format vfmt;
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    doioctl(fd, VIDIOC_G_FMT, &vfmt, "VIDIOC_G_FMT");

    return vfmt;
}

static std::string getDriverInfo(int fd, const std::string &device)
{
    std::string result;
	struct v4l2_capability vcap;	/* list_cap */
	memset(&vcap, 0, sizeof(vcap));

    if (doioctl(fd, VIDIOC_QUERYCAP, &vcap, "VIDIOC_QUERYCAP") < 0)
    {
        LOG_PRINT("\n");
        LOG_WARNING("Cannot get capabilities for device " << device 
                << ",\n    it isn't a v4l2 driver. Check if it is a v4l1 driver.");
        return "";
    }

    result += "    Driver name   : " +  boost::lexical_cast<std::string>(vcap.driver) + "\n";
    result += "    Card type     : " + boost::lexical_cast<std::string>(vcap.card) + "\n";
    result += "    Bus info      : " + boost::lexical_cast<std::string>(vcap.bus_info) + "\n";
    result += "    Driver version: " +  boost::lexical_cast<std::string>(vcap.version) + "\n";

    return result;
}

static std::string getInputName(int fd)
{
    std::string result;
    int input;
    struct v4l2_input vin;		/* list_inputs */
    memset(&vin, 0, sizeof(vin));

    if (doioctl(fd, VIDIOC_G_INPUT, &input, "VIDIOC_G_INPUT") == 0) 
    {
        result += boost::lexical_cast<std::string>(input);
        vin.index = input;
        if (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0)
            result += " (" + boost::lexical_cast<std::string>(vin.name) + ")";
    }

    return result;
}

static void setCaptureFormat(int fd, v4l2_format format)
{
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    doioctl(fd, VIDIOC_S_FMT, &format, "VIDIOC_S_FMT");
}

/// Check current standard of v4l2 device to make sure it is what we expect
// FIXME: replace with a function that just returns the actual standard
// and then the client can compare against the expected standard
bool v4l2util::checkStandard(const std::string &expected, 
        std::string &actual,
        const std::string &device)
{
    using namespace boost::assign;
    bool result = false;
    v4l2_std_id std;
    int fd = -1;

    // map of format codes
    static std::map<std::string, unsigned long long> FORMATS = map_list_of
        ("PAL", 0xfff)
        ("NTSC", 0xf000)
        ("SECAM", 0xff0000)
        ("ATSC/HDTV", 0xf000000);

    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));

    if (doioctl(fd, VIDIOC_G_STD, &std, "VIDIOC_G_STD") == 0) 
    {
        std::map<std::string, unsigned long long>::const_iterator iter;
        for (iter = FORMATS.begin(); iter != FORMATS.end(); ++iter)
            if (std & (*iter).second)    // true if current format matches this iter's key
            {
                result = (result or (expected == (*iter).first)); // can have multiple positives, hence the or
                actual = (*iter).first; // save the actual standard
            }
    }

    close(fd);
    return result;
}


/// public version
std::string v4l2util::getStandard(const std::string &device)
{
    int fd = -1;

    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));

    std::string result = getStandard(fd);

    close(fd);
    return result;
}

std::string v4l2util::getStandard(int fd)
{
    using namespace boost::assign;
    std::string result;
    v4l2_std_id std;

    // map of format codes
    static std::map<std::string, unsigned long long> FORMATS = map_list_of
        ("PAL", 0xfff)
        ("NTSC", 0xf000)
        ("SECAM", 0xff0000)
        ("ATSC/HDTV", 0xf000000);

    if (doioctl(fd, VIDIOC_G_STD, &std, "VIDIOC_G_STD") == 0) 
    {
        std::map<std::string, unsigned long long>::const_iterator iter;
        for (iter = FORMATS.begin(); iter != FORMATS.end() and result == ""; ++iter)
            if (std & (*iter).second)    // true if current format matches this iter's key
                result = (*iter).first; // save the actual standard
    }
    return result;
}


std::string v4l2util::field2s(int val)
{
    switch (val) {
        case V4L2_FIELD_ANY:
            return "Any";
        case V4L2_FIELD_NONE:
            return "None";
        case V4L2_FIELD_TOP:
            return "Top";
        case V4L2_FIELD_BOTTOM:
            return "Bottom";
        case V4L2_FIELD_INTERLACED:
            return "Interlaced";
        case V4L2_FIELD_SEQ_TB:
            return "Sequential Top-Bottom";
        case V4L2_FIELD_SEQ_BT:
            return "Sequential Bottom-Top";
        case V4L2_FIELD_ALTERNATE:
            return "Alternating";
        case V4L2_FIELD_INTERLACED_TB:
            return "Interlaced Top-Bottom";
        case V4L2_FIELD_INTERLACED_BT:
            return "Interlaced Bottom-Top";
        default:
            return "Unknown (" + num2s(val) + ")";
    }
}

std::string v4l2util::num2s(unsigned num)
{
    std::ostringstream buf;
    buf << std::hex << num;

    return buf.str();
}

std::string v4l2util::fcc2s(unsigned int val)
{
    std::string s;

    s += val & 0xff;
    s += (val >> 8) & 0xff;
    s += (val >> 16) & 0xff;
    s += (val >> 24) & 0xff;
    return s;
}

void v4l2util::printCaptureFormat(const std::string &device)
{
    int fd = -1;
    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));
    v4l2_format vfmt = getCaptureFormat(fd);

    // this will be empty if we're dealing with a non v4l2 device
    std::string driverInfo(getDriverInfo(fd, device));
    if (not driverInfo.empty())
    {
        LOG_PRINT("\nVideo4Linux Camera " << device << ":" << std::endl);
        LOG_PRINT(driverInfo);
        LOG_PRINT("    Video input   : " << getInputName(fd) << "\n");
        LOG_PRINT("    All inputs    : " << inputsPerDevice(fd) << "\n");
        LOG_PRINT("    Standard      : " << getStandard(fd) << "\n");
        LOG_PRINT("    Width/Height  : " << vfmt.fmt.pix.width << "x" << vfmt.fmt.pix.height << "\n");
        LOG_PRINT("    Pixel Format  : " << fcc2s(vfmt.fmt.pix.pixelformat) << "\n");
        LOG_PRINT("    Capture Type  : " << vfmt.type << "\n");
        LOG_PRINT("    Field         : " << field2s(vfmt.fmt.pix.field) << "\n");
        LOG_PRINT("    Bytes per Line: " << vfmt.fmt.pix.bytesperline << "\n");
        LOG_PRINT("    Size Image    : " << vfmt.fmt.pix.sizeimage << "\n");
        LOG_PRINT("    Colorspace    : " << colorspace2s(vfmt.fmt.pix.colorspace) << "\n");
        printSupportedSizes(fd);
    }
    close(fd);
}


unsigned v4l2util::captureWidth(const std::string &device)
{
    int fd = -1;
    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));
    v4l2_format vfmt = getCaptureFormat(fd);
    close(fd);
    return vfmt.fmt.pix.width;
}


unsigned v4l2util::captureHeight(const std::string &device)
{
    int fd = -1;
    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));
    v4l2_format vfmt = getCaptureFormat(fd);
    close(fd);
    return vfmt.fmt.pix.height;
}

std::string v4l2util::colorspace2s(int val)
{
    switch (val) {
        case V4L2_COLORSPACE_SMPTE170M:
            return "Broadcast NTSC/PAL (SMPTE170M/ITU601)";
        case V4L2_COLORSPACE_SMPTE240M:
            return "1125-Line (US) HDTV (SMPTE240M)";
        case V4L2_COLORSPACE_REC709:
            return "HDTV and modern devices (ITU709)";
        case V4L2_COLORSPACE_BT878:
            return "Broken Bt878";
        case V4L2_COLORSPACE_470_SYSTEM_M:
            return "NTSC/M (ITU470/ITU601)";
        case V4L2_COLORSPACE_470_SYSTEM_BG:
            return "PAL/SECAM BG (ITU470/ITU601)";
        case V4L2_COLORSPACE_JPEG:
            return "JPEG (JFIF/ITU601)";
        case V4L2_COLORSPACE_SRGB:
            return "SRGB";
        default:
            return "Unknown (" + num2s(val) + ")";
    }
}

typedef std::vector<std::string> DeviceList;
#ifdef HAVE_BOOST_FILESYSTEM
DeviceList getDevices()
{
    namespace fs = boost::filesystem;

    fs::path full_path("/dev/");


    if ( !fs::exists(full_path) )
        THROW_CRITICAL("\nPath " << full_path << " not found");

    DeviceList deviceList;

    fs::directory_iterator end_iter;
    for (fs::directory_iterator dir_itr(full_path);
            dir_itr != end_iter;
            ++dir_itr )
    {
        try
        {
            std::string pathString(dir_itr->path().string());
            if (pathString.find("video") != std::string::npos)  // devices matching video
                if (pathString.find("1394") == std::string::npos)   // that don't contain 1394
                    deviceList.push_back(pathString);
        }
        catch (const std::exception & ex)
        {
            THROW_CRITICAL(dir_itr->path() << " " << ex.what());
        }
    }
    return deviceList;
}
#endif



void v4l2util::listCameras()
{
    DeviceList names(getDevices());

    for (DeviceList::const_iterator deviceName = names.begin(); deviceName != names.end(); ++deviceName)
        printCaptureFormat(*deviceName);
}

bool v4l2util::isInterlaced(const std::string &device)
{
    if (boost::filesystem::exists(device))
    {
        int fd = -1;
        if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
            THROW_ERROR("Failed to open " << device << ": " << strerror(errno));
        v4l2_format vfmt = getCaptureFormat(fd);
        close(fd);
        if (vfmt.fmt.pix.field == V4L2_FIELD_INTERLACED)
            return true;
        else
            return false;
    }
    else
    {
        LOG_ERROR("No device " << device);
        return false;
    }
}


bool formatsMatch(const v4l2_format &lhs, const v4l2_format &rhs)
{
    return lhs.type == rhs.type and
        lhs.fmt.pix.width == rhs.fmt.pix.width and
        lhs.fmt.pix.height == rhs.fmt.pix.height;
}


void v4l2util::printSupportedSizes(int fd)
{
    typedef std::pair<int, int> Size;
    typedef std::vector< Size > SizeList;
    SizeList sizes;
    sizes.push_back(Size(924, 576));
    sizes.push_back(Size(768, 480));
    sizes.push_back(Size(720, 480));
    sizes.push_back(Size(704, 480));    // 4CIF
    sizes.push_back(Size(704, 240));    // 2CIF
    sizes.push_back(Size(640, 480));    // SD
    sizes.push_back(Size(352, 240));    // CIF
    sizes.push_back(Size(320, 240));    
    sizes.push_back(Size(176, 120));    // QCIF

    v4l2_format format = getCaptureFormat(fd);

    // save values
    int oldWidth = format.fmt.pix.width;
    int oldHeight = format.fmt.pix.height;

    for (SizeList::iterator size = sizes.begin(); size != sizes.end(); ++size)
    {
        // change some fields
        format.fmt.pix.width = size->first;
        format.fmt.pix.height = size->second;
        setCaptureFormat(fd, format);
        v4l2_format currentFormat = getCaptureFormat(fd);

        if (formatsMatch(format, currentFormat))
            LOG_PRINT("    Format " << size->first << "x" << size->second << " supported\n");

    }

    // restore old format
    format.fmt.pix.width = oldWidth;
    format.fmt.pix.height = oldHeight;
    setCaptureFormat(fd, format);
    v4l2_format currentFormat = getCaptureFormat(fd);
    if (!formatsMatch(format, currentFormat))
        LOG_WARNING("Format " << oldWidth << "x" << oldHeight << "not reverted correctly");
}

void v4l2util::setFormatVideo(const std::string &device, int width, int height)
{
#define FmtWidth		(1L<<0)
#define FmtHeight		(1L<<1)
    unsigned int set_fmts = 0;
    int fd = -1;
    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));

    v4l2_format vfmt = getCaptureFormat(fd);
    vfmt.fmt.pix.width = width;
    set_fmts |= FmtWidth;
    vfmt.fmt.pix.height = height;
    set_fmts |= FmtHeight;
    struct v4l2_format in_vfmt;

    in_vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (doioctl(fd, VIDIOC_G_FMT, &in_vfmt, "VIDIOC_G_FMT") == 0) 
    {
        if (set_fmts & FmtWidth)
            in_vfmt.fmt.pix.width = vfmt.fmt.pix.width;
        if (set_fmts & FmtHeight)
            in_vfmt.fmt.pix.height = vfmt.fmt.pix.height;
        doioctl(fd, VIDIOC_S_FMT, &in_vfmt, "VIDIOC_S_FMT");
    }

    close(fd);
#undef FmtWidth
#undef FmtHeight
}


void v4l2util::setStandard(const std::string &device, const std::string &standard)
{
    int fd = -1;
    bool usingDefault;
    v4l2_std_id std;		/* get_std/set_std */
    struct v4l2_standard vs;	/* list_std */
    const std::string DEFAULT_STANDARD("NTSC");
    memset(&vs, 0, sizeof(vs));

    if (standard == "NTSC")
        std = V4L2_STD_NTSC;
    else if (standard == "PAL")
        std = V4L2_STD_PAL;
    else
    {
        LOG_WARNING("Unsupported standard " << standard << ", using NTSC instead");
        std = V4L2_STD_NTSC;
        usingDefault = true;
    }

    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));

    if (std & (1ULL << 63)) 
    {
        vs.index = std & 0xffff;
        if (ioctl(fd, VIDIOC_ENUMSTD, &vs) >= 0)
            std = vs.id;
    }
    if (doioctl(fd, VIDIOC_S_STD, &std, "VIDIOC_S_STD") == 0)
    {
        if (not usingDefault)
            LOG_INFO("Standard set to " << standard);
        else 
            LOG_INFO("Standard set to " << DEFAULT_STANDARD);
        LOG_DEBUG("Standard set to " << std::hex << (unsigned long long)std << std::dec);
    }
    close(fd);
}

void v4l2util::setInput(const std::string &device, int input)
{
    int fd = -1;
    struct v4l2_input vin;		/* list_inputs */
    memset(&vin, 0, sizeof(vin));

    if ((fd = open(device.c_str(), O_RDONLY)) < 0) 
        THROW_ERROR("Failed to open " << device << ": " << strerror(errno));

    if (doioctl(fd, VIDIOC_S_INPUT, &input, "VIDIOC_S_INPUT") == 0) 
    {
        vin.index = input;
        std::string longName;
        if (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0)
            longName = " (" + boost::lexical_cast<std::string>(vin.name) + ")";
        LOG_INFO("Video input set to " << input << longName);
    }
    else
        THROW_ERROR("Failed to set input to " << input << " on device " << device);
    close(fd);
}


std::string v4l2util::inputsPerDevice(int fd)
{
    struct v4l2_input vin;		/* list_inputs */
    memset(&vin, 0, sizeof(vin));
    vin.index = 0;
    std::string result;

    while (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) 
    {
        if (vin.index)
            result += ", ";
        result += boost::lexical_cast<std::string>(vin.index) + " (" + boost::lexical_cast<std::string>(vin.name) + ")";
        vin.index++;
    }
    return result;
}
