#include <iostream>
#include <fstream>
using namespace std;



int currObjCount;


class transaction
{
    public:
    int number;
    int currState;//0 for unused, 1 for waiting, 2 for commit, 3 for abort.
    transaction* waitingFor;


};


class object
{
    public:
    char name;
    int currState; //0 for no lock, 1 for share lock, 2 for exclusive lock
    transaction* shareLock[10];
    transaction* exclusiveLock[10];
};

//return the object with name c, return nullptr of not found
object* searchObj(object* obj[], char c)
{
    object* tempObj = nullptr;
    char tempC;
    if(currObjCount == 0)
        return tempObj;
    for(int i = 0;i<currObjCount;i++)
    {
        tempC = obj[i]->name;

        if(tempC==c)
        {
            tempObj = obj[i];
        }
    }


    return tempObj;
}

//add new object to the object list
void addObj(object* obj[], char c)
{
    object* tempObj = searchObj(obj, c);

    if(tempObj==nullptr)
    {
        obj[currObjCount] = new object();
        obj[currObjCount]->name = c;
        obj[currObjCount]->currState = 0;
        for(int i = 0;i<10;i++)
        {
            obj[currObjCount]->shareLock[i] = nullptr;
            obj[currObjCount]->exclusiveLock[i] = nullptr;
        }

        currObjCount++;
        return;
    }

}

//return true if a transaction list is empty, false otherwise
bool checkEmpty(transaction* transList[])
{
    for(int i = 0;i<10;i++)
    {
        if(transList[i]!=nullptr)
            return false;
    }
    return true;
}

//return true if the given transaction is in the transaction list
bool checkExist(transaction* transList[], int num_trans)
{
    for(int i = 0;i<10;i++)
    {
        if(transList[i]!=nullptr)
            if(transList[i]->number==num_trans)
                return true;
    }
    return false;
}

//set the transaction to waiting phase
void waiting(transaction* tran, transaction* transList[])
{
    for(int i = 0;i<10;i++)
    {
        if(transList[i]!=nullptr)
            if(tran->number!=transList[i]->number)
                {tran->waitingFor = transList[i];
                tran->currState=1;}
    }
    return;
}

//find the number of transaction with the fewest number of locks or latest
int abortTran(object* obj[])
{
    int leastLocks;
    int Lockcount[10];
    for(int i=0;i<10;i++)
        Lockcount[i]=0;
    for(int i = 0;i<currObjCount;i++)
    {
        for(int j = 0;j<10;j++)
        {
            if(obj[i]->shareLock[j]!=nullptr)
                Lockcount[j]++;
            if(obj[i]->exclusiveLock[j]!=nullptr)
                Lockcount[j]++;

        }
    }

    leastLocks = 1;
    for(int i=0;i<10;i++)
     {
         if(Lockcount[i]!=0)
         {
             if(Lockcount[i]==leastLocks)
            leastLocks = -1;
        else if(Lockcount[i]<leastLocks)
            leastLocks = Lockcount[i];
         }

     }
     return leastLocks;


}


//return the transaction should be aborted if there is a cycle
transaction* checkCricle(transaction* trans[], object* obj[], int num_tran)
{
    for(int i = 0;i<10;i++)
    {
        if(trans[i]!=nullptr)
        {
            if(trans[i]->currState==1)
            {
                transaction* start = trans[i];
                int startNum = start->number;
                int n = 0;
                while(start->waitingFor->currState==1&&n<10)
                {
                    start = start->waitingFor;
                    if(start->number==startNum)
                    {
                        int abortNum = abortTran(obj);
                        if(abortNum==-1)
                            return trans[num_tran];
                        else
                            return trans[abortNum];
                    }
                    n++;

                }

            }

        }
    }
    return nullptr;
}

