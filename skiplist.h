#pragma once
#include<string>
#include<cstring>
#include<mutex>
#include<iostream>
#include<fstream>
#include<cstring>

const std::string STORE_FILE("store/dumpFile");
std::mutex mtx;
const std::string delimiter=":";

template<typename K,typename V>
class Node
{
    public:
        Node(){}
        Node(K k,V v,int level)
        {
            key=k;
            value=v;
            nodeLevel=level;
            forward=new Node<K,V>*[level+1];
            memset(forward,0,sizeof(Node<K,V>*)*(level+1));
        }
        ~Node(){delete []forward ;}

        K getKey()const{return key;}
        V getValue()const{return value;}
        void setValue(V value){this->value=value;}

        Node<K,V> **forward;//存储指向下一个不同等级节点的指针的数组
        int nodeLevel;
    private:
        K key;
        V value;
};

template<typename K,typename V>
class SkipList
{
    public:
        SkipList(int);
        ~SkipList();
        
        auto getRandomLevel();
        auto creatNode(K,V,int);
        auto insertElement(K,V);
        auto displayList();
        auto searchElement(K);
        auto deleteElement(K);
        auto dumpFile();
        auto loadFile();
        auto size();
    private:
        auto getKeyValueFromString(const std::string& str,std::string* key,std::string* value);
        auto isValidString(const std::string& str);
    private:
        int maxLevel;
        int skipListLevel;
        Node<K,V>* header;

        std::ofstream fileWriter;
        std::ifstream fileReader;

        int elementCount;
};
template<typename K, typename V> 
SkipList<K, V>::SkipList(int maxLevel)
{
    this->maxLevel=maxLevel;
    this->skipListLevel=0;
    this->elementCount=0;

    K key;
    V value;
    this->header=new Node<K,V>(key,value,maxLevel);
}

template<typename K, typename V> 
SkipList<K, V>::~SkipList()
{
    if(fileWriter.is_open())fileWriter.close();
    if(fileReader.is_open())fileReader.close();
    delete header;
}
template<typename K, typename V>
auto SkipList<K, V>::getRandomLevel()
{
    int k=1;
    while(rand()%2)
    {
        k++;
    }
    k=(k<maxLevel)?k:maxLevel;
    return k;
}

template<typename K,typename V>
auto SkipList<K,V>::creatNode(const K key,const V value,int level)
{
    Node<K,V> *n=new Node<K,V>(key,value,level);
    return n;
}

template<typename K,typename V>
auto SkipList<K,V>::displayList()
{
    std::cout << "\n*****Skip List*****"<<"\n"; 
    for(int i=0;i<=skipListLevel;i++)
    {
        Node<K,V>* node=this->header->forward[i];
        std::cout << "Level " << i << ": ";
        while(node!=nullptr)
        {
            std::cout << node->getKey() << ":" << node->getValue() << ";";
            node=node->forward[i];
        }
        std::cout<<std::endl;
    }
    return;
}

template<typename K,typename V>
auto SkipList<K,V>::insertElement(const K key,const V value)
{
    mtx.lock();
    Node<K,V>* current(header);
    Node<K,V>* update[maxLevel+1];
    memset(update,0,sizeof(Node<K,V>*)*(maxLevel+1));

    for(int i=skipListLevel;i>=0;i--)//逐层查找
    {
        while(current->forward[i]!=nullptr&&current->forward[i]->getKey()<key)
        {
            current=current->forward[i];//如果要插入的key小于当前key，current指针后移
        }
        update[i]=current;//找到合适位置，赋值暂存
    }
    current=current->forward[0];

    if(current!=nullptr&&current->getKey()==key)
    {
        std::cout<<"key: "<<key<<", exists"<<std::endl;
        mtx.unlock();
        return 1;
    }
    if(current==nullptr||current->getKey()!=key)
    {
        int randomLevel=getRandomLevel();
        if(randomLevel>skipListLevel)
        {
            for(int i=skipListLevel+1;i<randomLevel+1;i++)
            {
                update[i]=header;
            }
            skipListLevel=randomLevel;
        }
        Node<K,V>* insertedNode=creatNode(key,value,randomLevel);
        for(int i=0;i<=randomLevel;i++)
        {
            insertedNode->forward[i]=update[i]->forward[i];
            update[i]->forward[i]=insertedNode;
        }
        std::cout << "Successfully inserted key:" << key << ", value:" << value << std::endl;
        elementCount++;
    }
    mtx.unlock();
    return 0;
}
template<typename K,typename V>
auto SkipList<K,V>::dumpFile()
{
    std::cout << "dump file-----------------" << std::endl;
    fileWriter.open(STORE_FILE);
    Node<K,V>* node=this->header->forward[0];
    while(node!=nullptr)
    {
        fileWriter<<node->getKey()<<":"<<node->getValue()<<"\n";
        std::cout<<node->getKey()<<":"<<node->getValue()<<";\n";
        node=node->forward[0];
    }

    fileWriter.flush();
    fileWriter.close();
    return;
}
template<typename K,typename V>
auto SkipList<K,V>::loadFile()
{
    fileReader.open(STORE_FILE);
    std::cout << "load file-----------------" << std::endl;
    
    std::string line;
    std::string* key=new std::string();
    std::string* value=new std::string();
    while(std::getline(fileReader,line))
    {
        getKeyValueFromString(line,key,value);
        if(key->empty()||value->empty())continue;
        insertElement(*key,*value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    fileReader.close();
}

template<typename K,typename V>
auto SkipList<K,V>::size(){return elementCount;}

template<typename K,typename V>
auto SkipList<K,V>::getKeyValueFromString(const std::string& str, std::string* key, std::string* value)
{
    if(isValidString(str))return;
    *key=str.substr(0,str.find(delimiter));
    *value=str.substr(str.find(delimiter)+1,str.length());
}

template<typename K,typename V>
auto SkipList<K,V>::isValidString(const std::string& str)
{
    if(str.empty()) return false;
    if (str.find(delimiter) == std::string::npos)return false;
    return true;
}

template<typename K,typename V>
auto SkipList<K,V>::deleteElement(K key)
{
    mtx.lock();
    Node<K,V>* current=this->header;
    Node<K,V>* update[maxLevel+1];
    memset(update,0,sizeof(Node<K,V>*)*(maxLevel+1));

    for(int i=skipListLevel;i>=0;i--)
    {
        while(current->forward[i]!=nullptr&&current->forward[i]->getKey()<key)
        {
            current=current->forward[i];
        }
        update[i]=current;
    }
    current=current->forward[0];

    if(current!=nullptr&&current->getKey()==key)
    {
        for(int i=0;i<=skipListLevel;i++)
        {
            if(update[i]->forward[i]!=current)break;
            update[i]->forward[i]=current->forward[i];
        }
        while(skipListLevel>0&&header->forward[skipListLevel]==0)
        {
            skipListLevel--;
        }
        std::cout << "Successfully deleted key "<< key << std::endl;
        elementCount--;
    }
    mtx.unlock();
    return ;
}

template<typename K,typename V>
auto SkipList<K,V>::searchElement(K key)
{
    std::cout << "search_element-----------------" << std::endl;
    Node<K,V>* current=header;

    for (int i = skipListLevel; i>=0; i--)
    {
        while(current->forward[i]!=nullptr&&current->forward[i]->getKey()<key)
        {
            current=current->forward[i];
        }
    }
    current=current->forward[0];

    if(current!=nullptr && current->getKey()==key)
    {
        std::cout << "Found key: " << key << ", value: " << current->getValue() << std::endl;
        return true;
    }

    std::cout<<"Not Found Key:" << key << std::endl;
    return false;
}