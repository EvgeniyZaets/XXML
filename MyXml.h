#ifndef MYXML_H
#define MEXML_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

//функция для сравнения конца комментария 
int ends_with(const char* hystack, const char* needle)
{
    int h_len = strlen(hystack);
    int n_len = strlen(needle);
    if(h_len < n_len)
        return FALSE;

    for(int i=0; i < n_len; i++){
        if( hystack [h_len-n_len+i] != needle[i])
            return FALSE;
    }
    return TRUE;   
}

struct _XMLAttribute
{
    char* key;      //ключ 
    char* value;    //значение
};
typedef struct _XMLAttribute XMLAttribute;

void XMLAttribute_free(XMLAttribute* attr);

struct _XMLAttributeList //список аттрибутов
{
    int heap_size;
    int size;
    XMLAttribute* data;
};

typedef struct _XMLAttributeList XMLAttributeList;

void XMLAttributeList_init(XMLAttributeList* list); //функция инициализации списка аттрибутов тега
void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attr);  //функция добавления аттрибута к списку аттрибутов тега

struct _XMLNodeList //список узлов 
{
    int heap_size;
    int size;
    struct _XMLNode** data;
};
typedef struct _XMLNodeList XMLNodeList; 

void XMLNodeList_init(XMLNodeList* list); //функция инициализации списка узлов
void XMLNodeList_add(XMLNodeList* list, struct _XMLNode* node);  //функция добавления узла к списку узлов
struct _XMLNode* XMLNodeList_at(XMLNodeList* list, int index);

void XMLNodeList_free(XMLNodeList* list);

struct _XMLNode //структура узла
{
    char* tag;                  //тег
    char* inner_text;           //текст внутри тега
    struct _XMLNode* parent;    //указатель на родительский узел
    XMLAttributeList attributes;//атрибуты тега
    XMLNodeList children;       //список дочерних узлов 
};

typedef struct _XMLNode XMLNode; //узел XML

XMLNode* XMLNode_new(XMLNode* parent); //функция создания нового узла 
void XMLNode_free(XMLNode* parent); //функция освобождения узла
XMLNode* XMLNode_child(XMLNode* parent, int index); //добавление дочернего элемента
XMLNodeList* XMLNode_children(XMLNode* parent, const char* tag);    
char* XMLNode_attr_val(XMLNode* node, char* key);   //получаем значение атрибуса по ключу
XMLAttribute* XMLNode_attr(XMLNode* node, char* key);

struct _XMLDoc //структура документа XML
{
    XMLNode* root;  //главный узел в документе 
    char* version;  //версия XML
    char* encoding; //кодировка файла
};

typedef struct _XMLDoc XMLDoc;


int XMLDoc_load(XMLDoc* doc, const char* path); //функция загрузки документа
void XMLDoc_free(XMLDoc* doc);  //функция освобождения документа
int XMLDoc_write(XMLDoc* doc, const char* path, int indent);

XMLNode* XMLNode_new(XMLNode* parent)
{
    XMLNode* node = (XMLNode*)malloc(sizeof(XMLNode));  // выделяем память под новый узел
    node->parent = parent;  //передаем ему указатель на родительский узел
    node->tag = NULL;   //инициализируем тег
    node->inner_text = NULL;    //инициализируем текст
    XMLAttributeList_init(&node->attributes); // инициализируем список аттрибутов
    XMLNodeList_init(&node->children);  //инициализируем список дочерних узлов
    if(parent) 
        XMLNodeList_add(&parent->children, node);
    return node;    //возвращаем новый узел
}

void XMLNode_free(XMLNode* node){ //освобождаем память от узла
    if(node->tag) free(node->tag);
    if(node->inner_text) free(node->inner_text);
    for(int i=0; i < node->attributes.size;i++)
        XMLAttribute_free(&node->attributes.data[i]);
    
    free(node);
}

XMLNode* XMLNode_child(XMLNode* parent, int index){
    return parent->children.data[index];
}

char* XMLNode_attr_val(XMLNode* node, char* key){
    
    for(int i=0; i < node->attributes.size; i++)
    {
        XMLAttribute attr = node->attributes.data[i];
        if(!strcmp(attr.key,key)) return attr.value;
    }
    return NULL;
}

XMLAttribute* XMLNode_attr(XMLNode* node, char* key){
     for(int i=0; i < node->attributes.size; i++)
    {
        XMLAttribute* attr = &node->attributes.data[i];
        if(!strcmp(attr->key,key)) return attr;
    }
    return NULL;
}
enum _TagType
{
    TAG_START,
    TAG_INLINE
};
typedef enum _TagType TagType;