//abort the transaction
void abort(object* obj[], transaction* tran)
{
    for(int i = 0;i<currObjCount;i++)
    {
        obj[i]->exclusiveLock[tran->number] = nullptr;
        obj[i]->shareLock[tran->number] = nullptr;
    }
    tran->waitingFor=nullptr;
    tran->currState = 3;
}


//share lock action
int SLock(object* obj[],transaction* trans[],char obj_name,int num_tran)
{
    object* tempObj = searchObj(obj, obj_name);

    if(tempObj==nullptr)
        return -1;
    if(checkEmpty(tempObj->exclusiveLock))
    {
        if(checkExist(tempObj->shareLock, num_tran))
            return -1;
        else
        {
            tempObj->shareLock[num_tran] = trans[num_tran];
        }

    }
    else
    {
        waiting(trans[num_tran], tempObj->exclusiveLock);
    }
    transaction* abortTran = checkCricle(trans,obj,num_tran);
    if(abortTran!=nullptr)
    {
        abort(obj, abortTran);
        return abortTran->number;
    }
    return -1;


}

//exclusive lock action
int XLock(object* obj[],transaction* trans[],char obj_name,int num_tran)
{
    object* tempObj = searchObj(obj, obj_name);
    if(checkEmpty(tempObj->shareLock))
    {
        if(checkEmpty(tempObj->exclusiveLock))
            tempObj->exclusiveLock[num_tran] = trans[num_tran];
        else
            waiting(trans[num_tran], tempObj->exclusiveLock);
    }
    else
        waiting(trans[num_tran], tempObj->shareLock);

     transaction* abortTran = checkCricle(trans,obj,num_tran);
    if(abortTran!=nullptr)
    {
        abort(obj, abortTran);
        return abortTran->number;
    }
    return -1;

}


int commit(object* obj[],transaction* trans[],int num_tran)
{
    trans[num_tran] = nullptr;
    for(int i = 0;i<currObjCount;i++)
    {
        obj[i]->shareLock[num_tran] = nullptr;
        obj[i]->exclusiveLock[num_tran] = nullptr;
    }
    return -1;
}





int main()
{
    //read from txt file
    ifstream inFile;
    inFile.open("input3.txt");

    //convert to string
    string s( (istreambuf_iterator<char>(inFile) ),
                       (istreambuf_iterator<char>()));

    //for output file
    ofstream outFile("output.txt");

    //store objects in a array
    object* obj[100];
    currObjCount = 0;

    for(int i = 0;i<s.length();i++)
    {
        char c = s[i];
        if(c=='(')
        {
            c = s[i+1];
            addObj(obj, c);
        }

    }

     //store transactions in a array
    transaction* trans[10];
    for(int i = 0;i<10;i++)
    {
        trans[i] = nullptr;
    }

    for(int i = 0;i<s.length();i++)
    {
        char c = s[i];
        if(c==':')
        {
            int num = s[i-1]-'0';
            if(trans[num]==nullptr)
            {
                trans[num] = new transaction();
                trans[num]->number = num;
                trans[num]->currState = 0;
                trans[num]->waitingFor = nullptr;

            }
        }
    }




    bool isDeadlocked = false;
    char obj_name;
    int num_tran;
    int deadlockNum=-1;
    for(int i = 0;i<s.length();i++)
    {
        char c = s[i];
        if(c==':')
        {
            c = s[i+1];
            num_tran = s[i-1]-'0';
            if(c=='S')
            {
                obj_name = s[i+3];
                deadlockNum=SLock(obj, trans, obj_name, num_tran);
            }

            if(c=='X')
            {
                obj_name = s[i+3];
                deadlockNum=XLock(obj, trans, obj_name, num_tran);
            }

            if(c=='C')
                deadlockNum=commit(obj, trans, num_tran);
            if(deadlockNum!=-1)
            {
                outFile<<"T";
                outFile<<deadlockNum;
                isDeadlocked = true;
            }

        }
    }
    if(!isDeadlocked)
        outFile<<"OK";





    inFile.close();
    outFile.close();

    return 0;
}
