#include"skiplist.h"
#include<iostream>

int main()
{
    SkipList<int ,std::string> skipList(6);
    skipList.insertElement(1, "歪比巴伯"); 
	skipList.insertElement(3, "发病棉袄"); 
	skipList.insertElement(7, "红皮鸭子"); 
	skipList.insertElement(8, "捞的不谈"); 
	skipList.insertElement(9, "还追"); 
	skipList.insertElement(19, "移速七百多"); 
	skipList.insertElement(19, "失了智"); 

    skipList.displayList();

	skipList.dumpFile();

    // skipList.load_file();

    skipList.searchElement(9);
    skipList.searchElement(18);


    skipList.displayList();



    std::cout << "skipList size:" << skipList.size() << std::endl;
    return 0;
}