//функция для считывания атрибутов
static TagType parse_attribute(char* buffer, int* i, char* lex, int* lexi, XMLNode* curr_node)
{
    XMLAttribute curr_attr = {0,0};
    while (buffer[*i]!='>'){
         lex[(*lexi)++] = buffer[(*i)++];//сохраняем тег
    //имя тега
        if(buffer[*i] == ' ' && !curr_node->tag)
    {
        lex[*lexi] = '\0';
        curr_node->tag = strdup(lex);   //дублируем строку с выделением памяти под новую
        (*lexi) = 0;
        (*i)++;
        continue;
    }
    //игнорируем пробелы
    if(lex[(*lexi)-1]== ' '){
        (*lexi)--;
        continue;
    }
    //считываем значение ключа
    if(buffer[*i]=='='){
        lex[*lexi] = '\0';
        curr_attr.key = strdup(lex);
        *lexi = 0;
        continue;
    }
        if(buffer[*i]=='"'){
        if(!curr_attr.key){
        fprintf(stderr,"нет значения в полк ключа атрибута\n");
        return TAG_START;
    }
        *lexi = 0;
        (*i)++;
   //добавляем значение аттрибута в списко атрибутов узла
        while (buffer[*i] !='"')
            lex[(*lexi)++] = buffer[(*i)++];
        lex[*lexi] = '\0';
        curr_attr.value = strdup(lex);
        XMLAttributeList_add(&curr_node->attributes, &curr_attr);
        curr_attr.key=NULL; //сбрасываем значение переменной
        curr_attr.value = NULL;
        (*lexi) = 0;
        (*i)++;
        continue;
        }
        if(buffer[*i-1] == '/' && buffer[*i] == '>'){
            lex[*lexi] = '\0';
            if(!curr_node->tag)
                curr_node->tag = strdup(lex);
            (*i)++;
            return TAG_INLINE;
        }
    }
    return TAG_START;
}

int XMLDoc_load(XMLDoc* doc, const char* path) //хагружаем документ
{
    FILE* file = fopen(path, "r");
    if(!file)
    {
        fprintf(stderr,"Не удалось загрузить файл : %s\n",path);
        return FALSE;
    }
    fseek(file, 0, SEEK_END); //перемещаем указатель позиции в потоке в конец
    int size = ftell(file); //считываем колличество символов в файле через указатель текущего положения
    fseek(file,0,SEEK_SET); //теперь идем в начало файла

    char* buffer = (char*) malloc(sizeof(char) * size + 1); //динамически выделяем память под файл
    fread(buffer,1,size,file);
    fclose(file);
    buffer[size] = '\0'; //добавляем символ завершения строки
    
    doc->root = XMLNode_new(NULL);  //для корневого узла указываем на NULL

    char lex[1024];
    int lexi = 0;
    int i = 0;

    XMLNode* curr_node = doc->root; //указатель на текущий узел в цикле изначально указывает на главный узел

    while (buffer[i]!='\0')
    {
        if(buffer[i] == '<')
        { //начало тега для узла
            lex[lexi] = '\0';
            //проверяем узел
            if(lexi>0)
            { 
                if(!curr_node)
                {
                    fprintf(stderr, "текст вне документа\n");
                    return FALSE;
                }
                curr_node->inner_text = strdup(lex);
                lexi = 0;
                }        
            //мы в конце узла ?
            if(buffer[i+1]=='/')
            {
                i+=2;
                while (buffer[i]!='>')
                    lex[lexi++] = buffer[i++];//сохраняем тег
                lex[lexi] = '\0';//ставим хавершающий элемент в строке тега

                if(!curr_node)
                {
                    fprintf(stderr,"Мы находимся в корневом узле\n");
                    return FALSE;
                }
                if(strcmp(curr_node->tag,lex))
                {
                    fprintf(stderr,"тег не совпадает с текущим узлом(%s != %s)\n",curr_node->tag,lex);
                    return FALSE; 
                }

                curr_node = curr_node->parent;
                i++;
                continue;
            }
            //если попался комментарий
            if(buffer[i+1]=='!')
            {
                while (buffer[i]!=' ' && buffer[i] != '>')
                    lex[lexi++]= buffer[i++];
                lex[lexi] = '\0';
                
                if(!strcmp(lex,"<!--")){
                    lex[lexi] = '\0';
                    while (!ends_with(lex,"-->"))
                    {
                        lex[lexi++] = buffer[i++];
                        lex[lexi] = '\0';
                    }
                    continue;
                }
            }
            //теги декларирования - узнаем значение полей версия и кодировка
            if(buffer[i+1]=='?'){
                while (buffer[i]!=' ' && buffer[i] != '>')
                    lex[lexi++]= buffer[i++];
                lex[lexi] = '\0';
                if(!strcmp(lex,"<?xml")){
                    lexi = 0;
                    XMLNode* desc = XMLNode_new(NULL);
                    parse_attribute(buffer,&i,lex,&lexi,desc);
                    doc->version = XMLNode_attr_val(desc, "version");
                    doc->encoding = XMLNode_attr_val(desc, "encoding");
                    continue;
                }
            }

            //создаем новые узлы
            curr_node = XMLNode_new(curr_node);
            //начало чтения тега
            i++;

            if(parse_attribute(buffer,&i,lex,&lexi,curr_node)==TAG_INLINE)
            {
                curr_node = curr_node->parent;
                i++;
                continue;
            }
            
            //установить имя тега если его нет
            lex[lexi] = '\0';
            if(!curr_node->tag)
                curr_node->tag = strdup(lex);
            
            //перезапуск чтения
            lexi = 0;   //сбрасываем итератор в начало строки lex что бы ее потом перезаписать
            i++;
            continue;
        }
        else
        {
            lex[lexi++] = buffer[i++];
        }
    }
    return TRUE;
}

