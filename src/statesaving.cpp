/** 
 * Saves a project as a XML file.
 * Compile with `xml2-config --cflags --libs`
 */
#include <iostream>
#include <stdio.h> // for snprintf
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

namespace statesaving
{
    const char *ROOT_NODE = "project";
    const char *PROJECT_NAME_PROPERTY = "name";
    const char *FILE_NAME = "project.xml";
    const char *CLIPS_NODE = "clips";
    const char *CLIP_NODE = "clip";
    const char *IMAGES_NODE = "images";
    const char *IMAGE_NODE = "image";
    void write_project_file();
};
/**
 * Creates DOM tree and saves it to a file. 
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

    int i;
    int j;
    char buff[256]; // buff for node names
    for (i = 5; i < 7; i++) 
    {
        sprintf(buff, "clip%d", i);
        xmlNodePtr clip_node = xmlNewChild(clips_node, NULL, BAD_CAST buff, NULL);
        xmlNodePtr images_node = xmlNewChild(clip_node, NULL, BAD_CAST "images", NULL);
        for (j = 1; j < 4; j++) {
            sprintf(buff, "image%d%d", i, j);
            xmlNodePtr image_node = xmlNewChild(images_node, NULL, BAD_CAST buff, NULL);
            xmlNewProp(image_node, BAD_CAST "path", BAD_CAST "relative/path/to/image.jpg");
        }
    }

    // Save document to file
    xmlSaveFormatFileEnc(output_file_name.c_str(), doc, "UTF-8", 1);
    // Free the document + global variables that may have been allocated by the parser.
    xmlFreeDoc(doc);
    xmlCleanupParser();
}

int main(int argc, char *argv[])
{
    for (int i = 0; i < 10000; i++)
    {
        statesaving::write_project_file();
        std::cout << i << " " << std::endl;
    }
    return 0;
};
