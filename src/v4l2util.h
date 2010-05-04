/* v4l2util.h
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

#ifndef _V4L2UTIL_H_
#define _V4L2UTIL_H_

#include <string>
 	
/// FIXME: this should be a real object that only opens the device ONCE
class v4l2util
{
    private:
        static std::string fcc2s(unsigned int val);
        static std::string field2s(int val);
        static std::string num2s(unsigned num);
        static std::string colorspace2s(int val);
        static void printCaptureFormat(const std::string &device);
        static std::string inputsPerDevice(int fd);
        static std::string getStandard(int fd);
        static void printSupportedSizes(int fd);
    public:
        static std::string getStandard(const std::string &device);
        static bool checkStandard(const std::string &expected, 
                std::string &actual, const std::string &device);
        static void setFormatVideo(const std::string &device, int width, int height);
        static unsigned captureWidth(const std::string &device);
        static unsigned captureHeight(const std::string &device);
        static void listCameras();
        static bool isInterlaced(const std::string &device);
        static void setStandard(const std::string &device, const std::string &standard);
        static void setInput(const std::string &device, int input);
};

#endif // _V4L2UTIL_H_