void XMLDoc_free(XMLDoc* doc)   //освобождаем память от документа
{
    XMLNode_free(doc->root);
}

static void node_out(FILE* file, XMLNode* node,int indent, int times)
{
    for(int i=0; i< node->children.size; i++){
        XMLNode* child = node->children.data[i];

        if(times > 0)
            fprintf(file,"%0*s", indent * times, " ");

        fprintf(file,"<%s", child->tag);
        for(int i=0; i < child->attributes.size; i++)
        {
            XMLAttribute attr = child->attributes.data[i];
            if(!attr.value || !strcmp(attr.value,""))
                continue;
            fprintf(file," %s=\"%s\"",attr.key,attr.value);
        }

        if(child->children.size == 0 && !child->inner_text)
        {
            fprintf(file, " />\n");
        } else {
            fprintf(file,">");
            if(child->children.size == 0)
            {
                fprintf(file,"%s</%s>\n",child->inner_text,child->tag);
            }else{
                fprintf(file,"\n");
                node_out(file, child, indent, times + 1);
                if(times > 0)
                    fprintf(file,"%0*s", indent * times, " ");
                fprintf(file,"</%s>\n",child->tag);
            }
        }
    }
}

int XMLDoc_write(XMLDoc* doc, const char* path, int indent){
    FILE* file = fopen(path, "w");
    if(!file){
        fprintf(stderr,"Ошибка при открытии файла: '%s'\n",path);
        return FALSE;
    }

    fprintf(file,"<?xml version=\"%s\" encoding=\"%s\"?>\n ",
        (doc->version) ? doc->version : "1.0",
        (doc->encoding) ? doc->encoding : "UTF-8"       
    );
    node_out(file,doc->root,indent,0);
    fclose(file);
}

void XMLAttribute_free(XMLAttribute* attr){ //освобождаем память от  аттрибута
    free(attr->key);
    free(attr->value);
}

void XMLAttributeList_init(XMLAttributeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLAttribute*)malloc(sizeof(XMLAttribute)*list->heap_size);
}
void XMLAttributeList_add(XMLAttributeList* list, XMLAttribute* attr)
{
    while (list->size >= list->heap_size) //проверяем что мы не превышаем размер нам доступный
    {
        list->heap_size +=2;    //если не хватает увеличиваем счетчик атрибутов в списке
        list->data = (XMLAttribute*) realloc(list->data, sizeof(XMLAttribute)*list->heap_size); //добавляем память по возможности
    }
    
    list->data[list->size++] = *attr;   //добавляем аттрибут в список 
}

void XMLNodeList_init(XMLNodeList* list)
{
    list->heap_size = 1;
    list->size = 0;
    list->data = (XMLNode**)malloc(sizeof(XMLNode*)*list->heap_size);
}

void XMLNodeList_add(XMLNodeList* list, XMLNode* node)
{
    while (list->size >= list->heap_size) //проверяем что мы не превышаем размер нам доступный
    {
        list->heap_size +=2;    //если не хватает увеличиваем счетчик атрибутов в списке
        list->data = (XMLNode**) realloc(list->data, sizeof(XMLNode*)*list->heap_size); //добавляем память по возможности
    }
    
    list->data[list->size++] = node;   //добавляем аттрибут в список 
}

XMLNode* XMLNodeList_at(XMLNodeList* list, int index)
{
    return list->data[index];
}

void XMLNodeList_free(XMLNodeList* list){
    free(list);
}

XMLNodeList* XMLNode_children(XMLNode* parent, const char* tag){
    XMLNodeList* list = (XMLNodeList*)malloc(sizeof(XMLNodeList));
    XMLNodeList_init(list);

    
    for(int i=0; i< parent->children.size; i++) {
        XMLNode* child = parent->children.data[i];
        if(!strcmp(child->tag,tag))
            XMLNodeList_add(list,child);
    }

    return list;
}

#endif //MYXML_H