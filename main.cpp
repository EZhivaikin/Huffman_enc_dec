#include <stdio.h>
#include <cstring>
#include <iostream>
#include <ctime>
#include <fstream>

#include <assert.h>
#include <locale>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <list>

using namespace std;
class Node
{
  public:
    int digit;
    char32_t c;
    Node *leftChild, *rightChild;

    Node()
    {
        leftChild = rightChild = NULL;
    }

    Node(Node *L, Node *R)
    {
        leftChild = L;
        rightChild = R;
        digit = L->digit + R->digit;
    }
};

vector<bool> code;
map<char32_t, vector<bool>> table;
list<Node *> t;
wstring_convert<std::codecvt_utf8<char32_t>,char32_t> convert;

struct Comparator
{
    bool operator()(const Node *left, const Node *right) const { return left->digit < right->digit; }
};
struct ComparatorForMap
{
    bool operator()(const pair<char32_t,vector<bool>> first,const pair<char32_t,vector<bool>> second) const {return first.second.size()<second.second.size();}
};

void printTable()
{
    cout<<"Таблица символов с их кодами:"<<endl;
    list<pair<char32_t,vector<bool>>> templist;
    //int count=0;
    for(auto &item : table)
    {
       templist.push_back(item);
    }
    templist.sort(ComparatorForMap());
    for (auto &item : templist)
    {
        std::stringstream ss;
        string temp= convert.to_bytes(item.first);
        if(temp == "\n" )
            temp="\\n";
        if(temp == "\t")
            temp="\\t";
        for (size_t i = 0; i < item.second.size(); ++i)
        {
            if (i != 0)
                ss << ",";
            ss << item.second[i];
        }
        //count++;
        std::string s = ss.str();
        cout << temp << "\t" << s << endl;
    }
    templist.clear();
    //cout<<count<<endl;
}

void printSortedList()
{   
    cout<<"Все символы из строки упорядоченные по количеству вхождений в текст:"<<endl;
    t.sort(Comparator());
    //int count=0;
    for(auto& item : t)
    {
        //count++;
        string temp= convert.to_bytes(item->c);
        if(temp == "\n" )
            temp="\\n";
        if(temp == "\t")
            temp="\\t";
        cout<<temp<<"\t"<<item->digit<<endl;
    }
   // cout<<count<<endl;
}
void BuildTable(Node *root)
{
    if (root->leftChild != NULL)
    {
        code.push_back(0);
        BuildTable(root->leftChild);
    }

    if (root->rightChild != NULL)
    {
        code.push_back(1);
        BuildTable(root->rightChild);
    }

    if (root->rightChild == NULL && root->leftChild == NULL)
    {
        table[root->c] = code;
    }
    if (code.size() != 0)
        code.pop_back();
}
int main()
{
    map<char32_t, int> charQuantity;

    std::ifstream fin("txt.txt");
    fin.seekg(0, std::ios::end);
    size_t size = fin.tellg();
    string input(size, ' ');
    fin.seekg(0);
    fin.read(&input[0], size);
    fin.close();
    
    u32string utf32String = convert.from_bytes(input);
    
    string u8 = convert.to_bytes(utf32String);
    
    cout << "Исходный текст: "<< endl<<u8.length()<<endl;;

    for (size_t i = 0; i < utf32String.length(); i++)
    {
        charQuantity[utf32String[i]]++;
    }

    for (auto &item : charQuantity)
    {
        string temp= convert.to_bytes(item.first);
        Node *p = new Node;
        p->c = item.first;
        p->digit = item.second;
        t.push_back(p);
    }
    //построение дерева

    printSortedList();
    int charcount = 0;
    if(t.size()==1)
    {
        Node *leftchild=t.front();
        Node *rightchild=new Node;
        rightchild->c=1;
        rightchild->digit=1;
        t.pop_front();
        Node *parent= new Node(leftchild,rightchild);
        t.push_back(parent);
    }

    while (t.size() != 1)
    {
        t.sort(Comparator());
        Node *LeftChild = t.front();
        t.pop_front();
        Node *RightChild = t.front();
        t.pop_front();

        Node *parent = new Node(LeftChild, RightChild);
        t.push_back(parent);
    }
    Node *root = t.front();
    BuildTable(root);
    printTable();
    ofstream g("txtoutput.txt");
   // charcount = 0;
    int count = 0;
    char buf = 0;
    cout << endl;
    cout << "Код на запись:" <<endl;
    for (size_t i = 0; i < utf32String.length(); i++)
    {
        char32_t c = utf32String[i];
        vector<bool> x = table[c];
        for (int n = 0; n < x.size(); n++)
        {  
            cout<<x[n];
            buf = buf | x[n] << (7 - count); 
            count++;
            if (count == 8) 
            {
                count = 0;
                charcount++;
                g << buf;
                buf = 0;
            }
            if (i == utf32String.length() - 1 && n == x.size() - 1)
            {
                g << buf;
            }
        }
    }
    for (size_t i = 0; i < utf32String.length(); i++)
    g.close();
    cout << endl
         << endl;

    //считывание из бинарного файла
    ifstream F("txtoutput.txt", ios::in | ios::binary);
    Node *p = root;
    count = 0;
    charcount = 0;
    char byte;
    u32string u32string;
    byte = F.get();
    while (!F.eof())
    {
        bool b = byte & (1 << (7 - count));
        if (b)
            p = p->rightChild;

        else
            p = p->leftChild;

        if ((p->leftChild == NULL) && (p->rightChild == NULL))
        {
            u32string += p->c;
            charcount++;
            p = root;
        }
        count++;

        if (count == 8)
        {
            count = 0;
            byte = F.get();
            continue;
        }
        if (charcount == utf32String.size())
        {
            break;
        }
    }
    cout<<"Текст после дешифрования из файла:"<<endl;
    string utf8 = convert.to_bytes(u32string);
    cout << utf8<<endl;
    cout<<utf8.length()<<endl;
    F.close();


    fstream out("txtoutput.txt");
    fstream in("txt.txt");
    in.seekg (0, std::ios::end);
    out.seekg(0,std::ios::end);
    cout << "Ваш файл весил : " <<  in.tellg() << " байт" << endl
    <<"Теперь весит : "<< out.tellg()<<" байт"<<endl;;
    in.close();
    out.close();


    return 0;
}
