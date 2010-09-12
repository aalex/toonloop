/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * Saves a project as a XML file.
 * Compile with `xml2-config --cflags --libs`
 * 
 * The XML file looks like this:
 * 
 * <?xml version="1.0" encoding="UTF-8"?>
 * <project name="default">
 *   <clips>
 *     <clip id="0">
 *       <images>
 *         <image path="image0.jpg"/>
 *         <image path="image1.jpg"/>
 *       </images>
 *     </clip>
 *     <clip id="1">
 *       <images>
 *         <image path="image2.jpg"/>
 *         <image path="image3.jpg"/>
 *       </images>
 *     </clip>
 *   </clips>
 * </project>
 * 
 */
#include <iostream>
#include <stdio.h> // for snprintf
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "statesaving.h"

/**
 * Creates DOM tree and saves it to a file. 
 *
 * Libxml2 automagically allocates the necessary amount of memory to it.
 */
void statesaving::write_project_file()
{
    namespace ss = statesaving;
    std::string project_name("default");
    std::string output_file_name(ss::FILE_NAME);

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    // "project" node with its "name" attribute
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST ss::ROOT_NODE);
    xmlDocSetRootElement(doc, root_node);
    xmlNewProp(root_node, BAD_CAST ss::PROJECT_NAME_PROPERTY, BAD_CAST project_name.c_str());
    // "clips" node
    xmlNodePtr clips_node = xmlNewChild(root_node, NULL, BAD_CAST ss::CLIPS_NODE, NULL); // No text contents

    char buff[256]; // buff for node names
    for (int i = 5; i < 7; i++) 
    {
        xmlNodePtr clip_node = xmlNewChild(clips_node, NULL, BAD_CAST ss::CLIP_NODE, NULL);
        sprintf(buff, "%d", i); // clip id
        xmlNewProp(clip_node, BAD_CAST ss::CLIP_ID_PROPERTY, BAD_CAST buff);
        xmlNodePtr images_node = xmlNewChild(clip_node, NULL, BAD_CAST ss::IMAGES_NODE, NULL);
        for (int j = 1; j < 4; j++) {
            xmlNodePtr image_node = xmlNewChild(images_node, NULL, BAD_CAST ss::IMAGE_NODE, NULL);
            xmlNewProp(image_node, BAD_CAST "path", BAD_CAST "relative/path/to/image.jpg");
        }
    }

    // Save document to file
    xmlSaveFormatFileEnc(output_file_name.c_str(), doc, "UTF-8", 1);
    // Free the document + global variables that may have been allocated by the parser.
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

