#include "MyXml.h"
#include <locale.h>

int main(int argc, char const *argv[])
{
    setlocale(LC_ALL,"ru");
    XMLDoc doc;
    if(XMLDoc_load(&doc,"test.xml")){
        printf("XML Document (версия =%s, кодировка =%s)\n",doc.version,doc.encoding);
        XMLNode* str = XMLNode_child(doc.root,0);
        printf("Struct: %s\n",XMLNode_attr_val(str,"name"));

        XMLNodeList* fields = XMLNode_children(str,"field");
        for(int i = 0; i < fields->size; i++)
        {
            XMLNode*field = XMLNodeList_at(fields,i);
            XMLAttribute* type = XMLNode_attr(field,"type");
            type->value = "";
        }
        XMLDoc_write(&doc, "out.xml", 4);
        XMLDoc_free(&doc);
    }
    return 0;
}
